#ifndef INVENTORY_H_INCLUDED
#define INVENTORY_H_INCLUDED

struct Bot;
struct Message;
enum BitField {NEW, FAVORITE, EQUIPPED, EQUIPPABLE, RUSTED};

struct Inventory {
    char name[100];
    int attr_health, attr_defense, attr_intelligence, attr_strength;
    int attr_mdef, req_level, weight, socket_one, socket_two, socket_three;
    int type, attr_mana, rarity;

    //This will be used to store multiple values of settings
    //which can only be 0 or 1, listed in the BitField enum
    //LSB->GSB
    unsigned char bitfield;
};

void print_inventory(struct Message *);
void equip_inventory(struct Message *, int, int, int);
void equip_toggle(int, int);
void acknowledge_new (int, int);
void favorite_toggle (int, int);
int is_equipped(int, int);
int is_rusted(int, int);
int is_equippable(int, int);
int is_favorite(int, int);
int is_new(int, int);
#endif
