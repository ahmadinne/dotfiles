#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <sys/signalfd.h>
#include <stdint.h>

static char last_class[16] = "";

double read_temp_file(const char *path) {
    int fd = open(path, O_RDONLY);
    if (fd < 0) return -1;

    char buf[16];
    int n = read(fd, buf, sizeof(buf) - 1);
    close(fd);
    if (n <= 0) return -1;

    buf[n] = 0;
    return atof(buf) / 1000.0;
}

double max_temp() {
    DIR *d = opendir("/sys/class/thermal");
    if (!d) return -1;

    struct dirent *ent;
    double max = -1;
    char path[256];

    while ((ent = readdir(d))) {
        if (strncmp(ent->d_name, "thermal_zone", 12) != 0)
            continue;

        snprintf(path, sizeof(path),
                 "/sys/class/thermal/%s/temp", ent->d_name);

        double t = read_temp_file(path);
        if (t > max) max = t;
    }
    closedir(d);
    return max;
}

const char *classify(double t) {
    if (t >= 85) return "critical";
    if (t >= 75) return "warning";
    if (t >= 60) return "mid";
    return "default";
}

void emit(int force) {
    double t = max_temp();
    if (t < 0) return;

    const char *cls = classify(t);

    if (!force && !strcmp(cls, last_class))
        return;

    printf("{\"text\":\"    \",\"tooltip\":\"Temp %.1f°C\",\"class\":\"%s\"}\n",
           t, cls);
    fflush(stdout);
    strcpy(last_class, cls);
}

int main() {
    int ep = epoll_create1(0);

    int tfd = timerfd_create(CLOCK_MONOTONIC, 0);
    struct itimerspec its = {
        .it_interval = {3, 0},
        .it_value = {1, 0}
    };
    timerfd_settime(tfd, 0, &its, NULL);

    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    sigaddset(&mask, SIGTERM);
    sigprocmask(SIG_BLOCK, &mask, NULL);
    int sfd = signalfd(-1, &mask, 0);

    struct epoll_event ev = { .events = EPOLLIN };
    ev.data.fd = tfd;
    epoll_ctl(ep, EPOLL_CTL_ADD, tfd, &ev);
    ev.data.fd = sfd;
    epoll_ctl(ep, EPOLL_CTL_ADD, sfd, &ev);

    while (1) {
        struct epoll_event events[2];
        int n = epoll_wait(ep, events, 2, -1);

        for (int i = 0; i < n; i++) {
            if (events[i].data.fd == tfd) {
                uint64_t x;
                read(tfd, &x, sizeof(x));
                emit(0);
            } else {
                struct signalfd_siginfo si;
                read(sfd, &si, sizeof(si));
                if (si.ssi_signo == SIGUSR1) emit(1);
                else return 0;
            }
        }
    }
}
