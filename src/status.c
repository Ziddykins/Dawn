#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include "status.h"
#include "limits.h"
#include "network.h"
#include "player.h"

//Prototype
void update_weather (Bot *);

void set_timer (int timer, Bot *dawn, int amount) {
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
                        if ((dawn->players[i].health + 5)
                                <= dawn->players[i].max_health) {
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
