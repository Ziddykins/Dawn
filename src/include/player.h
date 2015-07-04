#ifndef PLAYER_H_INCLUDED
#define PLAYER_H_INCLUDED
#include <openssl/sha.h>
#include "status.h"
#include "network.h"
#include "map.h"
#include "network.h"
#include "stats.h"
#include "colors.h"
#include "inventory.h"
#include "limits.h"
#include "parse.h"

//Prototypes
void save_players (struct Bot *);
void load_players (struct Bot *);
void print_sheet (struct Bot *, struct Message *);
void init_new_character (struct Bot *, struct Message *);
void check_levelup (struct Bot *, struct Message *);
void assign_attr_points (struct Bot *, struct Message *, char [], int);
void revive (struct Bot *, struct Message *);
unsigned long long get_nextlvl_exp (struct Bot *, const char []);
int get_pindex (struct Bot *, const char []);
int get_bindex (struct Bot *, const char [], const char []);

struct Player {
    char username[64], hostmask[128]; //limits used in parse.c
    char salt[16]; //limits used in parse.c and player.c
    unsigned char pwd[SHA256_DIGEST_LENGTH];
    char first_class[64], second_class[64], title[64];
    struct Inventory inventory[MAX_INVENTORY_SLOTS];
    struct Monsters personal_monster;
    struct Map current_map;
    long stone, steel, wood, ore;
    long bronze, diamond, mail, leather;
    long kills, deaths, gold, health;
    unsigned long long experience;
    int alive, available, level, contribution, max_health, max_mana;
    int available_slots, available_capacity, mana, strength;
    int intelligence, defense, m_def, alignment, attr_pts;
    short addiction, x_pos, y_pos, fullness;

    struct TravelTimer travel_timer;
};

#endif
