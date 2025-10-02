#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <fcntl.h>

#define MAX_LEN 20
#define BUF_SIZE 1024

// Replace "weird" fullwidth symbols with normal ASCII
// void normalize(char *s) {
//     unsigned char *p = (unsigned char *)s;
//     char buf[BUF_SIZE];
//     int j = 0;
//
//     while (*p) {
//         // check for UTF-8 3-byte sequences starting with E3 80 xx
//         if (p[0] == 0xE3 && p[1] == 0x80) {
//             switch (p[2]) {
//                 case 0x96: // U+3016 〖
//                     buf[j++] = '[';
//                     p += 3;
//                     continue;
//                 case 0x97: // U+3017 〗
//                     buf[j++] = ']';
//                     p += 3;
//                     continue;
//                 case 0x8E: // U+300E 『
//                 case 0x8C: // U+300C 「
//                     buf[j++] = '[';
//                     p += 3;
//                     continue;
//                 case 0x8F: // U+300F 』
//                     buf[j++] = ']';
//                     p += 3;
//                     continue;
//             }
//         }
//
//         // default: copy byte as-is
//         buf[j++] = *p++;
//     }
//
//     buf[j] = '\0';
//     strcpy(s, buf);
// }

// Non-blocking check if new data is available on a FILE*
int data_available(FILE *fp) {
    int fd = fileno(fp);
    fd_set fds;
    struct timeval tv = {0, 0};
    FD_ZERO(&fds);
    FD_SET(fd, &fds);
    return select(fd + 1, &fds, NULL, NULL, &tv) > 0;
}

int main() {
    char teks[BUF_SIZE];
    char teks_gabung[BUF_SIZE * 2];

    FILE *fp = popen("playerctl metadata --format '{{title}}' --follow", "r");
    if (!fp) {
        perror("popen");
        return 1;
    }

    while (fgets(teks, sizeof(teks), fp)) {
        // strip newline
        size_t len = strlen(teks);
        if (len > 0 && teks[len - 1] == '\n') {
            teks[len - 1] = '\0';
            len--;
        }

        if (len == 0) {
            strncpy(teks, "Nothing played rn.", sizeof(teks));
            len = strlen(teks);
        }

        // normalize symbols
        // normalize(teks);
        len = strlen(teks);

        if (len > MAX_LEN) {
            snprintf(teks_gabung, sizeof(teks_gabung), "%s %s", teks, teks);

            int i = 0;
            while (1) {
                char potong[MAX_LEN + 1];
                strncpy(potong, teks_gabung + (i % len), MAX_LEN);
                potong[MAX_LEN] = '\0';

                printf("{\"text\": \"%s\", \"class\": \"\"}\n", potong);
                fflush(stdout);
                usleep(500000); // scroll speed

                if (data_available(fp)) break;
                i++;
            }
        } else {
            while (1) {
                printf("{\"text\": \"%s\", \"class\": \"\"}\n", teks);
                fflush(stdout);
                sleep(2);

                if (data_available(fp)) break;
            }
        }
    }

    pclose(fp);
    return 0;
}

