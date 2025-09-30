// hypr_ws.c
// clang hypr_ws.c -o hypr_ws

#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>

static void set_unbuffered_stdout(void) {
    setvbuf(stdout, NULL, _IONBF, 0);
}

// try env var names Waybar/Hyprland commonly set
static const char* env_pref_socket2_names[] = {
    "HYPRLAND_SOCKET2", // sometimes exported
    NULL
};
static const char* env_pref_socket1_names[] = {
    "HYPRLAND_SOCKET",
    NULL
};

// try to find event socket path: prefer HYPRLAND_SOCKET2, then try HYPRLAND_SOCKET + replace, then /tmp/hypr/<sig>...
static char* detect_socket2_path(void) {
    for (const char** p = env_pref_socket2_names; *p; ++p) {
        const char* v = getenv(*p);
        if (v && v[0]) return strdup(v);
    }
    // fallback: try HYPRLAND_SOCKET and replace ".socket.sock" -> ".socket2.sock"
    for (const char** p = env_pref_socket1_names; *p; ++p) {
        const char* v = getenv(*p);
        if (v && v[0]) {
            // if it ends with ".socket.sock" replace last 9 chars with "2.sock"
            size_t L = strlen(v);
            char* out = malloc(L + 1 + 16);
            if (!out) return NULL;
            strcpy(out, v);
            const char* suffix = ".socket.sock";
            size_t suf_len = strlen(suffix);
            if (L > suf_len && strcmp(v + L - suf_len, suffix) == 0) {
                // build .socket2.sock
                out[L - suf_len] = '\0';
                strcat(out, ".socket2.sock");
            } else {
                // naive: append "2" to try (rare)
                strcat(out, "2");
            }
            return out;
        }
    }
    // final fallback: try building from HYPRLAND_INSTANCE_SIGNATURE
    const char* sig = getenv("HYPRLAND_INSTANCE_SIGNATURE");
    if (sig && sig[0]) {
        char *buf = malloc(512);
        if (!buf) return NULL;
        snprintf(buf, 512, "/tmp/hypr/%s/.socket2.sock", sig);
        return buf;
    }
    return NULL;
}

static char* detect_socket1_path(void) {
    for (const char** p = env_pref_socket1_names; *p; ++p) {
        const char* v = getenv(*p);
        if (v && v[0]) return strdup(v);
    }
    const char* sig = getenv("HYPRLAND_INSTANCE_SIGNATURE");
    if (sig && sig[0]) {
        char *buf = malloc(512);
        if (!buf) return NULL;
        snprintf(buf, 512, "/tmp/hypr/%s/.socket.sock", sig);
        return buf;
    }
    return NULL;
}

static int connect_unix_socket(const char* path) {
    if (!path) return -1;
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) return -1;
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path, sizeof(addr.sun_path)-1);
    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(fd);
        return -1;
    }
    return fd;
}

// run hyprctl command and capture stdout (caller free)
static char* run_cmd_capture(const char* cmd) {
    FILE* fp = popen(cmd, "r");
    if (!fp) return NULL;
    size_t cap = 4096;
    char *buf = malloc(cap);
    if (!buf) { pclose(fp); return NULL; }
    size_t len = 0;
    while (!feof(fp)) {
        if (len + 1024 > cap) {
            cap *= 2;
            char *n = realloc(buf, cap);
            if (!n) break;
            buf = n;
        }
        size_t r = fread(buf + len, 1, 1024, fp);
        if (r == 0) break;
        len += r;
    }
    buf[len] = '\0';
    pclose(fp);
    return buf;
}

// Minimal JSON-ish parsing to get current (focused) workspace and max id from "hyprctl workspaces -j" or similar JSON text.
// This naive parser scans for patterns: "id":<num> and "focused":true
static void parse_workspaces_json_minimal(const char* json, int* out_current, int* out_max) {
    if (!json) return;
    int maxid = -1;
    int current = -1;
    const char* p = json;
    while ((p = strstr(p, "\"id\"")) != NULL) {
        // find ':' after "id"
        const char* q = strchr(p, ':');
        if (!q) { p += 4; continue; }
        int id = 0;
        if (sscanf(q+1, "%d", &id) == 1) {
            if (id > maxid) maxid = id;
            // search backward from p to see if "focused" appears nearby for this object
            // naive: find object start '{' before p, and search forward to next '}' for "focused":true
            const char* obj_start = p;
            while (obj_start > json && *obj_start != '{') obj_start--;
            const char* obj_end = p;
            while (obj_end && *obj_end && *obj_end != '}') obj_end++;
            if (obj_end) {
                if (strstr(obj_start, "\"focused\"") && strstr(obj_start, "true")) {
                    current = id;
                } else {
                    // maybe "focused":true appears after p but before obj_end
                    if (strstr(p, "\"focused\"") && strstr(p, "true")) current = id;
                }
            }
        }
        p += 4;
    }
    if (out_current) *out_current = current >= 0 ? current : -1;
    if (out_max) *out_max = maxid >= 0 ? maxid : -1;
}

// send simple query to socket1: Hyprland expects requests like "j/workspaces" (Waybar uses such path). We write that and read a reply.
// Some Hyprland variants accept sending "j/workspaces" (without newline). We'll write and then read.
static char* query_socket1_for_workspaces(const char* sock1_path) {
    if (!sock1_path) return NULL;
    int fd = connect_unix_socket(sock1_path);
    if (fd < 0) return NULL;
    const char* req = "j/workspaces";
    ssize_t w = write(fd, req, strlen(req));
    (void)w;
    // read response
    char tmp[4096];
    size_t cap = 8192;
    char *buf = malloc(cap);
    if (!buf) { close(fd); return NULL; }
    size_t len = 0;
    while (1) {
        ssize_t r = read(fd, tmp, sizeof(tmp));
        if (r <= 0) break;
        if (len + (size_t)r + 1 > cap) {
            cap *= 2;
            char *n = realloc(buf, cap);
            if (!n) break;
            buf = n;
        }
        memcpy(buf + len, tmp, r);
        len += r;
    }
    buf[len] = '\0';
    close(fd);
    return buf;
}

int main(void) {
    set_unbuffered_stdout();

    // detect sockets
    char *sock2 = detect_socket2_path();
    char *sock1 = detect_socket1_path(); // for initial query if available

    int current = -1, max = -1;

    // if sock1 available, query initial workspaces
    if (sock1) {
        char* reply = query_socket1_for_workspaces(sock1);
        if (reply) {
            parse_workspaces_json_minimal(reply, &current, &max);
            free(reply);
        } else {
            // fallback to hyprctl -j if socket1 doesn't reply
            char* out = run_cmd_capture("hyprctl workspaces -j 2>/dev/null");
            if (out) {
                parse_workspaces_json_minimal(out, &current, &max);
                free(out);
            }
        }
    } else {
        // if no socket1: fallback to hyprctl command
        char* out = run_cmd_capture("hyprctl workspaces -j 2>/dev/null");
        if (out) {
            parse_workspaces_json_minimal(out, &current, &max);
            free(out);
        }
    }

    // If still unknown, try hyprctl activeworkspace
    if (current < 0) {
        char *a = run_cmd_capture("hyprctl activeworkspace 2>/dev/null");
        if (a) {
            // hyprctl activeworkspace prints lines; search "workspace ID <num>"
            char *p = strstr(a, "workspace ID");
            if (p) {
                int id;
                if (sscanf(p, "workspace ID %d", &id) == 1) current = id;
            }
            free(a);
        }
    }

    // connect to socket2 event stream if available
    int evfd = -1;
    if (sock2) {
        evfd = connect_unix_socket(sock2);
    }

    // If event socket not present, we will fall back to polling mode using hyprctl every 2 seconds (less CPU than 1s)
    int polling_mode = (evfd < 0);

    // print initial JSON if we have values; else print zeros
    if (current < 0) current = 0;
    if (max < 0) max = current > 0 ? current : 0;

    // print initial line
	printf("{\"text\":\"%d/%d\"}\n", current, max);
    fflush(stdout);

    if (polling_mode) {
        // Polling loop (less ideal). Poll every 2s.
        while (1) {
            // sleep(2);
            int new_current = -1, new_max = -1;
            char* out = run_cmd_capture("hyprctl workspaces -j 2>/dev/null");
            if (out) {
                parse_workspaces_json_minimal(out, &new_current, &new_max);
                free(out);
            }
            if (new_current < 0) {
                char* a = run_cmd_capture("hyprctl activeworkspace 2>/dev/null");
                if (a) {
                    char *p = strstr(a, "workspace ID");
                    if (p) {
                        int id;
                        if (sscanf(p, "workspace ID %d", &id) == 1) new_current = id;
                    }
                    free(a);
                }
            }
            if (new_current < 0) new_current = current;
            if (new_max < 0) new_max = max;

            if (new_current != current || new_max != max) {
                current = new_current; max = new_max;
				printf("{\"text\":\"%d/%d\"}\n", current, max);
                fflush(stdout);
            }
        }
    } else {
        // Event-driven mode: read events and update values
        char evbuf[4096];
        while (1) {
            ssize_t r = read(evfd, evbuf, sizeof(evbuf)-1);
            if (r <= 0) {
                if (r < 0) perror("read event socket");
                close(evfd);
                free(sock2);
                sock2 = detect_socket2_path();
                // sleep(1);
                evfd = sock2 ? connect_unix_socket(sock2) : -1;
                if (evfd < 0) {
                    // fall back to polling
                    polling_mode = 1;
                    break;
                } else continue;
            }
            evbuf[r] = '\0';
            // handle possibly multiple events in buffer separated by newlines
            char* line = evbuf;
            while (line < evbuf + r) {
                // find end of this line (some Hyprland builds send events as single message)
                char* eol = memchr(line, '\n', (evbuf + r) - line);
                size_t len = eol ? (size_t)(eol - line) : strlen(line);
                char tmp[1024];
                if (len >= sizeof(tmp)) len = sizeof(tmp)-1;
                memcpy(tmp, line, len);
                tmp[len] = '\0';

                // Parse events of interest
                if (strncmp(tmp, "workspace>>", 11) == 0) {
                    int id = atoi(tmp + 11);
                    if (id >= 0) {
                        if (id != current) {
                            current = id;
                        }
                        if (id > max) max = id;
                    }
                } else if (strncmp(tmp, "createworkspace>>", 17) == 0) {
                    int id = atoi(tmp + 17);
                    if (id > max) max = id;
                } else if (strncmp(tmp, "destroyworkspace>>", 18) == 0) {
                    int id = atoi(tmp + 18);
                    if (id == max) {
                        // coarse: recompute max by querying socket1 or hyprctl
                        int new_max = -1;
                        char *reply = NULL;
                        if (sock1) reply = query_socket1_for_workspaces(sock1);
                        if (!reply) reply = run_cmd_capture("hyprctl workspaces -j 2>/dev/null");
                        if (reply) {
                            parse_workspaces_json_minimal(reply, NULL, &new_max);
                            free(reply);
                        }
                        if (new_max >= 0) max = new_max;
                        else if (max > 0) max--;
                    }
                } else {
                    // other events ignored
                }

                // print only when changed (or immediately after handling event)
				printf("{\"text\":\"%d/%d\"}\n", current, max);
                fflush(stdout);

                if (!eol) break;
                line = eol + 1;
            }
        }

        // if we broke out to polling mode
        if (polling_mode) {
            while (1) {
                // sleep(2);
                int new_current = -1, new_max = -1;
                char* out = run_cmd_capture("hyprctl workspaces -j 2>/dev/null");
                if (out) {
                    parse_workspaces_json_minimal(out, &new_current, &new_max);
                    free(out);
                }
                if (new_current < 0) {
                    char* a = run_cmd_capture("hyprctl activeworkspace 2>/dev/null");
                    if (a) {
                        char *p = strstr(a, "workspace ID");
                        if (p) {
                            int id;
                            if (sscanf(p, "workspace ID %d", &id) == 1) new_current = id;
                        }
                        free(a);
                    }
                }
                if (new_current < 0) new_current = current;
                if (new_max < 0) new_max = max;

                if (new_current != current || new_max != max) {
                    current = new_current; max = new_max;
					printf("{\"text\":\"%d/%d\"}\n", current, max);
                    fflush(stdout);
                }
            }
        }
    }

    // cleanup
    if (evfd >= 0) close(evfd);
    if (sock1) free(sock1);
    if (sock2) free(sock2);
    return 0;
}

