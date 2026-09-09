#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <fcntl.h>
#include <wchar.h>
#include <locale.h>

#define MAX_WIDTH 20 // This is now maximum visual columns, not character count
#define BUF_SIZE 1024

int data_available(FILE *fp) {
	int fd = fileno(fp);
	fd_set fds;
	struct timeval tv = {0, 0};
	FD_ZERO(&fds);
	FD_SET(fd, &fds);
	return select(fd + 1, &fds, NULL, NULL, &tv) > 0;
}

// Helper to calculate the total visual width of a wide string
int get_display_width(const wchar_t *str) {
    int width = 0;
    while (*str) {
        int w = wcwidth(*str);
        if (w > 0) width += w;
        str++;
    }
    return width;
}

int main() {
	setlocale(LC_ALL, ""); 

	char teks[BUF_SIZE];
	wchar_t wteks[BUF_SIZE];
	wchar_t wteks_gabung[BUF_SIZE * 2];

	FILE *fp = popen("playerctl metadata title --follow", "r");
	if (!fp) {
		perror("popen");
		return 1;
	}

	printf("{\"text\": \"-\", \"class\": \"idle\"}\n");
	fflush(stdout);

	while (fgets(teks, sizeof(teks), fp)) {
		size_t blen = strlen(teks);
		if (blen > 0 && teks[blen - 1] == '\n') {
			teks[blen - 1] = '\0';
		}

		if (teks[0] == '\0') {
			strcpy(teks, "-");
		}

		mbstowcs(wteks, teks, BUF_SIZE);
        
		// Check total visual width instead of character count
		int total_width = get_display_width(wteks);

		if (total_width > MAX_WIDTH) {
			swprintf(wteks_gabung, BUF_SIZE * 2, L"%ls %ls", wteks, wteks);
			int len = wcslen(wteks);
			int i = 0;

			while (1) {
				wchar_t wpotong[BUF_SIZE];
				int current_width = 0;
				int char_idx = 0;
				int start_idx = i % (len + 1); // +1 accounts for the space we added in swprintf

				// Build the substring until we hit the MAX_WIDTH limit
				while (wteks_gabung[start_idx] != L'\0') {
					wchar_t wc = wteks_gabung[start_idx];
					int w = wcwidth(wc);
					if (w < 0) w = 0; // Handle non-printable characters safely

					if (current_width + w > MAX_WIDTH) {
						// If we have 1 column of space left but the next char is 2 columns wide,
						// we pad it with a space so the UI doesn't jump around.
						if (current_width < MAX_WIDTH) {
							wpotong[char_idx++] = L' ';
						}
						break;
					}
					
					wpotong[char_idx++] = wc;
					current_width += w;
					start_idx++;
				}
				wpotong[char_idx] = L'\0';

				char potong[BUF_SIZE];
				wcstombs(potong, wpotong, BUF_SIZE);

				printf("{\"text\": \"%s\", \"class\": \"\"}\n", potong);
				fflush(stdout);
				usleep(700000);

				if (data_available(fp))
					break;

				i++;
			}
		} else {
			printf("{\"text\": \"%s\"}\n", teks);
			fflush(stdout);
		}
	}

	pclose(fp);
	return 0;
}
