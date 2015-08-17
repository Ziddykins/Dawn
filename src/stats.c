#include <string.h>
#include <stdio.h>
#include "include/status.h"
#include "include/limits.h"
#include "include/network.h"
#include "include/player.h"
#include "include/stats.h"

void get_stat(struct Message *m, int stats[6]) {
    int i = get_pindex(m->sender_nick);
    for (int j = 0; j < (MAX_INVENTORY_SLOTS - dawn->players[i].available_slots); j++) {
        if (dawn->players[i].inventory[j].equipped) {
            stats[0] += dawn->players[i].inventory[j].attr_health;
            stats[1] += dawn->players[i].inventory[j].attr_mana;
            stats[2] += dawn->players[i].inventory[j].attr_strength;
            stats[3] += dawn->players[i].inventory[j].attr_intelligence;
            stats[4] += dawn->players[i].inventory[j].attr_mdef;
            stats[5] += dawn->players[i].inventory[j].attr_defense;
        }
    }
}
