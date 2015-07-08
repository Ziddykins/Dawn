#include "include/commands.h"

char regex_group[15][2048];

void init_cmds() {
    init_cmdsys();

    registerCmd(0, ";help", "[command] | Prints general help or for a specific command | [] = optional, <> = necessary", AL_NOAUTH, cmd_help);
    registerCmd(0, ";new", "Create a new account for this nick", AL_NOAUTH, cmd_new);
    registerCmd(0, ";sheet", "[user] | Ascertain knowledge of your or another players' stats", AL_USER, cmd_sheet);
    registerCmd(0, ";equip", "<itemID> | Equip an item from your inventory", AL_USER, cmd_equip);
    registerCmd(0, ";unequip", "<itemID> | Unequip an item", AL_USER, cmd_unequip);
    registerCmd(0, ";gmelee", "Attack a global monster", AL_USER, cmd_gmelee);
    registerCmd(0, ";hunt", "Attack a personal monster", AL_USER, cmd_hunt);
    registerCmd(0, ";revive", "Flourish once again when you have passed", AL_USER, cmd_revive);
    registerCmd(0, ";drop", "<itemID> | Discard of an item in your inventory", AL_USER, cmd_drop);
    registerCmd(0, ";info", "<itemID> | Ascertain info on an item in your inventory", AL_USER, cmd_info);
    registerCmd(0, ";givexp", "<user> <amount> | Give XP points to a user", AL_ADMIN, cmd_givexp);
    registerCmd(0, ";make", "snow angles | Make snow angles!", AL_USER, cmd_make);
    registerCmd(0, ";location", "Ascertain knowledge about your current location", AL_USER, cmd_location);
    registerCmd(0, ";slay", "<gold amount> | For a bit of gold you can have someone help you out in battle", AL_USER, cmd_slay);
    registerCmd(0, ";gslay", "[gold amount] | Contribute to slaying a global monster or check how much is needed", AL_USER, cmd_gslay);
    registerCmd(0, ";check", "Check whether there is a personal monster in this room", AL_USER, cmd_check);
    registerCmd(0, ";gcheck", "Check whether there is a global monster", AL_USER, cmd_gcheck);
    registerCmd(0, ";assign", "<type> <amount> | assign attribute points to your stats", AL_USER, cmd_assign);
    registerCmd(0, ";ap", "Learn how many attribute points you have left to spend", AL_USER, cmd_ap);
    registerCmd(0, ";travel", "<x> <y> | Travel to a location on the map", AL_USER, cmd_travel);
    registerCmd(0, ";locate", "<building> | Find out where a certain sight is located", AL_USER, cmd_locate);
    registerCmd(0, ";materials", "Discover what materials you are a proud owner of", AL_USER, cmd_materials);
    registerCmd(0, ";market", "[buy|sell] [material] [amount] | Check what the prices are, buy or sell materials", AL_USER, cmd_market);
    registerCmd(0, ";save", "Save the current state of the game to disk", AL_ADMIN, cmd_save);
    registerCmd(0, ";[", "Express your pure sadness", AL_NOAUTH, cmd_cry);
    registerCmd(0, ";gib", "Cheat yourself some gold", AL_ADMIN, cmd_gib);

    finalizeCmdSys(0);
}

void free_cmds() {
    freeCmdSys(0);
}

void cmd_help(int pindex, struct Message * msg) {
    extern void* commands;
    struct cmdSys * ccs = commands;
    assert(ccs);

    if(check_if_matches_regex(msg->message, CMD_LIT" ("CMD_LIT_MID")")) {
        invokeCmd(0, pindex, regex_group[1], msg, CMD_HELP);
    } else {
        char * out = malloc(MAX_MESSAGE_BUFFER);
        int len = snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :", msg->receiver);
        for(size_t i = 0; i < ccs->len; i++) {
            len += snprintf(out+len, (size_t)(MAX_MESSAGE_BUFFER-len), "%s ", ccs->cmds[i]);
        }
        len += snprintf(out+len, (size_t)(MAX_MESSAGE_BUFFER-len), "\r\n");
        addMsg(out, (size_t)(len));
        free(out);
    }
}

void cmd_new(int pindex __attribute__((unused)), struct Message * msg) {
    init_new_character(dawn, msg);
}

void cmd_sheet(int pindex __attribute__((unused)), struct Message * msg) {
    if (strcmp(msg->message, CMD_LIT) == 0) {
        print_sheet(dawn, msg);
    } else if (check_if_matches_regex(msg->message, CMD_LIT" (\\w+)")) {
        strncpy(msg->sender_nick, to_lower(regex_group[1]), MAX_NICK_LENGTH);
        if (get_pindex(dawn, regex_group[1]) != -1) {
            print_sheet(dawn, msg);
        }
    }
}

void cmd_equip(int pindex __attribute__((unused)), struct Message * msg) {
    if (check_if_matches_regex(msg->message, CMD_LIT" (\\d+)")) {
        int slot = atoi(regex_group[1]);
        equip_inventory(dawn, msg, slot, 0);
    }
}

void cmd_unequip(int pindex __attribute__((unused)), struct Message * msg) {
    if (check_if_matches_regex(msg->message, CMD_LIT" (\\d+)")) {
        int slot = atoi(regex_group[1]);
        equip_inventory(dawn, msg, slot, 1);
    }
}

void cmd_gmelee(int pindex __attribute__((unused)), struct Message * msg) {
    player_attacks(dawn, msg, 1);
}

void cmd_hunt(int pindex, struct Message * msg) {
    if (!dawn->players[pindex].personal_monster.active) {
        call_monster(dawn, msg->sender_nick, 0);
    }
}

void cmd_revive(int pindex __attribute__((unused)), struct Message * msg) {
    revive(dawn, msg);
}

void cmd_drop(int pindex __attribute__((unused)), struct Message * msg) {
    if (check_if_matches_regex(msg->message, CMD_LIT" (\\d+)")) {
        int slot = atoi(regex_group[1]);
        drop_item(dawn, msg, slot);
    }
}

void cmd_info(int pindex __attribute__((unused)), struct Message * msg) {
    if (check_if_matches_regex(msg->message, CMD_LIT" (\\d+)")) {
        int slot = atoi(regex_group[1]);
        get_item_info(dawn, msg, slot);
    }
}

void cmd_givexp(int pindex __attribute__((unused)), struct Message * msg) {
    if (check_if_matches_regex(msg->message, CMD_LIT" (\\w+) (\\d+)")) {
        char * username = calloc(MAX_NICK_LENGTH, 1);
        char *eptr;
        int index;
        unsigned long long amount = strtoull(regex_group[2], &eptr, 10);

        strncpy(username, to_lower(regex_group[1]), MAX_NICK_LENGTH);
        index = get_pindex(dawn, username);
        dawn->players[index].experience += amount;

        if (index == -1) return;

        while (dawn->players[index].experience > get_nextlvl_exp(dawn, username)) {
            struct Message temp;
            strcpy(temp.sender_nick, username);
            strcpy(temp.receiver, dawn->active_room);
            check_levelup(dawn, &temp);
        }
        free(username);
    }
}

void cmd_make(int pindex __attribute__((unused)), struct Message * msg) {
    char * out = malloc(MAX_MESSAGE_BUFFER);
    if (check_if_matches_regex(msg->message, CMD_LIT" snow angels")) {
        if (dawn->weather == SNOWING) {
            snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :%s falls to the ground and begins making snow angels!\r\n",
                    msg->receiver, msg->sender_nick);
            addMsg(out, strlen(out));
        } else {
            snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :There is no snow\r\n", msg->receiver);
            addMsg(out, strlen(out));
        }
    }
    free(out);
}
void cmd_location(int pindex __attribute__((unused)), struct Message * msg) {
    print_location(dawn, get_pindex(dawn, msg->sender_nick));
}

void cmd_slay(int pindex __attribute__((unused)), struct Message * msg) {
    slay_monster(dawn, msg->sender_nick, 0, 0);
}

void cmd_gslay(int pindex __attribute__((unused)), struct Message * msg) {
    if(check_if_matches_regex(msg->message, CMD_LIT" (\\d+)")) {
        slay_monster(dawn, msg->sender_nick, 1, atoi(regex_group[1]));
    } else {
        char * out = malloc(MAX_MESSAGE_BUFFER);
        snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :The %s will cost %d more gold to slay. Contribute to hiring a warrior "
                "with ;gslay <amount>\r\n", msg->receiver, dawn->global_monster.name,
                dawn->global_monster.slay_cost);
        addMsg(out, strlen(out));
        free(out);
    }
}

void cmd_check(int pindex __attribute__((unused)), struct Message * msg) {
    print_monster(dawn, msg->sender_nick, 0);
}
void cmd_gcheck(int pindex __attribute__((unused)), struct Message * msg) {
    print_monster(dawn, msg->sender_nick, 1);
}
void cmd_assign(int pindex, struct Message * msg) {
    if (check_if_matches_regex(msg->message, CMD_LIT" (\\w+) (\\d+)")) {
        assign_attr_points(dawn, msg, to_lower(regex_group[1]), atoi(regex_group[2]));
    } else {
        char * out = malloc(MAX_MESSAGE_BUFFER);
        snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :To assign your attribute points, use \";assign <type> <amount>\"; type can be "
                "str, def, int or mdef. You have %d attribute points left which you can assign\r\n",
                msg->receiver, dawn->players[pindex].attr_pts);
        addMsg(out, strlen(out));
        free(out);
    }
}

void cmd_ap(int pindex, struct Message * msg) {
    char * out = malloc(MAX_MESSAGE_BUFFER);
    snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :%s, you have %d attribute points which you can assign\r\n",
            msg->receiver, msg->sender_nick, dawn->players[pindex].attr_pts);
    addMsg(out, strlen(out));
    free(out);
}

void cmd_travel(int pindex __attribute__((unused)), struct Message * msg) {
    if (check_if_matches_regex(msg->message, CMD_LIT" (\\d+),(\\d+)")) {
        move_player(dawn, msg, atoi(regex_group[1]), atoi(regex_group[2]));
    }
}

void cmd_locate(int pindex __attribute__((unused)), struct Message * msg) {
    if (check_if_matches_regex(msg->message, CMD_LIT" (\\w+)")) {
        find_building(dawn, msg, to_lower(regex_group[1]));
    }
}

void cmd_materials(int pindex __attribute__((unused)), struct Message * msg) {
    print_materials(dawn, msg);
}

void cmd_market(int pindex __attribute__((unused)), struct Message * msg) {
    if (check_if_matches_regex(msg->message, CMD_LIT" sell (\\w+) (\\d+)")) {
        char *eptr;
        long amount = strtol(regex_group[2], &eptr, 10);
        market_buysell(dawn, msg, 0, regex_group[1], amount);
    } else if (check_if_matches_regex(msg->message, CMD_LIT" buy (\\w+) (\\d+)")) {
        char *eptr;
        long amount = strtol(regex_group[2], &eptr, 10);
        market_buysell(dawn, msg, 1, regex_group[1], amount);
    } else {
        print_market(dawn);
    }
}

void cmd_save(int pindex __attribute__((unused)), struct Message * msg) {
    char * out = malloc(MAX_MESSAGE_BUFFER);
    persistent_save(dawn);
    snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :Saved.\r\n", msg->receiver);
    addMsg(out, strlen(out));
    free(out);
}


void cmd_cry(int pindex __attribute__((unused)), struct Message * msg) {
    char * out = malloc(MAX_MESSAGE_BUFFER);
    snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :You break down in tears\r\n", msg->receiver);
    addMsg(out, strlen(out));
    free(out);
}

void cmd_gib(int pindex, struct Message * msg) {
    if (check_if_matches_regex(msg->message, CMD_LIT" gold (\\d+)")) {
        dawn->players[pindex].gold = atoi(regex_group[1]);
    }
}
