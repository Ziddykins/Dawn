#include <string.h>
#include <stdio.h>
#include "status.h"
#include "limits.h"
#include "network.h"
#include "player.h"

void get_stat (Bot *b, Message *m, int stats[6]) {
    int i = get_pindex(b, m->sender_nick);
    unsigned int j;
    for (j=0; j<(MAX_INVENTORY_SLOTS - b->players[i].available_slots); j++) {
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
