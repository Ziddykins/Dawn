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

//Prototypes
void set_timer (int, time_t);
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
    struct Monsters monsters[MAX_MONSTERS];
    struct Monsters global_monster;
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

char * eventToStr(enum Events x);

typedef void * EventList;

EventList createEventList(void);
void deleteEventList(void);
void addEvent(enum Events event, int eData, unsigned int offset, int unique);
void removeEvent(struct eventNode * prev);
void selectList(EventList);

void printFromNode(struct eventNode * x);
void printList(void);

struct event * retrEvent(void);
void updateAlarm(void);
time_t timeToNextMsg(void);

size_t listLen(void);

int nextIsDue(void);
void eventHandler(int sig);

#endif
