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
    struct Timers timer[MAX_TIMERS];
    struct Monsters monsters[MAX_MONSTERS];
    struct Monsters global_monster;
};

struct event {
    int event;
    int PID; //playerID
};

struct eventNode {
    struct eventNode * next;
    struct event * elem;
    time_t event_time;
};

enum Events  {HEALING, SAVING, HOURLY, SUNNY, RAINING, SNOWING, TRAVEL};

typedef void * EventList;

static EventList elist = 0; //currently selected list (there may only be one at a time)
static struct Bot * bot;

EventList createEventList(void);
void deleteEventList(void);
void addEvent(int event, int playerID, unsigned int offset);
void selectList(EventList);

void printFromNode(struct eventNode * x);
void printList(void);

struct event * retrMsg(void);
void updateAlarm(void);
time_t timeToNextMsg(void);

size_t listLen(void);

int nextIsNow(void);
void eventHandler(int sig);

#endif
