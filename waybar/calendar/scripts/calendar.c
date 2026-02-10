#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ---------- date helpers ---------- */

int days_in_month(int y, int m) {
    static int d[] = {31,28,31,30,31,30,31,31,30,31,30,31};
    if (m == 1) {
        int leap = (y%4==0 && y%100!=0) || (y%400==0);
        return 28 + leap;
    }
    return d[m];
}

/* ---------- main ---------- */

int main(int argc, char **argv) {
    if (argc < 2) return 0;

    time_t now = time(NULL);
    struct tm t = *localtime(&now);

    int year  = t.tm_year + 1900;
    int month = t.tm_mon;
    int today = t.tm_mday;

    struct tm first = t;
    first.tm_mday = 1;
    mktime(&first);

    int first_wday = first.tm_wday; // 0=Su
    int dim        = days_in_month(year, month);
    int prev_dim   = days_in_month(year, (month+11)%12);

    /* ---------- build grid ---------- */

		int grid[6][7];
		int is_current[6][7];

		int d = 1 - first_wday;

		for (int r=0; r<6; r++) {
				for (int c=0; c<7; c++, d++) {
						if (d < 1) {
								grid[r][c] = prev_dim + d;
								is_current[r][c] = 0;
						}
						else if (d > dim) {
								grid[r][c] = d - dim;
								is_current[r][c] = 0;
						}
						else {
								grid[r][c] = d;
								is_current[r][c] = 1;
						}
				}
		}

    /* ---------- month title ---------- */

    if (!strcmp(argv[1], "month")) {
        static const char *m[] = {
            "January","February","March","April","May","June",
            "July","August","September","October","November","December"
        };
        printf("{\"text\":\"%s %d\"}\n", m[month], year);
        return 0;
    }

    /* ---------- weekday headers ---------- */

    static const char *wd[] = {"Su","Mo","Tu","We","Th","Fr","Sa"};
    static const char *wdkey[] = {
        "sunday","monday","tuesday","wednesday",
        "thursday","friday","saturday"
    };

    for (int i=0; i<7; i++) {
        if (!strcmp(argv[1], wdkey[i])) {
            printf("{\"text\":\"%s\"}\n", wd[i]);
            return 0;
        }
    }

    /* ---------- cell lookup (su1, mo3, fr5, etc.) ---------- */

    const char *cols = "sumotuwethfrsa";
    char colstr[3] = {argv[1][0], argv[1][1], 0};
    int row = argv[1][2] - '1';

    const char *p = strstr(cols, colstr);
    if (!p || row < 0 || row > 5) return 0;

    int col = (p - cols) / 2;
    int val = grid[row][col];

		const char *cls =
				(is_current[row][col] && val == today) ? "today" :
				(is_current[row][col]) ? "not-today" :
				"other-month";

    printf("{\"text\":\"%2d\",\"class\":\"%s\"}\n", val, cls);
    return 0;
}
