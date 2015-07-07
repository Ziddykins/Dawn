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
#include "include/market.h"

//Prototype
void check_famine (struct Bot *, int);

static void random_shrine (struct Bot *b, char * username) { //username -> MAX_NICK_LENGTH
    char out[MAX_MESSAGE_BUFFER];
    int index = get_pindex(b, username);
    int which = rand() % MAX_SHRINE_TYPE;

    switch (which) {
        case 0:
            b->players[index].health = b->players[index].max_health;
            b->players[index].mana   = b->players[index].max_mana;
            sprintf(out, "PRIVMSG %s :%s has come across a spring! %s hops in and relaxes. HP/MP replenished!\r\n",
                    b->active_room, username, username);
            addMsg(out, strlen(out));
            break;
        case 1:
        {
            int health_gain = 15;
            b->players[index].health += health_gain;
            if (b->players[index].health >= b->players[index].max_health) {
                b->players[index].health = b->players[index].max_health;
            }
            sprintf(out, "PRIVMSG %s :%s has come across a spring! %s hops in and relaxes... A bit too much. %s"
                   " has peed in the spring! HP +%d\r\n", b->active_room, username, username, username, health_gain);
            addMsg(out, strlen(out));
            break;
        }
        case 2:
        {
            int experience_gain = rand() % (b->players[index].level * 10) + 1;
            struct Message temp;
            strcpy(temp.sender_nick, username);
            strcpy(temp.receiver, b->active_room);
            b->players[index].experience += (unsigned long long)experience_gain;
            check_levelup(b, &temp);
            sprintf(out, "PRIVMSG %s :%s has come across a spring! They notice etchings all around the spring."
                    " Taking the time to understand the etchings makes them feel more experienced! Exp +%d\r\n",
                    temp.receiver, username, experience_gain);
            addMsg(out, strlen(out));
            break;
        }
    }
}

static void random_reward (struct Bot *b, char * username) { //username -> MAX_NICK_LENGTH
    char out[MAX_MESSAGE_BUFFER];
    int index = get_pindex(b, username);
    int which = rand() % MAX_REWARD_TYPE;

    switch (which) {
        case 0:
        {
            int gold_gain = rand() % (b->players[index].level * 75) + 1;
            b->players[index].gold += gold_gain;
            sprintf(out, "PRIVMSG %s :While %s was walking, they found a pouch on the ground. Upon further inspection,"
                   " the pouch contained some gold! Score! Gold +%d\r\n", b->active_room, username, gold_gain);
            addMsg(out, strlen(out));
            break;
        }
        case 1:
        {
            int gold_gain = rand() % (b->players[index].level * 125) + 1;
            b->players[index].gold += gold_gain;
            sprintf(out, "PRIVMSG %s :%s has tripped! As they bring their head up, a large sack is spotted behind"
                   " a rock. %s hops up and goes to inspect the sack, to discover it is full of gold! Gold +%d\r\n",
                   b->active_room, username, username, gold_gain);
            addMsg(out, strlen(out));
            break;
        }
        case 2:
            b->players[index].fullness += 5;
            if (b->players[index].fullness > 100) b->players[index].fullness = 100;
            sprintf(out, "PRIVMSG %s :%s comes across a vendor giving out free cheese samples. You decide to try a "
                    "wedge, and it is delicious! Fullness +5\r\n", b->active_room, username);
            addMsg(out, strlen(out));
            break;
    }
}

static void random_punishment (struct Bot *b, char const * username) { //username -> MAX_NICK_LENGTH
    char out[MAX_MESSAGE_BUFFER];
    int index = get_pindex(b, username);
    int which = rand() % MAX_PUNISHMENT_TYPE;

    switch (which) {
        case 0:
        {
            int damage_taken = rand() % 25 + 1;
            struct Message temp;
            strcpy(temp.sender_nick, username);
            strcpy(temp.receiver, b->active_room);
            b->players[index].health -= damage_taken;
            sprintf(out, "PRIVMSG %s :A pack of wolves corner %s and munch on them a little bit. HP -%d\r\n",
                    b->active_room, username, damage_taken);
            addMsg(out, strlen(out));
            check_alive(b, &temp);
            break;
        }
        case 1:
        {
            int damage_taken = rand() % 45 + 1;
            struct Message temp;
            strcpy(temp.sender_nick, username);
            strcpy(temp.receiver, b->active_room);
            b->players[index].health -= damage_taken;
            sprintf(out, "PRIVMSG %s :While walking, %s was struck by lightning! HP -%d\r\n",
                    b->active_room, username, damage_taken);
            addMsg(out, strlen(out));
            check_alive(b, &temp);
            break;
        }
        case 2:
            b->players[index].fullness -= 5;
            sprintf(out, "PRIVMSG %s :%s comes across a vendor giving out free cheese samples. You decide to "
                    "try a wedge, and it is awful! You promptly vomit.  Fullness -5\r\n", b->active_room, username);
            addMsg(out, strlen(out));
            check_famine(b, index);
            break;
    }
}

//Select a random event and a random player
//which the event is directed at, should there
//be a receiver. If the room happens to be empty,
//no event will be chosen
void hourly_events (struct Bot *b) {
    int event  = rand() % MAX_EVENT_TYPE;
    int player = rand() % b->player_count;

    while (b->players[player].available != 1) {
        player = rand() % b->player_count;
    }

    switch (event) {
        case 0:
            random_reward(b, b->players[player].username);
            break;
        case 1:
            random_punishment(b, b->players[player].username);
            break;
        case 2:
            random_shrine(b, b->players[player].username);
            break;
        case 3:
            call_monster(b, NULL, 1);
            break;
        case 4:
            update_weather(b);
            break;
    }

    for (int i=0; i<b->player_count; i++) {
        b->players[i].fullness--;
    }

    //Sending -1 checks all users
    check_famine(b, -1);
    fluctuate_market(b);
}

void update_weather (struct Bot *b) {
    int weather = rand() % MAX_WEATHER_TYPE;
    char out[MAX_MESSAGE_BUFFER];

    //Avoid same weather
    while (weather == b->weather)
        weather = rand() % MAX_WEATHER_TYPE;

    switch (weather) {
        case 0:
            sprintf(out, "PRIVMSG %s :The clouds disperse, allowing "
                        "the sunlight to shine down upon %s\r\n",
                        b->active_room, b->active_room);
            b->weather = SUNNY;
            break;
        case 1:
            sprintf(out, "PRIVMSG %s :Dark clouds roll in, plunging the town"
                         " of stacked into darkness. Rain begins to fall from"
                         " the sky.\r\n", b->active_room);
            b->weather = RAINING;
            break;
        case 2:
            sprintf(out, "PRIVMSG %s :Snow begins to float down from the skies!\r\n",
                         b->active_room);
            b->weather = SNOWING;
            break;
    }
    addMsg(out, strlen(out));
}

void check_famine (struct Bot *b, int whom) {
    struct Message temp;
    if (whom == -1) {
        for (int i=0; i<b->player_count; i++) {
            strcpy(temp.sender_nick, b->players[i].username);
            strcpy(temp.receiver, b->active_room);
            check_alive(b, &temp);
        }
    } else {
        strcpy(temp.sender_nick, b->players[whom].username);
        strcpy(temp.receiver, b->active_room);
        check_alive(b, &temp);
    }
}
