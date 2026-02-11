CFLAGS+=
LDADD+=
LDFLAGS=
EXEC=calendar

PREFIX?= /usr
BINDIR?= ${PREFIX}/bin

CC=gcc

all: $(EXEC)

calendar: calendar.c
	$(CC) -Os -o $@ $+

clean:
	rm -f $(EXEC)
