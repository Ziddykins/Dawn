#include <string.h>
#include <stdio.h>
#include "status.h"
#include "parse.h"
#include "player.h"
#include "limits.h"

Map set_map (int which) {
    Map return_map;
    switch (which) {
        case 0:
        {
            strcpy(return_map.name, "The Sanctuary");
            strcpy(return_map.name, nultrm(return_map.name));
            return_map.exitx    = 0;
            return_map.exity    = 0;
            return_map.shrine.x = 7;
            return_map.shrine.y = 4;
            return_map.shop.x   = 11;
            return_map.shop.y   = 2;
        }
    }
    return return_map;
}

void print_location (Bot *dawn, int i) {
    char out[MAX_MESSAGE_BUFFER];
    sprintf(out, "PRIVMSG %s :%s, you are at %d,%d in %s\r\n", dawn->active_room, dawn->players[i].username,
            dawn->players[i].current_map.cur_x, dawn->players[i].current_map.cur_y,
            dawn->players[i].current_map.name);
    send_socket(out);
}
