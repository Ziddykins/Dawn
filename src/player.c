#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "include/status.h"
#include "include/player.h"

//Prototypes
unsigned long long get_nextlvl_exp (struct Bot *, const char *);

int get_pindex (struct Bot *b, char const * username) { //username -> MAX_NICK_LENGTH
    for (int i=0; i<b->player_count; i++) {
        if (strcasecmp(b->players[i].username, username) == 0)
            return i;
    }
    return -1;
}
/* DEPRECATED
int get_bindex (struct Bot *b, char const * username, char const * location) { //username -> MAX_NICK_LENGTH, location -> 64
    int pindex = get_pindex(b, username);
    for (int i=0; i<MAX_BUILDINGS; i++) {
        if (strcmp(location, b->players[pindex].current_map.buildings[i].name) == 0) {
            return i;
        }
    }
    return -1;
}
*/

void init_new_character (struct Bot *b, struct Message *message) {
    //Check if user exists
    char out[MAX_MESSAGE_BUFFER];
    char tmp_pwd[] = "temp";

    if (get_pindex(b, message->sender_nick) != -1) {
        sprintf(out, "PRIVMSG %s :You already have an account!\r\n", b->active_room);
        add_msg(out, strlen(out));
        return;
    }

    struct Player np;
    bzero(&np, sizeof np);

    np.pos_x = 0;
    np.pos_y = 1;

    strncpy(np.username, message->sender_nick, MAX_NICK_LENGTH-1);
    gen_salt(np.salt, 16);
    hash_pwd(np.pwd, np.salt, tmp_pwd);
    strcpy(np.hostmask, message->sender_hostmask);
    strcpy(np.first_class, "None");
    strcpy(np.second_class, "None");
    strcpy(np.title, "Newbie");
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
    np.auth_level = AL_USER;
    np.max_auth = AL_USER;
    memset(np.inventory, 0, sizeof np.inventory);
    struct Inventory sword  = {"Wooden Sword", 0, 0, 0, 5, 0, 1, 15, 0, 0, 0, 0, 0, 0, 1, 0, 0};
    struct Inventory shield = {"Wooden Shield", 5, 5, 5, 5, 5, 1, 15, 0, 0, 0, 1, 0, 0, 1, 5, 0};
    np.inventory[0] = sword;
    np.inventory[1] = shield;


    b->players[b->player_count] = np;
    b->player_count++;

    sprintf(out, "PRIVMSG %s :Account created for user %s@%s, please set your password by sending me "
            "a private message containing: ;set password <your_password>\r\n", b->active_room,
            message->sender_nick, message->sender_hostmask);
    save_players(b);
    add_msg(out, strlen(out));
}

void save_players (struct Bot *b) {
    if(b == 0)
        return;
    FILE *file = fopen("players.bin", "wb");
    if(!file) {
        PRINTWARN("Could not save players")
        return;
    }
    size_t len;
    CALLEXIT(!(len = fwrite(b, sizeof *b, 1, file)))
    fclose(file);
    printf(INFO "Saved players (%zu bytes)\n", len*(sizeof *b));
}

void load_players (struct Bot *b) {
    FILE *file = fopen("players.bin", "rb");
    if (!file) {
        fprintf(stderr, WARN "Could not load players\n");
        return;
    }
    CALLEXIT(!(fread(b, sizeof *b, 1, file)))
    fclose(file);
    printf(INFO "Players loaded (%zu bytes)\n", sizeof *b);
}

//Lol this entire thing
static const char* progress_bar (struct Bot *b, char const * username) { //username -> MAX_NICK_LENGTH
    static char bar[48];
    int i = get_pindex(b, username);
    unsigned long long nextlvl_exp = get_nextlvl_exp(b, username);
    nextlvl_exp = nextlvl_exp ? nextlvl_exp : 5;
    long double temp_cyan = ((b->players[i].experience / (long double)nextlvl_exp) * 100) / 10;
    int blue_count  = 9  - (int)temp_cyan;
    int cyan_count  = 10 - blue_count;
    sprintf(bar, "%s,10%0*d%s,02%0*d%s", IRC_CYAN, cyan_count, 0, IRC_DBLUE, blue_count, 0, IRC_NORMAL);
    return bar;
}

unsigned long long get_nextlvl_exp(struct Bot *b, char const * username) { //username -> MAX_NICK_LENGTH
    unsigned long long curr_level = (unsigned long long)b->players[get_pindex(b, username)].level;
    if (curr_level > 10) {
        return (500ULL * curr_level * curr_level * curr_level - 500ULL * curr_level);
    } else {
        return (100ULL * curr_level * curr_level * curr_level - 100ULL * curr_level);
    }
}

void print_sheet (struct Bot *b, struct Message *message) {
    int i = get_pindex(b, message->sender_nick);
    char out[MAX_MESSAGE_BUFFER];
    int stats[6] = { b->players[i].max_health, b->players[i].max_mana,
                     b->players[i].strength, b->players[i].intelligence,
                     b->players[i].m_def, b->players[i].defense };
    get_stat(b, message, stats);

    sprintf(out,
            "PRIVMSG %s :[%s (Lv: %d)] [%ld/%d \0034HP\003] - [%d/%d \00310MP\003] Str: %d - Int: %d - MDef: %d"
            " - Def: %d (%ldK/%ldD) [EXP: %llu / %llu %s - Gold: %s%ld%s] [Fullness: %hd%%]\r\n", message->receiver, message->sender_nick,
            b->players[i].level, b->players[i].health, stats[0], b->players[i].mana, stats[1],
            stats[2], stats[3], stats[4], stats[5], b->players[i].kills, b->players[i].deaths,
            b->players[i].experience, get_nextlvl_exp(b, b->players[i].username),
            progress_bar(b, message->sender_nick), IRC_ORANGE, b->players[i].gold, IRC_NORMAL,
            b->players[i].fullness);

    add_msg(out, strlen(out));
    return;
}

void check_levelup (struct Bot *b, struct Message *message) {
    int i = get_pindex(b, message->sender_nick);
    char out[MAX_MESSAGE_BUFFER];
    unsigned long long next_level_exp = get_nextlvl_exp(b, message->sender_nick);
    unsigned long long curr_level_exp = b->players[i].experience;
    int curr_level = b->players[i].level;

    if (curr_level_exp >= next_level_exp) {
        b->players[i].level++;
        b->players[i].max_health += 25;
        b->players[i].max_mana   += 25;
        b->players[i].attr_pts   += 20;
        b->players[i].health      = b->players[i].max_health;
        b->players[i].mana        = b->players[i].max_mana;
        if (get_nextlvl_exp(b, message->sender_nick) > curr_level_exp) {
            sprintf(out, "PRIVMSG %s :%s has achieved level %d. 20 attribute points ready for assignment, HP and MP"
                    " increased +25! Increase your attributes using the ;assign command!\r\n",
                    message->receiver, message->sender_nick, curr_level + 1);
            add_msg(out, strlen(out));
        }
    }
}

void assign_attr_points (struct Bot *b, struct Message *message, char *which, int amount) {
    int pindex   = get_pindex(b, message->sender_nick);
    int attr_pts = b->players[pindex].attr_pts;
    char out[MAX_MESSAGE_BUFFER];

    if (strcmp(which, "str") == 0 && attr_pts >= amount) {
        b->players[pindex].strength += amount;
    } else if (strcmp(which, "def") == 0 && attr_pts >= amount) {
        b->players[pindex].defense += amount;
    } else if (strcmp(which, "int") == 0 && attr_pts >= amount) {
        b->players[pindex].intelligence += amount;
    } else if (strcmp(which, "mdef") == 0 && attr_pts >= amount) {
        b->players[pindex].m_def += amount;
    } else {
        sprintf(out, "PRIVMSG %s :%s, you have chosen an invalid attribute to increase or do not have the amount "
                "of attribute points required: (%d points available)\r\n", message->receiver, message->sender_nick, attr_pts);
        add_msg(out, strlen(out));
        return;
    }
    b->players[pindex].attr_pts -= amount;
    sprintf(out, "PRIVMSG %s :%s has increased %s by %d, you have %d attribute points available\r\n", message->receiver,
            message->sender_nick, which, amount, attr_pts - amount);
    add_msg(out, strlen(out));
}

void revive (struct Bot *b, struct Message *message) {
    char out[MAX_MESSAGE_BUFFER];
    int pindex = get_pindex(b, message->sender_nick);
    int cur_x  = b->players[pindex].pos_x;
    int cur_y  = b->players[pindex].pos_y;

    if (cur_x == 7 && cur_y == 4) {
        if (!b->players[pindex].alive) {
            b->players[pindex].health = b->players[pindex].max_health;
            b->players[pindex].mana = b->players[pindex].max_mana;
            b->players[pindex].alive = 1;
            b->players[pindex].fullness = 100;
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
