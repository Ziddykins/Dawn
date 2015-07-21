#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "include/status.h"
#include "include/colors.h"

static char *get_monstring (enum MonsterType type, char *temp) {
    switch (type) {
        case BERSERKER: snprintf(temp, 100, "%s%s%s\r\n", IRC_RED, "Berserker (2x Stats)", IRC_NORMAL); break;
        case ELDER:     snprintf(temp, 100, "%s%s%s\r\n", IRC_GREEN, "Elder (3x Experience)", IRC_NORMAL); break;
        case THIEF:     snprintf(temp, 100, "%s%s%s\r\n", IRC_ORANGE, "Thief (5x Gold)", IRC_NORMAL); break;
        case TANK:      snprintf(temp, 100, "%s%s%s\r\n", IRC_DBLUE, "Tank (4x Defense)", IRC_NORMAL); break;
        case BOSS:      snprintf(temp, 100, "%s%s%s\r\n", IRC_BROWN, "BOSS (10x ALL 6x Payout)", IRC_NORMAL); break;
        case DEFMON:    snprintf(temp, 100, "\r\n");
    }
    return temp;
}

static void monster_buff (struct Bot *b, enum MonsterType type, int pindex, int global) {
    switch (type) {
        case BERSERKER:
            global ? (b->global_monster.hp    *= 2) : (b->players[pindex].personal_monster.hp    *= 2);
            global ? (b->global_monster.str   *= 2) : (b->players[pindex].personal_monster.str   *= 2);
            global ? (b->global_monster.def   *= 2) : (b->players[pindex].personal_monster.def   *= 2);
            global ? (b->global_monster.intel *= 2) : (b->players[pindex].personal_monster.intel *= 2);
            global ? (b->global_monster.mdef  *= 2) : (b->players[pindex].personal_monster.mdef  *= 2);
            break;
        case ELDER:
            global ? (b->global_monster.exp *= 3) : (b->players[pindex].personal_monster.exp *= 3);
            break;
        case THIEF:
            global ? (b->global_monster.def *= 4) : (b->players[pindex].personal_monster.def *= 4);
            break;
        case TANK:
            global ? (b->global_monster.gold *= 5) : (b->players[pindex].personal_monster.gold *= 4);
            break;
        case BOSS:
            global ? (b->global_monster.hp    *= 10) : (b->players[pindex].personal_monster.hp    *= 10);
            global ? (b->global_monster.str   *= 10) : (b->players[pindex].personal_monster.str   *= 10);
            global ? (b->global_monster.def   *= 10) : (b->players[pindex].personal_monster.def   *= 10);
            global ? (b->global_monster.intel *= 10) : (b->players[pindex].personal_monster.intel *= 10);
            global ? (b->global_monster.mdef  *= 10) : (b->players[pindex].personal_monster.mdef  *= 10);
            global ? (b->global_monster.exp   *= 6)  : (b->players[pindex].personal_monster.exp   *= 6);
            global ? (b->global_monster.gold  *= 6)  : (b->players[pindex].personal_monster.gold  *= 6);
            break;
        case DEFMON:
            break;
    }
}

void call_monster (struct Bot *b, char const * username, int global) { //username -> MAX_NICK_LENGTH
    char out[MAX_MESSAGE_BUFFER];
    char monspecial[100];
    char pstring[64];
    int i, pindex, chance;
    i = pindex = 0;

    //There will be a 15% chance of encountering a special monster
    chance = rand() % 100;
    
    if (global) {
        if (b->global_monster.active) {
            sprintf(out, "PRIVMSG %s :The %s got bored and left town. Unfortunately his friend wandered in...\r\n",
                    b->active_room, b->global_monster.name);
            add_msg(out, strlen(out));
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

    //Buff the current monster and construct a string, depending on which random
    //special type of monster is picked and assign the buffed stats to the tmp monster
    //for display purposes
    if (chance < 15) {
        enum MonsterType type = rand() % MAX_SPECIAL_MONSTERS;
        global ? (b->global_monster.type = type) : (b->players[pindex].personal_monster.type = type);
        monster_buff(b, type, pindex, global);
        global ? tmp = b->global_monster : b->players[pindex].personal_monster;
    }

    //Construct the final output string and send to server
    sprintf(out, "PRIVMSG %s :Monster spawned in room%s [%s] [%d/%d %sHP%s] - [%d STR] - [%d DEF] - [%d INT] -"
                 " [%d MDEF] %s",
                 b->active_room, pstring, tmp.name, tmp.hp, tmp.mhp, IRC_RED, IRC_NORMAL, tmp.str, tmp.def,
                 tmp.intel, tmp.mdef, get_monstring(tmp.type, monspecial));
    add_msg(out, strlen(out));
}

void slay_monster (struct Bot *b, char const * username, int global, int amount) { //username -> MAX_NICK_LENGTH
    char out[MAX_MESSAGE_BUFFER];
    int pindex = get_pindex(b, username);

    if (global && b->global_monster.active) {
        if (amount < 1 || amount >= MAX_SLAY_GOLD) {
            sprintf(out, "PRIVMSG %s :Please enter an amount in between 1 - %d gold\r\n", b->active_room, MAX_SLAY_GOLD);
            add_msg(out, strlen(out));
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

        add_msg(out, strlen(out));

        if (b->global_monster.slay_cost <= 0) {
            sprintf(out, "PRIVMSG %s :The people have raised enough money to rid their town of the horrid monster "
                    "and hire a warrior to come dispose of the entity. In no time, a warrior rides into town "
                    "and slices the monster in half with one precise strike. Well that was easy...\r\n", b->active_room);
            b->global_monster.active = 0;
            b->global_monster.slay_cost = b->global_monster.gold;
            b->global_monster.hp = b->global_monster.mhp;
            b->global_monster.type = DEFMON;
            add_msg(out, strlen(out));
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
            b->players[pindex].personal_monster.type = DEFMON;
        } else {
            sprintf(out, "PRIVMSG %s :%s, you don't have enough gold to pay a true warrior to dispose "
                    "of this monster [cost: 150 gold]\r\n", b->active_room, username);
        }
        add_msg(out, strlen(out));
    }
}

void print_monster (struct Bot *b, char const * username, int global) { //username -> MAX_NICK_LENGTH
    char out[MAX_MESSAGE_BUFFER];
    int pindex = get_pindex(b, username);
    if (global) {
        if (!b->global_monster.active) {
            sprintf(out, "PRIVMSG %s :There is no monster in the room currently\r\n", b->active_room);
        } else {
            sprintf(out, "PRIVMSG %s :A %s is in the room: [Health: %d] [%d Str - %d Def - %d Int - %d MDef]",
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
    add_msg(out, strlen(out));
}
