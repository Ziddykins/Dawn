#include "include/commands.h"

char regex_group[15][2048];

void cmd_help(int pindex __attribute__((unused)), struct Message * msg) {
    char * out = malloc(MAX_MESSAGE_BUFFER);
    extern void* commands;
    struct cmdSys * ccs = commands;
    assert(ccs);

    int len = snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s : ", msg->receiver);
    for(size_t i = 0; i < ccs->len; i++) {
        len += snprintf(out+len+1, (size_t)(MAX_MESSAGE_BUFFER-len-1), "%s ", ccs->cmds[i]);
    }
    addMsg(out, (size_t)(len));
    free(out);
}

void cmd_sheet(int pindex __attribute__((unused)), struct Message * msg) {
    if (strcmp(msg->message, CMD_LIT) == 0) {
        print_sheet(dawn, msg);
    } else if (check_if_matches_regex(msg->message, CMD_LIT"(\\w+)")) {
        strncpy(msg->sender_nick, to_lower(regex_group[1]), MAX_NICK_LENGTH);
        if (get_pindex(dawn, regex_group[1]) != -1) {
            print_sheet(dawn, msg);
        }
    }
}

void cmd_equip(int pindex __attribute__((unused)), struct Message * msg) {
    if (check_if_matches_regex(msg->message, CMD_LIT"(\\d+)")) {
        int slot = atoi(regex_group[1]);
        equip_inventory(dawn, msg, slot, 0);
    }
}

void cmd_unequip(int pindex __attribute__((unused)), struct Message * msg) {
    if (check_if_matches_regex(msg->message, CMD_LIT"(\\d+)")) {
        int slot = atoi(regex_group[1]);
        equip_inventory(dawn, msg, slot, 1);
    }
}

void cmd_gmelee(int pindex __attribute__((unused)), struct Message * msg) {
    if (strcmp(msg->message, CMD_LIT) == 0) {
        player_attacks(dawn, msg, 1);
    }
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
    if (check_if_matches_regex(msg->message, CMD_LIT"(\\d+)")) {
        int slot = atoi(regex_group[1]);
        drop_item(dawn, msg, slot);
    }
}

void cmd_info(int pindex __attribute__((unused)), struct Message * msg) {
    if (check_if_matches_regex(msg->message, CMD_LIT"(\\d+)")) {
        int slot = atoi(regex_group[1]);
        get_item_info(dawn, msg, slot);
    }
}

void cmd_givexp(int pindex __attribute__((unused)), struct Message * msg) {
    if (check_if_matches_regex(msg->message, CMD_LIT"(\\w+) (\\d+)")) {
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
    if (check_if_matches_regex(msg->message, CMD_LIT"snow angels")) {
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
    if(check_if_matches_regex(msg->message, CMD_LIT"(\\d+)")) {
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
    if (check_if_matches_regex(msg->message, CMD_LIT"(\\w+) (\\d+)")) {
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
    if (check_if_matches_regex(msg->message, CMD_LIT"(\\d+),(\\d+)")) {
        move_player(dawn, msg, atoi(regex_group[1]), atoi(regex_group[2]));
    }
}

void cmd_locate(int pindex __attribute__((unused)), struct Message * msg) {
    if (check_if_matches_regex(msg->message, CMD_LIT"(\\w+)")) {
        find_building(dawn, msg, to_lower(regex_group[1]));
    }
}

void cmd_materials(int pindex __attribute__((unused)), struct Message * msg) {
    print_materials(dawn, msg);
}

void cmd_market(int pindex __attribute__((unused)), struct Message * msg) {
    if (check_if_matches_regex(msg->message, CMD_LIT"sell (\\w+) (\\d+)")) {
        char *eptr;
        long amount = strtol(regex_group[2], &eptr, 10);
        market_buysell(dawn, msg, 0, regex_group[1], amount);
    } else if (check_if_matches_regex(msg->message, CMD_LIT"buy (\\w+) (\\d+)")) {
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
    if (check_if_matches_regex(msg->message, CMD_LIT"gold (\\d+)")) {
        dawn->players[pindex].gold = atoi(regex_group[1]);
    }
}
