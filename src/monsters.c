#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "include/status.h"
#include "include/colors.h"

void call_monster (struct Bot *dawn, const char username[64], int global) {
    char out[MAX_MESSAGE_BUFFER];
    char pstring[64];
    int i, pindex;
    i = pindex = 0;

    if (global) {
        if (dawn->global_monster.active) {
            sprintf(out, "PRIVMSG %s :The %s got bored and left town. Unfortunately his friend wandered in...\r\n",
                    dawn->active_room, dawn->global_monster.name);
            send_socket(out);
            dawn->global_monster.slay_cost = dawn->global_monster.gold;
            dawn->global_monster.hp = dawn->global_monster.mhp;
        }

        dawn->global_monster = dawn->monsters[rand()%MAX_MONSTERS];
        dawn->global_monster.active = 1;

        for (i=0; i<dawn->player_count; i++) {
            dawn->players[i].contribution = 0;
        }

        strcpy(pstring, ":");
    } else {
        pindex = get_pindex(dawn, username);
        if (dawn->players[pindex].personal_monster.active) return;
        dawn->players[pindex].personal_monster = dawn->monsters[rand()%MAX_MONSTERS];
        dawn->players[pindex].personal_monster.active = 1;
        sprintf(pstring, " and attacks %s:", username);
    }

    struct Monsters tmp = global ? dawn->global_monster : dawn->players[pindex].personal_monster;

    sprintf(out, "PRIVMSG %s :Monster spawned in room%s [%s] [%d/%d %sHP%s] - [%d STR] - [%d DEF] - [%d INT] -"
                 " [%d MDEF]\r\n",
                 dawn->active_room, pstring, tmp.name, tmp.hp, tmp.mhp, red, normal, tmp.str, tmp.def,
                 tmp.intel, tmp.mdef);
    send_socket(out);
}

void slay_monster (struct Bot *dawn, const char username[64], int global, int amount) {
    char out[MAX_MESSAGE_BUFFER];
    int pindex = get_pindex(dawn, username);

    if (global && dawn->global_monster.active) {
        if (amount < 1 || amount >= MAX_SLAY_GOLD) {
            sprintf(out, "PRIVMSG %s :Please enter an amount in between 1 - 10,000,000 gold\r\n", dawn->active_room);
            send_socket(out);
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

        send_socket(out);

        if (dawn->global_monster.slay_cost <= 0) {
            sprintf(out, "PRIVMSG %s :The people have raised enough money to rid their town of the horrid monster "
                    "and hire a warrior to come dispose of the entity. In no time, a warrior rides into town "
                    "and slices the monster in half with one precise strike. Well that was easy...\r\n", dawn->active_room);
            dawn->global_monster.active = 0;
            dawn->global_monster.slay_cost = dawn->global_monster.gold;
            dawn->global_monster.hp = dawn->global_monster.mhp;
            send_socket(out);
        }
    } else {
        pindex = get_pindex(dawn, username);
        if (!dawn->players[pindex].personal_monster.active) return;
        if (dawn->players[pindex].gold >= 150) {
            dawn->players[pindex].gold -= 150;
            sprintf(out, "PRIVMSG %s :%s has paid a true warrior to dispose of the monster which "
                    "they couldn't handle. [Gold -150]\r\n", dawn->active_room, username);
            dawn->players[pindex].personal_monster.hp = dawn->players[pindex].personal_monster.mhp;
            dawn->players[pindex].personal_monster.active = 0;
        } else {
            sprintf(out, "PRIVMSG %s :%s, you don't have enough gold to pay a true warrior to dispose "
                    "of this monster [cost: 150 gold]\r\n", dawn->active_room, username);
        }
        send_socket(out);
    }
}

void print_monster (struct Bot *dawn, const char username[64], int global) {
    char out[MAX_MESSAGE_BUFFER];
    int pindex = get_pindex(dawn, username);
    if (global) {
        if (!dawn->global_monster.active) {
            sprintf(out, "PRIVMSG %s :There is no monster in the room currently\r\n", dawn->active_room);
        } else {
            sprintf(out, "PRIVMSG %s :A %s is in the room: [Health: %d][%dS - %dD - %dI - %dMD]\r\n",
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
    send_socket(out);
}

