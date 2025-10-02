CFLAGS+=
LDADD+=
LDFLAGS=
EXEC=scroll-title thumbnail thumbnail-opt

PREFIX?= /usr
BINDIR?= ${PREFIX}/bin

CC=gcc

all: $(EXEC)

scroll-title: scroll-title.c
	$(CC) -Os -o $@ $+

thumbnail: thumbnail.c
	$(CC) -Os -o $@ $+

thumbnail-opt: thumbnail-opt.c
	$(CC) -Os -o $@ $+ 

clean:
	rm -f $(EXEC)
