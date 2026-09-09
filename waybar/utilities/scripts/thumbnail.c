#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

#define BUF_SIZE 2048

// Run a command safely (argv must be NULL-terminated)
int run_cmd(char *const argv[]) {
    pid_t pid = fork();
    if (pid == 0) {
        execvp(argv[0], argv);
        perror("execvp failed");
        _exit(127);
    } else if (pid < 0) {
        perror("fork failed");
        return -1;
    }
    int status;
    waitpid(pid, &status, 0);
    return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
}

// Strip "file://" prefix if present
void strip_file_prefix(char *s) {
    const char *prefix = "file://";
    size_t prefix_len = strlen(prefix);
    if (strncmp(s, prefix, prefix_len) == 0) {
        memmove(s, s + prefix_len, strlen(s + prefix_len) + 1);
    }
}

// Copy file src → dst (overwrite if exists)
int copy_file(const char *src, const char *dst) {
    int in_fd = open(src, O_RDONLY);
    if (in_fd < 0) return -1;

    int out_fd = open(dst, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (out_fd < 0) {
        close(in_fd);
        return -1;
    }

    char buf[8192];
    ssize_t n;
    while ((n = read(in_fd, buf, sizeof(buf))) > 0) {
        if (write(out_fd, buf, n) != n) {
            close(in_fd);
            close(out_fd);
            return -1;
        }
    }

    close(in_fd);
    close(out_fd);
    return (n < 0) ? -1 : 0;
}

// Compare file metadata (size + inode)
int same_file(const char *a, const char *b) {
    struct stat sa, sb;
    if (stat(a, &sa) < 0) return 0;
    if (stat(b, &sb) < 0) return 0;
    return (sa.st_ino == sb.st_ino && sa.st_size == sb.st_size);
}

int main(void) {
    const char *home = getenv("HOME");
    char defaultone[BUF_SIZE];
    char mask[BUF_SIZE];

    if (home) {
        snprintf(defaultone, sizeof(defaultone), "%s/.config/waybar/utilities/thumbnail.png", home);
        snprintf(mask, sizeof(mask), "%s/.config/waybar/utilities/mask.png", home);
    } else {
        strncpy(defaultone, "/tmp/thumbnail.png", sizeof(defaultone));
        strncpy(mask, "/tmp/mask.png", sizeof(mask));
    }

    const char *thumbnail = "/tmp/mediathumbnail";
    const char *fullthumb = "/tmp/fullthumbnail";

    char lastUrl[BUF_SIZE] = "";
    char artUrl[BUF_SIZE];

	// Copy Default when launch
	copy_file(defaultone, fullthumb);
	copy_file(defaultone, thumbnail);

    // Open playerctl follow mode (no sed)
    FILE *fp = popen("playerctl --follow metadata --format '{{mpris:artUrl}}'", "r");
    if (!fp) {
        perror("popen failed");
        return 1;
    }

    while (fgets(artUrl, sizeof(artUrl), fp)) {
        // strip newline
        size_t len = strlen(artUrl);
        if (len > 0 && artUrl[len - 1] == '\n') {
            artUrl[len - 1] = '\0';
        }

        // remove file:// if present
        strip_file_prefix(artUrl);

        // Only act if changed path
        if (strcmp(artUrl, lastUrl) == 0) {
            continue;
        }
        strncpy(lastUrl, artUrl, sizeof(lastUrl));
        lastUrl[sizeof(lastUrl)-1] = '\0';

        if (artUrl[0] != '\0' && access(artUrl, F_OK) == 0) {
            // Skip work if file is identical to current fullthumb
            if (!same_file(artUrl, fullthumb)) {
                copy_file(artUrl, fullthumb);

                // Generate masked thumbnail
                char *magick_args[] = {
                    "magick", (char *)artUrl,
                    "-thumbnail", "720x720",
                    "-gravity", "center",
                    "-extent", "720x720",
                    "-alpha", "Set",
                    (char *)mask,
                    "-compose", "DstIn",
                    "-composite",
                    (char *)thumbnail,
                    NULL
                };
                run_cmd(magick_args);
            }
        } else {
            // Fallback to default (skip if already copied)
			copy_file(defaultone, fullthumb);
			copy_file(defaultone, thumbnail);
        }
    }

    pclose(fp);
    return 0;
}

