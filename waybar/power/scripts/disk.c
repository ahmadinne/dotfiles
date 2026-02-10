#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/statvfs.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <sys/signalfd.h>
#include <stdint.h>

static char last_class[16] = "";

double disk_usage(const char *path) {
    struct statvfs v;
    if (statvfs(path, &v) < 0) return -1;

    unsigned long long total = v.f_blocks * v.f_frsize;
    unsigned long long avail = v.f_bavail * v.f_frsize;
    if (!total) return -1;

    return 100.0 * (total - avail) / total;
}

const char *classify(double d) {
    if (d >= 95) return "critical";
    if (d >= 85) return "warning";
    if (d >= 70) return "mid";
    return "default";
}

void emit(int force) {
    double d = disk_usage("/");
    if (d < 0) return;

    const char *cls = classify(d);

    if (!force && !strcmp(cls, last_class))
        return;

    printf("{\"text\":\"    \",\"tooltip\":\"Disk %.1f%%\",\"class\":\"%s\"}\n",
           d, cls);
    fflush(stdout);
    strcpy(last_class, cls);
}

int main() {
    int ep = epoll_create1(0);

    int tfd = timerfd_create(CLOCK_MONOTONIC, 0);
    struct itimerspec its = {
        .it_interval = {10, 0},
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
