#ifndef STATUS_H_INCLUDED
#define STATUS_H_INCLUDED
#include <time.h>
#include "limits.h"
#include "monsters.h"

typedef struct {
    time_t time_finished;
} Timers;

typedef struct {
    unsigned int x, y;
} Building;

typedef struct {
    char name[100];
    unsigned int max_x, max_y;
    unsigned int cur_x, cur_y;
    unsigned int exitx, exity;
    Building shop, stable, shrine, gym;
    Building cshop, wepshop, armshop, bank;
    unsigned int min_level;
} Map;

typedef struct {
   char name[100];
   unsigned int attr_health, attr_defense, attr_intelligence, attr_strength;
   unsigned int attr_mdef, req_level, weight;
   unsigned int socket_one, socket_two, socket_three;
   unsigned int type, rusted, equipped, equippable;
   unsigned int attr_mana, rarity;
} Inventory;

typedef struct {
    Inventory inventory[MAX_INVENTORY_SLOTS];
    char username[64], password[64];
    char first_class[64], second_class[64], title[64];
    long stone, steel, wood, ore, bronze, diamond, mail, leather, health;
    long kills, deaths, gold, experience;
    short addiction, x_pos, y_pos, fullness;
    unsigned int alive, available, level, contribution, max_health, max_mana;
    unsigned int available_slots, available_capacity;
    unsigned int mana, strength, intelligence, defense, m_def, alignment;
    Monsters personal_monster;
    Map current_map;
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
    Monsters monsters[MAX_MONSTERS];
    Monsters global_monster;
} Bot;

enum Events  {HEALING, SAVING, HOURLY};
enum Weather {SUNNY, RAINING, SNOWING};

//Prototypes
void set_timer (int, Bot *, time_t);
void check_timers (Bot *);
void init_timers (Bot *);
void call_monster (Bot *, char [], int);
void print_location (Bot *, int);
Map  set_map (int);

#endif
