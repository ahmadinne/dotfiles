CFLAGS = -O2 -march=native -Wall

all: cpu ram temp disk bat

cpu: cpu.c
	$(CC) $(CFLAGS) cpu.c -o cpu

ram: ram.c
	$(CC) $(CFLAGS) ram.c -o ram

temp: temp.c
	$(CC) $(CFLAGS) temp.c -o temp

disk: disk.c
	$(CC) $(CFLAGS) disk.c -o disk

bat: bat.c
	$(CC) $(CFLAGS) bat.c -o bat

clean:
	rm -f cpu ram temp disk

