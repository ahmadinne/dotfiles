#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/select.h>

#define FIFO_PATH "/tmp/waybar_cal_fifo"

/* ---------- helpers ---------- */

int days_in_month(int y, int m) {
    static int d[] = {31,28,31,30,31,30,31,31,30,31,30,31};
    if (m == 1) {
        int leap = (y%4==0 && y%100!=0) || (y%400==0);
        return 28 + leap;
    }
    return d[m];
}

unsigned seconds_until_midnight(void) {
    time_t now = time(NULL);
    struct tm t = *localtime(&now);
    t.tm_hour = 24; t.tm_min = 0; t.tm_sec = 0;
    return (unsigned)difftime(mktime(&t), now) + 1; // +1 to be safe
}

/* ---------- main ---------- */

int main(int argc, char **argv) {
    /* --- CLIENT MODE: Send command to daemon and exit --- */
    if (argc > 1) {
        int fd = open(FIFO_PATH, O_WRONLY | O_NONBLOCK);
        if (fd >= 0) {
            write(fd, argv[1], strlen(argv[1]));
            close(fd);
        }
        return 0;
    }

    /* --- DAEMON MODE: Run continuously for Waybar --- */
    
    // Create FIFO (ignore error if it already exists)
    mkfifo(FIFO_PATH, 0666);
    
    // Open RDWR so select() doesn't spam EOF when no writers are connected
    int fd = open(FIFO_PATH, O_RDWR | O_NONBLOCK);
    if (fd < 0) return 1;

    int month_offset = 0;
	const char *color_month = "#d9bc8c";
    const char *color_today = "#89b4fa"; 
    const char *color_other = "#6c7086"; 

    static const char *m_names[] = {
        "January","February","March","April","May","June",
        "July","August","September","October","November","December"
    };
    static const char *wd[] = {"Su","Mo","Tu","We","Th","Fr","Sa"};

    // Infinite loop for Waybar
    for (;;) {
        time_t now = time(NULL);
        struct tm t = *localtime(&now);

        int actual_year  = t.tm_year + 1900;
        int actual_month = t.tm_mon;
        int today        = t.tm_mday;
        int today_wday   = t.tm_wday;

        // Apply navigation offset
        int display_month = actual_month + month_offset;
        int display_year  = actual_year;

        while (display_month > 11) { display_month -= 12; display_year++; }
        while (display_month < 0)  { display_month += 12; display_year--; }

        int is_actual_month = (display_month == actual_month && display_year == actual_year);

        struct tm first = t;
        first.tm_year = display_year - 1900;
        first.tm_mon  = display_month;
        first.tm_mday = 1;
        first.tm_hour = 12; // Prevent daylight saving time skips
        mktime(&first);

        int first_wday = first.tm_wday;
        int dim        = days_in_month(display_year, display_month);
        int prev_dim   = days_in_month(display_year, (display_month+11)%12);

        /* ---------- build grid ---------- */
        int grid[6][7];
        int is_current[6][7];
        int d = 1 - first_wday;

        for (int r=0; r<6; r++) {
            for (int c=0; c<7; c++, d++) {
                if (d < 1) {
                    grid[r][c] = prev_dim + d;
                    is_current[r][c] = 0;
                } else if (d > dim) {
                    grid[r][c] = d - dim;
                    is_current[r][c] = 0;
                } else {
                    grid[r][c] = d;
                    is_current[r][c] = 1;
                }
            }
        }

        /* ---------- build massive string ---------- */
        char buffer[2048] = "";

		const char *title_color = is_actual_month ? color_month : color_other;

        sprintf(buffer + strlen(buffer), "<span size='x-large' color='%s'><b>%s %d</b></span>\\n\\n", title_color, m_names[display_month], display_year);

        for (int i=0; i<7; i++) {
            if (is_actual_month && i == today_wday) {
                sprintf(buffer + strlen(buffer), "<span color='%s'><b>%s</b></span>", color_today, wd[i]);
            } else {
                sprintf(buffer + strlen(buffer), "%s", wd[i]);
            }
            if (i < 6) sprintf(buffer + strlen(buffer), " ");
        }
        sprintf(buffer + strlen(buffer), "\\n\\n");

        for (int r=0; r<6; r++) {
            for (int c=0; c<7; c++) {
                int val = grid[r][c];
                char num_str[16];
                
                if (val < 10) sprintf(num_str, "&#160;%d", val);
                else sprintf(num_str, "%d", val);
                
                if (is_actual_month && is_current[r][c] && val == today) {
                    sprintf(buffer + strlen(buffer), "<span color='%s'><b>%s</b></span>", color_today, num_str);
                } else if (!is_current[r][c]) {
                    sprintf(buffer + strlen(buffer), "<span color='%s'>%s</span>", color_other, num_str);
                } else {
                    sprintf(buffer + strlen(buffer), "%s", num_str);
                }
                
                if (c < 6) sprintf(buffer + strlen(buffer), " ");
            }
            if (r < 5) sprintf(buffer + strlen(buffer), "\\n\\n");
        }

        /* ---------- print to waybar ---------- */
        printf("{\"text\":\"%s\"}\n", buffer);
        fflush(stdout); // CRITICAL: Forces Waybar to read the line immediately

        /* ---------- wait for event ---------- */
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(fd, &readfds);

        struct timeval tv;
        tv.tv_sec  = seconds_until_midnight();
        tv.tv_usec = 0;

        // Sleep until midnight OR until a message arrives in the FIFO
        int ret = select(fd + 1, &readfds, NULL, NULL, &tv);

        if (ret > 0 && FD_ISSET(fd, &readfds)) {
            char cmd[32] = {0};
            read(fd, cmd, sizeof(cmd) - 1);
            
            if (strstr(cmd, "next")) month_offset++;
            else if (strstr(cmd, "prev")) month_offset--;
            else if (strstr(cmd, "curr")) month_offset = 0;
            
        } else if (ret == 0) {
            // Timeout reached (Midnight). Jump back to current month.
            month_offset = 0;
        }
    }

    close(fd);
    return 0;
}
