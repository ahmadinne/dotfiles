#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUF_SIZE 1024

// Abbreviate artist name: first two words full, remaining initials
void abbreviate_artist(const char *input, char *output, size_t out_size) {
    output[0] = '\0';
    int word_count = 0;
    const char *p = input;

    while (*p && strlen(output) < out_size - 1) {
        // Skip leading spaces
        while (*p == ' ') p++;
        if (!*p) break;

        // Find end of word
        const char *start = p;
        while (*p && *p != ' ') p++;
        size_t wlen = p - start;

        if (word_count < 2) {
            // Copy first two words full
            if (word_count > 0) strncat(output, " ", out_size - strlen(output) - 1);
            strncat(output, start, wlen);
        } else {
            // Abbreviate remaining words
            strncat(output, " ", out_size - strlen(output) - 1);
            char abbr[4];
            snprintf(abbr, sizeof(abbr), "%c.", *start);
            strncat(output, abbr, out_size - strlen(output) - 1);
        }
        word_count++;
    }

    if (strlen(output) == 0) {
        strncpy(output, "Unknown", out_size);
    }
}

int main() {
    char artist_raw[BUF_SIZE];
    char artist_prev[BUF_SIZE] = "";

    printf("{\"text\": \"Unknown\"}\n");
    fflush(stdout);
    strncpy(artist_prev, "Unknown", sizeof(artist_prev) - 1);
    artist_prev[sizeof(artist_prev) - 1] = '\0';

    // Follow playerctl artist metadata
    FILE *fp = popen("playerctl metadata artist --follow 2>/dev/null", "r");
    if (!fp) {
        perror("popen");
        return 1;
    }

    while (fgets(artist_raw, sizeof(artist_raw), fp)) {
        // Strip newline
        size_t len = strlen(artist_raw);
        if (len > 0 && artist_raw[len - 1] == '\n') {
            artist_raw[len - 1] = '\0';
        }

        char artist_abbr[BUF_SIZE];
        abbreviate_artist(artist_raw, artist_abbr, sizeof(artist_abbr));

        // Only print if changed
        if (strcmp(artist_abbr, artist_prev) != 0) {
            printf("{\"text\": \"%s\"}\n", artist_abbr);
            fflush(stdout);
            strncpy(artist_prev, artist_abbr, sizeof(artist_prev) - 1);
            artist_prev[sizeof(artist_prev) - 1] = '\0';
        }
    }

		// playerctl exited → reset artist
		printf("{\"text\": \"Unknown\"}\n");
		fflush(stdout);

    pclose(fp);
    return 0;
}

