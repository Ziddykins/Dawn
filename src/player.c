#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "player.h"
#include "status.h"
#include "network.h"
#include "stats.h"

void init_new_character (char username[64], char password[64], Bot *dawn, Message *message) {
    //Check if user exists
    int i;
    char out[MAX_MESSAGE_BUFFER];
    for (i=0; i<=dawn->player_count; i++) {
        if (strcmp(dawn->players[i].username, username) == 0) {
            sprintf(out, "PRIVMSG %s :You already have an account!\r\n", message->receiver);
            send_socket(out);
            return;
        }
    }
            
    Player np;
    strcpy(np.username, username);
    strcpy(np.password, password);
    strcpy(np.first_class, "None");
    strcpy(np.second_class, "None");
    strcpy(np.title, "Newbie");
    np.stone = 0;
    np.steel = 0;
    np.wood  = 0;
    np.ore   = 0;
    np.bronze = 0;
    np.diamond = 0;
    np.mail = 0;
    np.leather = 0;
    np.health = 100;
    np.addiction = 0;
    np.x_pos = 1;
    np.y_pos = 1;
    np.hunger = 0;
    np.alignment = 0;
    np.alive = 1;
    np.available = 1;
    np.level = 1;
    np.contribution = 0;
    np.max_health = 100;
    np.max_mana = 100;
    np.kills = 0;
    np.deaths = 0;
    np.gold = 100;
    np.experience = 0;
    np.mana = 100;
    np.strength = 5;
    np.intelligence = 5;
    np.defense = 5;
    np.m_def = 5;
    np.available_slots = 23;
    np.available_capacity = 70;
    //health, def, int, str, mdef, req lvl, weight, s1, s2, s3
    //type, rusted, equipped, equippable, name, mana
    Inventory sword  = {0, 0, 0, 5, 0, 1, 15, 0, 0, 0, 1, 0, 0, 1, "Wooden Sword", 0};
    Inventory shield = {5, 5, 5, 5, 5, 1, 15, 0, 0, 0, 2, 0, 0, 1, "Wooden Shield", 5};
    np.inventory[0] = sword;
    np.inventory[1] = shield;
    
    dawn->players[dawn->player_count] = np;
    dawn->player_count++;
    sprintf(out, "PRIVMSG %s :Account created for user %s\r\n", message->receiver, username);
    send_socket(out);
}

void save_players (Bot *dawn, size_t size) {
    FILE *file = fopen("players.db", "wb");
    if (file != NULL) {
        fwrite(dawn, size, 1, file);
        fclose(file);
        printf("Wrote %zu bytes to players.db\r\n", size);
    }
}

void load_players (Bot *dawn, size_t size) {
    FILE *file = fopen("players.db", "rb");
    if (file != NULL) {
        fread(dawn, size, 1, file);
        fclose(file);
        printf("loaded %zu bytes from players.db\n", size);
    }
}

void print_sheet (Bot *dawn, Message *message) {
    int i;
    char out[MAX_MESSAGE_BUFFER];
    for (i=0; i<dawn->player_count; i++) {
        printf("at user %s\n", dawn->players[i].username);
        if (strcmp(dawn->players[i].username, message->sender_nick) == 0) {
            int stats[6] = { dawn->players[i].max_health, dawn->players[i].max_mana,
                             dawn->players[i].strength, dawn->players[i].intelligence,
                             dawn->players[i].m_def, dawn->players[i].defense };
            get_stat(dawn, message, stats);
            sprintf(out, 
                    "PRIVMSG %s :[%s] [%ld/%d \0034HP\003] - [%d/%d \00310MP\003] Str: %d - Int: %d - MDef: %d"
                    " - Def: %d\r\n", message->receiver, message->sender_nick, dawn->players[i].health,
                    stats[0], dawn->players[i].mana, stats[1], stats[2], stats[3], stats[4], stats[5]);
            send_socket(out);
            return;
        }
    }
}
