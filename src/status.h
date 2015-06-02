#ifndef STATUS_H_INCLUDED
#define STATUS_H_INCLUDED
#include "player.h"
#include "limits.h"


typedef struct {
    int time_finished;
} Timers;

typedef struct {
    char nickname[64];
    char realname[64];
    char ident[64];
    char password[64];
    char active_room[64];
    int login_sent;
    int in_rooms;
    int player_count;
    int weather;
    Player players[100];
    Timers timer[MAX_TIMERS];
} Bot;


enum Events {WEATHER,
             HEALING,
             BATTLE
};

enum Weather {SUNNY,
              RAINING,
              SNOWING
};

//Prototypes
void set_timers (int, Bot *, int);
void check_timers(Bot *);
void init_timers(Bot *);

#endif
