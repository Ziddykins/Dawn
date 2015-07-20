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

struct Map * global_map = 0;

void init_map() {
    CALLEXIT(!(global_map = malloc(sizeof *global_map)))
    global_map->dim = 1<<10;
    CALLEXIT(!(global_map->heightmap = malloc((unsigned long)(global_map->dim * global_map->dim) * sizeof *global_map->heightmap)))

    generate_map();
}

void generate_map() {
    diamond_square(global_map->heightmap, global_map->dim, 4000.0, global_map->dim);
}

void free_map() {
    free(global_map->heightmap);
    free(global_map);
}

void print_location (struct Bot *b, int i) {
    char out[MAX_MESSAGE_BUFFER];
    sprintf(out, "PRIVMSG %s :%s, you are at %d,%d\r\n", b->active_room, b->players[i].username,
            b->players[i].pos_x, b->players[i].pos_y);
    add_msg(out, strlen(out));
}

void move_player (struct Bot *b, struct Message *message, int x, int y) {
    char out[MAX_MESSAGE_BUFFER];
    int pindex = get_pindex(b, message->sender_nick);
    int dx, dy, cx, cy;
    double travel_time;
    if (x < 0 || x >= global_map->dim || y < 0 || y >= global_map->dim) {
        sprintf(out, "PRIVMSG %s :Invalid location, this map is %dx%d\r\n", message->receiver, global_map->dim, global_map->dim);
        add_msg(out, strlen(out));
        return;
    }

    cx = b->players[pindex].pos_x;
    cy = b->players[pindex].pos_y;

    dx = abs(x - cx);
    dy = abs(y - cy);
    travel_time = sqrt(dx*dx+dy*dy)*TRAVEL_TIME_MULT;
    assert(travel_time < (double)((((unsigned int)1<<(sizeof(unsigned int) * 8 - 1))-1)/TRAVEL_TIME_MULT));
    add_event(TRAVEL, pindex, (unsigned int)travel_time, UNIQUE);
    b->players[pindex].travel_timer.x = x;
    b->players[pindex].travel_timer.y = y;
    b->players[pindex].travel_timer.active = 1;

    sprintf(out, "PRIVMSG %s :%s, you are traveling from (%d,%d) to (%d,%d). This will take %u seconds.\r\n",
            message->receiver, message->sender_nick, cx, cy, x, y, (unsigned int)travel_time);
    add_msg(out, strlen(out));
}
/* DEPRECATED
void find_building (struct Bot *b, struct Message *message, char location[48]) {
    char out[MAX_MESSAGE_BUFFER];
    int pindex = get_pindex(b, message->sender_nick);
    int bindex = get_bindex(b, message->sender_nick, location);

    if (bindex != -1) {
        sprintf(out, "PRIVMSG %s :The %s in %s is located at %d,%d\r\n", message->receiver, location,
                b->players[pindex].current_map.name, b->players[pindex].current_map.buildings[bindex].x,
                b->players[pindex].current_map.buildings[bindex].y);
    } else {
        sprintf(out, "PRIVMSG %s :Unknown location '%s'\r\n", message->receiver, location);
    }
    add_msg(out, strlen(out));
}
*/
/* DEPRECATED
void check_special_location (struct Bot *b, int pindex) {
    char out[MAX_MESSAGE_BUFFER];
    int cur_x = b->players[pindex].current_map.cur_x;
    int cur_y = b->players[pindex].current_map.cur_y;

    for (int i=0; i<MAX_BUILDINGS; i++) {
        if (b->players[pindex].current_map.buildings[i].x == cur_x &&
                b->players[pindex].current_map.buildings[i].y == cur_y) {
            if (cur_x == 0 && cur_y == 0) continue;
            sprintf(out, "PRIVMSG %s :%s stands in front of the %s\r\n",
                    b->active_room, b->players[pindex].username, b->players[pindex].current_map.buildings[i].name);
            add_msg(out, strlen(out));
        }
    }
}
*/
void diamond_square(float *heightmap, int dim, float sigma, int level) {
    if (level < 1) return;

    // diamonds
    for (int i = level; i < dim; i += level) {
        for (int j = level; j < dim; j += level) {
            float a = heightmap[(i-level)*dim + j-level];
            float b = heightmap[i*dim + j-level];
            float c = heightmap[(i-level)*dim + j];
            float d = heightmap[i*dim+j];
            heightmap[(i-level/2)*dim + j-level/2] = (a + b + c + d) / 4 + (float)gaussrand() * sigma;
        }
    }
    // squares
    for (int i = 2 * level; i < dim; i += level) {
        for (int j = 2 * level; j < dim; j += level) {
            float a = heightmap[(i-level)*dim + j-level];
            float b = heightmap[i*dim + j-level];
            float c = heightmap[(i-level)*dim + j];
            float e = heightmap[(i-level/2)*dim + j-level/2];

            heightmap[(i-level)*dim + (j-level/2)] = (a + c + e + heightmap[(i-3*level/2)*dim + (j-level/2)]) / 4 + (float)gaussrand() * sigma;
            heightmap[(i-level/2)*dim + j-level] = (a + b + e + heightmap[(i-level/2)*dim + (j-3*level/2)]) / 4 + (float)gaussrand() * sigma;
        }
    }
    diamond_square(heightmap, dim, sigma / 2, level / 2);
}
