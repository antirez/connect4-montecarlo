all: c4

c4: c4.c
	$(CC) -O2 -Wall -W -std=c99 c4.c -o c4

clean:
	rm -f c4
