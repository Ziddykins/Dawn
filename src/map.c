#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "include/status.h"
#include "include/parse.h"
#include "include/player.h"
#include "include/limits.h"
#include "include/network.h"

#define TRAVEL_TIME_MULT (3.0)

struct Map set_map (int which) {
    struct Map return_map;
    enum BUILDINGS {SHRINE, SHOP};
    switch (which) {
        case 0:
        {
            strcpy(return_map.name, "The Sanctuary");
            strcpy(return_map.name, nultrm(return_map.name));
            strcpy(return_map.buildings[SHRINE].name, "shrine");
            strcpy(return_map.buildings[SHOP].name, "shop");

            return_map.exitx = 0;
            return_map.exity = 0;
            return_map.max_x = 9;
            return_map.max_y = 9;

            return_map.buildings[SHRINE].x = 7;
            return_map.buildings[SHRINE].y = 4;

            return_map.buildings[SHOP].x = 11;
            return_map.buildings[SHOP].y = 2;

        }
    }
    return return_map;
}

void print_location (struct Bot *dawn, int i) {
    char out[MAX_MESSAGE_BUFFER];
    sprintf(out, "PRIVMSG %s :%s, you are at %d,%d in %s\r\n", dawn->active_room, dawn->players[i].username,
            dawn->players[i].current_map.cur_x, dawn->players[i].current_map.cur_y,
            dawn->players[i].current_map.name);
    send_socket(out);
}

void move_player (struct Bot *dawn, struct Message *message, int x, int y) {
    char out[MAX_MESSAGE_BUFFER];
    int pindex = get_pindex(dawn, message->sender_nick);
    int dx, dy, cx, cy;
    int max_x = dawn->players[pindex].current_map.max_x;
    int max_y = dawn->players[pindex].current_map.max_y;
    double travel_time;

    if ((x > max_x || x < 0) || (y > max_y || y < 0)) {
        sprintf(out, "PRIVMSG %s :Invalid location, this map is %dx%d\r\n", message->receiver, max_x, max_y);
        send_socket(out);
        return;
    }

    cx = dawn->players[pindex].current_map.cur_x;
    cy = dawn->players[pindex].current_map.cur_y;

    if (cx == x && cy == y) return;

    dx = abs(x - cx);
    dy = abs(y - cy);
    travel_time = sqrt(dx*dx+dy*dy)*TRAVEL_TIME_MULT;
    assert(travel_time < (double)((((unsigned int)1<<(sizeof(unsigned int) * 8 - 1))-1)/TRAVEL_TIME_MULT));
    addEvent(TRAVEL, pindex, (unsigned int)travel_time, 1);
    dawn->players[pindex].travel_timer.x = x;
    dawn->players[pindex].travel_timer.y = y;
    dawn->players[pindex].travel_timer.active = 1;

    sprintf(out, "PRIVMSG %s :%s, you are traveling from (%d,%d) to (%d,%d). This will take %u seconds.\r\n",
            message->receiver, message->sender_nick, cx, cy, x, y, (unsigned int)travel_time);
    send_socket(out);
}

void find_building (struct Bot *dawn, struct Message *message, char location[48]) {
    char out[MAX_MESSAGE_BUFFER];
    int pindex = get_pindex(dawn, message->sender_nick);
    int bindex = get_bindex(dawn, message->sender_nick, location);

    if (bindex != -1) {
        sprintf(out, "PRIVMSG %s :The %s in %s is located at %d,%d\r\n", message->receiver, location,
                dawn->players[pindex].current_map.name, dawn->players[pindex].current_map.buildings[bindex].x,
                dawn->players[pindex].current_map.buildings[bindex].y);
    } else {
        sprintf(out, "PRIVMSG %s :Unknown location '%s'\r\n", message->receiver, location);
    }
    send_socket(out);
}

void check_special_location (struct Bot *dawn, int pindex) {
    char out[MAX_MESSAGE_BUFFER];
    int cur_x = dawn->players[pindex].current_map.cur_x;
    int cur_y = dawn->players[pindex].current_map.cur_y;

    for (int i=0; i<MAX_BUILDINGS; i++) {
        if (dawn->players[pindex].current_map.buildings[i].x == cur_x &&
                dawn->players[pindex].current_map.buildings[i].y == cur_y) {
            if (cur_x == 0 && cur_y == 0) continue;
            sprintf(out, "PRIVMSG %s :%s stands in front of the %s\r\n",
                    dawn->active_room, dawn->players[pindex].username, dawn->players[pindex].current_map.buildings[i].name);
            send_socket(out);
        }
    }
}
