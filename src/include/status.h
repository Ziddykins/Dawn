#ifndef STATUS_H_INCLUDED
#define STATUS_H_INCLUDED

#include <time.h>
#include "limits.h"
#include "player.h"
#include "monsters.h"
#include "bounty.h"

extern struct Bot * dawn;

//Prototypes
void init_timers (struct Bot *, const char *);

struct Market {
    int materials[MAX_MATERIAL_TYPE];
    int prevprice[MAX_MATERIAL_TYPE];
};

struct Bot {
    char nickname[64], realname[64], ident[64], password[64], active_room[64];
    int login_sent, in_rooms, player_count, weather;
    long long lottery;
    struct Bounty gbounty;
    struct Player players[100];
    struct Monsters monsters[MAX_MONSTERS];
    struct Monsters global_monster;
    struct Market market;
};

enum Events  {
    HEALING, SAVING, HOURLY,
    TRAVEL, MSGSEND,
    LOTTERY_COLLECT,
    LOTTERY_REWARD,
    BOUNTY_RESET,
    BOUNTY_INIT,
};

enum Weather {
    SUNNY, RAINING, SNOWING,
};

struct event {
    enum Events event;
    int data; //playerID
};

struct event_node {
    struct event_node * next;
    struct event * elem;
    time_t event_time;
};

struct event_list {
    struct event_node * head;
    size_t len;
};

enum event_mode {
    NORMAL = 0,
    UNIQUE = 1<<0,
    KEEP  = 1<<1
};

char * event_to_str(enum Events x);

typedef struct event_list * EventList;

EventList init_event_list(void);
void free_event_list(void);
void add_event(enum Events event, int e_data, unsigned int offset, int flags);
void remove_event(struct event_node * prev);
void select_list(EventList);

struct event * retr_event(void);
void update_alarm(void);

int is_next_due(void);
void event_handler(int sig);

void save_events(char const *);
void load_events(char const *);

#endif
