#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "include/status.h"
#include "include/network.h"
#include "include/player.h"
#include "include/limits.h"
#include "include/colors.h"
#include "include/parse.h"
#include "include/inventory.h"
#include "include/items.h"
#include "include/util.h"

#define COMMON    65.00f //65%
#define UNCOMMON  85.00f //20%
#define RARE      90.00f //5%
#define MYTHICAL  93.00f //3%
#define EPIC      94.00f //1%
#define LEGENDARY 94.75f //0.75%
#define GODLY     95.00f //0.25%
#define FORSAKEN  95.02f //0.02%

void generate_drop(struct Message *message) {
    char out[MAX_MESSAGE_BUFFER];
    const char *rarity[]  = {NULL, "Common ", "Uncommon ", "Rare ", "Mythical ", "Epic ", "Legendary ", "Godly ", "Forsaken "};
    const char *weapons[] = {"Sword", "Mace", "Scimitar", "Axe", "Club", "Staff", "Wand", "Pike", "Polearm", "Lance", "Spear", "Floppy Dildo"};
    const char *shields[] = {"Tower Shield", "Round Shield", "Small Shield", "Large Shield", "Medium Shield"};
    const char *armor[]   = {"Leather Armor", "Breastplate Armor", "Fullplate Armor", "Chainmail Armor", "Cloth Armor"};
    char item_name[200];
    item_name[0] = '\0';

    float rarity_chance = rand()/(RAND_MAX/100.0f);
    int type_chance     = rand()%MAX_ITEM_TYPE;
    int p_index = get_pindex(message->sender_nick);
    int drop_level = dawn->global_monster.drop_level;
    int str, def, intel, mdef, hp;
    str = def = intel = mdef = hp = 0;

    if (dawn->players[p_index].available_slots == 0) {
        sprintf(out, "PRIVMSG %s :%s has found an item, but has no more room in his inventory!\r\n",
                message->receiver, message->sender_nick);
        add_msg(out, strlen(out));
        return;
    }

    struct Inventory item_dropped;
    item_dropped.type = type_chance;

    if (rarity_chance <= COMMON) {
        item_dropped.rarity = 1;
    } else if (rarity_chance <= UNCOMMON) {
        strcat(item_name, IRC_CYAN);
        item_dropped.rarity = 2;
    } else if (rarity_chance <= RARE) {
        strcat(item_name, IRC_ORANGE);
        item_dropped.rarity = 3;
    } else if (rarity_chance <= MYTHICAL) {
        strcat(item_name, IRC_PURPLE);
        item_dropped.rarity = 4;
    } else if (rarity_chance <= EPIC) {
        strcat(item_name, IRC_DBLUE);
        item_dropped.rarity = 5;
    } else if (rarity_chance <= LEGENDARY) {
        strcat(item_name, IRC_BROWN);
        item_dropped.rarity = 6;
    } else if (rarity_chance <= GODLY) {
        strcat(item_name, IRC_BOLD);
        strcat(item_name, IRC_PINK);
        item_dropped.rarity = 7;
    } else if (rarity_chance <= FORSAKEN) {
        strcat(item_name, IRC_BOLD);
        strcat(item_name, IRC_UNDRL);
        strcat(item_name, IRC_RED);
        item_dropped.rarity = 8;
    } else {
        enum material {WOOD, LEATHER, STONE, ORE, BRONZE, MAIL, STEEL, DIAMOND};
        char *mat_types[] = {"wood", "leather", "stone", "ore", "bronze", "mail", "steel", "diamond"};
        char plural[7], mat_name[10];
        int type, amount;

        if (drop_level < 4)       type = rand() % 2;
        else if (drop_level < 6)  type = rand() % 3;
        else if (drop_level < 8)  type = rand() % 4;
        else if (drop_level < 10) type = rand() % 6;
        else if (drop_level < 11) type = rand() % 7;
        else                      type = rand() % MAX_MATERIAL_TYPE;

        amount = 1 + rand() % 4;
        amount > 1 ? strcpy(plural, "scraps") : strcpy(plural, "scrap");
        strcpy(mat_name, mat_types[type]);
        dawn->players[p_index].materials[type] += amount;
/*
        switch (type) {
            case WOOD:    dawn->players[p_index].wood    += amount; break;
            case LEATHER: dawn->players[p_index].leather += amount; break;
            case STONE:   dawn->players[p_index].stone   += amount; break;
            case ORE:     dawn->players[p_index].ore     += amount; break;
            case BRONZE:  dawn->players[p_index].bronze  += amount; break;
            case MAIL:    dawn->players[p_index].mail    += amount; break;
            case STEEL:   dawn->players[p_index].steel   += amount; break;
            case DIAMOND: dawn->players[p_index].diamond += amount; break;
        }
*/
        sprintf(out, "PRIVMSG %s :%s finds %d %s %s\r\n",
                message->receiver, message->sender_nick, amount, mat_name, plural);
        add_msg(out, strlen(out));

        return;
    }

    strcat(item_name, rarity[item_dropped.rarity]);

    switch (type_chance) {
        case 0:
        {
            char weapon_type[48];
            str = 1 + rand() % ((drop_level * dawn->players[p_index].level) * 2 * item_dropped.rarity);
            intel = 1 + rand() % ((drop_level * dawn->players[p_index].level) * 2 * item_dropped.rarity);
            def   = 1 + rand() % str;
            mdef  = 1 + rand() % str;
            strcpy(weapon_type, weapons[rand()%MAX_WEAPON_TYPE]);
            strcat(item_name, weapon_type);
            break;
        }
        case 1:
        {
            char shield_type[48];
            def = 1 + rand() % ((drop_level * dawn->players[p_index].level) * 2 * item_dropped.rarity);
            intel = 1 + rand() % def;
            str   = 1 + rand() % def;
            mdef = 1 + rand() % ((drop_level * dawn->players[p_index].level) * 2 * item_dropped.rarity);
            strcpy(shield_type, shields[rand()%MAX_SHIELD_TYPE]);
            strcat(item_name, shield_type);
            break;
        }
        case 2:
        {
            char armor_type[48];
            def = 1 + rand() % ((drop_level * dawn->players[p_index].level) * 2 * item_dropped.rarity);
            intel = 1 + rand() % def;
            str   = 1 + rand() % def;
            mdef = 1 + rand() % ((drop_level * dawn->players[p_index].level) * 2 * item_dropped.rarity);
            strcpy(armor_type, armor[rand()%MAX_ARMOR_TYPE]);
            strcat(item_name, armor_type);
            break;
        }
        default:
            PRINTERR("ITEM ERROR")
    }

    if (item_dropped.rarity >= 3) {
        hp = 1 + rand() % ((drop_level * dawn->players[p_index].level) * 2 * item_dropped.rarity);
    }

        
    strcat(item_name, IRC_NORMAL);
    //strcpy(item_name, item_name); programming drunk. not even once.
    size_t len = strlen(item_name);
    item_name[len+1] = '\0';

    strcpy(item_dropped.name, item_name);

    item_dropped.attr_strength     = str;
    item_dropped.attr_defense      = def;
    item_dropped.attr_intelligence = intel;
    item_dropped.attr_mdef         = mdef;
    item_dropped.attr_health = hp;
    item_dropped.req_level = 1;
    item_dropped.weight = 5;
    item_dropped.socket_one = 0;
    item_dropped.socket_two = 0;
    item_dropped.socket_three = 0;
    item_dropped.attr_mana = 0;

    //see inventory.h
    item_dropped.bitfield &= ~(1 << RUSTED);     //Not rusted
    item_dropped.bitfield &= ~(1 << EQUIPPED);   //Not equipped
    item_dropped.bitfield |=  (1 << EQUIPPABLE); //Equippable
    item_dropped.bitfield |=  (1 << NEW);        //New
    item_dropped.bitfield &= ~(1 << FAVORITE);  //Not favorite


    for (int i=0; i<MAX_INVENTORY_SLOTS; i++) {
        printf("%d. '%s'\n", i, dawn->players[p_index].inventory[i].name);
        if (strlen(dawn->players[p_index].inventory[i].name) < 2) {
            if (dawn->players[p_index].available_slots > 0) {
                dawn->players[p_index].inventory[i] = item_dropped;
                dawn->players[p_index].available_slots--;
                printf("%s got %s %d slots left\n", message->sender_nick, item_dropped.name, dawn->players[p_index].available_slots);
                break;
            } else {
                sprintf(out, "PRIVMSG %s :%s, you have no room in your inventory!\r\n",
                        message->receiver, message->sender_nick);
                add_msg(out, strlen(out));
                return;
            }
        } else {
            printf("wat?\n");
        }
    }

    sprintf(out, "PRIVMSG %s :%s has found a %s [S: %u - D: %u I: %u - MD: %u] on the corpse of the monster!\r\n",
            message->receiver, message->sender_nick, item_name, str, def, intel, mdef);
    add_msg(out, strlen(out));
}

void drop_item(struct Message *message, int slot) {
    char out[MAX_MESSAGE_BUFFER];
    char item_name[128];
    int pindex = get_pindex(message->sender_nick);
    int total_items = MAX_INVENTORY_SLOTS - dawn->players[pindex].available_slots;
    static const struct Inventory empty;

    if (is_favorite(pindex, slot)) {
        sprintf(out, "PRIVMSG %s :%s, %s is a favorite item and can not be dropped\r\n",
                dawn->active_room, dawn->players[pindex].username, dawn->players[pindex].inventory[slot].name);
        add_msg(out, strlen(out));
        return;
    }

    item_name[0]   = '\0';
    strcpy(item_name, dawn->players[pindex].inventory[slot].name);

    if (total_items > 0 && slot < total_items && slot > -1) {
        int last_index = total_items - 1;
        if (slot == last_index) {
            dawn->players[pindex].inventory[slot] = empty;
        } else {
            for (int i=slot; i<last_index; i++) {
                dawn->players[pindex].inventory[i] = dawn->players[pindex].inventory[i + 1];
            }
            dawn->players[pindex].inventory[last_index] = empty;
        }
        dawn->players[pindex].available_slots++;
        sprintf(out, "PRIVMSG %s :%s has dropped the %s\r\n", message->receiver, message->sender_nick, item_name);
        add_msg(out, strlen(out));
    }
}

void get_item_info(struct Message *message, int slot) {
    char item_name[128];
    char out[MAX_MESSAGE_BUFFER];
    int index = get_pindex(message->sender_nick);
    int total_items = MAX_INVENTORY_SLOTS - dawn->players[index].available_slots;
    int str    = dawn->players[index].inventory[slot].attr_strength;
    int intel  = dawn->players[index].inventory[slot].attr_intelligence;
    int def    = dawn->players[index].inventory[slot].attr_defense;
    int mdef   = dawn->players[index].inventory[slot].attr_mdef;
    int hp     = dawn->players[index].inventory[slot].attr_health;
    int mp     = dawn->players[index].inventory[slot].attr_mana;
    int weight = dawn->players[index].inventory[slot].weight;
    int reqlvl = dawn->players[index].inventory[slot].req_level;

    if (slot < total_items && slot > -1) {
        strcpy(item_name, dawn->players[index].inventory[slot].name);
        sprintf(out, "PRIVMSG %s :%s - STR: %d - DEF: %d - INT: %d - MDEF: %d - HP: %d - MP: %d"
               " - Weight: %d - Req lvl: %d\r\n", message->receiver, item_name, str, def, intel, mdef, hp, mp, weight, reqlvl);
        add_msg(out, strlen(out));
    }
}
