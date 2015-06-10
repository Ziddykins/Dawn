#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include "status.h"
#include "limits.h"
#include "network.h"
#include "player.h"
#include "colors.h"

//Prototype
void update_weather (Bot *);

void set_timer (int timer, Bot *dawn, time_t amount) {
    time_t epoch = time(NULL);
    dawn->timer[timer].time_finished = epoch + amount;
    printf("Timer %d set to %ld\n", timer, (epoch + amount));
}

void check_timers (Bot *dawn) {
    int i;
    time_t epoch = time(NULL);
    for (i=0; i<MAX_TIMERS; i++) {
        if (dawn->timer[i].time_finished - epoch <= 0) {
            printf("Timer %d expired\n", i);
            switch (i) {
                case WEATHER:
                    update_weather(dawn);
                    set_timer(WEATHER, dawn, WEATHER_INTERVAL);
                    break;
                case HEALING: {
                    int j;
                    for (j=0; j<dawn->player_count; j++) {
                        if ((dawn->players[i].health + 5) <= dawn->players[i].max_health) {
                            dawn->players[i].health += 5;
                        } else {
                            dawn->players[i].health = dawn->players[i].max_health;
                        }
                    }
                    set_timer(HEALING, dawn, HEALING_INTERVAL);
                    break;
                }
                case SAVING: {
                    Bot temp;
                    size_t size = sizeof(temp);
                    save_players(dawn, size);
                    set_timer(SAVING, dawn, SAVING_INTERVAL);
                    break;
                }
                case BATTLE: {
                    call_monster(dawn, -1);
                    set_timer(BATTLE, dawn, BATTLE_INTERVAL);
                    break;
                }
                default:
                    continue;
            }
        }
    }
}

void init_timers (Bot *dawn) {
    //Times are defined in limits.h and are in seconds
    set_timer(WEATHER, dawn, SAVING_INTERVAL);
    set_timer(HEALING, dawn, HEALING_INTERVAL);
    set_timer(SAVING,  dawn, SAVING_INTERVAL);
    set_timer(BATTLE,  dawn, BATTLE_INTERVAL);
    printf("Timers started\n");
}

void update_weather (Bot *dawn) {
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

void call_monster (Bot *dawn, int which) {
    char out[MAX_MESSAGE_BUFFER];
    int i;
    if (which >= MAX_MONSTERS) return;
    //If a global monster already exists, reset its health and call a random one
    //into the room and assign global_monster to the one chosen
    if (dawn->global_monster.active) {
        dawn->global_monster.hp = dawn->global_monster.mhp;
        dawn->global_monster = dawn->monsters[which == -1 ? rand()%MAX_MONSTERS : which];
    } else {
        dawn->global_monster = dawn->monsters[which == -1 ? rand()%MAX_MONSTERS : which];
    }

    sprintf(out, "PRIVMSG %s :Monster spawned in room: [%s] [%d/%d %sHP%s] - [%d STR] - [%d DEF] - [%d INT] -"
                 " [%d MDEF]\r\n", 
                 dawn->active_room, dawn->global_monster.name, dawn->global_monster.hp,
                 dawn->global_monster.mhp, red, normal, dawn->global_monster.str, dawn->global_monster.def,
                 dawn->global_monster.intel, dawn->global_monster.mdef);
    send_socket(out);
    
    //set active monster and not told of deceased monster
    dawn->global_monster.active = 1;
    dawn->global_monster.told  = 0;
    
    //Reset damage contribution for users
    for (i=0; i<dawn->player_count; i++) {
        dawn->players[i].contribution = 0;
    }
}
