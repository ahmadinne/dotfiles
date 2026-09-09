#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <limits.h>
#include <sys/inotify.h>
#include <sys/stat.h>

#define EVENT_SIZE  (sizeof(struct inotify_event))
#define BUF_LEN     (1024 * (EVENT_SIZE + NAME_MAX + 1))

// Structure to define and track each file we want to monitor
typedef struct {
    char dir_path[PATH_MAX];   // Directory to watch (e.g., ~/.config/gtk-3.0)
    char file_name[NAME_MAX];  // Target file (e.g., settings.ini)
    char full_path[PATH_MAX];  // Combined full path
    const char *search_str;    // What to check for
    const char *append_str;    // What to append if missing
    int wd;                    // Watch descriptor
} MonitoredFile;

void process_file(MonitoredFile *file) {
    bool found = false;
    FILE *f = fopen(file->full_path, "r");
    
    if (f) {
        char line[1024];
        while (fgets(line, sizeof(line), f)) {
            if (strstr(line, file->search_str) != NULL) {
                found = true;
                break;
            }
        }
        fclose(f);
    } else {
        return; // File doesn't exist yet, nothing to parse
    }

    // If the setting wasn't found, append it immediately
    if (!found) {
        f = fopen(file->full_path, "a");
        if (f) {
            fprintf(f, "%s\n", file->append_str);
            fclose(f);
            printf("[GTK Monitor] Injected settings into: %s\n", file->full_path);
        }
    }
}

int main() {
    const char *home = getenv("HOME");
    if (!home) {
        fprintf(stderr, "Could not get HOME environment variable.\n");
        return 1;
    }

    MonitoredFile files[] = {
        { "", "settings.ini", "", "gtk-decoration-layout", "gtk-decoration-layout=:\ngtk-enable-animations=0", -1 }, // GTK 4.0
        { "", "settings.ini", "", "gtk-decoration-layout", "gtk-decoration-layout=:\ngtk-enable-animations=0", -1 }, // GTK 3.0
        { "", ".gtkrc-2.0",   "", "gtk-enable-animations", "gtk-enable-animations=0", -1 }                           // GTK 2.0
    };
    int num_files = sizeof(files) / sizeof(files[0]);

    // Construct the absolute paths for directories and full files
    snprintf(files[0].dir_path, PATH_MAX, "%s/.config/gtk-4.0", home);
    snprintf(files[1].dir_path, PATH_MAX, "%s/.config/gtk-3.0", home);
    snprintf(files[2].dir_path, PATH_MAX, "%s", home); // Home dir for .gtkrc-2.0

    int inotify_fd = inotify_init();
    if (inotify_fd < 0) {
        perror("inotify_init");
        return 1;
    }

    // Set up directory watches for IN_CLOSE_WRITE (standard save) and IN_MOVED_TO (atomic save rename)
    for (int i = 0; i < num_files; i++) {
        snprintf(files[i].full_path, PATH_MAX, "%s/%s", files[i].dir_path, files[i].file_name);
        
        files[i].wd = inotify_add_watch(inotify_fd, files[i].dir_path, IN_CLOSE_WRITE | IN_MOVED_TO);
        if (files[i].wd < 0) {
            printf("[GTK Monitor] Note: Directory %s does not exist yet.\n", files[i].dir_path);
        }
    }

    printf("Starting instantaneous GTK config monitor...\n");

    // We need a buffer that accounts for event filenames (NAME_MAX)
    char buf[BUF_LEN] __attribute__ ((aligned(__alignof__(struct inotify_event))));
    const struct inotify_event *event;
    ssize_t len;

    while (1) {
        len = read(inotify_fd, buf, BUF_LEN);
        if (len <= 0) {
            perror("read");
            break;
        }

        // Loop over all events delivered by the kernel
        for (char *ptr = buf; ptr < buf + len; ptr += EVENT_SIZE + event->len) {
            event = (const struct inotify_event *) ptr;

            // Only process events that have a filename attached
            if (event->len > 0) {
                for (int i = 0; i < num_files; i++) {
                    // Match the watch descriptor (directory) AND the exact filename
                    if (event->wd == files[i].wd && strcmp(event->name, files[i].file_name) == 0) {
                        
                        // We caught a save or a move targeting our specific config file.
                        // Process it instantly.
                        process_file(&files[i]);
                    }
                }
            }
        }
    }

    close(inotify_fd);
    return 0;
}
