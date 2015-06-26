#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include "include/status.h"
#include "include/limits.h"
#include "include/network.h"
#include "include/player.h"
#include "include/colors.h"
#include "include/events.h"
#include "include/map.h"

void set_timer (int timer, struct Bot *dawn, time_t amount) {
    time_t epoch = time(NULL);
    dawn->timer[timer].time_finished = epoch + amount;
    printf("Timer %d set to %ld\n", timer, (epoch + amount));
}

void check_timers (struct Bot *dawn) {
    int i;
    time_t epoch = time(NULL);
    for (i=0; i<MAX_TIMERS; i++) {
        if (dawn->timer[i].time_finished - epoch <= 0) {
            printf("Timer %d expired\n", i);
            switch (i) {
                case HEALING:
                {
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
                case SAVING:
                {
                    struct Bot temp;
                    size_t size = sizeof(temp);
                    save_players(dawn, size);
                    set_timer(SAVING, dawn, SAVING_INTERVAL);
                    break;
                }
                case HOURLY:
                    hourly_events(dawn);
                    set_timer(HOURLY, dawn, 3600);
                    break;
                default:
                    continue;
            }
        }
    }

    for (i=0; i<dawn->player_count; i++) {
        if (dawn->players[i].travel_timer.expires < epoch && dawn->players[i].travel_timer.active) {
            char out[MAX_MESSAGE_BUFFER];
            dawn->players[i].current_map.cur_x = dawn->players[i].travel_timer.x;
            dawn->players[i].current_map.cur_y = dawn->players[i].travel_timer.y;
            sprintf(out, "PRIVMSG %s :%s has arrived at %d,%d\r\n", dawn->active_room, dawn->players[i].username,
                    dawn->players[i].current_map.cur_x, dawn->players[i].current_map.cur_y);
            send_socket(out);
            dawn->players[i].travel_timer.active = 0;
            check_special_location(dawn, i);
        }
    }
}

void init_timers (struct Bot *dawn) {
    //Times are defined in limits.h and are in seconds
    set_timer(HEALING, dawn, HEALING_INTERVAL);
    set_timer(SAVING,  dawn, SAVING_INTERVAL);
    set_timer(HOURLY,  dawn, 3600);
    printf("Timers started\n");
}
