#ifndef MAP_H_INCLUDED
#define MAP_H_INCLUDED

#include <math.h>
#include <limits.h>
#include <png.h>

#include "status.h"
#include "network.h"
#include "limits.h"
#include "util.h"

//Prototypes
struct Message;
void move_player (struct Bot *, struct Message *, int, int);
void find_building (struct Bot *, struct Message *, char []);
//void check_special_location (struct Bot *, int); //DEPRECATED
void diamond_square(float *heightmap, int dim, float sigma, int level);


void init_map(char const * const fn);
void free_map(void);
void save_map(char const * const fn);

void generate_map(void);

struct location {
    int x, y;
};

enum path_flags {
    RECONSTRUCT = 1<<0,
    ALL_TARGETS = 1<<1
};

enum direction {
    NORTH = 1<<0,
    EAST = 1<<1,
    SOUTH = 1<<2,
    WEST = 1<<3,
};

int is_water(int x, int y);
int is_obstructed(int x, int y);
float transfer_cost(int x1, int y1, int x2, int y2);
int iter_to_dirflag(int iter);

float pathlen(int x1, int y1, int x2, int y2);
float runpath(struct location ** rop, int x1, int y1, int x2, int y2, int flags);

struct Building {
    char name[64];
    int x, y;
};

struct TravelTimer {
    int x, y, active;
};

enum map_flags {
    MAP_PRESENT = 1<<0,
    MAP_SAVED = 1<<1
};

struct Map {
    float * heightmap; //dim*dim
    float * oredistr; //dim*dim*ORE_COUNT
    float water_level;
    int dim;
    int flags;

    //char pad[4];
};
#endif
