#include <string.h>
#include <stdio.h>
#include "include/status.h"
#include "include/limits.h"
#include "include/network.h"
#include "include/player.h"
#include "include/stats.h"

void get_stat (struct Bot *b, struct Message *m, int stats[6]) {
    int i = get_pindex(b, m->sender_nick);
    for (int j=0; j<(MAX_INVENTORY_SLOTS - b->players[i].available_slots); j++) {
        if (b->players[i].inventory[j].equipped) {
            stats[0] += b->players[i].inventory[j].attr_health;
            stats[1] += b->players[i].inventory[j].attr_mana;
            stats[2] += b->players[i].inventory[j].attr_strength;
            stats[3] += b->players[i].inventory[j].attr_intelligence;
            stats[4] += b->players[i].inventory[j].attr_mdef;
            stats[5] += b->players[i].inventory[j].attr_defense;
        }
    }
}
