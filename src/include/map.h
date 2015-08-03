#ifndef MAP_H_INCLUDED
#define MAP_H_INCLUDED

#include <math.h>
#include <limits.h>
#include <png.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

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

int is_water(int x, int y);
int is_obstructed(int x, int y);
int iter_to_dirflag(int iter);

float pathlen(int x1, int y1, int x2, int y2);

struct location {
    int x, y;
};

struct travel_timer {
    struct location pos;
    int active;
};
#endif
