#include "include/player.h"

#include "include/status.h"
#include "include/network.h"
#include "include/map.h"
#include "include/network.h"
#include "include/stats.h"
#include "include/colors.h"
#include "include/inventory.h"
#include "include/parse.h"
#include "include/spells.h"
#include "include/util.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//Prototypes
unsigned long long get_nextlvl_exp(const char *);

int get_pindex(char const *username) { //username -> MAX_NICK_LENGTH
    for (int i = 0; i < dawn->player_count; i++) {
        if (strcasecmp(dawn->players[i].username, username) == 0)
            return i;
    }
    return -1;
}

int get_bindex (struct Bot *b, char const * username, char const * location) { //username -> MAX_NICK_LENGTH, location -> 64
    int pindex = get_pindex(username);
    for (int i=0; i<MAX_BUILDINGS; i++) {
        if (strcmp(location, dawn->players[pindex].current_map.buildings[i].name) == 0) {
            return i;
        }
    }
    return -1;
}

void init_new_character(struct Message *message) {
    //Check if user exists
    char out[MAX_MESSAGE_BUFFER];
    char tmp_pwd[] = "temp";

    if (get_pindex(message->sender_nick) != -1) {
        sprintf(out, "PRIVMSG %s :You already have an account!\r\n", dawn->active_room);
        add_msg(out, strlen(out));
        return;
    }

    struct Player np;
    bzero(&np, sizeof np);

    np.pos_x = 0;
    np.pos_y = 1;

    strncpy(np.username, message->sender_nick, MAX_NICK_LENGTH);
    gen_salt(np.salt, 16);
    hash_pwd(np.pwd, np.salt, tmp_pwd);
    strcpy(np.hostmask, message->sender_hostmask);
    strncpy(np.first_class, "None\0", 5);
    strncpy(np.second_class, "None\0", 5);
    strncpy(np.title, "Newbie\0", 7);

    bzero(np.materials, sizeof np.materials);

    np.health       = 100;
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
    np.attr_pts     = 1;
    np.available_slots = 23;
    np.available_capacity = 70;
    np.travel_timer.active = 0;
    np.bounty = 0;
    np.auth_level = AL_USER;
    np.max_auth = AL_USER;
    np.cheese = 1;
    memset(np.inventory, 0, sizeof np.inventory);
    //Name, hp, def, int, str, mdef, reqlvl, weight, soc1,2,3, type, mana, rarity
    struct Inventory sword  = {"Wooden Sword", 0, 0, 0, 5, 0, 1, 15, 0, 0, 0, 0, 0, 0, 1};
    struct Inventory shield = {"Wooden Shield", 5, 5, 5, 5, 5, 1, 15, 0, 0, 0, 1, 5, 0, 1};
    //see inventory.h
    sword.bitfield  &= ~(1 << RUSTED);
    sword.bitfield  &= ~(1 << EQUIPPED);
    sword.bitfield  |=  (1 << EQUIPPABLE);
    shield.bitfield &= ~(1 << RUSTED);
    shield.bitfield &= ~(1 << EQUIPPED);
    shield.bitfield |=  (1 << EQUIPPABLE);
    np.inventory[0] = sword;
    np.inventory[1] = shield;

    dawn->players[dawn->player_count] = np;
    dawn->player_count++;

    sprintf(out, "PRIVMSG %s :Account created for user %s@%s, please set your password by sending me "
                    "a private message containing: ;set password <your_password>\r\n", dawn->active_room,
            message->sender_nick, message->sender_hostmask);
    save_players();
    add_msg(out, strlen(out));
}

void save_players(void) {
    if (dawn == 0)
        return;
    FILE *file = fopen("players.bin", "wb");
    if(!file) {
        PRINTWARN("Could not save players")
        return;
    }
    size_t len;
    CALLEXIT(!(len = fwrite(dawn, sizeof *dawn, 1, file)))
    fclose(file);
    printf(INFO "Saved players (%zu bytes)\n", len * (sizeof *dawn));
}

void load_players(void) {
    FILE *file = fopen("players.bin", "rb");

    if (!file) {
        fprintf(stderr, WARN "Could not load players\n");
        return;
    }

    CALLEXIT(!(fread(dawn, sizeof *dawn, 1, file)))
    fclose(file);
    printf(INFO "Players loaded (%zu bytes)\n", sizeof *dawn);
}

//Lol this entire thing
static const char *progress_bar(char const *username) { //username -> MAX_NICK_LENGTH
    static char bar[48];
    int i = get_pindex(username);
    unsigned long long nextlvl_exp = get_nextlvl_exp(username);
    nextlvl_exp = nextlvl_exp ? nextlvl_exp : 5;
    long double temp_cyan = ((dawn->players[i].experience / (long double) nextlvl_exp) * 100) / 10;
    int blue_count  = 9  - (int)temp_cyan;
    int cyan_count  = 10 - blue_count;
    snprintf(bar, 47, "%s,10%0*d%s,02%0*d%s", IRC_CYAN, cyan_count, 0, IRC_DBLUE, blue_count, 0, IRC_NORMAL);
    return bar;
}

unsigned long long get_nextlvl_exp(char const *username) { //username -> MAX_NICK_LENGTH
    unsigned long long curr_level = (unsigned long long) dawn->players[get_pindex(username)].level;
    if (curr_level > 10) {
        return (500ULL * curr_level * curr_level * curr_level - 500ULL * curr_level);
    } else {
        return (100ULL * curr_level *curr_level * curr_level - 100ULL * curr_level);
    }
}

void print_sheet(struct Message *message) {
    int i = get_pindex(message->sender_nick);
    char out[MAX_MESSAGE_BUFFER];
    int stats[7] = {dawn->players[i].max_health, dawn->players[i].max_mana,
                    dawn->players[i].strength, dawn->players[i].intelligence,
                    dawn->players[i].m_def, dawn->players[i].defense, dawn->players[i].alignment};
    get_stat(message, stats);

    sprintf(out,
            "PRIVMSG %s :[%s (%s%d%s)(Lv: %d)] [%ld/%d \0034HP\003] - [%d/%d \00310MP\003] Str: %d - Int: %d - MDef: %d"
            " - Def: %d (%ldK/%ldD) [EXP: %llu / %llu %s - Gold: %s%ld%s] [Fullness: %hd%%]\r\n", message->receiver, message->sender_nick,
            dawn->players[i].alignment > 25 ? IRC_YELLOW : dawn->players[i].alignment < -25 ? IRC_RED : IRC_NORMAL, stats[6], IRC_NORMAL,
            dawn->players[i].level, dawn->players[i].health, stats[0], dawn->players[i].mana, stats[1],
            stats[2], stats[3], stats[4], stats[5], dawn->players[i].kills, dawn->players[i].deaths,
            dawn->players[i].experience, get_nextlvl_exp(dawn->players[i].username),
            progress_bar(message->sender_nick), IRC_ORANGE, dawn->players[i].gold, IRC_NORMAL,
            dawn->players[i].fullness);

    add_msg(out, strlen(out));
    return;
}

void check_levelup(struct Message *message) {
    int i = get_pindex(message->sender_nick);
    char out[MAX_MESSAGE_BUFFER];
    unsigned long long next_level_exp = get_nextlvl_exp(message->sender_nick);
    unsigned long long curr_level_exp = dawn->players[i].experience;
    int curr_level = dawn->players[i].level;

    if (curr_level_exp >= next_level_exp) {
        dawn->players[i].level++;
        dawn->players[i].max_health += 25;
        dawn->players[i].max_mana += 25;
        dawn->players[i].attr_pts += 20;
        dawn->players[i].health = dawn->players[i].max_health;
        dawn->players[i].mana = dawn->players[i].max_mana;
        check_learn_spells(message->sender_nick);

        if (dawn->players[i].experience > get_nextlvl_exp(message->sender_nick)) {
            check_levelup(message);
            return;
        }

        if (dawn->players[i].experience < get_nextlvl_exp(message->sender_nick)) {
            sprintf(out, "PRIVMSG %s :%s has achieved level %d. 20 attribute points ready for assignment, HP and MP"
                    " increased +25! Increase your attributes using the ;assign command!\r\n",
                    message->receiver, message->sender_nick, curr_level + 1);
            add_msg(out, strlen(out));
        }
    }
}

void assign_attr_points(struct Message *message, char *which, int amount) {
    int pindex = get_pindex(message->sender_nick);
    int attr_pts = dawn->players[pindex].attr_pts;
    char out[MAX_MESSAGE_BUFFER];

    if (strcmp(which, "str") == 0 && attr_pts >= amount) {
        dawn->players[pindex].strength += amount;
    } else if (strcmp(which, "def") == 0 && attr_pts >= amount) {
        dawn->players[pindex].defense += amount;
    } else if (strcmp(which, "int") == 0 && attr_pts >= amount) {
        dawn->players[pindex].intelligence += amount;
    } else if (strcmp(which, "mdef") == 0 && attr_pts >= amount) {
        dawn->players[pindex].m_def += amount;
    } else {
        sprintf(out, "PRIVMSG %s :%s, you have chosen an invalid attribute to increase or do not have the amount "
                "of attribute points required: (%d points available)\r\n", message->receiver, message->sender_nick, attr_pts);
        add_msg(out, strlen(out));
        return;
    }
    dawn->players[pindex].attr_pts -= amount;
    sprintf(out, "PRIVMSG %s :%s has increased %s by %d, you have %d attribute points available\r\n", message->receiver,
            message->sender_nick, which, amount, attr_pts - amount);
    add_msg(out, strlen(out));
}

void revive(struct Message *message) {
    char out[MAX_MESSAGE_BUFFER];
    int pindex = get_pindex(message->sender_nick);
    int cur_x = dawn->players[pindex].pos_x;
    int cur_y = dawn->players[pindex].pos_y;

    if (cur_x == 7 && cur_y == 4) {
        if (!dawn->players[pindex].alive) {
            dawn->players[pindex].health = dawn->players[pindex].max_health;
            dawn->players[pindex].mana = dawn->players[pindex].max_mana;
            dawn->players[pindex].alive = 1;
            dawn->players[pindex].fullness = 100;
            sprintf(out, "PRIVMSG %s :%s, you have been revived!\r\n", message->receiver, message->sender_nick);
        } else {
            sprintf(out, "PRIVMSG %s :%s, you are not dead\r\n", message->receiver, message->sender_nick);
        }
    } else {
        sprintf(out, "PRIVMSG %s :%s, you must be at a shrine to revive!\r\n", message->receiver, message->sender_nick);
    }
    add_msg(out, strlen(out));
}


char * auth_level_to_str(int al) { //enum auth_level
    switch(al) {
        case AL_NOAUTH:
            return "NO AUTH";
        case AL_USER:
            return "USER";
        case AL_REG:
            return "REG";
        case AL_ADMIN:
            return "ADMIN";
        case AL_ROOT:
            return "ROOT";
        default:
            PRINTERR("INVALID AUTH")
    }
    return "INVALID AUTH";
}

enum auth_level str_to_auth_level(char * al) {
    if(strcasecmp(al, "USER") == 0) {
        return AL_USER;
    } else if(strcasecmp(al, "REG") == 0) {
        return AL_REG;
    } else if(strcasecmp(al, "ADMIN") == 0) {
        return AL_ADMIN;
    } else if(strcasecmp(al, "ROOT") == 0) {
        return AL_ROOT;
    }
    return AL_NOAUTH;
}
