#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <wctype.h>  // Required for iswspace() to detect CJK spaces
#include <locale.h>

#define BUF_SIZE 1024

// Abbreviate artist name using wide characters
void abbreviate_artist(const wchar_t *input, wchar_t *output, size_t out_size) {
    output[0] = L'\0';
    int word_count = 0;
    const wchar_t *p = input;

    // Loop through the wide string safely
    while (*p && wcslen(output) < out_size - 1) {
        // Skip leading spaces (handles both ASCII and full-width CJK spaces)
        while (*p && iswspace(*p)) p++;
        if (!*p) break;

        // Find the end of the current word
        const wchar_t *start = p;
        while (*p && !iswspace(*p)) p++;
        size_t wlen = p - start;

        if (word_count < 2) {
            // Copy the first two words fully
            if (word_count > 0) wcsncat(output, L" ", out_size - wcslen(output) - 1);
            wcsncat(output, start, wlen);
        } else {
            // Abbreviate remaining words safely by grabbing the first wide character
            wcsncat(output, L" ", out_size - wcslen(output) - 1);
            wchar_t abbr[4];
            swprintf(abbr, sizeof(abbr) / sizeof(wchar_t), L"%lc.", *start);
            wcsncat(output, abbr, out_size - wcslen(output) - 1);
        }
        word_count++;
    }

    if (wcslen(output) == 0) {
        wcsncpy(output, L"Unknown", out_size);
    }
}

int main() {
    // Required to enable UTF-8 multibyte conversions
    setlocale(LC_ALL, "");

    char artist_raw[BUF_SIZE];
    char artist_prev[BUF_SIZE] = "";

    printf("{\"text\": \"Unknown\"}\n");
    fflush(stdout);
    strncpy(artist_prev, "Unknown", sizeof(artist_prev) - 1);
    artist_prev[sizeof(artist_prev) - 1] = '\0';

    // Follow playerctl artist metadata
    FILE *fp = popen("playerctl metadata artist --follow", "r");
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

        // 1️⃣ Convert raw UTF-8 input to a wide-character string
        wchar_t w_artist_raw[BUF_SIZE];
        mbstowcs(w_artist_raw, artist_raw, BUF_SIZE);

        wchar_t w_artist_abbr[BUF_SIZE];
        abbreviate_artist(w_artist_raw, w_artist_abbr, BUF_SIZE);

        // 2️⃣ Convert back to a standard multibyte char array for JSON printing
        char artist_abbr[BUF_SIZE];
        wcstombs(artist_abbr, w_artist_abbr, BUF_SIZE);

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
