#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "include/status.h"

#define TRAVEL_TIME_MULT (1.0f)

struct Map * global_map = 0;

enum path_flags { //for pathfinding
    RECONSTRUCT = 1<<0, //retain the came_from array to reconstruct the path
    ALL_TARGETS = 1<<1 //do not stop when the target has been reached
};

enum direction { //used in pathfinding
    NORTH = 1<<0,
    EAST = 1<<1,
    SOUTH = 1<<2,
    WEST = 1<<3,
};

enum map_flags { //represent the state of the map
    MAP_PRESENT = 1<<0,
    MAP_SAVED = 1<<1
};

struct town {
    char *name;
    struct location pos;
    float matdistr[MAT_COUNT]; //which materials are present in this town
};

struct shop {
    char * name;
    enum shop_type {
        SHOP_ARMORY,
        SHOP_DOCTOR,
        //to be extended
    } type;
};

enum map_entity {
    MENT_TOWN,
    MENT_STABLES,
    MENT_SHOP,
    MENT_SHRINE,
};

struct Map {
    float * heightmap; //dim*dim
    struct town towns[TOWN_COUNT];
    float water_level;
    int dim;
    int flags;

    //char pad[4];
};

void init_map(char const * const fn) {
    CALLEXIT(!(global_map = malloc(sizeof *global_map)))
    global_map->dim = 1<<10;
    global_map->flags = 0;
    CALLEXIT(!(global_map->heightmap = calloc(sizeof *global_map->heightmap, (size_t)(global_map->dim * global_map->dim))))
    FILE *file = fopen(fn, "rb");
    if(!file) {
        printf(INFO "Generating new heightmap\n");
        generate_map();
        save_map(fn);
    } else {
        CALLEXIT(!(fread(&global_map->dim, sizeof global_map->dim, 1, file)))
        CALLEXIT(!(fread(&global_map->flags, sizeof global_map->flags, 1, file)))
        CALLEXIT(!(fread(&global_map->water_level, sizeof global_map->water_level, 1, file)))
        CALLEXIT(!(fread(global_map->heightmap, (size_t)(global_map->dim * global_map->dim) * sizeof *global_map->heightmap, 1, file)))
        fclose(file);
        printf(INFO "Map loaded\n");
    }
}

void save_map(char const * const fn) {
    CALLEXIT(!global_map)
    if(global_map->flags & MAP_PRESENT && !(global_map->flags & MAP_SAVED)) {
        FILE *file = fopen(fn, "wb");
        if(!file) {
            PRINTWARN("Could not save heightmap")
            return;
        }
        CALLEXIT(!(fwrite(&global_map->dim, sizeof global_map->dim, 1, file)))
        CALLEXIT(!(fwrite(&global_map->flags, sizeof global_map->flags, 1, file)))
        CALLEXIT(!(fwrite(&global_map->water_level, sizeof global_map->water_level, 1, file)))

        size_t size = (size_t)(global_map->dim * global_map->dim) * sizeof *global_map->heightmap;
        CALLEXIT(!(fwrite(global_map->heightmap, size, 1, file)))
        printf(INFO "Saved map\n");
        fclose(file);
    }
}

static inline int town_too_close(struct town *towns, int index) {
    for(int i = 0; i < index; i++) {
        if(sqrt(abs(towns[i].pos.x-towns[index].pos.x)+abs(towns[i].pos.y-towns[index].pos.y)) < 10) {
            return 1;
        }
    }
    return 0;
}

void generate_map() {
    diamond_square(global_map->heightmap, global_map->dim, 4000.0, global_map->dim);
    global_map->flags |= MAP_PRESENT;

    float * copy;
    int dim = global_map->dim;
    CALLEXIT(!(copy = malloc((size_t)(dim*dim) * sizeof *copy)))
    for(int i = 0; i < dim*dim; i++) {
        copy[i] = global_map->heightmap[i];
    }
    qsort(copy, (size_t) (dim*dim), sizeof *copy, &compare_float_asc);
    global_map->water_level = copy[(int)(1.0/6.0*dim*dim)];
    free(copy);

    permute();
    for(int i = 0; i < TOWN_COUNT; i++) {
        do {
            global_map->towns[i].pos.x = (int) (randd() * dim);
            global_map->towns[i].pos.y = (int) (randd() * dim);
        } while(town_too_close(global_map->towns, i));
        for(int j = 0; j < MAT_COUNT; j++) {
            global_map->towns[i].matdistr[j] = noise(global_map->towns[i].pos.x * PERLIN_SCALE, global_map->towns[i].pos.x * PERLIN_SCALE, j*PERLIN_V_SCALE);
        }
    }
    //create markets
}

static inline int is_valid(int x, int y, int dim) {
    return x >= 0 && x < dim && y >= 0 && y < dim;
}

int is_water(int x, int y) {
    return global_map->heightmap[y*global_map->dim+x] < global_map->water_level;
}

int is_obstructed(int x, int y) { //will also handle rocks or other unreachable area
    return is_water(x,y);
}

//(x2,y2) may only have a distance of sqrt(2) from (x1,y1)
float transfer_cost(int x1, int y1, int x2, int y2) {
    int dim = global_map->dim;
    return absf(global_map->heightmap[y1*dim+x1] - global_map->heightmap[y2*dim+x2])
            + ((abs(x2-x1)+abs(y2-y1) < 2) ? 1 : (float)sqrt(2));
}

static inline void move_step(int *rop_x, int *rop_y, int x, int y, int dir) {
    *rop_x = x;
    *rop_y = y;

    if(dir & NORTH) {
        (*rop_y)--;
    } else if(dir & SOUTH) {
        (*rop_y)++;
    }

    if(dir & EAST) {
        (*rop_x)++;
    } else if(dir & WEST) {
        (*rop_x)--;
    }
}

int iter_to_dirflag(int iter) {
    switch(iter) {
        case 0:
            return NORTH;
        case 1:
            return NORTH | EAST;
        case 2:
            return EAST;
        case 3:
            return EAST | SOUTH;
        case 4:
            return SOUTH;
        case 5:
            return SOUTH | WEST;
        case 6:
            return WEST;
        case 7:
            return WEST | NORTH;
        default:
            PRINTERR("INTERNAL PATHFINDING ERROR")
    }
    exit(1);
}

static inline float manhattan(int x1, int y1, int x2, int y2) {
    return (absf(x1-x2)+absf(y1-y2));
}

static float runpath(struct location ** rop, int x1, int y1, int x2, int y2, int flags) {
    int dim = global_map->dim;

    struct location * came_from;
    CALLEXIT(!(came_from = malloc((unsigned int)(dim*dim) * sizeof *came_from)))
    for(int i = 0; i < dim*dim; i++) {
        came_from->x = -1;
        came_from->y = -1;
    }

    float *cost;
    CALLEXIT(!(cost = malloc((unsigned int)(dim*dim) * sizeof *cost)))
    for(int i = 0; i < dim*dim; i++) {
        cost[i] = -1;
    }

    came_from[x1*dim+x1].x = x1;
    came_from[x1*dim+x1].y = y1;
    cost[y1*dim+x1] = 0;

    PriorityQueue pq = init_priority_queue();
    struct location * start;
    CALLEXIT(!(start = malloc(sizeof *start)))
    start->x = x1;
    start->y = y1;
    priority_insert(pq, 0, start);
    while(!priority_empty(pq)) {
        struct location * current = priority_remove_min(pq);
        int x = current->x, y = current->y;
        free(current);

        if(x == x2 && y == y2 && !(flags & ALL_TARGETS)) {
            break;
        }

        int cur_x, cur_y;

        for(int i = 0; i < 8; i++) {
            move_step(&cur_x, &cur_y, x, y, iter_to_dirflag(i));

            if(!is_valid(cur_x, cur_y, dim)) {
                continue;
            }

            float new_cost = cost[y * dim + x] + transfer_cost(x, y, cur_x, cur_y);

            if(cost[cur_y*dim+cur_x] > new_cost || cost[cur_y*dim+cur_x] < 0) {
                cost[cur_y*dim+cur_x] = new_cost;
                came_from[cur_y*dim+cur_x].x = x;
                came_from[cur_y*dim+cur_x].y = y;
                struct location * nloc;
                //dynamic memory to avoid filling stack space with &((struct location){.x = x-1, .y = y})
                CALLEXIT(!(nloc = malloc(sizeof *nloc)))
                nloc->x = cur_x;
                nloc->y = cur_y;
                priority_insert(pq, new_cost+manhattan(nloc->x, nloc->y, x2, y2), nloc);
            }
        }
    }
    if(flags & RECONSTRUCT) {
        *rop = came_from;
    } else {
        free(came_from);
    }

    float final_cost = cost[y2*dim+x2];
    free(cost);
    free_priority_queue(pq, 1);
    return final_cost;
}

float pathlen(int x1, int y1, int x2, int y2) {
    return runpath(0, x1, y1, x2, y2, 0);
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
    float travel_time;
    if (x < 0 || x >= global_map->dim || y < 0 || y >= global_map->dim) {
        sprintf(out, "PRIVMSG %s :Invalid location, this map is %dx%d\r\n", message->receiver, global_map->dim, global_map->dim);
        add_msg(out, strlen(out));
        return;
    }
    if(is_obstructed(x, y)) {
        snprintf(out, MAX_MESSAGE_BUFFER,"PRIVMSG %s :(%d,%d) is obstructed :(\r\n", message->receiver, x, y);
        add_msg(out, strlen(out));
        return;
    }

    int cx = b->players[pindex].pos_x;
    int cy = b->players[pindex].pos_y;

    travel_time = pathlen(cx, cy, x, y)*TRAVEL_TIME_MULT;
    if(travel_time < 0) {
        snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :%s, (%d,%d) cannot be reached :(\r\n", message->receiver, message->sender_nick, x, y);
        add_msg(out, strlen(out));
        return;
    }
    if((unsigned int)travel_time >= (unsigned int)((((unsigned int)1<<(sizeof(unsigned int) * 8 - 1))-1)/TRAVEL_TIME_MULT)) {
        snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :%s, you cannot travel that far! It would take %.2f hours!\r\n", message->receiver, message->sender_nick, travel_time/60/60);
        add_msg(out, strlen(out));
        return;
    }
    add_event(TRAVEL, pindex, (unsigned int)travel_time, UNIQUE);
    b->players[pindex].travel_timer.pos.x = x;
    b->players[pindex].travel_timer.pos.y = y;
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

