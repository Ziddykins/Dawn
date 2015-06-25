#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "include/status.h"
#include "include/limits.h"
#include "include/player.h"
#include "include/network.h"
#include "include/colors.h"
#include "include/combat.h"
#include "include/events.h"
#include "include/monsters.h"

//Prototype
void check_famine (struct Bot *, int);

static void random_shrine (struct Bot *dawn, char username[64]) {
    char out[MAX_MESSAGE_BUFFER];
    int index = get_pindex(dawn, username);
    int which = rand() % MAX_SHRINE_TYPE;

    switch (which) {
        case 0:
            dawn->players[index].health = dawn->players[index].max_health;
            dawn->players[index].mana   = dawn->players[index].max_mana;
            sprintf(out, "PRIVMSG %s :%s has come across a spring! %s hops in and relaxes. HP/MP replenished!\r\n",
                    dawn->active_room, username, username);
            send_socket(out);
            break;
        case 1: {
            int health_gain = 15;
            dawn->players[index].health += health_gain;
            if (dawn->players[index].health >= dawn->players[index].max_health) {
                dawn->players[index].health = dawn->players[index].max_health;
            }
            sprintf(out, "PRIVMSG %s :%s has come across a spring! %s hops in and relaxes... A bit too much. %s"
                   " has peed in the spring! HP +%d\r\n", dawn->active_room, username, username, username, health_gain);
            send_socket(out);
            break;
        }
        case 2: {
            int experience_gain = rand() % (dawn->players[index].level * 10) + 1;
            struct Message temp;
            strcpy(temp.sender_nick, username);
            strcpy(temp.receiver, dawn->active_room);
            dawn->players[index].experience += experience_gain;
            check_levelup(dawn, &temp);
            sprintf(out, "PRIVMSG %s :%s has come across a spring! They notice etchings all around the spring."
                    " Taking the time to understand the etchings makes them feel more experienced! Exp +%d\r\n",
                    temp.receiver, username, experience_gain);
            send_socket(out);
            break;
        }
    }
}

static void random_reward (struct Bot *dawn, char username[64]) {
    char out[MAX_MESSAGE_BUFFER];
    int index = get_pindex(dawn, username);
    int which = rand() % MAX_REWARD_TYPE;

    switch (which) {
        case 0: {
            int gold_gain = rand() % (dawn->players[index].level * 75) + 1;
            dawn->players[index].gold += gold_gain;
            sprintf(out, "PRIVMSG %s :While %s was walking, they found a pouch on the ground. Upon further inspection,"
                   " the pouch contained some gold! Score! Gold +%d\r\n", dawn->active_room, username, gold_gain);
            send_socket(out);
            break;
        }
        case 1: {
            int gold_gain = rand() % (dawn->players[index].level * 125) + 1;
            dawn->players[index].gold += gold_gain;
            sprintf(out, "PRIVMSG %s :%s has tripped! As they bring their head up, a large sack is spotted behind"
                   " a rock. %s hops up and goes to inspect the sack, to discover it is full of gold! Gold +%d\r\n",
                   dawn->active_room, username, username, gold_gain);
            send_socket(out);
            break;
        }
        case 2:
            dawn->players[index].fullness += 5;
            if (dawn->players[index].fullness > 100) dawn->players[index].fullness = 100;
            sprintf(out, "PRIVMSG %s :%s comes across a vendor giving out free cheese samples. You decide to try a "
                    "wedge, and it is delicious! Fullness +5\r\n", dawn->active_room, username);
            send_socket(out);
            break;
    }
}

static void random_punishment (struct Bot *dawn, char username[64]) {
    char out[MAX_MESSAGE_BUFFER];
    int index = get_pindex(dawn, username);
    int which = rand() % MAX_PUNISHMENT_TYPE;

    switch (which) {
        case 0: {
            int damage_taken = rand() % 25 + 1;
            struct Message temp;
            strcpy(temp.sender_nick, username);
            strcpy(temp.receiver, dawn->active_room);
            dawn->players[index].health -= damage_taken;
            sprintf(out, "PRIVMSG %s :A pack of wolves corner %s and munch on them a little bit. HP -%d\r\n",
                    dawn->active_room, username, damage_taken);
            send_socket(out);
            check_alive(dawn, &temp);
            break;
        }
        case 1: {
            int damage_taken = rand() % 45 + 1;
            struct Message temp;
            strcpy(temp.sender_nick, username);
            strcpy(temp.receiver, dawn->active_room);
            dawn->players[index].health -= damage_taken;
            sprintf(out, "PRIVMSG %s :While walking, %s was struck by lightning! HP -%d\r\n",
                    dawn->active_room, username, damage_taken);
            send_socket(out);
            check_alive(dawn, &temp);
            break;
        }
        case 2:
            dawn->players[index].fullness -= 5;
            sprintf(out, "PRIVMSG %s :%s comes across a vendor giving out free cheese samples. You decide to "
                    "try a wedge, and it is awful! You promptly vomit.  Fullness -5\r\n", dawn->active_room, username);
            send_socket(out);
            check_famine(dawn, index);
            break;
    }
}

//Select a random event and a random player
//which the event is directed at, should there
//be a receiver. If the room happens to be empty,
//no event will be chosen
void hourly_events (struct Bot *dawn) {
    int event  = rand() % MAX_EVENT_TYPE;
    int player = rand() % dawn->player_count;
    int count  = 0;
    while (dawn->players[player].available != 1 && count < 100) {
        player = rand() % dawn->player_count;
        count++;
    }
    if (count == 99) return;

    switch (event) {
        case 0:
            random_reward(dawn, dawn->players[player].username);
            break;
        case 1:
            random_punishment(dawn, dawn->players[player].username);
            break;
        case 2:
            random_shrine(dawn, dawn->players[player].username);
            break;
        case 3: {
            call_monster(dawn, NULL, 1);
            break;
        }
    }
    for (int i=0; i<dawn->player_count; i++) {
        dawn->players[i].fullness--;
    }
    check_famine(dawn, -1);
}

void update_weather (struct Bot *dawn) {
    int weather = rand() % 4;
    char out[MAX_MESSAGE_BUFFER];

    //Avoid same weather
    while (weather == dawn->weather) weather = rand() % 4;

    switch (weather) {
        case 0:
            sprintf(out, "PRIVMSG %s :The clouds disperse, allowing "
                        "the sunlight to shine down upon %s\r\n",
                        dawn->active_room, dawn->active_room);
            send_socket(out);
            dawn->weather = SUNNY;
            break;
        case 1:
            sprintf(out, "PRIVMSG %s :Dark clouds roll in, plunging the town"
                         " of stacked into darkness. Rain begins to fall from"
                         " the sky.\r\n", dawn->active_room);
            send_socket(out);
            dawn->weather = RAINING;
            break;
        case 2:
            sprintf(out, "PRIVMSG %s :Snow begins to float down from the skies!\r\n",
                         dawn->active_room);
            send_socket(out);
            dawn->weather = SNOWING;
            break;
    }
}

void check_famine (struct Bot *dawn, int whom) {
    struct Message temp;
    if (whom == -1) {
        for (int i=0; i<dawn->player_count; i++) {
            strcpy(temp.sender_nick, dawn->players[i].username);
            strcpy(temp.receiver, dawn->active_room);
            check_alive(dawn, &temp);
        }
    } else {
        strcpy(temp.sender_nick, dawn->players[whom].username);
        strcpy(temp.receiver, dawn->active_room);
        check_alive(dawn, &temp);
    }
}
