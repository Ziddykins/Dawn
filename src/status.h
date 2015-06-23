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
void call_monster (struct Bot *, char [], int);
void print_location (struct Bot *, int);
struct Map set_map (int);
//

struct Timers {
    time_t time_finished;
};

struct Bot {
    char nickname[64];
    char realname[64];
    char ident[64];
    char password[64];
    char active_room[64];
    int login_sent;
    int in_rooms;
    int player_count;
    int weather;
    struct Player players[100];
    struct Timers timer[MAX_TIMERS];
    struct Monsters monsters[MAX_MONSTERS];
    struct Monsters global_monster;
};

enum Events  {HEALING, SAVING, HOURLY};
enum Weather {SUNNY, RAINING, SNOWING};


#endif
