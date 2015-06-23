#ifndef INVENTORY_H_INCLUDED
#define INVENTORY_H_INCLUDED

struct Bot;
struct Message;

struct Inventory {
   char name[100];
   unsigned int attr_health, attr_defense, attr_intelligence, attr_strength;
   unsigned int attr_mdef, req_level, weight;
   unsigned int socket_one, socket_two, socket_three;
   unsigned int type, rusted, equipped, equippable;
   unsigned int attr_mana, rarity;
};

void print_inventory (struct Bot *, struct Message *);
void equip_inventory (struct Bot *, struct Message *, unsigned int, int);
#endif
