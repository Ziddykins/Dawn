CC=gcc
CFLAGS=-c -Wall -O3 -std=gnu11 -Wno-float-equal
LDFLAGS=-lpcre -lm -lssl -lcrypto
SRCDIR=src
SOURCES=$(wildcard $(SRCDIR)/*.c) 
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=dawn
PREFIX=/usr/bin

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
		$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

.c.o:
		$(CC) $(CFLAGS) $< -o $@

clean:
		rm -f $(SRCDIR)/*.o

install:
		cp $(EXECUTABLE) $(PREFIX)

remove:
		rm -vi $(PREFIX)/$(EXECUTABLE)
