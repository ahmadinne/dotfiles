#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/inotify.h>
#include <string.h>

#define BAT_PATH "/sys/class/power_supply/BAT0/capacity"
#define BUF_LEN (1024)

int read_capacity(void) {
    int fd = open(BAT_PATH, O_RDONLY);
    if (fd < 0) return -1;

    char buf[8];
    int len = read(fd, buf, sizeof(buf) - 1);
    close(fd);

    if (len <= 0) return -1;

    buf[len] = '\0';
    return atoi(buf);
}

int main(void) {
    int last = -1;

    int inotify_fd = inotify_init1(IN_NONBLOCK);
    if (inotify_fd < 0) {
        perror("inotify_init");
        return 1;
    }

    int wd = inotify_add_watch(
        inotify_fd,
        BAT_PATH,
        IN_MODIFY
    );

    if (wd < 0) {
        perror("inotify_add_watch");
        return 1;
    }

    while (1) {
        char buf[BUF_LEN];
        read(inotify_fd, buf, sizeof(buf)); // blocks until change

        int now = read_capacity();
        if (now < 0 || now == last) continue;

        last = now;

        if (now > 99)
            printf("100\n");
        else
            printf("%d%%\n", now);

        fflush(stdout);
    }
}
