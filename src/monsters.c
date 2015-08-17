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

static void monster_buff(enum MonsterType type, int pindex, int global) {
    switch (type) {
        case BERSERKER:
            global ? (dawn->global_monster.hp *= 2) : (dawn->players[pindex].personal_monster.hp *= 2);
            global ? (dawn->global_monster.str *= 2) : (dawn->players[pindex].personal_monster.str *= 2);
            global ? (dawn->global_monster.def *= 2) : (dawn->players[pindex].personal_monster.def *= 2);
            global ? (dawn->global_monster.intel *= 2) : (dawn->players[pindex].personal_monster.intel *= 2);
            global ? (dawn->global_monster.mdef *= 2) : (dawn->players[pindex].personal_monster.mdef *= 2);
            break;
        case ELDER:
            global ? (dawn->global_monster.exp *= 3) : (dawn->players[pindex].personal_monster.exp *= 3);
            break;
        case THIEF:
            global ? (dawn->global_monster.def *= 4) : (dawn->players[pindex].personal_monster.def *= 4);
            break;
        case TANK:
            global ? (dawn->global_monster.gold *= 5) : (dawn->players[pindex].personal_monster.gold *= 4);
            break;
        case BOSS:
            global ? (dawn->global_monster.hp *= 10) : (dawn->players[pindex].personal_monster.hp *= 10);
            global ? (dawn->global_monster.str *= 10) : (dawn->players[pindex].personal_monster.str *= 10);
            global ? (dawn->global_monster.def *= 10) : (dawn->players[pindex].personal_monster.def *= 10);
            global ? (dawn->global_monster.intel *= 10) : (dawn->players[pindex].personal_monster.intel *= 10);
            global ? (dawn->global_monster.mdef *= 10) : (dawn->players[pindex].personal_monster.mdef *= 10);
            global ? (dawn->global_monster.exp *= 6) : (dawn->players[pindex].personal_monster.exp *= 6);
            global ? (dawn->global_monster.gold *= 6) : (dawn->players[pindex].personal_monster.gold *= 6);
            break;
        case DEFMON:
            break;
    }
}

void call_monster(char const *username, int global) { //username -> MAX_NICK_LENGTH
    char out[MAX_MESSAGE_BUFFER];
    char monspecial[100];
    char pstring[64];
    int i, pindex, chance;
    i = pindex = 0;

    //There will be a 15% chance of encountering a special monster
    chance = rand() % 100;
    
    if (global) {
        if (dawn->global_monster.active) {
            sprintf(out, "PRIVMSG %s :The %s got bored and left town. Unfortunately his friend wandered in...\r\n",
                    dawn->active_room, dawn->global_monster.name);
            add_msg(out, strlen(out));
            dawn->global_monster.slay_cost = dawn->global_monster.gold;
            dawn->global_monster.hp = dawn->global_monster.mhp;
        }

        dawn->global_monster = dawn->monsters[rand() % MAX_MONSTERS];
        dawn->global_monster.active = 1;

        for (i = 0; i < dawn->player_count; i++) {
            dawn->players[i].contribution = 0;
        }

        strcpy(pstring, ":");
    } else {
        pindex = get_pindex(username);
        if (dawn->players[pindex].personal_monster.active) return;
        dawn->players[pindex].personal_monster = dawn->monsters[rand() % MAX_MONSTERS];
        dawn->players[pindex].personal_monster.active = 1;
        sprintf(pstring, " and attacks %s:", username);
    }

    struct Monsters tmp = global ? dawn->global_monster : dawn->players[pindex].personal_monster;

    //Buff the current monster and construct a string, depending on which random
    //special type of monster is picked and assign the buffed stats to the tmp monster
    //for display purposes
    if (chance < 15) {
        enum MonsterType type = (enum MonsterType)(rand() % MAX_SPECIAL_MONSTERS);
        global ? (dawn->global_monster.type = type) : (dawn->players[pindex].personal_monster.type = type);
        monster_buff(type, pindex, global);
        global ? tmp = dawn->global_monster : dawn->players[pindex].personal_monster;
    }

    //Construct the final output string and send to server
    sprintf(out, "PRIVMSG %s :Monster spawned in room%s [%s] [%d/%d %sHP%s] - [%d STR] - [%d DEF] - [%d INT] -"
                 " [%d MDEF] %s",
            dawn->active_room, pstring, tmp.name, tmp.hp, tmp.mhp, IRC_RED, IRC_NORMAL, tmp.str, tmp.def,
                 tmp.intel, tmp.mdef, get_monstring(tmp.type, monspecial));
    add_msg(out, strlen(out));
}

void slay_monster(char const *username, int global, int amount) { //username -> MAX_NICK_LENGTH
    char out[MAX_MESSAGE_BUFFER];
    int pindex = get_pindex(username);

    if (global && dawn->global_monster.active) {
        if (amount < 1 || amount >= MAX_SLAY_GOLD) {
            sprintf(out, "PRIVMSG %s :Please enter an amount in between 1 - %d gold\r\n", dawn->active_room,
                    MAX_SLAY_GOLD);
            add_msg(out, strlen(out));
            return;
        }

        if (dawn->players[pindex].gold >= amount) {
            dawn->players[pindex].gold -= amount;
            dawn->global_monster.slay_cost -= amount;
            sprintf(out, "PRIVMSG %s :%s has contributed %d towards the cost of hiring a warrior to slay "
                            "the monster. %d gold still has to be raised!\r\n", dawn->active_room, username, amount,
                    dawn->global_monster.slay_cost);
        } else {
            sprintf(out, "PRIVMSG %s :%s, you do not have that much gold\r\n", dawn->active_room, username);
        }

        add_msg(out, strlen(out));

        if (dawn->global_monster.slay_cost <= 0) {
            sprintf(out, "PRIVMSG %s :The people have raised enough money to rid their town of the horrid monster "
                    "and hire a warrior to come dispose of the entity. In no time, a warrior rides into town "
                            "and slices the monster in half with one precise strike. Well that was easy...\r\n",
                    dawn->active_room);
            dawn->global_monster.active = 0;
            dawn->global_monster.slay_cost = dawn->global_monster.gold;
            dawn->global_monster.hp = dawn->global_monster.mhp;
            dawn->global_monster.type = DEFMON;
            add_msg(out, strlen(out));
        }
    } else {
        pindex = get_pindex(username);
        if (!dawn->players[pindex].personal_monster.active) return;
        if (dawn->players[pindex].gold >= 150) {
            dawn->players[pindex].gold -= 150;
            sprintf(out, "PRIVMSG %s :%s has paid a true warrior to dispose of the monster which "
                    "they couldn't handle. [Gold -150]\r\n", dawn->active_room, username);
            dawn->players[pindex].personal_monster.hp = dawn->players[pindex].personal_monster.mhp;
            dawn->players[pindex].personal_monster.active = 0;
            dawn->players[pindex].personal_monster.type = DEFMON;
        } else {
            sprintf(out, "PRIVMSG %s :%s, you don't have enough gold to pay a true warrior to dispose "
                    "of this monster [cost: 150 gold]\r\n", dawn->active_room, username);
        }
        add_msg(out, strlen(out));
    }
}

void print_monster(char const *username, int global) { //username -> MAX_NICK_LENGTH
    char out[MAX_MESSAGE_BUFFER];
    int pindex = get_pindex(username);

    if (pindex == -1) return;

    if (global) {
        if (!dawn->global_monster.active) {
            sprintf(out, "PRIVMSG %s :There is no monster in the room currently\r\n", dawn->active_room);
        } else {
            snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :A %s is in the room: [Health: %d] [%d Str - %d Def - %d Int - %d MDef]\r\n",
                     dawn->active_room, dawn->global_monster.name, dawn->global_monster.hp,
                     dawn->global_monster.str, dawn->global_monster.def, dawn->global_monster.intel,
                     dawn->global_monster.mdef);
        }
    } else {
        if (!dawn->players[pindex].personal_monster.active) {
            sprintf(out, "PRIVMSG %s :%s, you are not fighting any monster currently\r\n", dawn->active_room, username);
        } else {
            sprintf(out, "PRIVMSG %s :%s, you are fighting a %s: [Health: %d][%dS - %dD - %dI - %dMD]\r\n",
                    dawn->active_room, username, dawn->players[pindex].personal_monster.name,
                    dawn->players[pindex].personal_monster.hp,
                    dawn->players[pindex].personal_monster.str,
                    dawn->players[pindex].personal_monster.def,
                    dawn->players[pindex].personal_monster.intel,
                    dawn->players[pindex].personal_monster.mdef);
        }
    }
    add_msg(out, strlen(out));
}
