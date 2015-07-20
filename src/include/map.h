#ifndef MAP_H_INCLUDED
#define MAP_H_INCLUDED
#include "status.h"
#include "network.h"
#include "limits.h"

//Prototypes
struct Message;
void move_player (struct Bot *, struct Message *, int, int);
void find_building (struct Bot *, struct Message *, char []);
//void check_special_location (struct Bot *, int); //DEPRECATED
void diamondSquare(float *heightmap, int dim, float roughness, float sigma, int level);


void init_map();
void free_map();

struct Building {
    char name[64];
    int x, y;
};

struct TravelTimer {
    int x, y, active;
};

struct Map {
    float * heightmap;
    int dim;

    char pad[4];
};
#endif
