#ifndef STATUS_H_INCLUDED
#define STATUS_H_INCLUDED
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <assert.h>
#include "limits.h"
#include "monsters.h"
#include "map.h"
#include "inventory.h"
#include "player.h"
#include "persistence.h"
#include "util.h"

extern struct Bot * dawn;

//Prototypes
void set_timer (int, time_t);
void init_timers (struct Bot *, const char *);
void print_location (struct Bot *, int);
struct Map set_map (int);

struct Timers {
    time_t time_finished;
    };

struct Market {
    int materials[MAX_MATERIAL_TYPE];
    int prevprice[MAX_MATERIAL_TYPE];
    };

struct Bot {
    char nickname[64], realname[64], ident[64], password[64], active_room[64];
    int login_sent, in_rooms, player_count, weather;
    struct Player players[100];
    struct Monsters monsters[MAX_MONSTERS];
    struct Monsters global_monster;
    struct Market market;
    int pad; //unused padding data
    };

enum Events  {
    HEALING, SAVING, HOURLY,
    SUNNY, RAINING, SNOWING,
    TRAVEL,
    MSGSEND,
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

typedef void * EventList;

EventList init_event_list(void);
void free_event_list(void);
void add_event(enum Events event, int e_data, unsigned int offset, int flags);
void remove_event(struct event_node * prev);
void select_list(EventList);
void print_next_event(void);
void print_from_node(struct event_node * x);
void print_list(void);

struct event * retr_event(void);
void update_alarm(void);
time_t time_to_next_msg(void);

size_t event_list_len(void);

int is_next_due(void);
void event_handler(int sig);

void save_events(char const *);
void load_events(char const *);

#endif
