#include "include/market.h"

int rrange (int min, int max) {
    int rval = min + (rand() % (int)(max - min + 1));
    printf("%d (%d-%d)\n", rval, min, max);
    return rval;
}

void fluctuate_market (struct Bot *b) {
    char out[MAX_MESSAGE_BUFFER];
    b->market.materials[WOOD]    = rrange(100, 500);
    b->market.materials[LEATHER] = rrange(250, 800);
    b->market.materials[ORE]     = rrange(400, 1200);
    b->market.materials[STONE]   = rrange(500, 1400);
    b->market.materials[BRONZE]  = rrange(700, 2000);
    b->market.materials[MAIL]    = rrange(1000, 3000);
    b->market.materials[STEEL]   = rrange(4000, 12000);
    b->market.materials[DIAMOND] = rrange(25000, 100000);
    sprintf(out, "PRIVMSG %s :Market prices have been updated\r\n", b->active_room);
    addMsg(out, strlen(out));
}

void print_market (struct Bot *b) {
    char out[MAX_MESSAGE_BUFFER];
    sprintf(out, "PRIVMSG %s :[Wood: %d] [Leather: %d] [Ore: %d] [Stone: %d] [Bronze: %d]"
            " [Mail: %d] [Steel: %d] [Diamond: %d]\r\n", b->active_room, b->market.materials[WOOD],
            b->market.materials[LEATHER], b->market.materials[ORE], b->market.materials[STONE],
            b->market.materials[BRONZE], b->market.materials[MAIL], b->market.materials[STEEL],
            b->market.materials[DIAMOND]);
    addMsg(out, strlen(out));
}

int get_itemID (char *which) {
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
    

void market_buysell (struct Bot *b, struct Message *m, int buysell, char *which, int amount) {
    int pindex = get_pindex(b, m->sender_nick);
    int material = get_itemID(which);
    char out[MAX_MESSAGE_BUFFER];
 
    if (material == -1 || amount < 1 || amount >= INT_MAX) return;

    if (buysell == 0) {
        //sell
        if (b->players[pindex].materials[material] >= amount) {
            b->players[pindex].materials[material] -= amount;
            b->players[pindex].gold += b->market.materials[material] * amount;
            sprintf(out, "PRIVMSG %s :%s, you have sold %d %s for %d gold\r\n",
                    m->receiver, m->sender_nick, amount, which, b->market.materials[material] * amount);

        } else {
            sprintf(out, "PRIVMSG %s :%s, you do not have enough %s to do that\r\n",
                    m->receiver, m->sender_nick, which);
        }
    } else {
        //buy
        if (b->players[pindex].gold >= b->market.materials[material] * amount) {
            b->players[pindex].gold -= b->market.materials[material] * amount;
            b->players[pindex].materials[material] += amount;
            sprintf(out, "PRIVMSG %s :%s, you have purchased %d %s for %d gold\r\n",
                    m->receiver, m->sender_nick, amount, which, b->market.materials[material] * amount);
        } else {
            sprintf(out, "PRIVMSG %s :%s, you do not have enough gold to purchase that much %s\r\n",
                    m->receiver, m->sender_nick, which);
        }
    }
    addMsg(out, strlen(out));
}

void print_materials (struct Bot *b, struct Message *m) {
    int pindex = get_pindex(b, m->sender_nick);
    char out[MAX_MESSAGE_BUFFER];
    sprintf(out, "PRIVMSG %s :%s, [Materials: - Wood: %ld - Leather %ld - Ore: %ld - Stone: %ld - "
               " Bronze: %ld - Mail: %ld - Steel: %ld - Diamond: %ld]\r\n",
               m->receiver, m->sender_nick, b->players[pindex].materials[WOOD],
               b->players[pindex].materials[LEATHER], b->players[pindex].materials[ORE],
               b->players[pindex].materials[STONE], b->players[pindex].materials[BRONZE],
               b->players[pindex].materials[MAIL], b->players[pindex].materials[STEEL],
               b->players[pindex].materials[DIAMOND]);
    addMsg(out, strlen(out));
}
