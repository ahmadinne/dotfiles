#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUF_SIZE 128

int main() {
    char status[BUF_SIZE];
    char prev_status[BUF_SIZE] = "Paused";

    // Emit default state when nothing is playing
    printf("{\"class\": \"stopped\"}\n");
    fflush(stdout);

    FILE *fp = popen("playerctl status --follow 2>/dev/null", "r");
    if (!fp) {
        perror("popen");
        return 1;
    }

    while (fgets(status, sizeof(status), fp)) {
        size_t len = strlen(status);
        if (len > 0 && status[len - 1] == '\n') {
            status[len - 1] = '\0';
        }

        if (strcmp(status, prev_status) != 0) {
            if (strcmp(status, "Playing") == 0) {
                printf("{\"class\": \"playing\"}\n");
            } else {
                printf("{\"class\": \"paused\"}\n");
            }
            fflush(stdout);
            strncpy(prev_status, status, sizeof(prev_status) - 1);
            prev_status[sizeof(prev_status) - 1] = '\0';
        }
    }

		// If playerctl exits, assume paused
		printf("{\"class\": \"stopped\"}\n");
		fflush(stdout);

    pclose(fp);
    return 0;
}
