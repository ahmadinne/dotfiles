CFLAGS+=
LDADD+=
LDFLAGS=
EXEC=escape

PREFIX?= /usr
BINDIR?= ${PREFIX}/bin

CC=gcc

all: $(EXEC)

escape: escape.c
	$(CC) -O2 -o $@ $+ 

clean:
	rm -f $(EXEC)
