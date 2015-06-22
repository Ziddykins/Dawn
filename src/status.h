#ifndef STATUS_H_INCLUDED
#define STATUS_H_INCLUDED
#include <time.h>
#include "limits.h"
#include "monsters.h"
#include "map.h"

struct Map;

struct Timers {
    time_t time_finished;
};

struct Inventory {
   char name[100];
   unsigned int attr_health, attr_defense, attr_intelligence, attr_strength;
   unsigned int attr_mdef, req_level, weight;
   unsigned int socket_one, socket_two, socket_three;
   unsigned int type, rusted, equipped, equippable;
   unsigned int attr_mana, rarity;
};

struct Player {
    struct Inventory inventory[MAX_INVENTORY_SLOTS];
    struct Monsters personal_monster;
    struct Map current_map;
    char username[64], password[64];
    char first_class[64], second_class[64], title[64];
    long stone, steel, wood, ore, bronze, diamond, mail, leather, health;
    long kills, deaths, gold, experience;
    short addiction, x_pos, y_pos, fullness;
    unsigned int alive, available, level, contribution, max_health, max_mana;
    unsigned int available_slots, available_capacity;
    unsigned int mana, strength, intelligence, defense, m_def, alignment;
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

//Prototypes
void set_timer (int, struct Bot *, time_t);
void check_timers (struct Bot *);
void init_timers (struct Bot *);
void call_monster (struct Bot *, char [], int);
void print_location (struct Bot *, int);
struct Map set_map (int);

#endif
