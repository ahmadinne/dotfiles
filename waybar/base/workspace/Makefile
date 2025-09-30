CFLAGS+=
LDADD+=
LDFLAGS=
EXEC=workspaces

PREFIX?= /usr
BINDIR?= ${PREFIX}/bin

CC=gcc

all: $(EXEC)

# workspaces: workspaces.o
# 	$(CC) -Os -o $@ $+

install: all
	install -Dm 755 workspaces ${DESTDIR}${BINDIR}/workspaces

clean:
	rm -f workspaces *.o
