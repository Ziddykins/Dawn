#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "status.h"
#include "network.h"
#include "player.h"
#include "limits.h"
#include "colors.h"

#define COMMON    65.00f //65%
#define UNCOMMON  85.00f //20%
#define RARE      90.00f //5%
#define MYTHICAL  93.00f //3%
#define EPIC      94.00f //1%
#define LEGENDARY 94.75f //0.75%
#define GODLY     95.00f //0.25%
#define FORSAKEN  95.02f //0.02%

void generate_drop (Bot *dawn, Message *message) {
    char out[MAX_MESSAGE_BUFFER];
    char *rarity[]  = {"h", "Common ", "Uncommon ", "Rare ", "Mythical ", "Epic ", "Legendary ", "Godly ", "Forsaken "};
    char *weapons[] = {"Sword", "Mace", "Scimitar", "Axe", "Club", "Staff", "Wand"};
    char *shields[] = {"Tower Shield", "Round Shield", "Small Shield", "Large Shield", "Medium Shield"};
    char *armor[]   = {"Leather Armor", "Breastplate Armor", "Fullplate Armor", "Chainmail Armor", "Cloth Armor"};
    char item_name[200];
    item_name[0] = '\0';
    
    float rarity_chance = (float)rand()/(float)(RAND_MAX/100.0f);
    int type_chance     = rand()%ITEM_MAX_TYPE;
    int p_index         = get_pindex(dawn, message->sender_nick);
    int drop_level      = dawn->global_monster.drop_level;
    unsigned int str, def, intel, mdef;

    Inventory item_dropped;
    item_dropped.type = type_chance;

    if (rarity_chance <= COMMON) {
        item_dropped.rarity = 1;
    } else if (rarity_chance <= UNCOMMON) {
        strcat(item_name, cyan);
        item_dropped.rarity = 2;
    } else if (rarity_chance <= RARE) {
        strcat(item_name, orange);
        item_dropped.rarity = 3;
    } else if (rarity_chance <= MYTHICAL) {
        strcat(item_name, purple);
        item_dropped.rarity = 4;
    } else if (rarity_chance <= EPIC) {
        strcat(item_name, dblue);
        item_dropped.rarity = 5;
    } else if (rarity_chance <= LEGENDARY) {
        strcat(item_name, brown);
        item_dropped.rarity = 6;
    } else if (rarity_chance <= GODLY) {
        strcat(item_name, bold);
        strcat(item_name, pink);
        item_dropped.rarity = 7;
    } else if (rarity_chance <= FORSAKEN) {
        strcat(item_name, bold);
        strcat(item_name, undrl);
        strcat(item_name, red);
        item_dropped.rarity = 8;
    } else {
        //will be potions and stuff i guess
        item_dropped.rarity = 1;
    }

    strcat(item_name, rarity[item_dropped.rarity]);

    switch (type_chance) {
        case 0: {
            char weapon_type[48];
            weapon_type[0] = '\0';
            str   = 1 + rand() % ((drop_level * dawn->players[p_index].level) * 2 * item_dropped.rarity);
            intel = 1 + rand() % ((drop_level * dawn->players[p_index].level) * 2 * item_dropped.rarity);
            def   = 1 + rand() % str;
            mdef  = 1 + rand() % str;
            strcat(weapon_type, weapons[rand()%WEAPON_MAX_TYPE]);
            strcat(item_name, weapon_type);
            break;
        }
        case 1: {
            char shield_type[48];
            shield_type[0] = '\0';
            def   = 1 + rand() % ((drop_level * dawn->players[p_index].level) * 2 * item_dropped.rarity);
            intel = 1 + rand() % def;
            str   = 1 + rand() % def;
            mdef  = 1 + rand() % ((drop_level * dawn->players[p_index].level) * 2 * item_dropped.rarity);
            strcat(shield_type, shields[rand()%SHIELD_MAX_TYPE]);
            strcat(item_name, shield_type);
            break;
        }
        case 2: {
            char armor_type[48];
            armor_type[0] = '\0';
            def   = 1 + rand() % ((drop_level * dawn->players[p_index].level) * 2 * item_dropped.rarity);
            intel = 1 + rand() % def;
            str   = 1 + rand() % def;
            mdef  = 1 + rand() % ((drop_level * dawn->players[p_index].level) * 2 * item_dropped.rarity);
            strcat(armor_type, armor[rand()%ARMOR_MAX_TYPE]);
            strcat(item_name, armor_type);
            break;
        }
    }

    strcat(item_name, normal);

    strcpy(item_dropped.name, item_name);
    item_dropped.attr_strength     = str;
    item_dropped.attr_defense      = def;
    item_dropped.attr_intelligence = intel;
    item_dropped.attr_mdef         = mdef;
    item_dropped.attr_health = 0;
    item_dropped.req_level = 1;
    item_dropped.weight = 5;
    item_dropped.socket_one = 0;
    item_dropped.socket_two = 0;
    item_dropped.socket_three = 0;
    item_dropped.rusted = 0;
    item_dropped.equipped = 0;
    item_dropped.equippable = 1;
    item_dropped.attr_mana = 0;

    for (int i=0; i<MAX_INVENTORY_SLOTS; i++) {
        if (strlen(dawn->players[p_index].inventory[i].name) < 1) {
            if (dawn->players[p_index].available_slots > 0) {
                dawn->players[p_index].inventory[i] = item_dropped;
                dawn->players[p_index].available_slots--;
                break;
            } else {
                sprintf(out, "PRIVMSG %s :%s, you have no room in your inventory!\r\n",
                        message->receiver, message->sender_nick);
                send_socket(out);
                return;
            }
        }
    }

    sprintf(out, "PRIVMSG %s :%s has found a %s [S: %u - D: %u I: %u - MD: %u] on the corpse of the monster!\r\n", 
            message->receiver, message->sender_nick, item_name, str, def, intel, mdef);
    send_socket(out);
}

void drop_item (Bot *dawn, Message *message, int slot) {
    char out[MAX_MESSAGE_BUFFER];
    char item_name[128];
    int pindex = get_pindex(dawn, message->sender_nick);
    int total_items = MAX_INVENTORY_SLOTS - dawn->players[pindex].available_slots;
    strcpy(item_name, dawn->players[pindex].inventory[slot].name);
    if (total_items > 0 && slot < total_items && slot > -1) {
        int last_index = total_items - 1;
        for (int i = slot; i<last_index; i++) {
            dawn->players[pindex].inventory[i] = dawn->players[pindex].inventory[i+1];
        }
        dawn->players[pindex].available_slots++;
        sprintf(out, "PRIVMSG %s :%s has dropped the %s\r\n", message->receiver, message->sender_nick, item_name);
        send_socket(out);
    }
}
