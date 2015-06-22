#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include "status.h"
#include "limits.h"
#include "network.h"
#include "player.h"
#include "colors.h"
#include "events.h"

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
}

void init_timers (struct Bot *dawn) {
    //Times are defined in limits.h and are in seconds
    set_timer(HEALING, dawn, HEALING_INTERVAL);
    set_timer(SAVING,  dawn, SAVING_INTERVAL);
    set_timer(HOURLY,  dawn, 3600);
    printf("Timers started\n");
}
