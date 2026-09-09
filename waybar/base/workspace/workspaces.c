#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

// Function to get the current active workspace ID
int get_active_workspace() {
    FILE *fp = popen("hyprctl activeworkspace -j", "r");
    if (!fp) return -1;

    char buffer[1024];
    int active_id = -1;

    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        char *match = strstr(buffer, "\"id\":");
        if (match) {
            char *colon = strchr(match, ':');
            if (colon) {
                active_id = atoi(colon + 1);
                break;
            }
        }
    }
    pclose(fp);
    return active_id;
}

// Function to get the highest workspace ID (excluding <= 0)
int get_max_workspace() {
    FILE *fp = popen("hyprctl workspaces -j", "r");
    if (!fp) return -1;

    char buffer[1024];
    int max_id = -1;

    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        char *match = strstr(buffer, "\"id\":");
        if (match) {
            char *colon = strchr(match, ':');
            if (colon) {
                int id = atoi(colon + 1);
                if (id >= 1 && id > max_id) {
                    max_id = id;
                }
            }
        }
    }
    pclose(fp);
    return max_id;
}

// Function to check state and print if changed
void update_and_print() {
    static int prev_active = -1;
    static int prev_max = -1;

    int active = get_active_workspace();
    int max = get_max_workspace();

    if (active != -1 && max != -1) {
        if (active != prev_active || max != prev_max) {
            printf("{\"text\": \"%d/%d\"}\n", active, max);
            // fflush is required here to push the output immediately
            fflush(stdout); 
            
            prev_active = active;
            prev_max = max;
        }
    }
}

int main() {
    // Disable output buffering
    setvbuf(stdout, NULL, _IONBF, 0);

    // Print the initial state immediately upon starting
    update_and_print();

    // Get Hyprland environment variables to locate the socket
    const char *his = getenv("HYPRLAND_INSTANCE_SIGNATURE");
    const char *xrd = getenv("XDG_RUNTIME_DIR");

    if (!his) {
        fprintf(stderr, "Error: HYPRLAND_INSTANCE_SIGNATURE not set. Are you running Hyprland?\n");
        return 1;
    }

    // Construct the socket path
    char socket_path[1024];
    if (xrd) {
        snprintf(socket_path, sizeof(socket_path), "%s/hypr/%s/.socket2.sock", xrd, his);
    } else {
        snprintf(socket_path, sizeof(socket_path), "/tmp/hypr/%s/.socket2.sock", his);
    }

    // Create a UNIX socket
    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("Socket creation failed");
        return 1;
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);

    // Connect to the Hyprland event socket
    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("Failed to connect to Hyprland socket");
        close(sock);
        return 1;
    }

    // Convert the socket file descriptor to a standard FILE stream for easy reading
    FILE *fp = fdopen(sock, "r");
    if (!fp) {
        perror("Failed to open socket stream");
        close(sock);
        return 1;
    }

    char event_buffer[1024];
    
    // fgets will "block" here and use 0% CPU until Hyprland sends a new event
    while (fgets(event_buffer, sizeof(event_buffer), fp) != NULL) {
        // Only trigger our hyprctl checks if the event is workspace-related
        if (strncmp(event_buffer, "workspace>>", 11) == 0 ||
            strncmp(event_buffer, "createworkspace>>", 17) == 0 ||
            strncmp(event_buffer, "destroyworkspace>>", 18) == 0 ||
			strncmp(event_buffer, "focusedmon>>", 12) == 0) {
            
            update_and_print();
        }
    }

    fclose(fp);
    return 0;
}
