CC=gcc
CFLAGS=$(shell pkg-config --cflags gtk+-3.0 webkit2gtk-4.1)
LIBS=$(shell pkg-config --libs gtk+-3.0 webkit2gtk-4.1) -lsqlite3
TARGET=archbrowser

all: $(TARGET)

$(TARGET): src/main.c
	$(CC) src/main.c $(CFLAGS) $(LIBS) -o $(TARGET)

clean:
	rm -f $(TARGET)
