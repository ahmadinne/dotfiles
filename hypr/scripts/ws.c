#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>

int hyprland_ipc(const char *socket_path, const char *command, char *response, size_t response_size) {
    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("DEBUG: socket() failed");
        return -1;
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);

    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        fprintf(stderr, "DEBUG: connect() failed for %s - %s\n", socket_path, strerror(errno));
        close(sock);
        return -1;
    }

    if (write(sock, command, strlen(command)) == -1) {
        perror("DEBUG: write() failed");
        close(sock);
        return -1;
    }

    int bytes_read = 0;
    if (response != NULL && response_size > 0) {
        bytes_read = read(sock, response, response_size - 1);
        if (bytes_read >= 0) {
            response[bytes_read] = '\0';
        } else {
            perror("DEBUG: read() failed");
        }
    }

    close(sock);
    return bytes_read;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "DEBUG: No target workspace provided. Usage: ./fast_switch <num> [move|m]\n");
        return 1;
    }

    int choice = atoi(argv[1]);
    const char *cmd = "workspace";
    
    if (argc >= 3 && (strcmp(argv[2], "move") == 0 || strcmp(argv[2], "m") == 0)) {
        cmd = "movetoworkspace";
    }

    // 1. Locate the correct socket path based on modern/legacy Hyprland locations
    const char *sig = getenv("HYPRLAND_INSTANCE_SIGNATURE");
    if (!sig) {
        fprintf(stderr, "DEBUG: HYPRLAND_INSTANCE_SIGNATURE not set. Are you running this inside Hyprland?\n");
        return 1;
    }

    const char *xdg_runtime = getenv("XDG_RUNTIME_DIR");
    char socket_path[512];
    
    if (xdg_runtime) {
        snprintf(socket_path, sizeof(socket_path), "%s/hypr/%s/.socket.sock", xdg_runtime, sig);
    } else {
        snprintf(socket_path, sizeof(socket_path), "/tmp/hypr/%s/.socket.sock", sig);
    }

    printf("DEBUG: Using socket path: %s\n", socket_path);

    // 2. Query Active Workspace
    char buffer[512];
    if (hyprland_ipc(socket_path, "activeworkspace", buffer, sizeof(buffer)) <= 0) {
        fprintf(stderr, "DEBUG: Failed to get active workspace from socket.\n");
        return 1;
    }

    // 3. Parse the output natively
    int current_ws = -1;
    char *id_ptr = strstr(buffer, "workspace ID ");
    if (id_ptr) {
        current_ws = atoi(id_ptr + 13);
    }

    printf("DEBUG: Parsed current workspace ID: %d, Target choice: %d\n", current_ws, choice);

    if (current_ws == choice) {
        printf("DEBUG: Already on target workspace. Exiting cleanly.\n");
        return 0;
    } else if (current_ws == -1) {
        fprintf(stderr, "DEBUG: Could not parse workspace ID from buffer string: \n%s\n", buffer);
        return 1;
    }

    // 4. Send Dispatch Command
    int target_ws = (current_ws < 11) ? choice : (choice + 10);
    char dispatch_cmd[256];
    
    snprintf(dispatch_cmd, sizeof(dispatch_cmd), "dispatch %s %d", cmd, target_ws);
    printf("DEBUG: Sending command: %s\n", dispatch_cmd);

    if (hyprland_ipc(socket_path, dispatch_cmd, NULL, 0) == -1) {
        fprintf(stderr, "DEBUG: Failed to send dispatch command.\n");
        return 1;
    }

    printf("DEBUG: Success.\n");
    return 0;
}
