#include "include/commands.h"

#include "include/parse.h"
#include "include/limits.h"
#include "include/cmdsys.h"
#include "include/util.h"
#include "include/spells.h"
#include "include/player.h"
#include "include/status.h"
#include "include/combat.h"
#include "include/items.h"
#include "include/market.h"
#include "include/persistence.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>

char regex_group[15][2048];

void init_cmds() {
    init_global_cmd_sys();

    //AL_NOAUTH
    register_cmd(NULL, ";_;", "Crave crying I bet", AL_NOAUTH, cmd_cry);
    register_cmd(NULL, ";[", "Express your pure sadness", AL_NOAUTH, cmd_cry);
    register_cmd(NULL, ";auth", "Authenticate your account to use Dawn", AL_NOAUTH, cmd_auth);
    register_cmd(NULL, ";help", "[command] | Prints general help or for a specific command | [] = optional, <> = necessary", AL_NOAUTH, cmd_help);
    register_cmd(NULL, ";new", "Create a new account for this nick", AL_NOAUTH, cmd_new);

    //AL_USER
    register_cmd(NULL, ";ap", "Learn how many attribute points you have left to spend", AL_USER, cmd_ap);
    register_cmd(NULL, ";assign", "<type> <amount> | assign attribute points to your stats", AL_USER, cmd_assign);
    register_cmd(NULL, ";check", "Check whether there is a personal monster in this room", AL_USER, cmd_check);
    register_cmd(NULL, ";cast", "<spell> [target] | cast a specified spell; spell may require a target", AL_USER, cmd_cast);
    register_cmd(NULL, ";drop", "<inventory slot> | Discard of an item in your inventory", AL_USER, cmd_drop);
    register_cmd(NULL, ";equip", "<inventory slot> | Equip an item from your inventory", AL_USER, cmd_equip);
    register_cmd(NULL, ";equipall", "Equips all pieces of equipment in your inventory", AL_USER, cmd_equipall);
    register_cmd(NULL, ";gcheck", "Check whether there is a global monster", AL_USER, cmd_gcheck);
    register_cmd(NULL, ";ghunt", "Summons a global monster in to the room; if one exists it will need to be killed by attacks or ;gslay first", AL_USER, cmd_ghunt);
    register_cmd(NULL, ";gmelee", "Performs a melee attack on a global monster", AL_USER, cmd_gmelee);
    register_cmd(NULL, ";gslay", "[gold amount] | Contribute to slaying a global monster or check how much is needed", AL_USER, cmd_gslay);
    register_cmd(NULL, ";hunt", "Hunt for a personal monster which only you can attack", AL_USER, cmd_hunt);
    register_cmd(NULL, ";info", "<inventory slot> | Ascertain info on an item in your inventory", AL_USER, cmd_info);
    register_cmd(NULL, ";inv", "Displays the items currently in your inventory", AL_USER, cmd_inv);
    //registerCmd(0, ";locate", "<building> | Find out where a certain sight is located", AL_USER, cmd_locate); //DEPRECATED
    register_cmd(NULL, ";location", "Ascertain knowledge about your current location", AL_USER, cmd_location);
    register_cmd(NULL, ";make", "snow angles | Make snow angles!", AL_USER, cmd_make);
    register_cmd(NULL, ";market", "[buy|sell] [material] [amount] | Check what the prices are, buy or sell materials", AL_USER, cmd_market);
    register_cmd(NULL, ";materials", "Discover what materials you are a proud owner of", AL_USER, cmd_materials);
    register_cmd(NULL, ";melee", "Performs a melee attack on a personal monster", AL_USER, cmd_melee);
    register_cmd(NULL, ";revive", "Flourish once again when you have passed", AL_USER, cmd_revive);
    register_cmd(NULL, ";sheet", "[user] | Ascertain knowledge of your or another players' stats", AL_USER, cmd_sheet);
    register_cmd(NULL, ";slay", "<gold amount> | For a bit of gold you can have someone help you out in battle", AL_USER, cmd_slay);
    register_cmd(NULL, ";travel", "<x> <y> | Travel to a location on the map", AL_USER, cmd_travel);
    register_cmd(NULL, ";unequip", "<inventory slot> | Unequip an item", AL_USER, cmd_unequip);
    register_cmd(NULL, ";unequipall", "Uneqippes all pieces of equipment from your inventory", AL_USER, cmd_unequipall);
    register_cmd(NULL, ";drink", "Get some rest and drink some good cold beer", AL_USER, cmd_drink);

    //AL_ADMIN
    register_cmd(NULL, ";fluctuate", "Forces the market prices to fluctuate", AL_ADMIN, cmd_fluctuate);
    register_cmd(NULL, ";fslay", "Force-slay the global monster currently in the room", AL_ADMIN, cmd_fslay);
    register_cmd(NULL, ";gib", "Cheat yourself some gold", AL_ADMIN, cmd_gib);
    register_cmd(NULL, ";givexp", "<user> <amount> | Give XP points to a user", AL_ADMIN, cmd_givexp);
    register_cmd(NULL, ";save", "Save the current state of the game to disk", AL_ADMIN, cmd_save);
    register_cmd(NULL, ";setauth", "Set a users authentication level. (noauth, user, reg, admin, root)", AL_ADMIN, cmd_setauth);

    //AL_ROOT
    register_cmd(NULL, ";stop", "Gracefully stops the server", AL_ROOT, cmd_stop);

    finalize_cmd_sys(0);
}

void free_cmds() {
    free_cmd_sys(0);
}

void cmd_help(int pindex, struct Message * msg) {
    extern CmdSys commands;
    struct cmd_sys * ccs = commands;
    assert(ccs);

    if(matches_regex(msg->message, CMD_LIT"\\s*([;\\s]"CMD_MATCH")")) {
        regex_group[1][0] = PREFIX_C;
        invoke_cmd(0, pindex, regex_group[1], msg, CMD_HELP);
    } else {
        char * out;
        CALLEXIT(!(out = malloc(MAX_MESSAGE_BUFFER)))
        int len = snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :", msg->receiver);
        for(size_t i = 0; i < ccs->len; i++) { //print each command to the string
            if(ccs->auth_levels[i] <= dawn->players[pindex].auth_level) {
                len += snprintf(out+len, (size_t)(MAX_MESSAGE_BUFFER-len), "%s ", ccs->cmds[i]);
            }
        }
        len += snprintf(out+len, (size_t)(MAX_MESSAGE_BUFFER-len), "\r\n");
        add_msg(out, (size_t)(len));
        free(out);
    }
}

void cmd_new(int pindex, struct Message * msg) {
    char * out;
    CALLEXIT(!(out = malloc(MAX_MESSAGE_BUFFER)))
    if(pindex == -1) {
        init_new_character(msg);
        snprintf(out, MAX_MESSAGE_BUFFER, "WHOIS %s\r\n", msg->sender_nick);
        add_msg(out, strlen(out));
    } else {
        sprintf(out, "PRIVMSG %s :You already have an account!\r\n", dawn->active_room);
        add_msg(out, strlen(out));
    }
    free(out);
}

char * auth_key;
int auth_key_valid;

void cmd_auth(int pindex, struct Message * msg) {
    char * out;
    CALLEXIT(!(out = malloc(MAX_MESSAGE_BUFFER)))
    if(matches_regex(msg->message, CMD_LIT" (\\w+)")) {
        if(auth_key_valid && strcmp(auth_key, regex_group[1]) == 0) {
            auth_key_valid = 0;
            free(auth_key);
            auth_key = 0;
            dawn->players[pindex].auth_level = AL_ROOT;
            dawn->players[pindex].max_auth = AL_ROOT;
            snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :%s has been verified. (%s)\r\n",
                dawn->active_room,
                dawn->players[pindex].username,
                auth_level_to_str(dawn->players[pindex].auth_level));
            add_msg(out, strlen(out));
        } else {
            snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :Invalid auth_key!\r\n", dawn->active_room);
            add_msg(out, strlen(out));
        }
    } else if(matches_regex(msg->message, CMD_LIT)) {
        snprintf(out, MAX_MESSAGE_BUFFER, "WHOIS %s\r\n", dawn->players[pindex].username);
        add_msg(out, strlen(out));
    }
    free(out);
}

void cmd_stop(int pindex __attribute__((unused)), struct Message * msg) {
    char * out;
    CALLEXIT(!(out = malloc(MAX_MESSAGE_BUFFER)))
    if (matches_regex(msg->message, CMD_LIT " (.*)")) {
        if(strcasecmp(regex_group[1], dawn->nickname) == 0) {
            close_socket(con_socket);
        } else {
            snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :Wrong nickname.\r\n", dawn->active_room);
            add_msg(out, strlen(out));
        }
    } else {
        snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :Please supply the nickname of the bot.\r\n", dawn->active_room);
        add_msg(out, strlen(out));
    }
    free(out);
}

void cmd_sheet(int pindex __attribute__((unused)), struct Message * msg) {
    if (matches_regex(msg->message, CMD_LIT" (\\w+)")) {
        strncpy(msg->sender_nick, to_lower(regex_group[1]), MAX_NICK_LENGTH);
        if (get_pindex(regex_group[1]) != -1) {
            print_sheet(msg);
        }
    } else {
        print_sheet(msg);
    }
}

void cmd_equip(int pindex __attribute__((unused)), struct Message * msg) {
    if (matches_regex(msg->message, CMD_LIT" (\\d+)")) {
        int slot = atoi(regex_group[1]);
        equip_inventory(msg, slot, 0, 0);
    }
}

void cmd_unequip(int pindex __attribute__((unused)), struct Message * msg) {
    if (matches_regex(msg->message, CMD_LIT" (\\d+)")) {
        int slot = atoi(regex_group[1]);
        equip_inventory(msg, slot, 1, 0);
    }
}

void cmd_equipall(int pindex __attribute__((unused)), struct Message * msg) {
    equip_inventory(msg, 0, 0, 1);
}

void cmd_unequipall(int pindex __attribute__((unused)), struct Message * msg) {
    equip_inventory(msg, 0, 1, 1);
}

void cmd_gmelee(int pindex __attribute__((unused)), struct Message * msg) {
    player_attacks(msg, 1);
}

void cmd_hunt(int pindex, struct Message * msg) {
    if (!dawn->players[pindex].personal_monster.active) {
        call_monster(msg->sender_nick, 0);
    }
}

void cmd_revive(int pindex __attribute__((unused)), struct Message * msg) {
    revive(msg);
}

void cmd_drop(int pindex __attribute__((unused)), struct Message * msg) {
    if (matches_regex(msg->message, CMD_LIT" (\\d+)")) {
        int slot = atoi(regex_group[1]);
        drop_item(msg, slot);
    }
}

void cmd_info(int pindex __attribute__((unused)), struct Message * msg) {
    if (matches_regex(msg->message, CMD_LIT" (\\d+)")) {
        int slot = atoi(regex_group[1]);
        get_item_info(msg, slot);
    }
}

void cmd_givexp(int pindex __attribute__((unused)), struct Message * msg) {
    if (matches_regex(msg->message, CMD_LIT" (\\w+) (\\d+)")) {
        char * username;
        CALLEXIT(!(username = calloc(MAX_NICK_LENGTH, 1)))
        char *eptr;
        int index;
        unsigned long long amount = strtoull(regex_group[2], &eptr, 10);

        strncpy(username, to_lower(regex_group[1]), MAX_NICK_LENGTH);
        index = get_pindex(username);
        dawn->players[index].experience += amount;

        if (index == -1) return;

        while (dawn->players[index].experience > get_nextlvl_exp(username)) {
            struct Message temp;
            strcpy(temp.sender_nick, username);
            strcpy(temp.receiver, dawn->active_room);
            check_levelup(&temp);
        }
        free(username);
    }
}

void cmd_make(int pindex __attribute__((unused)), struct Message * msg) {
    char * out;
    CALLEXIT(!(out = malloc(MAX_MESSAGE_BUFFER)))
    if (matches_regex(msg->message, CMD_LIT" snow angels")) {
        if (dawn->weather == SNOWING) {
            snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :%s falls to the ground and begins making snow angels!\r\n",
                    msg->receiver, msg->sender_nick);
            add_msg(out, strlen(out));
        } else {
            snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :There is no snow\r\n", msg->receiver);
            add_msg(out, strlen(out));
        }
    }
    free(out);
}
void cmd_location(int pindex __attribute__((unused)), struct Message * msg) {
    print_location(get_pindex(msg->sender_nick));
}

void cmd_slay(int pindex __attribute__((unused)), struct Message * msg) {
    slay_monster(msg->sender_nick, 0, 0);
}

void cmd_cast(int pindex, struct Message * msg) {
    char *out;
    CALLEXIT (!(out = malloc(MAX_MESSAGE_BUFFER)))
    if (dawn->players[pindex].health <= 0) {
        snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :%s, dead people can not cast spells\r\n",
                dawn->active_room, msg->sender_nick);
        add_msg(out, strlen(out));
        return;
    }
    if (matches_regex(msg->message, CMD_LIT" (\\w+)\\s?(\\w+)?")) {
        if (strcmp(regex_group[1], "heal") == 0) {
            cast_heal(msg->sender_nick, to_lower(regex_group[2]));
        } else if (strcmp(regex_group[1], "rain") == 0) {
            cast_rain(msg->sender_nick);
        } else if (strcmp(regex_group[1], "fireball") == 0) {
            cast_fireball(msg->sender_nick, to_lower(regex_group[2]));
        }
    }
    free(out);
}

void cmd_gslay(int pindex __attribute__((unused)), struct Message * msg) {
    if(matches_regex(msg->message, CMD_LIT" (\\d+)")) {
        slay_monster(msg->sender_nick, 1, atoi(regex_group[1]));
    } else {
        char * out;
        CALLEXIT(!(out = malloc(MAX_MESSAGE_BUFFER)))
        snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :The %s will cost %d more gold to slay. Contribute to hiring a warrior "
                "with ;gslay <amount>\r\n", msg->receiver, dawn->global_monster.name,
                dawn->global_monster.slay_cost);
        add_msg(out, strlen(out));
        free(out);
    }
}

void cmd_fslay(int pindex, struct Message * msg) {
    dawn->players[pindex].gold += 9999999;
    slay_monster(msg->sender_nick, 1, 9999999);
}

void cmd_check(int pindex __attribute__((unused)), struct Message * msg) {
    print_monster(msg->sender_nick, 0);
}
void cmd_gcheck(int pindex __attribute__((unused)), struct Message * msg) {
    print_monster(msg->sender_nick, 1);
}
void cmd_assign(int pindex, struct Message * msg) {
    if (matches_regex(msg->message, CMD_LIT" (\\w+) (\\d+)")) {
        assign_attr_points(msg, to_lower(regex_group[1]), atoi(regex_group[2]));
    } else {
        char * out;
        CALLEXIT(!(out = malloc(MAX_MESSAGE_BUFFER)))
        snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :To assign your attribute points, use \";assign <type> <amount>\"; type can be "
                "str, def, int or mdef. You have %d attribute points left which you can assign\r\n",
                msg->receiver, dawn->players[pindex].attr_pts);
        add_msg(out, strlen(out));
        free(out);
    }
}

void cmd_ap(int pindex, struct Message * msg) {
    char * out;
    CALLEXIT(!(out = malloc(MAX_MESSAGE_BUFFER)))
    snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :%s, you have %d attribute points which you can assign\r\n",
            msg->receiver, msg->sender_nick, dawn->players[pindex].attr_pts);
    add_msg(out, strlen(out));
    free(out);
}

void cmd_travel(int pindex __attribute__((unused)), struct Message * msg) {
    if (matches_regex(msg->message, CMD_LIT" (\\d+),(\\d+)")) {
        move_player(msg, atoi(regex_group[1]), atoi(regex_group[2]));
    }
}
/* DEPRECATED
void cmd_locate(int pindex __attribute__((unused)), struct Message * msg) {
    if (matches_regex(msg->message, CMD_LIT" (\\w+)")) {
        find_building(dawn, msg, to_lower(regex_group[1]));
    }
}
*/
void cmd_materials(int pindex __attribute__((unused)), struct Message * msg) {
    print_materials(msg);
}

void cmd_market(int pindex __attribute__((unused)), struct Message * msg) {
    if (matches_regex(msg->message, CMD_LIT" sell (\\w+) (\\d+)")) {
        char *eptr;
        long amount = strtol(regex_group[2], &eptr, 10);
        market_buysell(msg, 0, regex_group[1], amount);
    } else if (matches_regex(msg->message, CMD_LIT" buy (\\w+) (\\d+)")) {
        char *eptr;
        long amount = strtol(regex_group[2], &eptr, 10);
        market_buysell(msg, 1, regex_group[1], amount);
    } else {
        print_market();
    }
}

void cmd_save(int pindex __attribute__((unused)), struct Message * msg) {
    char * out;
    CALLEXIT(!(out = malloc(MAX_MESSAGE_BUFFER)))
    persistent_save(dawn);
    snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :Saved.\r\n", msg->receiver);
    add_msg(out, strlen(out));
    free(out);
}

void cmd_cry(int pindex __attribute__((unused)), struct Message * msg) {
    char * out;
    CALLEXIT(!(out = malloc(MAX_MESSAGE_BUFFER)))
    snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :You break down in tears\r\n", msg->receiver);
    add_msg(out, strlen(out));
    free(out);
}

void cmd_gib(int pindex, struct Message * msg) {
    if(matches_regex(msg->message, CMD_LIT" gold (\\d+)")) {
        dawn->players[pindex].gold += atoi(regex_group[1]);
    } else if(matches_regex(msg->message, CMD_LIT" ap (\\d+)")) {
        dawn->players[pindex].attr_pts += atoi(regex_group[1]);
    }
}

void cmd_inv (int pindex __attribute__((unused)), struct Message * msg) {
    print_inventory(msg);
}

void cmd_ghunt (int pindex __attribute__((unused)), struct Message * msg) {

    if (dawn->global_monster.active) {
        char *out;
        CALLEXIT(!(out = malloc(MAX_MESSAGE_BUFFER)))
        snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :You must either kill the "
                "monster or use ;gslay!\r\n", msg->receiver);
        add_msg(out, strlen(out));
        free(out);
    } else {
        call_monster(msg->sender_nick, 1);
    }
}

void cmd_melee (int pindex __attribute__((unused)), struct Message * msg) {
    player_attacks(msg, 0);
}

void cmd_fluctuate (int pindex __attribute__((unused)), struct Message * msg __attribute__((unused))) {
    fluctuate_market();
}

void cmd_drink(int pindex, struct Message * msg) {
    char * out;
    CALLEXIT(!(out = malloc(MAX_MESSAGE_BUFFER)))
    if(matches_regex(msg->message, CMD_LIT " beer")) {
        if (dawn->players[pindex].fullness + 5 > 100) {
            snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :%s takes two more sips and projectile vomits"
                    " all over the patrons. Perhaps you've had enough to drink. Fullness -15\r\n",
                    dawn->active_room, msg->sender_nick);
            dawn->players[pindex].fullness -= 15;
            add_msg(out, strlen(out));
            return;
        }
        snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :You help yourself to a good 'ol bottle of beer. Fullness +5\r\n", msg->receiver);
        dawn->players[pindex].fullness+=5;
        if(dawn->players[pindex].fullness > 100)
            dawn->players[pindex].fullness = 100;

    } else if(matches_regex(msg->message, CMD_LIT)) {
        snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :You can drink some beer if you like.\r\n", msg->receiver);
    }
    add_msg(out, strlen(out));
    free(out);
}

void cmd_setauth(int pindex, struct Message * msg) {
    char * out;
    CALLEXIT(!(out = malloc(MAX_MESSAGE_BUFFER)))
    if(matches_regex(msg->message, CMD_LIT " (\\w+) (\\w+)")) {
        int useridx = get_pindex(regex_group[1]);
        if(useridx == -1) {
            snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :This user does not exist.\r\n", msg->receiver);
            add_msg(out, strlen(out));
        } else {
            enum auth_level newlevel = str_to_auth_level(regex_group[2]);
            if (newlevel > dawn->players[pindex].auth_level) {
                snprintf(out, MAX_MESSAGE_BUFFER,
                         "PRIVMSG %s :You are not allowed to set this authentication level.\r\n", msg->receiver);
                add_msg(out, strlen(out));
            } else if(pindex == useridx) {
                snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :You cannot set your own authentication level.\r\n",
                         msg->receiver);
                add_msg(out, strlen(out));
            } else {
                dawn->players[useridx].max_auth = newlevel;
                dawn->players[useridx].auth_level = dawn->players[useridx].max_auth;
                snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :%s set to %s.\r\n",
                         msg->receiver,
                         dawn->players[useridx].username,
                         auth_level_to_str(newlevel));
                add_msg(out, strlen(out));
            }
        }
    } else {
        snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :Please provide a user and an authentication level.\r\n",
                 msg->receiver);
        add_msg(out, strlen(out));
    }
    free(out);
}
