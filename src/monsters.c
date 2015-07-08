#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "include/status.h"
#include "include/colors.h"

void call_monster (struct Bot *b, char const * username, int global) { //username -> MAX_NICK_LENGTH
    char out[MAX_MESSAGE_BUFFER];
    char pstring[64];
    int i, pindex;
    i = pindex = 0;

    if (global) {
        if (b->global_monster.active) {
            sprintf(out, "PRIVMSG %s :The %s got bored and left town. Unfortunately his friend wandered in...\r\n",
                    b->active_room, b->global_monster.name);
            addMsg(out, strlen(out));
            b->global_monster.slay_cost = b->global_monster.gold;
            b->global_monster.hp = b->global_monster.mhp;
        }

        b->global_monster = b->monsters[rand()%MAX_MONSTERS];
        b->global_monster.active = 1;

        for (i=0; i<b->player_count; i++) {
            b->players[i].contribution = 0;
        }

        strcpy(pstring, ":");
    } else {
        pindex = get_pindex(b, username);
        if (b->players[pindex].personal_monster.active) return;
        b->players[pindex].personal_monster = b->monsters[rand()%MAX_MONSTERS];
        b->players[pindex].personal_monster.active = 1;
        sprintf(pstring, " and attacks %s:", username);
    }

    struct Monsters tmp = global ? b->global_monster : b->players[pindex].personal_monster;

    sprintf(out, "PRIVMSG %s :Monster spawned in room%s [%s] [%d/%d %sHP%s] - [%d STR] - [%d DEF] - [%d INT] -"
                 " [%d MDEF]\r\n",
                 b->active_room, pstring, tmp.name, tmp.hp, tmp.mhp, IRC_RED, IRC_NORMAL, tmp.str, tmp.def,
                 tmp.intel, tmp.mdef);
    addMsg(out, strlen(out));
}

void slay_monster (struct Bot *b, char const * username, int global, int amount) { //username -> MAX_NICK_LENGTH
    char out[MAX_MESSAGE_BUFFER];
    int pindex = get_pindex(b, username);

    if (global && b->global_monster.active) {
        if (amount < 1 || amount >= MAX_SLAY_GOLD) {
            sprintf(out, "PRIVMSG %s :Please enter an amount in between 1 - 10,000,000 gold\r\n", b->active_room);
            addMsg(out, strlen(out));
            return;
        }

        if (b->players[pindex].gold >= amount) {
            b->players[pindex].gold -= amount;
            b->global_monster.slay_cost -= amount;
            sprintf(out, "PRIVMSG %s :%s has contributed %d towards the cost of hiring a warrior to slay "
                    "the monster. %d gold still has to be raised!\r\n", b->active_room, username, amount,
                    b->global_monster.slay_cost);
        } else {
            sprintf(out, "PRIVMSG %s :%s, you do not have that much gold\r\n", b->active_room, username);
        }

        addMsg(out, strlen(out));

        if (b->global_monster.slay_cost <= 0) {
            sprintf(out, "PRIVMSG %s :The people have raised enough money to rid their town of the horrid monster "
                    "and hire a warrior to come dispose of the entity. In no time, a warrior rides into town "
                    "and slices the monster in half with one precise strike. Well that was easy...\r\n", b->active_room);
            b->global_monster.active = 0;
            b->global_monster.slay_cost = b->global_monster.gold;
            b->global_monster.hp = b->global_monster.mhp;
            addMsg(out, strlen(out));
        }
    } else {
        pindex = get_pindex(b, username);
        if (!b->players[pindex].personal_monster.active) return;
        if (b->players[pindex].gold >= 150) {
            b->players[pindex].gold -= 150;
            sprintf(out, "PRIVMSG %s :%s has paid a true warrior to dispose of the monster which "
                    "they couldn't handle. [Gold -150]\r\n", b->active_room, username);
            b->players[pindex].personal_monster.hp = b->players[pindex].personal_monster.mhp;
            b->players[pindex].personal_monster.active = 0;
        } else {
            sprintf(out, "PRIVMSG %s :%s, you don't have enough gold to pay a true warrior to dispose "
                    "of this monster [cost: 150 gold]\r\n", b->active_room, username);
        }
        addMsg(out, strlen(out));
    }
}

void print_monster (struct Bot *b, char const * username, int global) { //username -> MAX_NICK_LENGTH
    char out[MAX_MESSAGE_BUFFER];
    int pindex = get_pindex(b, username);
    if (global) {
        if (!b->global_monster.active) {
            sprintf(out, "PRIVMSG %s :There is no monster in the room currently\r\n", b->active_room);
        } else {
            sprintf(out, "PRIVMSG %s :A %s is in the room: [Health: %d][%dS - %dD - %dI - %dMD]\r\n",
                    b->active_room, b->global_monster.name, b->global_monster.hp,
                    b->global_monster.str, b->global_monster.def, b->global_monster.intel,
                    b->global_monster.mdef);
        }
    } else {
        if (!b->players[pindex].personal_monster.active) {
            sprintf(out, "PRIVMSG %s :%s, you are not fighting any monster currently\r\n", b->active_room, username);
        } else {
            sprintf(out, "PRIVMSG %s :%s, you are fighting a %s: [Health: %d][%dS - %dD - %dI - %dMD]\r\n",
                    b->active_room, username, b->players[pindex].personal_monster.name,
                    b->players[pindex].personal_monster.hp,
                    b->players[pindex].personal_monster.str,
                    b->players[pindex].personal_monster.def,
                    b->players[pindex].personal_monster.intel,
                    b->players[pindex].personal_monster.mdef);
        }
    }
    addMsg(out, strlen(out));
}

