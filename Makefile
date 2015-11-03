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

clean:
	rm -f $(SRCDIR)/*.o

install:
	cp $(EXECUTABLE) $(PREFIX)

uninstall:
	rm -vi $(PREFIX)/$(EXECUTABLE)
	

src/analyzer.c.o: src/analyzer.c src/include/analyzer.h \
 src/include/tokenlist.h src/include/tokenlist.h src/include/util.h \
 src/include/colors.h src/include/colors.h
src/bounty.c.o: src/bounty.c src/include/status.h src/include/limits.h \
 src/include/player.h src/include/inventory.h src/include/monsters.h \
 src/include/map.h src/include/network.h src/include/spells.h \
 src/include/bounty.h src/include/bounty.h src/include/monsters.h \
 src/include/limits.h
src/cmdsys.c.o: src/cmdsys.c src/include/cmdsys.h src/include/network.h \
 src/include/limits.h src/include/player.h src/include/inventory.h \
 src/include/monsters.h src/include/map.h src/include/spells.h \
 src/include/util.h src/include/colors.h src/include/status.h \
 src/include/player.h src/include/bounty.h
src/combat.c.o: src/combat.c src/include/status.h src/include/limits.h \
 src/include/player.h src/include/inventory.h src/include/monsters.h \
 src/include/map.h src/include/network.h src/include/spells.h \
 src/include/bounty.h src/include/network.h src/include/stats.h \
 src/include/colors.h src/include/player.h src/include/items.h \
 src/include/combat.h src/include/bounty.h
src/commands.c.o: src/commands.c src/include/commands.h \
 src/include/network.h src/include/limits.h src/include/parse.h \
 src/include/cmdsys.h src/include/util.h src/include/colors.h \
 src/include/spells.h src/include/player.h src/include/inventory.h \
 src/include/monsters.h src/include/map.h src/include/spells.h \
 src/include/status.h src/include/player.h src/include/bounty.h \
 src/include/combat.h src/include/items.h src/include/market.h \
 src/include/status.h src/include/persistence.h
src/events.c.o: src/events.c src/include/status.h src/include/limits.h \
 src/include/player.h src/include/inventory.h src/include/monsters.h \
 src/include/map.h src/include/network.h src/include/spells.h \
 src/include/bounty.h src/include/limits.h src/include/player.h \
 src/include/network.h src/include/colors.h src/include/combat.h \
 src/include/events.h src/include/monsters.h src/include/market.h \
 src/include/status.h src/include/util.h src/include/colors.h
src/inventory.c.o: src/inventory.c src/include/status.h \
 src/include/limits.h src/include/player.h src/include/inventory.h \
 src/include/monsters.h src/include/map.h src/include/network.h \
 src/include/spells.h src/include/bounty.h src/include/player.h \
 src/include/network.h src/include/parse.h src/include/colors.h \
 src/include/limits.h
src/items.c.o: src/items.c src/include/status.h src/include/limits.h \
 src/include/player.h src/include/inventory.h src/include/monsters.h \
 src/include/map.h src/include/network.h src/include/spells.h \
 src/include/bounty.h src/include/network.h src/include/player.h \
 src/include/limits.h src/include/colors.h src/include/parse.h \
 src/include/inventory.h src/include/items.h src/include/util.h \
 src/include/colors.h
src/main.c.o: src/main.c src/include/status.h src/include/limits.h \
 src/include/player.h src/include/inventory.h src/include/monsters.h \
 src/include/map.h src/include/network.h src/include/spells.h \
 src/include/bounty.h src/include/commands.h src/include/util.h \
 src/include/colors.h src/include/persistence.h src/include/status.h \
 src/include/parse.h src/include/market.h src/include/cmdsys.h
src/map.c.o: src/map.c src/include/map.h src/include/network.h \
 src/include/limits.h src/include/util.h src/include/colors.h \
 src/include/status.h src/include/player.h src/include/inventory.h \
 src/include/monsters.h src/include/map.h src/include/spells.h \
 src/include/bounty.h
src/market.c.o: src/market.c src/include/market.h src/include/status.h \
 src/include/limits.h src/include/player.h src/include/inventory.h \
 src/include/monsters.h src/include/map.h src/include/network.h \
 src/include/spells.h src/include/bounty.h src/include/util.h \
 src/include/colors.h
src/monsters.c.o: src/monsters.c src/include/status.h \
 src/include/limits.h src/include/player.h src/include/inventory.h \
 src/include/monsters.h src/include/map.h src/include/network.h \
 src/include/spells.h src/include/bounty.h src/include/colors.h
src/namegen.c.o: src/namegen.c src/include/namegen.h \
 src/include/analyzer.h src/include/tokenlist.h src/include/analyzer.h \
 src/include/colors.h src/include/util.h src/include/colors.h
src/network.c.o: src/network.c src/include/network.h src/include/limits.h \
 src/include/status.h src/include/player.h src/include/inventory.h \
 src/include/monsters.h src/include/map.h src/include/network.h \
 src/include/spells.h src/include/bounty.h src/include/util.h \
 src/include/colors.h
src/parse.c.o: src/parse.c src/include/parse.h src/include/network.h \
 src/include/limits.h src/include/network.h src/include/status.h \
 src/include/player.h src/include/inventory.h src/include/monsters.h \
 src/include/map.h src/include/spells.h src/include/bounty.h \
 src/include/player.h src/include/inventory.h src/include/combat.h \
 src/include/items.h src/include/market.h src/include/status.h \
 src/include/util.h src/include/colors.h
src/persistence.c.o: src/persistence.c src/include/persistence.h \
 src/include/status.h src/include/limits.h src/include/player.h \
 src/include/inventory.h src/include/monsters.h src/include/map.h \
 src/include/network.h src/include/spells.h src/include/bounty.h
src/player.c.o: src/player.c src/include/player.h src/include/limits.h \
 src/include/inventory.h src/include/monsters.h src/include/map.h \
 src/include/network.h src/include/spells.h src/include/status.h \
 src/include/player.h src/include/bounty.h src/include/network.h \
 src/include/map.h src/include/stats.h src/include/colors.h \
 src/include/inventory.h src/include/parse.h src/include/spells.h \
 src/include/util.h src/include/colors.h
src/spells.c.o: src/spells.c src/include/status.h src/include/limits.h \
 src/include/player.h src/include/inventory.h src/include/monsters.h \
 src/include/map.h src/include/network.h src/include/spells.h \
 src/include/bounty.h src/include/spells.h src/include/limits.h \
 src/include/combat.h
src/stats.c.o: src/stats.c src/include/status.h src/include/limits.h \
 src/include/player.h src/include/inventory.h src/include/monsters.h \
 src/include/map.h src/include/network.h src/include/spells.h \
 src/include/bounty.h src/include/limits.h src/include/network.h \
 src/include/player.h src/include/stats.h src/include/inventory.h
src/status.c.o: src/status.c src/include/status.h src/include/limits.h \
 src/include/player.h src/include/inventory.h src/include/monsters.h \
 src/include/map.h src/include/network.h src/include/spells.h \
 src/include/bounty.h src/include/events.h src/include/persistence.h \
 src/include/status.h src/include/util.h src/include/colors.h
src/tokenlist.c.o: src/tokenlist.c src/include/tokenlist.h \
 src/include/colors.h src/include/util.h src/include/colors.h
src/util.c.o: src/util.c src/include/colors.h src/include/util.h \
 src/include/colors.h
