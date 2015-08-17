#include <string.h>
#include "include/market.h"
#include "include/util.h"

static int rrange (int min, int max) {
    int rval = (int)(randd() * (max-min) + min);
    //printf("%d (%d-%d)\n", rval, min, max);
    return rval;
}

void fluctuate_market() {
    char out[MAX_MESSAGE_BUFFER];
    memcpy(dawn->market.prevprice, dawn->market.materials, sizeof(dawn->market.materials));
    dawn->market.materials[WOOD] = rrange(100, 500);
    dawn->market.materials[LEATHER] = rrange(250, 800);
    dawn->market.materials[ORE] = rrange(400, 1200);
    dawn->market.materials[STONE] = rrange(500, 1400);
    dawn->market.materials[BRONZE] = rrange(700, 2000);
    dawn->market.materials[MAIL] = rrange(1000, 3000);
    dawn->market.materials[STEEL] = rrange(4000, 12000);
    dawn->market.materials[DIAMOND] = rrange(25000, 100000);
    sprintf(out, "PRIVMSG %s :Market prices have been updated\r\n", dawn->active_room);
    add_msg(out, strlen(out));
}
static char *material_to_str(int index) {
    switch (index) {
        case WOOD:    return "Wood";
        case LEATHER: return "Leather";
        case ORE:     return "Ore";
        case STONE:   return "Stone";
        case BRONZE:  return "Bronze";
        case MAIL:    return "Mail";
        case STEEL:   return "Steel";
        case DIAMOND: return "Diamond";
        default: return "Error";
    }
}

void print_market() {
    char out[MAX_MESSAGE_BUFFER];
    sprintf(out, "PRIVMSG %s :", dawn->active_room);
    for (int i=0; i<MAX_MATERIAL_TYPE; i++) {
        char *temp;
        size_t len = strlen(out);
        CALLEXIT(!(temp = malloc(MAX_MESSAGE_BUFFER-len)))
        int difference = dawn->market.materials[i] - dawn->market.prevprice[i];
        snprintf(temp, MAX_MESSAGE_BUFFER-len, "[%s: %d (%s%+d" IRC_NORMAL ")] ",
                 material_to_str(i),
                 dawn->market.materials[i],
                 ((difference >= 0) ? IRC_GREEN : IRC_RED),
                 difference);
        strncat(out, temp, MAX_MESSAGE_BUFFER-len);
        free(temp);
    }
    strncat(out, "\r\n", 3);
    add_msg(out, strlen(out));
}

static int get_itemID (char *which) {
    if (strcmp(which, "wood") == 0)
        return WOOD;
    else if (strcmp(which, "leather") == 0)
        return LEATHER;
    else if (strcmp(which, "ore") == 0)
        return ORE;
    else if (strcmp(which, "stone") == 0)
        return STONE;
    else if (strcmp(which, "bronze") == 0)
        return BRONZE;
    else if (strcmp(which, "mail") == 0)
        return MAIL;
    else if (strcmp(which, "steel") == 0)
        return STEEL;
    else if (strcmp(which, "diamond") == 0)
        return DIAMOND;
    else
        return -1;
}


void market_buysell(struct Message *m, int buysell, char *which, long amount) {
    int pindex = get_pindex(m->sender_nick);
    int material = get_itemID(which);
    char out[MAX_MESSAGE_BUFFER];

    if (material == -1 || amount < 1 || amount >= LONG_MAX) return;

    if (buysell == 0) {
        //sell
        if (dawn->players[pindex].materials[material] >= amount) {
            if (dawn->players[pindex].gold + dawn->market.materials[material] * amount >= LONG_MAX) {
                sprintf(out, "PRIVMSG %s :%s, you can't carry that much gold\r\n", m->receiver, m->sender_nick);
                add_msg(out, strlen(out));
                return;
            }
            dawn->players[pindex].materials[material] -= amount;
            dawn->players[pindex].gold += dawn->market.materials[material] * amount;
            sprintf(out, "PRIVMSG %s :%s, you have sold %ld %s for %ld gold\r\n",
                    m->receiver, m->sender_nick, amount, which, dawn->market.materials[material] * amount);

        } else {
            sprintf(out, "PRIVMSG %s :%s, you do not have enough %s to do that\r\n",
                    m->receiver, m->sender_nick, which);
        }
    } else {
        //buy
        if (dawn->players[pindex].materials[material] + amount >= LONG_MAX) {
            sprintf(out, "PRIVMSG %s :%s, you can't carry anymore %s\r\n", m->receiver, m->sender_nick, which);
            add_msg(out, strlen(out));
            return;
        }
        if (dawn->players[pindex].gold >= dawn->market.materials[material] * amount) {
            dawn->players[pindex].gold -= dawn->market.materials[material] * amount;
            dawn->players[pindex].materials[material] += amount;
            sprintf(out, "PRIVMSG %s :%s, you have purchased %ld %s for %ld gold\r\n",
                    m->receiver, m->sender_nick, amount, which, dawn->market.materials[material] * amount);
        } else {
            sprintf(out, "PRIVMSG %s :%s, you do not have enough gold to purchase that much %s\r\n",
                    m->receiver, m->sender_nick, which);
        }
    }
    add_msg(out, strlen(out));
}

void print_materials(struct Message *m) {
    int pindex = get_pindex(m->sender_nick);
    char out[MAX_MESSAGE_BUFFER];
    sprintf(out, "PRIVMSG %s :%s, [Materials: - Wood: %ld - Leather %ld - Ore: %ld - Stone: %ld - "
               " Bronze: %ld - Mail: %ld - Steel: %ld - Diamond: %ld]\r\n",
            m->receiver, m->sender_nick, dawn->players[pindex].materials[WOOD],
            dawn->players[pindex].materials[LEATHER], dawn->players[pindex].materials[ORE],
            dawn->players[pindex].materials[STONE], dawn->players[pindex].materials[BRONZE],
            dawn->players[pindex].materials[MAIL], dawn->players[pindex].materials[STEEL],
            dawn->players[pindex].materials[DIAMOND]);
    add_msg(out, strlen(out));
}
