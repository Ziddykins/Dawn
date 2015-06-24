#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "status.h"
#include "player.h"
#include "network.h"
#include "stats.h"
#include "colors.h"
#include "inventory.h"

//Prototypes
void save_players (struct Bot *, size_t);
long int get_nextlvl_exp (struct Bot *, const char []);

int get_pindex (struct Bot *dawn, const char username[64]) {
    for (int i=0; i<dawn->player_count; i++) {
        if (strcmp(dawn->players[i].username, username) == 0)
            return i;
    }
    return -1;
}


void init_new_character (const char username[64], const char password[64], struct Bot *dawn) {
    //Check if user exists
    char out[MAX_MESSAGE_BUFFER];

    if (get_pindex(dawn, username) != -1) {
        sprintf(out, "PRIVMSG %s :You already have an account!\r\n", dawn->active_room);
        send_socket(out);
        return;
    }
            
    struct Player np;

    np.current_map = set_map(0);
    np.current_map.cur_x = 0;
    np.current_map.cur_y = 1;

    strcpy(np.username, username);
    strcpy(np.password, password);
    strcpy(np.first_class, "None");
    strcpy(np.second_class, "None");
    strcpy(np.title, "Newbie");
    np.stone        = 0;
    np.steel        = 0;
    np.wood         = 0;
    np.ore          = 0;
    np.bronze       = 0;
    np.diamond      = 0;
    np.mail         = 0;
    np.leather      = 0;
    np.health       = 100;
    np.addiction    = 0;
    np.x_pos        = 1;
    np.y_pos        = 1;
    np.fullness     = 100;
    np.alignment    = 0;
    np.alive        = 1;
    np.available    = 1;
    np.level        = 1;
    np.contribution = 0;
    np.max_health   = 100;
    np.max_mana     = 100;
    np.kills        = 0;
    np.deaths       = 0;
    np.gold         = 100;
    np.experience   = 0;
    np.mana         = 100;
    np.strength     = 5;
    np.intelligence = 5;
    np.defense      = 5;
    np.m_def        = 5;
    np.available_slots = 23;
    np.available_capacity = 70;

    for (int i=0; i<MAX_INVENTORY_SLOTS; i++) {
        np.inventory[i].name[0] = '\0';
        np.inventory[i].attr_health = np.inventory[i].attr_defense = np.inventory[i].attr_intelligence = 
            np.inventory[i].attr_strength = np.inventory[i].attr_mdef = np.inventory[i].req_level = 
            np.inventory[i].weight = np.inventory[i].socket_one = np.inventory[i].socket_two =
            np.inventory[i].socket_three = np.inventory[i].type = np.inventory[i].rusted = np.inventory[i].equipped =
            np.inventory[i].equippable = np.inventory[i].attr_mana = np.inventory[i].rarity = 0;
    }

    //name, health, def, int, str, mdef, req lvl, weight, s1, s2, s3
    //type, rusted, equipped, equippable, mana, rarity
    struct Inventory sword  = {"Wooden Sword", 0, 0, 0, 5, 0, 1, 15, 0, 0, 0, 0, 0, 0, 1, 0, 0};
    struct Inventory shield = {"Wooden Shield", 5, 5, 5, 5, 5, 1, 15, 0, 0, 0, 1, 0, 0, 1, 5, 0};
    np.inventory[0] = sword;
    np.inventory[1] = shield;

    
    dawn->players[dawn->player_count] = np;
    dawn->player_count++;
    sprintf(out, "PRIVMSG %s :Account created for user %s\r\n", dawn->active_room, username);

    struct Bot temp;
    save_players (dawn, sizeof(temp));
    send_socket(out);
}

void save_players (struct Bot *dawn, size_t size) {
    FILE *file = fopen("players.db", "wb");
    if (file != NULL) {
        fwrite(dawn, size, 1, file);
        fclose(file);
        printf("Wrote %zu bytes to players.db\r\n", size);
    }
}

void load_players (struct Bot *dawn, size_t size) {
    FILE *file = fopen("players.db", "rb");
    if (file != NULL) {
        fread(dawn, size, 1, file);
        fclose(file);
        printf("loaded %zu bytes from players.db\n", size);
    }
}

//Lol this entire thing
static const char *progress_bar (struct Bot *dawn, const char username[64]) {
    static char bar[48];
    int i = get_pindex(dawn, username);
    long nextlvl_exp = get_nextlvl_exp(dawn, username);
    float temp_cyan = ((dawn->players[i].experience / (float)nextlvl_exp) * 100) / 10;
    int blue_count  = 9  - (int)temp_cyan;
    int cyan_count  = 10 - blue_count;
    sprintf(bar, "%s,10%0*d%s,02%0*d%s", cyan, cyan_count, 0, dblue, blue_count, 0, normal);
    return bar;
}

long int get_nextlvl_exp (struct Bot *dawn, const char username[64]) {
    int curr_level = dawn->players[get_pindex(dawn, username)].level;
    if (curr_level > 10) {
        return 500 * curr_level * curr_level * curr_level - 500 * curr_level;
    } else {
        return 100 * curr_level * curr_level * curr_level - 100 * curr_level;
    }
}

void print_sheet (struct Bot *dawn, struct Message *message) {
    int i = get_pindex(dawn, message->sender_nick);
    char out[MAX_MESSAGE_BUFFER];
    int stats[6] = { dawn->players[i].max_health, dawn->players[i].max_mana,
                     dawn->players[i].strength, dawn->players[i].intelligence,
                     dawn->players[i].m_def, dawn->players[i].defense };
    get_stat(dawn, message, stats);

    sprintf(out, 
            "PRIVMSG %s :[%s (%d)] [%ld/%d \0034HP\003] - [%d/%d \00310MP\003] Str: %d - Int: %d - MDef: %d"
            " - Def: %d (%ldK/%ldD) [EXP: %ld/%ld %s - Gold: %s%ld%s] [Fullness: %d%%]\r\n", message->receiver, message->sender_nick, 
            dawn->players[i].level, dawn->players[i].health, stats[0], dawn->players[i].mana, stats[1], 
            stats[2], stats[3], stats[4], stats[5], dawn->players[i].kills, dawn->players[i].deaths, 
            dawn->players[i].experience, get_nextlvl_exp(dawn, dawn->players[i].username),
            progress_bar(dawn, message->sender_nick), orange, dawn->players[i].gold, normal,
            dawn->players[i].fullness);

    send_socket(out);
    return;
}

void check_levelup (struct Bot *dawn, struct Message *message) {
    int i = get_pindex(dawn, message->sender_nick);
    char out[MAX_MESSAGE_BUFFER];
    long int next_level_exp = get_nextlvl_exp(dawn, message->sender_nick);
    long int curr_level_exp = dawn->players[i].experience;
    int curr_level = dawn->players[i].level;

    if (curr_level_exp >= next_level_exp) {
        sprintf(out, "PRIVMSG %s :%s has achieved level %d. Base stats increased +5, HP and MP"
                " increased +20!\r\n", message->receiver, message->sender_nick, curr_level + 1);
        send_socket(out);
        dawn->players[i].level++;
        dawn->players[i].strength     += 5;
        dawn->players[i].intelligence += 5;
        dawn->players[i].defense      += 5;
        dawn->players[i].m_def        += 5;
        dawn->players[i].max_health   += 20;
        dawn->players[i].max_mana     += 20;
        dawn->players[i].health        = dawn->players[i].max_health;
        dawn->players[i].mana          = dawn->players[i].max_mana;
    }
}
