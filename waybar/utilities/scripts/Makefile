CFLAGS+=
LDADD+=
LDFLAGS=
EXEC=scroll-title title thumbnail wmname artist player-status

PREFIX?= /usr
BINDIR?= ${PREFIX}/bin

CC=gcc

all: $(EXEC)

scroll-title: scroll-title.c
	$(CC) -O2 -o $@ $+

title: title.c
	$(CC) -O2 -o $@ $+

artist: artist.c
	$(CC) -O2 -o $@ $+

thumbnail: thumbnail.c
	$(CC) -O2 -o $@ $+

wmname: wmname.c
	$(CC) -O2 -o $@ $+ -lX11

player-status: player-status.c
	$(CC) -O2 -o $@ $+ 

clean:
	rm -f $(EXEC)
