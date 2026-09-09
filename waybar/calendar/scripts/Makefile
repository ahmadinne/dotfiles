CFLAGS+=
LDADD+=
LDFLAGS=
EXEC=calendar

PREFIX?= /usr
BINDIR?= ${PREFIX}/bin

CC=gcc

all: $(EXEC)

calendar: calendar.c
	$(CC) -O2 -o $@ $+

clean:
	rm -f $(EXEC)
