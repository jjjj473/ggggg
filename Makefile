CC=gcc
CFLAGS=$(shell pkg-config --cflags gtk+-3.0 webkit2gtk-4.1 libxml-2.0 libarchive openssl)
LIBS=$(shell pkg-config --libs gtk+-3.0 webkit2gtk-4.1 libxml-2.0 libarchive openssl) -lsqlite3
TARGET=archbrowser

all: $(TARGET)

SRCS=src/main.c src/input.c

$(TARGET): $(SRCS)
	$(CC) $(SRCS) $(CFLAGS) $(LIBS) -o $(TARGET)

clean:
	rm -f $(TARGET)
