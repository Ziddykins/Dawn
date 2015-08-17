#include "include/persistence.h"

#include "include/player.h"

#define MAP_FILE "map.bin"
#define EVENTS_FILE "events.bin"

void persistent_save() {
    save_players(dawn);
    save_map(MAP_FILE);

    save_events(EVENTS_FILE);
}

void persistent_load() {
    load_players(dawn);
    init_map(MAP_FILE);

    init_timers(dawn, EVENTS_FILE);
}
