#include "include/persistence.h"

#define MAP_FILE "map.bin"
#define EVENTS_FILE "events.bin"

void persistent_save(struct Bot * b) {
    save_players(b);
    save_events(EVENTS_FILE);
    save_map(MAP_FILE);
}

void persistent_load(struct Bot * b) {
    load_players(b);
    init_timers(b, EVENTS_FILE);
    init_map(MAP_FILE);
}
