#ifndef PLAYER_H_INCLUDED
#define PLAYER_H_INCLUDED

#include <openssl/sha.h>

#include "limits.h"
#include "inventory.h"
#include "monsters.h"
#include "map.h"
#include "spells.h"

// Prototypes
void save_players(void);

void load_players(void);

void print_sheet(struct Message *);

void init_new_character(struct Message *);

void check_levelup(struct Message *);

void assign_attr_points(struct Message *, char *, int);

void revive(struct Message *);

unsigned long long get_nextlvl_exp(char const *username);

int get_pindex(const char *);

int get_bindex(struct Bot *, char const *, char const *);

enum auth_level
{
    AL_NOAUTH,
    AL_USER,
    AL_REG,
    AL_ADMIN, // requires AL_REG
    AL_ROOT   // requires auth key
};

char *auth_level_to_str(int al); // enum auth_level
enum auth_level str_to_auth_level(char *al);

extern char *auth_key;
extern int auth_key_valid;
extern struct Map *global_map;

struct Player {
    char username[64], hostmask[128]; // limits used in parse.c
    char salt[16];                    // limits used in parse.c and player.c
    unsigned char pwd[SHA256_DIGEST_LENGTH];
    char first_class[64], second_class[64], title[64];
    struct Inventory inventory[MAX_INVENTORY_SLOTS];
    struct Monsters personal_monster;
    struct travel_timer travel_timer;
    struct SpellBook spellbook;
    struct Location current_map;

    int pos_x, pos_y, bounty;
    // wood, leather, ore, stone, bronze, mail, steel, diamond
    long materials[8];
    long kills, deaths, gold, health;
    unsigned long long experience;
    int alive, available, level, contribution, max_health, max_mana;
    int available_slots, available_capacity, mana, strength;
    int intelligence, defense, m_def, alignment, attr_pts;
    int cheese;
    unsigned char auth_level, max_auth;
    short fullness;
    char pad[4];
};

#endif
