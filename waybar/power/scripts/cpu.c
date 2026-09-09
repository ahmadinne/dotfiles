#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <sys/signalfd.h>
#include <stdint.h>

typedef struct {
    unsigned long long user, nice, system, idle, iowait, irq, softirq, steal;
} cpu_times;

static cpu_times prev;
static int have_prev = 0;

static double ema = -1.0;
#define EMA_ALPHA 0.25

static char last_class[16] = "";

/* ---------- CPU ---------- */

int read_cpu(cpu_times *t) {
    FILE *f = fopen("/proc/stat", "r");
    if (!f) return -1;

    fscanf(f, "cpu %llu %llu %llu %llu %llu %llu %llu %llu",
        &t->user, &t->nice, &t->system, &t->idle,
        &t->iowait, &t->irq, &t->softirq, &t->steal);

    fclose(f);
    return 0;
}

unsigned long long total(cpu_times *t) {
    return t->user + t->nice + t->system + t->idle +
           t->iowait + t->irq + t->softirq + t->steal;
}

double cpu_usage() {
    cpu_times cur;
    if (read_cpu(&cur) < 0) return -1;

    if (!have_prev) {
        prev = cur;
        have_prev = 1;
        return -1;
    }

    unsigned long long td = total(&cur) - total(&prev);
    unsigned long long id =
        (cur.idle + cur.iowait) - (prev.idle + prev.iowait);

    prev = cur;
    if (!td) return -1;

    double raw = 100.0 * (td - id) / td;

    if (ema < 0) ema = raw;
    else ema = EMA_ALPHA * raw + (1 - EMA_ALPHA) * ema;

    return ema;
}

/* ---------- CLASSIFY + HYSTERESIS ---------- */

const char *classify(double cpu) {
    static const char *cls = "default";

    if (!strcmp(cls, "default") && cpu > 30) cls = "mid";
    else if (!strcmp(cls, "mid") && cpu > 65) cls = "warning";
    else if (!strcmp(cls, "warning") && cpu > 85) cls = "critical";

    else if (!strcmp(cls, "critical") && cpu < 75) cls = "warning";
    else if (!strcmp(cls, "warning") && cpu < 55) cls = "mid";
    else if (!strcmp(cls, "mid") && cpu < 20) cls = "default";

    return cls;
}

/* ---------- OUTPUT ---------- */

void emit(int force) {
    double cpu = cpu_usage();
    if (cpu < 0) return;

    const char *cls = classify(cpu);

    if (!force && !strcmp(cls, last_class))
        return;

    printf("{\"text\":\"    \",\"tooltip\":\"CPU %.1f%%\",\"class\":\"%s\"}\n",
           cpu, cls);
    fflush(stdout);
    strcpy(last_class, cls);
}

/* ---------- MAIN ---------- */

int main() {
    int ep = epoll_create1(0);

    /* timer */
    int tfd = timerfd_create(CLOCK_MONOTONIC, 0);
    struct itimerspec its = {
        .it_interval = {1, 0},
        .it_value = {1, 0}
    };
    timerfd_settime(tfd, 0, &its, NULL);

    /* signals */
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

