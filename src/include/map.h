#ifndef MAP_H_INCLUDED
#define MAP_H_INCLUDED
#include "status.h"
#include "network.h"
#include "limits.h"

//Prototypes
struct Message;
void move_player (struct Bot *, struct Message *, int, int);
void find_building (struct Bot *, struct Message *, char []);
void check_special_location (struct Bot *, int);

struct Buildings {
    char name[64];
    int x, y;
};

struct TravelTimer {
    int x, y, active;
};

struct Map {
    char name[100];
    int max_x, max_y;
    int cur_x, cur_y;
    int exitx, exity;
    int min_level;
    struct Buildings buildings[MAX_BUILDINGS];
};
#endif
