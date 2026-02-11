#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

int is_base_waybar(pid_t pid) {
    char cmd[256];
    snprintf(
        cmd, sizeof(cmd),
        "ps -p %d -o args= | grep -q base/config.jsonc",
        pid
    );
    return system(cmd) == 0;
}

int main(void) {
    FILE *fp;
    char line[512];
		int active_visible_layer = 0;

    pid_t visible_pids[32];
    int visible_count = 0;

    fp = popen("hyprctl layers -j", "r");
    if (!fp) return 1;

    while (fgets(line, sizeof(line), fp)) {
        // Enter / leave level 2
				if (strstr(line, "\"2\"") || strstr(line, "\"3\"")) {
						active_visible_layer = 1;
						continue;
				}
				if (active_visible_layer && strstr(line, "]")) {
						active_visible_layer = 0;
				}

        // Look for visible waybar entries
        if (active_visible_layer && strstr(line, "\"namespace\": \"waybar\"")) {
            // Find pid in subsequent lines
            long pos = ftell(fp);
            while (fgets(line, sizeof(line), fp)) {
                if (strstr(line, "\"pid\"")) {
                    pid_t pid = atoi(strchr(line, ':') + 1);
                    if (pid > 0 && !is_base_waybar(pid)) {
                        visible_pids[visible_count++] = pid;
                    }
                    break;
                }
                if (strstr(line, "}"))
                    break;
            }
            fseek(fp, pos, SEEK_SET);
        }
    }

    pclose(fp);

    // Hide ONLY visible non-base waybars
    if (visible_count > 0) {
        for (int i = 0; i < visible_count; i++) {
            kill(visible_pids[i], SIGUSR1);
        }
        return 0;
    }

    // Otherwise → Escape
    system("hyprctl dispatch sendshortcut , Escape, ");
    return 0;
}

