#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include "status.h"
#include "limits.h"
#include "network.h"

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
        switch (i) {
            case WEATHER:
                if (dawn->timer[i].time_finished - epoch <= 0) {
                      update_weather(dawn);
                      set_timer(WEATHER, dawn, 120);
                      printf("Weather timer set to %ld\n", (epoch + 120));
                } else {
                    printf("%ld time remaining in weather timer\n", dawn->timer[i].time_finished - epoch);
                }
                break;
            default:
                printf("Timer not being set, skipping\n");
                continue;
        }
    }
}

void init_timers (Bot *dawn) {
    set_timer(WEATHER, dawn, 10);
    set_timer(HEALING, dawn, 120);
    printf("Timers started\n");
}

void update_weather (Bot *dawn) {
    int weather = rand() % 4;
    char out[MAX_MESSAGE_BUFFER];
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
                         "of stacked into darkness. Rain begins to fall from"
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
