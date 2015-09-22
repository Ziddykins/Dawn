CC=gcc
CFLAGS=-c -Wall -std=gnu11 -Wpedantic -Wextra -Ofast -mtune=native -march=native -g
LDFLAGS=-lpcre -lm -lssl -lcrypto
SRCDIR=src
SOURCES=$(wildcard $(SRCDIR)/*.c) 
HEADERS=$(wildcard $(SRCDIR)/include/*.h)
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=dawn
PREFIX=/usr/local/bin

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
