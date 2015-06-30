#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "include/status.h"
#include "include/player.h"
#include "include/network.h"
#include "include/stats.h"
#include "include/colors.h"
#include "include/inventory.h"
#include "include/limits.h"
#include "include/parse.h"
//Prototypes
void save_players (struct Bot *, size_t);
unsigned long long get_nextlvl_exp (struct Bot *, const char []);

int get_pindex (struct Bot *dawn, const char username[MAX_NICK_LENGTH]) {
    for (int i=0; i<dawn->player_count; i++) {
        if (strcmp(dawn->players[i].username, username) == 0)
            return i;
    }
    return -1;
}

int get_bindex (struct Bot *dawn, const char username[MAX_NICK_LENGTH], const char location[64]) {
    int pindex = get_pindex(dawn, username);
    for (int i=0; i<MAX_BUILDINGS; i++) {
        if (strcmp(location, dawn->players[pindex].current_map.buildings[i].name) == 0) {
            return i;
        }
    }
    return -1;
}

void init_new_character (struct Bot *dawn, struct Message *message) {
    //Check if user exists
    char out[MAX_MESSAGE_BUFFER];
    char tmp_pwd[] = "temp";

    if (get_pindex(dawn, message->sender_nick) != -1) {
        sprintf(out, "PRIVMSG %s :You already have an account!\r\n", dawn->active_room);
        addMsg(out, strlen(out));
        return;
    }

    struct Player np;

    np.current_map = set_map(0);
    np.current_map.cur_x = 0;
    np.current_map.cur_y = 1;

    strcpy(np.username, nultrm(message->sender_nick));
    strcpy(np.password, xor_flip(tmp_pwd));
    strcpy(np.hostmask, message->sender_hostmask);
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
    np.travel_timer.active = 0;
    
    memset(np.inventory, 0, sizeof np.inventory);
    struct Inventory sword  = {"Wooden Sword", 0, 0, 0, 5, 0, 1, 15, 0, 0, 0, 0, 0, 0, 1, 0, 0};
    struct Inventory shield = {"Wooden Shield", 5, 5, 5, 5, 5, 1, 15, 0, 0, 0, 1, 0, 0, 1, 5, 0};
    np.inventory[0] = sword;
    np.inventory[1] = shield;


    dawn->players[dawn->player_count] = np;
    dawn->player_count++;

    sprintf(out, "PRIVMSG %s :Account created for user %s@%s, please set your password by sending me "
            "a private message containing: ;set password <your_password>\r\n", dawn->active_room,
            message->sender_nick, message->sender_hostmask);

    struct Bot temp;
    save_players (dawn, sizeof(temp));
    addMsg(out, strlen(out));
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
static const char *progress_bar (struct Bot *dawn, const char username[MAX_NICK_LENGTH]) {
    static char bar[48];
    int i = get_pindex(dawn, username);
    unsigned long long nextlvl_exp = get_nextlvl_exp(dawn, username);
    nextlvl_exp = nextlvl_exp ? nextlvl_exp : 5;
    long double temp_cyan = ((dawn->players[i].experience / (long double)nextlvl_exp) * 100) / 10;
    int blue_count  = 9  - (int)temp_cyan;
    int cyan_count  = 10 - blue_count;
    sprintf(bar, "%s,10%0*d%s,02%0*d%s", cyan, cyan_count, 0, dblue, blue_count, 0, normal);
    return bar;
}

unsigned long long get_nextlvl_exp (struct Bot *dawn, const char username[MAX_NICK_LENGTH]) {
    int curr_level = dawn->players[get_pindex(dawn, username)].level;
    if (curr_level > 10) {
        return (500ULL * curr_level * curr_level * curr_level - 500ULL * curr_level);
    } else {
        return (500ULL * curr_level * curr_level * curr_level - 500ULL * curr_level);
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
            "PRIVMSG %s :[%s (Lv: %d)] [%ld/%d \0034HP\003] - [%d/%d \00310MP\003] Str: %d - Int: %d - MDef: %d"
            " - Def: %d (%ldK/%ldD) [EXP: %llu / %llu %s - Gold: %s%ld%s] [Fullness: %hd%%]\r\n", message->receiver, message->sender_nick,
            dawn->players[i].level, dawn->players[i].health, stats[0], dawn->players[i].mana, stats[1],
            stats[2], stats[3], stats[4], stats[5], dawn->players[i].kills, dawn->players[i].deaths,
            dawn->players[i].experience, get_nextlvl_exp(dawn, dawn->players[i].username),
            progress_bar(dawn, message->sender_nick), orange, dawn->players[i].gold, normal,
            dawn->players[i].fullness);

    addMsg(out, strlen(out));
    return;
}

void check_levelup (struct Bot *dawn, struct Message *message) {
    int i = get_pindex(dawn, message->sender_nick);
    char out[MAX_MESSAGE_BUFFER];
    unsigned long long next_level_exp = get_nextlvl_exp(dawn, message->sender_nick);
    unsigned long long curr_level_exp = dawn->players[i].experience;
    int curr_level = dawn->players[i].level;

    if (curr_level_exp >= next_level_exp) {
        dawn->players[i].level++;
        dawn->players[i].max_health += 25;
        dawn->players[i].max_mana   += 25;
        dawn->players[i].attr_pts   += 20;
        dawn->players[i].health      = dawn->players[i].max_health;
        dawn->players[i].mana        = dawn->players[i].max_mana;
        if (get_nextlvl_exp(dawn, message->sender_nick) > curr_level_exp) {
            sprintf(out, "PRIVMSG %s :%s has achieved level %d. 20 attribute points ready for assignment, HP and MP"
                    " increased +25! Increase your attributes using the ;assign command!\r\n",
                    message->receiver, message->sender_nick, curr_level + 1);
            addMsg(out, strlen(out));
        }

    }
}

void assign_attr_points (struct Bot *dawn, struct Message *message, char which[5], int amount) {
    int pindex   = get_pindex(dawn, message->sender_nick);
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
        addMsg(out, strlen(out));
        return;
    }
    dawn->players[pindex].attr_pts -= amount;
    sprintf(out, "PRIVMSG %s :%s has increased %s by %d, you have %d attribute points available\r\n", message->receiver,
            message->sender_nick, which, amount, attr_pts - amount);
    addMsg(out, strlen(out));
}

void revive (struct Bot *dawn, struct Message *message) {
    char out[MAX_MESSAGE_BUFFER];
    int pindex = get_pindex(dawn, message->sender_nick);
    int bindex = get_bindex(dawn, message->sender_nick, "shrine");
    int cur_x  = dawn->players[pindex].current_map.cur_x;
    int cur_y  = dawn->players[pindex].current_map.cur_y;

    if (dawn->players[pindex].current_map.buildings[bindex].x == cur_x
            && dawn->players[pindex].current_map.buildings[bindex].y == cur_y) {
        if (!dawn->players[pindex].alive) {
            dawn->players[pindex].health = dawn->players[pindex].max_health;
            dawn->players[pindex].mana = dawn->players[pindex].max_mana;
            dawn->players[pindex].alive = 1;
            sprintf(out, "PRIVMSG %s :%s, you have been revived!\r\n", message->receiver, message->sender_nick);
        } else {
            sprintf(out, "PRIVMSG %s :%s, you are not dead\r\n", message->receiver, message->sender_nick);
        }
    } else {
        sprintf(out, "PRIVMSG %s :%s, you must be at a shrine to revive!\r\n", message->receiver, message->sender_nick);
    }
    addMsg(out, strlen(out));
}
