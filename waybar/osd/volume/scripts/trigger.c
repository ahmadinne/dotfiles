#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>

#define PID_FILE "/tmp/waybar_vol_osd.pid"

// Global flag to track if we received a signal
volatile sig_atomic_t extensions_requested = 0;

void sig_handler(int signo) {
    if (signo == SIGUSR1) {
        extensions_requested = 1;
    }
}

// Helper function: drastically simplified and less resource-heavy
pid_t get_waybar_pid() {
    FILE *fp = popen("pgrep -f 'waybar.*volume'", "r");
    if (!fp) return -1;
    
    char buf[32];
    if (fgets(buf, sizeof(buf), fp) != NULL) {
        pclose(fp);
        return atoi(buf);
    }
    pclose(fp);
    return -1;
}

int main() {
    int fd = open(PID_FILE, O_RDWR | O_CREAT, 0666);
    if (fd < 0) { 
        perror("Failed to open PID file"); 
        return 1; 
    }

    struct flock fl = { .l_type = F_WRLCK, .l_whence = SEEK_SET };

    if (fcntl(fd, F_SETLK, &fl) == -1) {
        // Another instance is running! Read its PID and signal it.
        char buf[32] = {0};
        if (read(fd, buf, sizeof(buf)-1) > 0) {
            pid_t running_pid = atoi(buf);
            if (running_pid > 0) {
                kill(running_pid, SIGUSR1);
            }
        }
        close(fd);
        return 0; // Exit immediately
    }

    // Write our PID to the file
    ftruncate(fd, 0);
    char pid_str[32];
    int len = snprintf(pid_str, sizeof(pid_str), "%d\n", getpid());
    write(fd, pid_str, len);

    struct sigaction sa = { .sa_handler = sig_handler };
    sigaction(SIGUSR1, &sa, NULL);

    pid_t waybar_pid = get_waybar_pid();
    if (waybar_pid <= 0) {
        remove(PID_FILE);
        return 1;
    }
    
    kill(waybar_pid, SIGUSR1); // Show module

    // 0% CPU Timer Loop
    unsigned int time_left = 3; 
    
    while (time_left > 0) {
        // sleep() suspends execution. If interrupted by SIGUSR1, 
        // it wakes up immediately and returns the unslept seconds.
        time_left = sleep(time_left); 
        
        if (extensions_requested) {
            time_left = 3; // Reset timer back to full 3 seconds
            extensions_requested = 0;
        }
    }

    // Timer completely expired
    kill(waybar_pid, SIGUSR1); // Hide module

    // Cleanup
    remove(PID_FILE);
    close(fd);
    return 0;
}
