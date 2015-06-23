#ifndef PLAYER_H_INCLUDED
#define PLAYER_H_INCLUDED
#include "status.h"
#include "network.h"

//Prototypes
void save_players (struct Bot *, size_t);
void load_players (struct Bot *, size_t);
void print_sheet (struct Bot *, struct Message *);
void init_new_character (const char [], const char [], struct Bot *, struct Message *);
void check_levelup (struct Bot *, struct Message *);
long get_nextlvl_exp (struct Bot *, const char []);
int  get_pindex (struct Bot *, const char []);

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

#endif
