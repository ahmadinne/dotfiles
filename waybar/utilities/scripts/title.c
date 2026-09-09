#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUF_SIZE 1024

int main() {
	char title[BUF_SIZE];
	char prev_title[BUF_SIZE] = "-";

	printf("{\"text\": \"-\"}\n");
	fflush(stdout);

	FILE *fp = popen("playerctl metadata title --follow 2>/dev/null", "r");
	if (!fp) {
		perror("popen");
		return 1;
	}

	while (fgets(title, sizeof(title), fp)) {
		size_t len = strlen(title);
		if (len > 0 && title[len - 1] == '\n') {
			title[len - 1] = '\0';
		}

		if (title[0] == '\0') {
			strcpy(title, "-");
		}

		if (strcmp(title, prev_title) != 0) {
			printf("{\"text\": \"%s\"}\n", title);
			fflush(stdout);
			strncpy(prev_title, title, sizeof(prev_title) - 1);
			prev_title[sizeof(prev_title) - 1] = '\0';
		}
	}

	printf("{\"text\": \"-\"}\n");
	fflush(stdout);

	pclose(fp);
	return 0;
}
