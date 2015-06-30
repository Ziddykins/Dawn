#include "include/persistence.h"

void persistent_save(struct Bot * b) {
    save_players(b);
    save_events("events.db");
}

void persistent_load(struct Bot * b) {
    load_players(b);
    init_timers(b, "events.db");
}
