// thumb_a.c
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#define BUF_SIZE 4096

/* copy_file: copy src -> dst */
static int copy_file(const char *src, const char *dst) {
    int in_fd = open(src, O_RDONLY);
    if (in_fd < 0) return -1;
    int out_fd = open(dst, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (out_fd < 0) { close(in_fd); return -1; }
    char buf[8192];
    ssize_t n;
    while ((n = read(in_fd, buf, sizeof(buf))) > 0) {
        ssize_t w = 0;
        char *p = buf;
        while (n > 0) {
            ssize_t nw = write(out_fd, p, n);
            if (nw <= 0) { close(in_fd); close(out_fd); return -1; }
            n -= nw;
            p += nw;
        }
    }
    close(in_fd);
    close(out_fd);
    return (n < 0) ? -1 : 0;
}

/* run_cmd: fork+execvp, wait for exit code */
static int run_cmd(char *const argv[]) {
    pid_t pid = fork();
    if (pid == 0) {
        execvp(argv[0], argv);
        _exit(127);
    } else if (pid < 0) {
        return -1;
    }
    int status = 0;
    waitpid(pid, &status, 0);
    return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
}

/* cheap_compare: compare inode + size, return 1 if same file (content likely same) */
static int cheap_compare(const char *a, const char *b) {
    struct stat sa, sb;
    if (stat(a, &sa) < 0 || stat(b, &sb) < 0) return 0;
    if (sa.st_ino == sb.st_ino && sa.st_dev == sb.st_dev) return 1;
    if (sa.st_size == sb.st_size) return 1; // same size -> likely same
    return 0;
}

/* strip file:// prefix */
static void strip_file_prefix(char *s) {
    const char *pref = "file://";
    size_t lp = strlen(pref);
    if (strncmp(s, pref, lp) == 0) memmove(s, s + lp, strlen(s + lp) + 1);
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

    /* playerctl follow */
    FILE *fp = popen("playerctl --follow metadata --format '{{mpris:artUrl}}'", "r");
    if (!fp) { perror("popen"); return 1; }

    while (fgets(artUrl, sizeof(artUrl), fp)) {
        size_t len = strlen(artUrl);
        if (len && artUrl[len-1] == '\n') artUrl[len-1] = '\0';
        strip_file_prefix(artUrl);

        if (strcmp(artUrl, lastUrl) == 0) continue;
        strncpy(lastUrl, artUrl, sizeof(lastUrl)-1);
        lastUrl[sizeof(lastUrl)-1] = '\0';

        if (artUrl[0] != '\0' && access(artUrl, F_OK) == 0) {
            /* if fullthumb already equals artUrl (cheap), skip work */
            if (access(fullthumb, F_OK) == 0 && cheap_compare(artUrl, fullthumb)) {
                /* nothing to do */
                continue;
            }
            if (copy_file(artUrl, fullthumb) != 0) {
                fprintf(stderr, "copy_file failed %s -> %s: %s\n", artUrl, fullthumb, strerror(errno));
            }
            /* run magick only when new/changed */
            char *magick_args[] = {
                "magick", (char *)fullthumb,
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
            int rc = run_cmd(magick_args);
            if (rc != 0) fprintf(stderr, "magick returned %d\n", rc);
        } else {
            /* fallback */
            copy_file(defaultone, fullthumb);
            copy_file(defaultone, thumbnail);
        }
    }

    pclose(fp);
    return 0;
}

