#include "include/map.h"
#include "include/util.h"
#include "include/status.h"

#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

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

enum shop_type {
    SHOP_BLACKSMITH,
    SHOP_DOCTOR,
    SHOP_TAILOR,
    //to be extended
};


enum entity_type { //ORDERING IMPORTANT, see rand_ent()
    ENT_START = 0,
    ENT_NONE = 0,
    ENT_TOWN, //add road
    ENT_RESIDENTIAL,
    ENT_STABLES,
    ENT_SHOP,
    ENT_SHRINE,
    ENT_END,
};

struct entity {
    int type;
    union {
        int placeholder;
        //data
    };
};

struct town {
    float matdistr[MAT_COUNT]; //which materials are present in this town
    struct entity **t_ent; //point to m_ent entities
    int entitiy_count;
};

struct Map {
    float * heightmap; //dim*dim
    struct entity *m_ent; //dim*dim
    struct town *towns;
    int town_count;
    float water_level;
    int dim;
    int flags;
};

void init_map(char const * const fn) {
    CALLEXIT(!(global_map = calloc(sizeof *global_map, 1)))
    global_map->dim = 1<<10;
    global_map->flags = 0;

    FILE *file = fopen(fn, "rb");
    if(!file) {
        printf(INFO "Generating new heightmap\n");
        generate_map();
        save_map(fn);
    } else {

        CALLEXIT(!(fread(&global_map->dim, sizeof global_map->dim, 1, file)))
        CALLEXIT(!(fread(&global_map->flags, sizeof global_map->flags, 1, file)))
        CALLEXIT(!(fread(&global_map->water_level, sizeof global_map->water_level, 1, file)))

        int dim = global_map->dim;

        size_t nmemb = (size_t) (dim * dim);
        CALLEXIT(!(global_map->heightmap = malloc(nmemb * sizeof *global_map->heightmap)))
        CALLEXIT(fread(global_map->heightmap, sizeof *global_map->heightmap, nmemb, file) != nmemb)

        nmemb = (size_t) (dim * dim);
        CALLEXIT(!(global_map->m_ent = malloc(nmemb * sizeof *global_map->m_ent)))
        CALLEXIT(fread(global_map->m_ent, sizeof *global_map->m_ent, nmemb, file) != nmemb)

        CALLEXIT(!(fread(&global_map->town_count, sizeof global_map->town_count, 1, file)))
        CALLEXIT(!(global_map->towns = malloc((size_t) (global_map->town_count) * sizeof *global_map->towns)))

        struct location e_pos;

        for (int i = 0; i < global_map->town_count; i++) {
            CALLEXIT(fread(global_map->towns[i].matdistr, sizeof *global_map->towns[i].matdistr, MAT_COUNT, file) !=
                     MAT_COUNT)
            CALLEXIT(
                    !(fread(&(global_map->towns[i].entitiy_count), sizeof global_map->towns[i].entitiy_count, 1, file)))
            CALLEXIT(!(global_map->towns[i].t_ent = malloc(
                    (size_t) (global_map->towns[i].entitiy_count) * sizeof *global_map->towns[i].t_ent)))
            for (int j = 0; j < global_map->towns[i].entitiy_count; j++) {
                CALLEXIT(!(fread(&e_pos, sizeof e_pos, 1, file)))
                global_map->towns[i].t_ent[j] = &global_map->m_ent[e_pos.y * dim + e_pos.x];
            }
        }

        fclose(file);
        printf(INFO "Map loaded\n");
    }
}

static inline struct location entity_location(struct entity *ent) {
    int dim = global_map->dim;
    return (struct location) {
            .x = (int) ((ent - global_map->m_ent) % dim),
            .y = (int) ((ent - global_map->m_ent) / dim)
    };
}

void save_map(char const * const fn) {
    assert(global_map);
    if(global_map->flags & MAP_PRESENT && !(global_map->flags & MAP_SAVED)) {
        FILE *file = fopen(fn, "wb");
        if(!file) {
            PRINTWARN("Could not save heightmap")
            return;
        }
        CALLEXIT(!(fwrite(&global_map->dim, sizeof global_map->dim, 1, file)))
        CALLEXIT(!(fwrite(&global_map->flags, sizeof global_map->flags, 1, file)))
        CALLEXIT(!(fwrite(&global_map->water_level, sizeof global_map->water_level, 1, file)))

        int dim = global_map->dim;

        size_t nmemb = (size_t) (dim * dim);
        CALLEXIT(fwrite(global_map->heightmap, sizeof *global_map->heightmap, nmemb, file) != nmemb)

        nmemb = (size_t) (dim * dim);
        CALLEXIT(fwrite(global_map->m_ent, sizeof *global_map->m_ent, nmemb, file) != nmemb)

        CALLEXIT(!(fwrite(&global_map->town_count, sizeof global_map->town_count, 1, file)))
        for (int i = 0; i < global_map->town_count; i++) {
            CALLEXIT(fwrite(global_map->towns[i].matdistr, sizeof *global_map->towns[i].matdistr, MAT_COUNT, file) !=
                     MAT_COUNT)
            CALLEXIT(!(fwrite(&(global_map->towns[i].entitiy_count), sizeof global_map->towns[i].entitiy_count, 1,
                              file)))
            for (int j = 0; j < global_map->towns[i].entitiy_count; j++) {
                struct location e_pos = entity_location(global_map->towns[i].t_ent[0]);
                CALLEXIT(!(fwrite(&e_pos, sizeof e_pos, 1, file)))
            }
        }


        printf(INFO "Saved map\n");
        fclose(file);
    }
}

static inline int too_close_to_town(int x, int y) {
    for (int i = 0; i < global_map->town_count; i++) {
        if (global_map->towns[i].t_ent[0]) {
            struct location t_pos = entity_location(global_map->towns[i].t_ent[0]);
            if (sqrt(((t_pos.x - x) * (t_pos.x - x)) + ((t_pos.y - y) * (t_pos.y - y))) <= 10) {
                return 1;
            }
        }
    }
    return 0;
}

static inline int is_valid(int x, int y, int dim) {
    return x >= 0 && x < dim && y >= 0 && y < dim;
}

//requires perlin noise
static inline void place_town(int idx) {
    int dim = global_map->dim;
    struct town * _town = &(global_map->towns[idx]);

    _town->entitiy_count = (int)(10 + gaussrand() * 5);
    CALLEXIT(!(_town->t_ent = calloc((size_t) (_town->entitiy_count), sizeof *_town->t_ent)))
    int x = 0, y = 0, counter = 0;
    do {
        x = (int) (randd() * dim);
        y = (int) (randd() * dim);
        counter++;
    } while ((!is_valid(x, y, dim) || is_obstructed(x, y) ||
              too_close_to_town(x, y)) && counter < GENERATION_TRIALS);
    if (counter >= GENERATION_TRIALS) {
        PRINTERR("Map dimensions too small. Retry or increase map size.")
        exit(1);
    }
    _town->t_ent[0] = &global_map->m_ent[y * dim + x];
    _town->t_ent[0]->type = ENT_TOWN;
    for (int i = 0; i < MAT_COUNT; i++) {
        _town->matdistr[i] = noise(x * PERLIN_SCALE, y * PERLIN_SCALE, i * PERLIN_V_SCALE);
    }
}

static inline void rand_ent(struct entity * e) {
    //generate entity
    float probs[] = {
            0.0f,   //0% none
            0.0f,   //0% town
            0.6f, //60% reditential houses
            0.7f, //10% stables
            0.9f, //20% shops
            1.0f, //10% shrines
    };
    double rand_num = randd();
    int ent_type = 0;
    while(probs[ent_type] < rand_num) {
        ent_type++;
    }
    e->type = ent_type;
    switch(ent_type) {
        case ENT_TOWN:
            PRINTERR("Entity generation FATAL")
            exit(1);
        case ENT_RESIDENTIAL:

            break;
        case ENT_SHOP:

            break;
        case ENT_SHRINE:

            break;
        case ENT_STABLES:

            break;
        default:
            PRINTERR("Entity generation FATAL")
            exit(1);
    }
}

//unsafe, use with caution - must store retrieved values elsewhere before appending again
static inline struct location* q_pop(struct location *q, size_t *start, size_t end, size_t len) {
    assert(*start != end);
    struct location *rop = &q[*start];
    *start = ((*start)+1)%len;
    return rop;
}

static inline void q_append(struct location *q, size_t start, size_t *end, size_t len, struct location elem) {
    q[*end].x = elem.x;
    q[*end].y = elem.y;
    (*end) = ((*end)+1)%len;
    assert(*end != start);
}

static inline int queue_overlap(struct location *q, size_t start, size_t end, size_t len, int x, int y) {
    for(size_t i = start; i != end; i = (i+1)%len) {
        if(q[i].x == x && q[i].y == y) {
            return 1;
        }
    }
    return 0;
}

static inline int entity_overlap(int x, int y) {
    return global_map->m_ent[y * global_map->dim + x].type; //non-zero if entity exists
}

//do not inline, essentially a horrible flood-fill
static void grow_town(int idx) {
    struct town * _town = &(global_map->towns[idx]);
    int dim = global_map->dim;
    size_t pos = 0;
    size_t q_size = (size_t) (5 * (_town->entitiy_count + 1));
    struct location *queue;
    CALLEXIT(!(queue = calloc(q_size, sizeof *queue)))
    size_t start = 0, end = 0;

    struct location t_pos = entity_location(_town->t_ent[0]);
    q_append(queue, start, &end, q_size, (struct location) {.x = t_pos.x + 1, .y =  t_pos.y});
    q_append(queue, start, &end, q_size, (struct location) {.x = t_pos.x - 1, .y =  t_pos.y});
    q_append(queue, start, &end, q_size, (struct location) {.x = t_pos.x, .y =  t_pos.y + 1});
    q_append(queue, start, &end, q_size, (struct location) {.x = t_pos.x, .y =  t_pos.y - 1});
    while (pos < (size_t) (_town->entitiy_count)) {
        struct location *cur = q_pop(queue, &start, end, q_size);
        int x = cur->x;
        int y = cur->y;
        _town->t_ent[pos] = &(global_map->m_ent[y * dim + x]);
        rand_ent(_town->t_ent[pos]);

        //check von neumann neighborhood
        int app_x = x+1, app_y = y;
        if (is_valid(app_x, app_y, global_map->dim) && !is_obstructed(app_x, app_y) && !entity_overlap(app_x, app_y) &&
            !queue_overlap(queue, start, end, q_size, app_x, app_y)) {
            q_append(queue, start, &end, q_size, (struct location){.x = app_x, .y = app_y});
        }
        app_x = x-1, app_y = y;
        if (is_valid(app_x, app_y, global_map->dim) && !is_obstructed(app_x, app_y) && !entity_overlap(app_x, app_y) &&
            !queue_overlap(queue, start, end, q_size, app_x, app_y)) {
            q_append(queue, start, &end, q_size, (struct location){.x = app_x, .y = app_y});
        }
        app_x = x, app_y = y+1;
        if (is_valid(app_x, app_y, global_map->dim) && !is_obstructed(app_x, app_y) && !entity_overlap(app_x, app_y) &&
            !queue_overlap(queue, start, end, q_size, app_x, app_y)) {
            q_append(queue, start, &end, q_size, (struct location){.x = app_x, .y = app_y});
        }
        app_x = x, app_y = y-1;
        if (is_valid(app_x, app_y, global_map->dim) && !is_obstructed(app_x, app_y) && !entity_overlap(app_x, app_y) &&
            !queue_overlap(queue, start, end, q_size, app_x, app_y)) {
            q_append(queue, start, &end, q_size, (struct location){.x = app_x, .y = app_y});
        }

        pos++;
    }

    free(queue);
}

void generate_map() {
    FILE *urandom;
    CALLEXIT(!(urandom = fopen("/dev/urandom", "r")))

    unsigned int seed;
    CALLEXIT(!(fread(&seed, sizeof(seed), 1, urandom)))
    srand(seed);
    fclose(urandom);

    global_map->dim = 1 << 10;
    CALLEXIT(!(global_map->heightmap = calloc(sizeof *global_map->heightmap,
                                              (size_t) (global_map->dim * global_map->dim))))
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
    int town_count = (int) (10 + gaussrand() * 3);
    global_map->town_count = 0;
    CALLEXIT(!(global_map->towns = calloc((size_t) (town_count), sizeof *global_map->towns)))
    CALLEXIT(!(global_map->m_ent = calloc((size_t) (dim * dim), sizeof *global_map->m_ent)))
    perlin_init();
    for (int i = 0; i < town_count; i++) {
        place_town(i);
        global_map->town_count++;
    }
    assert(town_count == global_map->town_count);

    for (int i = 0; i < town_count; i++) {
        grow_town(i);
    }
    perlin_cleanup();
}

int is_water(int x, int y) {
    return global_map->heightmap[y*global_map->dim+x] < global_map->water_level;
}

int is_obstructed(int x, int y) { //will also handle rocks or other unreachable area
    return is_water(x,y);
}

//(x2,y2) may only have a distance of sqrt(2) from (x1,y1)
static inline float transfer_cost(int x1, int y1, int x2, int y2) {
    int dim = global_map->dim;
    float dist = ((abs(x2-x1)+abs(y2-y1) < 2) ? 1 : (float)sqrt(2));
    return absf((global_map->heightmap[y1*dim+x1]-global_map->heightmap[y2*dim+x2]))*dist + dist;
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
    for (int i = 0; i < global_map->town_count; i++) {
        free(global_map->towns[i].t_ent);
    }
    free(global_map->towns);
    free(global_map->m_ent);
    free(global_map);
}

void print_location(int i) {
    char out[MAX_MESSAGE_BUFFER];
    sprintf(out, "PRIVMSG %s :%s, you are at %d,%d\r\n", dawn->active_room, dawn->players[i].username,
            dawn->players[i].pos_x, dawn->players[i].pos_y);
    add_msg(out, strlen(out));
}

void move_player(struct Message *message, int x, int y, int teleport) {
    char out[MAX_MESSAGE_BUFFER];
    int pindex = get_pindex(message->sender_nick);
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

    int cx = dawn->players[pindex].pos_x;
    int cy = dawn->players[pindex].pos_y;

    if (is_obstructed(cx, cy)) {
        snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :You're stuck :(\r\n", message->receiver);
        add_msg(out, strlen(out));
        return;
    }

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

    if (teleport) {
        int teleport_cost = (int)(travel_time / 2.0f);
        if (dawn->players[pindex].mana > teleport_cost) {
            dawn->players[pindex].mana -= teleport_cost;
            dawn->players[pindex].pos_x = x;
            dawn->players[pindex].pos_y = y;
            snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :%s, you have teleported to %d,%d, saving you %u travel time!\r\n",
                    message->receiver, message->sender_nick, x, y, (unsigned int)travel_time);
            add_msg(out, strlen(out));
            return;
        } else {
            snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :%s, you do not have the required %u mana to teleport to %d, %d!\r\n",
                    dawn->active_room, message->sender_nick, (unsigned int)(travel_time / 2.0f), x, y);
            add_msg(out, strlen(out));
            return;
        }
    }

    add_event(TRAVEL, pindex, (unsigned int)travel_time, UNIQUE);
    dawn->players[pindex].travel_timer.pos.x = x;
    dawn->players[pindex].travel_timer.pos.y = y;
    dawn->players[pindex].travel_timer.active = 1;

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

