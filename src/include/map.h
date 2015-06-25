#ifndef MAP_H_INCLUDED
#define MAP_H_INCLUDED
#include "status.h"
#include "network.h"

//Prototypes
struct Message;
void move_player (struct Bot *, struct Message *, int, int);

struct Building {
    unsigned int x, y;
};

struct TravelTimer {
    unsigned int x, y;
    time_t expires;
    int active;
};

struct Map {
    char name[100];
    unsigned int max_x, max_y;
    unsigned int cur_x, cur_y;
    unsigned int exitx, exity;
    unsigned int min_level;
    struct Building shop, stable, shrine, gym;
    struct Building cshop, wepshop, armshop, bank;
};
#endif
