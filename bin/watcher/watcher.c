#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <limits.h>
#include <sys/inotify.h>

#define EVENT_SIZE  (sizeof(struct inotify_event))
#define BUF_LEN     (1024 * (EVENT_SIZE + 16))

// Structure to define and track each file we want to monitor
typedef struct {
    char path[PATH_MAX];
    const char *search_str;
    const char *append_str;
    int wd;
} MonitoredFile;

// Function to check the file and append strings if necessary
void process_file(MonitoredFile *file) {
    bool found = false;
    FILE *f = fopen(file->path, "r");
    
    if (f) {
        char line[1024];
        while (fgets(line, sizeof(line), f)) {
            // Check if the target string already exists
            if (strstr(line, file->search_str) != NULL) {
                found = true;
                break;
            }
        }
        fclose(f);
    } else {
        // If the file doesn't exist yet, we'll proceed to create/append it
        return; 
    }

    // If the setting wasn't found, append it
    if (!found) {
        f = fopen(file->path, "a");
        if (f) {
            fprintf(f, "%s\n", file->append_str);
            fclose(f);
            printf("[GTK Monitor] Injected missing settings into: %s\n", file->path);
        } else {
            perror("[GTK Monitor] Error opening file for appending");
        }
    }
}

int main() {
    const char *home = getenv("HOME");
    if (!home) {
        fprintf(stderr, "Could not get HOME environment variable.\n");
        return 1;
    }

    // Define the files to monitor, what to look for, and what to inject
    MonitoredFile files[] = {
        { "", "gtk-decoration-layout", "gtk-decoration-layout=:\ngtk-enable-animations=0", -1 }, // GTK 4.0
        { "", "gtk-decoration-layout", "gtk-decoration-layout=:\ngtk-enable-animations=0", -1 }, // GTK 3.0
        { "", "gtk-enable-animations", "gtk-enable-animations=0", -1 }                           // GTK 2.0
    };
    int num_files = sizeof(files) / sizeof(files[0]);

    // Construct the absolute paths
    snprintf(files[0].path, PATH_MAX, "%s/.config/gtk-4.0/settings.ini", home);
    snprintf(files[1].path, PATH_MAX, "%s/.config/gtk-3.0/settings.ini", home);
    snprintf(files[2].path, PATH_MAX, "%s/.gtkrc-2.0", home);

    int inotify_fd = inotify_init();
    if (inotify_fd < 0) {
        perror("inotify_init");
        return 1;
    }

    // Add initial watches. We only care when a write is closed, or when the watch is ignored (file replaced).
    for (int i = 0; i < num_files; i++) {
        files[i].wd = inotify_add_watch(inotify_fd, files[i].path, IN_CLOSE_WRITE);
        if (files[i].wd < 0) {
            printf("[GTK Monitor] Note: Could not watch %s (it may not exist yet).\n", files[i].path);
        }
    }

    printf("Starting GTK config monitor...\n");

    char buf[BUF_LEN] __attribute__ ((aligned(__alignof__(struct inotify_event))));
    const struct inotify_event *event;
    ssize_t len;

    // Main event loop
    while (1) {
        len = read(inotify_fd, buf, BUF_LEN);
        if (len <= 0) {
            perror("read");
            break;
        }

        // Loop over all events in the buffer
        for (char *ptr = buf; ptr < buf + len; ptr += EVENT_SIZE + event->len) {
            event = (const struct inotify_event *) ptr;

            // Find which file triggered the event
            for (int i = 0; i < num_files; i++) {
                if (event->wd == files[i].wd) {
                    
                    if (event->mask & IN_CLOSE_WRITE) {
                        // Sleep 0.2s to match your original script logic
                        // This allows editors to fully release file locks
                        usleep(200000); 
                        process_file(&files[i]);
                    } 
                    else if (event->mask & IN_IGNORED) {
                        // The file was overwritten via an atomic save rename.
                        // The old inode is gone, so we must re-attach the watch to the new inode.
                        usleep(200000);
                        files[i].wd = inotify_add_watch(inotify_fd, files[i].path, IN_CLOSE_WRITE);
                        
                        // Process immediately in case changes happened during the swap
                        if (files[i].wd >= 0) {
                            process_file(&files[i]);
                        }
                    }
                }
            }
        }
    }

    close(inotify_fd);
    return 0;
}
