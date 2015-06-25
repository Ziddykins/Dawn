#ifndef STATUS_H_INCLUDED
#define STATUS_H_INCLUDED
#include <time.h>
#include "limits.h"
#include "monsters.h"
#include "map.h"
#include "inventory.h"
#include "player.h"

//Prototypes
void set_timer (int, struct Bot *, time_t);
void check_timers (struct Bot *);
void init_timers (struct Bot *);
void print_location (struct Bot *, int);
struct Map set_map (int);

struct Timers {
    time_t time_finished;
};

struct Bot {
    char nickname[64], realname[64], ident[64], password[64], active_room[64];
    int login_sent, in_rooms, player_count, weather;
    struct Player players[100];
    struct Timers timer[MAX_TIMERS];
    struct Monsters monsters[MAX_MONSTERS];
    struct Monsters global_monster;
};

enum Events  {HEALING, SAVING, HOURLY};
enum Weather {SUNNY, RAINING, SNOWING};


#endif
