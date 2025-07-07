CC=gcc
CFLAGS=`pkg-config --cflags gtk+-3.0` -Wall -g
LDFLAGS=`pkg-config --libs gtk+-3.0`

all: gtkzip

gtkzip: src/main.c
	$(CC) $(CFLAGS) -o gtkzip src/main.c $(LDFLAGS)

clean:
	rm -f gtkzip
