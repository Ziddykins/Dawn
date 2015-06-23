#ifndef INVENTORY_H_INCLUDED
#define INVENTORY_H_INCLUDED

struct Bot;
struct Message;

struct Inventory {
   char name[100];
   int attr_health, attr_defense, attr_intelligence, attr_strength;
   int attr_mdef, req_level, weight, socket_one, socket_two, socket_three;
   int type, rusted, equipped, equippable, attr_mana, rarity;
};

void print_inventory (struct Bot *, struct Message *);
void equip_inventory (struct Bot *, struct Message *, int, int);
#endif
