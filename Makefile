CC=gcc
CFLAGS=`pkg-config --cflags gtk+-3.0` -Wall -g
LDFLAGS=`pkg-config --libs gtk+-3.0`

all: gtkzip

SRC=src/main.c src/sysmgr.c

gtkzip: $(SRC)
	$(CC) $(CFLAGS) -o gtkzip $(SRC) $(LDFLAGS)

clean:
	rm -f gtkzip
