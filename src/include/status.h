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

extern struct Bot * dawn;

//Prototypes
void set_timer (int, time_t);
void init_timers (struct Bot *, const char *);
void print_location (struct Bot *, int);
struct Map set_map (int);

struct Timers {
    time_t time_finished;
};

struct Bot {
    char nickname[64], realname[64], ident[64], password[64], active_room[64];
    int login_sent, in_rooms, player_count, weather;
    struct Player players[100];
    struct Monsters monsters[MAX_MONSTERS];
    struct Monsters global_monster;

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

struct eventNode {
    struct eventNode * next;
    struct event * elem;
    time_t event_time;
};

struct eventList {
    struct eventNode * head;
    size_t len;
};

enum eventMode {
    NORMAL = 0,
    UNIQUE = 1<<0,
    KEEP  = 1<<1
};

char * eventToStr(enum Events x);

typedef void * EventList;

EventList createEventList(void);
void deleteEventList(void);
void addEvent(enum Events event, int eData, unsigned int offset, int flags);
void removeEvent(struct eventNode * prev);
void selectList(EventList);
void printNextEvent(void);
void printFromNode(struct eventNode * x);
void printList(void);

struct event * retrEvent(void);
void updateAlarm(void);
time_t timeToNextMsg(void);

size_t listLen(void);

int nextIsDue(void);
void eventHandler(int sig);

void save_events(char const *);
void load_events(char const *);

#endif
