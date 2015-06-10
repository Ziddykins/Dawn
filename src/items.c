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
    printf("we in gen drop\n");
    //, unsigned int drop_level used for multplier^
    char *rarity[]  = {"Common ", "Uncommon ", "Rare ", "Mythical ", "Epic ", "Legendary ", "Godly ", "Forsaken "};
    char *type[]    = {"Weapon", "Shield", "Armor"};
    char *weapons[] = {"Sword", "Mace", "Scimitar", "Axe", "Club", "Staff", "Wand"};
    char *shields[] = {"Tower ", "Round ", "Small ", "Large ", "Medium "};
    char *armor[]   = {"Leather ", "Breastplate ", "Fullplate ", "Chainmail ", "Cloth "};
    char item_name[128] = "";
    char out[MAX_MESSAGE_BUFFER];
    
    float rarity_chance = (float)rand()/(float)(RAND_MAX/100.0f);
    int type_chance = rand()%ITEM_MAX_TYPE;
    int p_index = get_pindex(dawn, message->sender_nick);

    Inventory item_dropped;
    item_dropped.type = type_chance;

    if (rarity_chance <= COMMON) {
        item_dropped.rarity = 0;
    } else if (rarity_chance <= UNCOMMON) {
        strcat(item_name, cyan);
        item_dropped.rarity = 1;
    } else if (rarity_chance <= RARE) {
        strcat(item_name, orange);
        item_dropped.rarity = 2;
    } else if (rarity_chance <= MYTHICAL) {
        strcat(item_name, purple);
        item_dropped.rarity = 3;
    }

    strcat(item_name, rarity[item_dropped.rarity]);

    switch (type_chance) {
        case 0: {
            char weapon_type[48];
            strcat(weapon_type, weapons[rand()%WEAPON_MAX_TYPE]);
            strcat(item_name, weapon_type);
            break;
        }
        case 1: {
            char shield_type[48];
            strcat(shield_type, shields[rand()%SHIELD_MAX_TYPE]);
            strcat(item_name, shield_type);
            strcat(item_name, type[type_chance]);
            break;
        }
        case 2: {
            char armor_type[48];
            strcat(armor_type, armor[rand()%ARMOR_MAX_TYPE]);
            strcat(item_name, armor_type);
            strcat(item_name, type[type_chance]);
            break;
        }
    }

    strcat(item_name, normal);

    dawn->players[p_index].inventory[0] = item_dropped;
    sprintf(out, "PRIVMSG %s :%s has found a %s on the corpse of the monsters!\n", message->receiver,
            message->sender_nick, item_name);
    send_socket(out);
}
