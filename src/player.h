#ifndef INCLUDED_H_PLAYER
#define INCLUDED_H_PLAYER

typedef struct {
    unsigned short heal, blizzard, healmore, frost, rain, fireball;
    unsigned short burn, ultimate;
} Spells;

typedef struct {
    int attr_health, attr_defense, attr_intelligence, attr_strength;
    int attr_mdef, req_level, weight;
    int socket_one, socket_two, socket_three;
    int type, rusted, equipped, equippable;
    char name[100];
} Equipment;

typedef struct {
   unsigned int int_potions, def_potions, str_potions, dilaudid;
   unsigned int available_slots, available_capacity;
   Equipment equipment[25];
} Inventory;

typedef struct {
    Inventory inventory;
    char username[64], password[64];
    char first_class[64], second_class[64], title[64];
    long stone, steel, wood, ore, bronze, diamond, mail, leather, health;
    short addiction, x_pos, y_pos, hunger;
    int alignment;
    unsigned int alive, available, level, contribution;
    unsigned long max_health, max_mana, kills, deaths, gold, experience;
    unsigned long mana, strength, intelligence, defense, m_def;
} Player;
#endif
/*b2a=none
property=none
mount=none
bank=0
gps=0
b3b=0
b1c=0
b3c=0
buffed=none
mps=0
ssocket1=0
b3a=none
lps=0
b2b=0
contribution=0
msocket1=0
b1a=none
b1b=0
qcomp=0
b2c=0*/
