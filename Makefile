CC=gcc
CFLAGS=-c -Wall -O3 -std=gnu11 -Wpedantic -Wextra
LDFLAGS=-lpcre -lm -lssl -lcrypto
SRCDIR=src
SOURCES=$(wildcard $(SRCDIR)/*.c) 
HEADERS=$(wildcard $(SRCDIR)/include/*.h)
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=dawn
PREFIX=/usr/bin

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
		$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

.c.o:
		$(CC) $(CFLAGS) $< -o $@

$(OBJECTS): $(HEADERS)

clean:
		rm -f $(SRCDIR)/*.o

install:
		cp $(EXECUTABLE) $(PREFIX)

uninstall:
		rm -vi $(PREFIX)/$(EXECUTABLE)
