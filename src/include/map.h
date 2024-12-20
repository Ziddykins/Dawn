#ifndef MAP_H_INCLUDED
#define MAP_H_INCLUDED

#include "network.h"
#include "limits.h"

//Prototypes
struct Message;

void move_player(struct Message *, int, int, int);

void print_location(int);

void find_building(struct Bot *, struct Message *, char *);
void check_special_location (struct Bot *, int); //DEPRECATED
void diamond_square(float *heightmap, int dim, float sigma, int level);


void init_map(char const * const fn);
void free_map(void);
void save_map(char const * const fn);

void generate_map(void);

int is_water(int x, int y);
int is_obstructed(int x, int y);
int iter_to_dirflag(int iter);

float pathlen(int x1, int y1, int x2, int y2);

struct Building {
    char name[64];
    int x, y;
    int width;
    int height;
};

struct MapLocation {
    struct Building buildings[MAX_BUILDINGS];
    char name[64];
    int max_x, max_y;
    int x, y;
};

struct Location {
    int x, y;
};

struct travel_timer {
    struct Location pos;
    int active;
};

#endif
