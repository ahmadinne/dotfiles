CFLAGS+=
LDADD+=
LDFLAGS=
EXEC=scroll-title thumbnail thumbnail-opt wmname artist player-status

PREFIX?= /usr
BINDIR?= ${PREFIX}/bin

CC=gcc

all: $(EXEC)

scroll-title: scroll-title.c
	$(CC) -Os -o $@ $+

artist: artist.c
	$(CC) -Os -o $@ $+

thumbnail: thumbnail.c
	$(CC) -Os -o $@ $+

thumbnail-opt: thumbnail-opt.c
	$(CC) -Os -o $@ $+ 

wmname: wmname.c
	$(CC) -Os -o $@ $+ -lX11

player-status: player-status.c
	$(CC) -Os -o $@ $+ 

clean:
	rm -f $(EXEC)
