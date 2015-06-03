#ifndef STATUS_H_INCLUDED
#define STATUS_H_INCLUDED
#include "limits.h"

typedef struct {
    int time_finished;
} Timers;

typedef struct {
   int attr_health, attr_defense, attr_intelligence, attr_strength;
   int attr_mdef, req_level, weight;
   int socket_one, socket_two, socket_three;
   int type, rusted, equipped, equippable;
   char name[100];
   int attr_mana;
} Inventory;

typedef struct {
    Inventory inventory[25];
    char username[64], password[64];
    char first_class[64], second_class[64], title[64];
    long stone, steel, wood, ore, bronze, diamond, mail, leather, health;
    short addiction, x_pos, y_pos, hunger;
    int alignment, max_health, max_mana;
    unsigned int alive, available, level, contribution;
    long kills, deaths, gold, experience;
    int mana, strength, intelligence, defense, m_def;
    unsigned int available_slots, available_capacity;
} Player;

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
             SAVING,
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
