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
#include "include/util.h"

//Prototype
void check_famine(int);

static void gift_user (void) {
    char out[MAX_MESSAGE_BUFFER];
    int player = rand() % dawn->player_count;

    while (dawn->players[player].available != 1 
            || strcmp(dawn->players[player].username, dawn->nickname) == 0) {
        player = rand() % dawn->player_count;
    }

    sprintf(out, "PRIVMSG %s :%s was generous enough to pass along the good luck to %s! +%ld gold\r\n",
            dawn->active_room, dawn->nickname, dawn->players[player].username, dawn->players[0].gold);

    dawn->players[player].gold += dawn->players[0].gold;
    dawn->players[0].gold = 0;
    add_msg(out, strlen(out));
}

static void random_shrine(char *username) { //username -> MAX_NICK_LENGTH
    char out[MAX_MESSAGE_BUFFER];
    int index = get_pindex(username);
    int which = rand() % MAX_SHRINE_TYPE;

    switch (which) {
        case 0:
            dawn->players[index].health = dawn->players[index].max_health;
            dawn->players[index].mana = dawn->players[index].max_mana;
            sprintf(out, "PRIVMSG %s :%s has come across a spring! %s hops in and relaxes. HP/MP replenished!\r\n",
                    dawn->active_room, username, username);
            add_msg(out, strlen(out));
            break;
        case 1:
        {
            int health_gain = 15;
            dawn->players[index].health += health_gain;
            if (dawn->players[index].health >= dawn->players[index].max_health) {
                dawn->players[index].health = dawn->players[index].max_health;
            }
            sprintf(out, "PRIVMSG %s :%s has come across a spring! %s hops in and relaxes... A bit too much. %s"
                            " has peed in the spring! HP +%d\r\n", dawn->active_room, username, username, username,
                    health_gain);
            add_msg(out, strlen(out));
            break;
        }
        case 2:
        {
            int experience_gain = rand() % (dawn->players[index].level * 10) + 1;
            struct Message temp;
            strcpy(temp.sender_nick, username);
            strcpy(temp.receiver, dawn->active_room);
            dawn->players[index].experience += (unsigned long long) experience_gain;
            check_levelup(&temp);
            sprintf(out, "PRIVMSG %s :%s has come across a spring! They notice etchings all around the spring."
                    " Taking the time to understand the etchings makes them feel more experienced! Exp +%d\r\n",
                    temp.receiver, username, experience_gain);
            add_msg(out, strlen(out));
            break;
        }
        default:
            PRINTERR("INVALID SHRINE")
    }
}

static void random_reward(char *username) { //username -> MAX_NICK_LENGTH
    char out[MAX_MESSAGE_BUFFER];
    int index = get_pindex(username);
    int which = rand() % MAX_REWARD_TYPE;

    switch (which) {
        case 0:
        {
            int gold_gain = rand() % (dawn->players[index].level * 75) + 1;
            dawn->players[index].gold += gold_gain;
            sprintf(out, "PRIVMSG %s :While %s was walking, they found a pouch on the ground. Upon further inspection,"
                    " the pouch contained some gold! Score! Gold +%d\r\n", dawn->active_room, username, gold_gain);
            add_msg(out, strlen(out));
            break;
        }
        case 1:
        {
            int gold_gain = rand() % (dawn->players[index].level * 125) + 1;
            dawn->players[index].gold += gold_gain;
            sprintf(out, "PRIVMSG %s :%s has tripped! As they bring their head up, a large sack is spotted behind"
                   " a rock. %s hops up and goes to inspect the sack, to discover it is full of gold! Gold +%d\r\n",
                    dawn->active_room, username, username, gold_gain);
            add_msg(out, strlen(out));
            break;
        }
        case 2:
            dawn->players[index].fullness += 5;
            if (dawn->players[index].fullness > 100) dawn->players[index].fullness = 100;
            sprintf(out, "PRIVMSG %s :%s comes across a vendor giving out free cheese samples. You decide to try a "
                    "wedge, and it is delicious! Fullness +5\r\n", dawn->active_room, username);
            add_msg(out, strlen(out));
            break;
        default:
            PRINTERR("INVALID REWARD")
    }

    if (strcmp(dawn->players[index].username, dawn->nickname) == 0 && (which == 0 || which == 1))
        gift_user();

}

static void random_punishment(char const *username) { //username -> MAX_NICK_LENGTH
    char out[MAX_MESSAGE_BUFFER];
    int index = get_pindex(username);
    int which = rand() % MAX_PUNISHMENT_TYPE;

    switch (which) {
        case 0:
        {
            int damage_taken = rand() % 25 + 1;
            struct Message temp;
            strcpy(temp.sender_nick, username);
            strcpy(temp.receiver, dawn->active_room);
            dawn->players[index].health -= damage_taken;
            sprintf(out, "PRIVMSG %s :A pack of wolves corner %s and munch on them a little bit. HP -%d\r\n",
                    dawn->active_room, username, damage_taken);
            add_msg(out, strlen(out));
            check_alive(&temp);
            break;
        }
        case 1:
        {
            int damage_taken = rand() % 45 + 1;
            struct Message temp;
            strcpy(temp.sender_nick, username);
            strcpy(temp.receiver, dawn->active_room);
            dawn->players[index].health -= damage_taken;
            sprintf(out, "PRIVMSG %s :While walking, %s was struck by lightning! HP -%d\r\n",
                    dawn->active_room, username, damage_taken);
            add_msg(out, strlen(out));
            check_alive(&temp);
            break;
        }
        case 2:
            dawn->players[index].fullness -= 5;
            sprintf(out, "PRIVMSG %s :%s comes across a vendor giving out free cheese samples. You decide to "
                    "try a wedge, and it is awful! You promptly vomit.  Fullness -5\r\n", dawn->active_room, username);
            add_msg(out, strlen(out));
            check_famine(index);
            break;
        default:
            PRINTERR("INVALID PUNISHMENT")
    }
}

//Select a random event and a random player
//which the event is directed at, should there
//be a receiver. If the room happens to be empty,
//the bot will be chosen
void hourly_events(void) {
    int event  = rand() % MAX_EVENT_TYPE;
    if (!dawn->player_count) return;
    int player = rand() % dawn->player_count;

    while (dawn->players[player].available != 1) {
        player = rand() % dawn->player_count;
    }

    switch (event) {
        case 0:
            random_reward(dawn->players[player].username);
            break;
        case 1:
            random_punishment(dawn->players[player].username);
            break;
        case 2:
            random_shrine(dawn->players[player].username);
            break;
        case 3:
            call_monster(NULL, 1);
            break;
        case 4:
            update_weather();
            break;
        default:
            PRINTERR("INVALID EVENT")
    }

    for (int i = 0; i < dawn->player_count; i++) {
        dawn->players[i].fullness--;
    }

    //Sending -1 checks all users to see if they have starved
    check_famine(-1);
    fluctuate_market();
}

void update_weather(void) {
    int weather = rand() % MAX_WEATHER_TYPE;
    char out[MAX_MESSAGE_BUFFER];

    //Avoid same weather
    while (weather == dawn->weather)
        weather = rand() % MAX_WEATHER_TYPE;

    switch (weather) {
        case 0:
            sprintf(out, "PRIVMSG %s :The clouds disperse, allowing "
                        "the sunlight to shine down upon %s\r\n",
                    dawn->active_room, dawn->active_room);
            dawn->weather = SUNNY;
            break;
        case 1:
            sprintf(out, "PRIVMSG %s :Dark clouds roll in, plunging the town"
                         " of stacked into darkness. Rain begins to fall from"
                    " the sky.\r\n", dawn->active_room);
            dawn->weather = RAINING;
            break;
        case 2:
            sprintf(out, "PRIVMSG %s :Snow begins to float down from the skies!\r\n",
                    dawn->active_room);
            dawn->weather = SNOWING;
            break;
        default:
            PRINTERR("INVALID WEATHER")
    }
    add_msg(out, strlen(out));
}

//When receiving -1, all users are checked for starvation
//If a user is supplied, only that use is checked
//Death occurs if starvation takes place
void check_famine(int whom) {
    struct Message temp;
    if (whom == -1) {
        for (int i = 0; i < dawn->player_count; i++) {
            strcpy(temp.sender_nick, dawn->players[i].username);
            strcpy(temp.receiver, dawn->active_room);
            check_alive(&temp);
        }
    } else {
        strcpy(temp.sender_nick, dawn->players[whom].username);
        strcpy(temp.receiver, dawn->active_room);
        check_alive(&temp);
    }
}
