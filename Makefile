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

clean:
	rm -f $(SRCDIR)/*.o

install:
	cp $(EXECUTABLE) $(PREFIX)

uninstall:
	rm -vi $(PREFIX)/$(EXECUTABLE)
	
src/analyzer.o: src/analyzer.c src/include/analyzer.h \
 src/include/tokenlist.h src/include/tokenlist.h src/include/util.h \
 src/include/colors.h src/include/colors.h
src/bounty.o: src/bounty.c src/include/status.h src/include/limits.h \
 src/include/player.h src/include/inventory.h src/include/monsters.h \
 src/include/map.h src/include/network.h src/include/spells.h \
 src/include/bounty.h src/include/bounty.h src/include/monsters.h \
 src/include/limits.h
src/cmdsys.o: src/cmdsys.c src/include/cmdsys.h src/include/network.h \
 src/include/limits.h src/include/player.h src/include/inventory.h \
 src/include/monsters.h src/include/map.h src/include/spells.h \
 src/include/util.h src/include/colors.h src/include/status.h \
 src/include/player.h src/include/bounty.h
src/combat.o: src/combat.c src/include/status.h src/include/limits.h \
 src/include/player.h src/include/inventory.h src/include/monsters.h \
 src/include/map.h src/include/network.h src/include/spells.h \
 src/include/bounty.h src/include/network.h src/include/stats.h \
 src/include/colors.h src/include/player.h src/include/items.h \
 src/include/combat.h src/include/bounty.h
src/commands.o: src/commands.c src/include/commands.h \
 src/include/network.h src/include/limits.h src/include/parse.h \
 src/include/cmdsys.h src/include/util.h src/include/colors.h \
 src/include/spells.h src/include/player.h src/include/inventory.h \
 src/include/monsters.h src/include/map.h src/include/spells.h \
 src/include/status.h src/include/player.h src/include/bounty.h \
 src/include/combat.h src/include/items.h src/include/market.h \
 src/include/status.h src/include/persistence.h
src/events.o: src/events.c src/include/status.h src/include/limits.h \
 src/include/player.h src/include/inventory.h src/include/monsters.h \
 src/include/map.h src/include/network.h src/include/spells.h \
 src/include/bounty.h src/include/limits.h src/include/player.h \
 src/include/network.h src/include/colors.h src/include/combat.h \
 src/include/events.h src/include/monsters.h src/include/market.h \
 src/include/status.h src/include/util.h src/include/colors.h
src/inventory.o: src/inventory.c src/include/status.h \
 src/include/limits.h src/include/player.h src/include/inventory.h \
 src/include/monsters.h src/include/map.h src/include/network.h \
 src/include/spells.h src/include/bounty.h src/include/player.h \
 src/include/network.h src/include/parse.h src/include/colors.h \
 src/include/limits.h
src/items.o: src/items.c src/include/status.h src/include/limits.h \
 src/include/player.h src/include/inventory.h src/include/monsters.h \
 src/include/map.h src/include/network.h src/include/spells.h \
 src/include/bounty.h src/include/network.h src/include/player.h \
 src/include/limits.h src/include/colors.h src/include/parse.h \
 src/include/inventory.h src/include/items.h src/include/util.h \
 src/include/colors.h
src/main.o: src/main.c src/include/status.h src/include/limits.h \
 src/include/player.h src/include/inventory.h src/include/monsters.h \
 src/include/map.h src/include/network.h src/include/spells.h \
 src/include/bounty.h src/include/commands.h src/include/util.h \
 src/include/colors.h src/include/persistence.h src/include/status.h \
 src/include/parse.h src/include/market.h src/include/cmdsys.h
src/map.o: src/map.c src/include/map.h src/include/network.h \
 src/include/limits.h src/include/util.h src/include/colors.h \
 src/include/status.h src/include/player.h src/include/inventory.h \
 src/include/monsters.h src/include/map.h src/include/spells.h \
 src/include/bounty.h
src/market.o: src/market.c src/include/market.h src/include/status.h \
 src/include/limits.h src/include/player.h src/include/inventory.h \
 src/include/monsters.h src/include/map.h src/include/network.h \
 src/include/spells.h src/include/bounty.h src/include/util.h \
 src/include/colors.h
src/monsters.o: src/monsters.c src/include/status.h src/include/limits.h \
 src/include/player.h src/include/inventory.h src/include/monsters.h \
 src/include/map.h src/include/network.h src/include/spells.h \
 src/include/bounty.h src/include/colors.h
src/namegen.o: src/namegen.c src/include/namegen.h src/include/analyzer.h \
 src/include/tokenlist.h src/include/analyzer.h src/include/colors.h \
 src/include/util.h src/include/colors.h
src/network.o: src/network.c src/include/network.h src/include/limits.h \
 src/include/status.h src/include/player.h src/include/inventory.h \
 src/include/monsters.h src/include/map.h src/include/network.h \
 src/include/spells.h src/include/bounty.h src/include/util.h \
 src/include/colors.h
src/parse.o: src/parse.c src/include/parse.h src/include/network.h \
 src/include/limits.h src/include/network.h src/include/status.h \
 src/include/player.h src/include/inventory.h src/include/monsters.h \
 src/include/map.h src/include/spells.h src/include/bounty.h \
 src/include/player.h src/include/inventory.h src/include/combat.h \
 src/include/items.h src/include/market.h src/include/status.h \
 src/include/util.h src/include/colors.h
src/persistence.o: src/persistence.c src/include/persistence.h \
 src/include/status.h src/include/limits.h src/include/player.h \
 src/include/inventory.h src/include/monsters.h src/include/map.h \
 src/include/network.h src/include/spells.h src/include/bounty.h
src/player.o: src/player.c src/include/player.h src/include/limits.h \
 src/include/inventory.h src/include/monsters.h src/include/map.h \
 src/include/network.h src/include/spells.h src/include/status.h \
 src/include/player.h src/include/bounty.h src/include/network.h \
 src/include/map.h src/include/stats.h src/include/colors.h \
 src/include/inventory.h src/include/parse.h src/include/spells.h \
 src/include/util.h src/include/colors.h
src/spells.o: src/spells.c src/include/status.h src/include/limits.h \
 src/include/player.h src/include/inventory.h src/include/monsters.h \
 src/include/map.h src/include/network.h src/include/spells.h \
 src/include/bounty.h src/include/spells.h src/include/limits.h \
 src/include/combat.h
src/stats.o: src/stats.c src/include/status.h src/include/limits.h \
 src/include/player.h src/include/inventory.h src/include/monsters.h \
 src/include/map.h src/include/network.h src/include/spells.h \
 src/include/bounty.h src/include/limits.h src/include/network.h \
 src/include/player.h src/include/stats.h src/include/inventory.h
src/status.o: src/status.c src/include/status.h src/include/limits.h \
 src/include/player.h src/include/inventory.h src/include/monsters.h \
 src/include/map.h src/include/network.h src/include/spells.h \
 src/include/bounty.h src/include/events.h src/include/persistence.h \
 src/include/status.h src/include/util.h src/include/colors.h
src/tokenlist.o: src/tokenlist.c src/include/tokenlist.h \
 src/include/colors.h src/include/util.h src/include/colors.h
src/util.o: src/util.c src/include/colors.h src/include/util.h \
 src/include/colors.h
