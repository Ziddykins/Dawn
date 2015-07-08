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



/*  if (check_if_matches_regex(msg->message, ";givexp (\\w+) (\\d+)")) {
        //Just temp so I can level people back up who don't want to start over
        if (strcmp(msg->sender_nick, "ziddy") == 0) {
            char * username = calloc(MAX_NICK_LENGTH, 1);
            char *eptr;
            int index;
            unsigned long long amount = strtoull(regex_group[2], &eptr, 10);

            strncpy(username, to_lower(regex_group[1]), MAX_NICK_LENGTH);
            index = get_pindex(dawn, username);
            b->players[index].experience += amount;

            if (index == -1) return;

            while (b->players[index].experience > get_nextlvl_exp(dawn, username)) {
                struct Message temp;
                strcpy(temp.sender_nick, username);
                strcpy(temp.receiver, b->active_room);
                check_levelup(dawn, &temp);
            }
        } else {
            sprintf(out, "PRIVMSG %s :You're not my real mother!\r\n", msg->receiver);
            addMsg(out, strlen(out));
        }
    } else if (strcmp(msg->message, ";make snow angels") == 0) {
        if (b->weather == SNOWING) {
            sprintf(out, "PRIVMSG %s :%s falls to the ground and begins making snow angels!\r\n",
                    msg->receiver, msg->sender_nick);
            addMsg(out, strlen(out));
        } else {
            sprintf(out, "PRIVMSG %s :There is no snow\r\n", msg->receiver);
            addMsg(out, strlen(out));
        }
    } else if (strcmp(msg->message, ";location") == 0) {
        print_location(dawn, get_pindex(dawn, msg->sender_nick));
    } else if (strcmp(msg->message, ";slay") == 0) {
        slay_monster(dawn, msg->sender_nick, 0, 0);
    } else if (strcmp(msg->message, ";gslay") == 0) {
        sprintf(out, "PRIVMSG %s :The %s will cost %d more gold to slay. Contribute to hiring a warrior "
                "with ;gslay <amount>\r\n", msg->receiver, b->global_monster.name,
                b->global_monster.slay_cost);
        addMsg(out, strlen(out));
    } else if (check_if_matches_regex(msg->message, ";gslay (\\d+)")) {
        slay_monster(dawn, msg->sender_nick, 1, atoi(regex_group[1]));
    } else if (strcmp(msg->message, ";check") == 0) {
        print_monster(dawn, msg->sender_nick, 0);
    } else if (strcmp(msg->message, ";gcheck") == 0) {
        print_monster(dawn, msg->sender_nick, 1);
    } else if (strcmp(msg->message, ";assign") == 0) {
        int pindex = get_pindex(dawn, msg->sender_nick);
        sprintf(out, "PRIVMSG %s :To assign your attribute points, use \";assign <type> <amount>\"; type can be "
                "str, def, int or mdef. You have %d attribute points left which you can assign\r\n",
                msg->receiver, b->players[pindex].attr_pts);
        addMsg(out, strlen(out));
    } else if (strcmp(msg->message, ";ap") == 0) {
        int pindex = get_pindex(dawn, msg->sender_nick);
        sprintf(out, "PRIVMSG %s :%s, you have %d attribute points which you can assign\r\n",
                msg->receiver, msg->sender_nick, b->players[pindex].attr_pts);
        addMsg(out, strlen(out));
    } else if (check_if_matches_regex(msg->message, ";assign (\\w+) (\\d+)")) {
        assign_attr_points(dawn, message, to_lower(regex_group[1]), atoi(regex_group[2]));
    } else if (check_if_matches_regex(msg->message, ";travel (\\d+),(\\d+)")) {
        move_player(dawn, message, atoi(regex_group[1]), atoi(regex_group[2]));
    } else if (check_if_matches_regex(msg->message, ";locate (\\w+)")) {
        find_building(dawn, message, to_lower(regex_group[1]));
    } else if (strcmp(msg->message, ";materials") == 0) {
        print_materials(dawn, message);
    } else if (check_if_matches_regex(msg->message, ";market sell (\\w+) (\\d+)")) {
        char *eptr;
        long amount = strtol(regex_group[2], &eptr, 10);
        market_buysell(dawn, message, 0, regex_group[1], amount);
    } else if (check_if_matches_regex(msg->message, ";market buy (\\w+) (\\d+)")) {
        char *eptr;
        long amount = strtol(regex_group[2], &eptr, 10);
        market_buysell(dawn, message, 1, regex_group[1], amount);
    } else if (strcmp(msg->message, ";save") == 0) {
        persistent_save(dawn);
        sprintf(out, "PRIVMSG %s :Saved.\r\n", msg->receiver);
        addMsg(out, strlen(out));
    } else if (strcmp(msg->message, ";market") == 0) {
        print_market(b);
    } else if (strcmp(msg->message, ";[") == 0) {
        sprintf(out, "PRIVMSG %s :You break down in tears\r\n", msg->receiver);
        addMsg(out, strlen(out));
    } else if (strcmp(msg->message, ";gib gold") == 0) {
        int pindex = get_pindex(dawn, msg->sender_nick);
        b->players[pindex].gold = INT_MAX;
    } else if (strcmp(msg->message, ";help") == 0) {
        sprintf(out, "PRIVMSG %s :;ghunt, ;hunt, ;gmelee, ;drop <slot>, ;inv, ;equip <slot>, ;unequip <slot>,"
                " ;info <slot>, ;sheet, ;sheet <user>, ;location, ;make snow angels, ;slay, ;gslay, ;check,"
                " ;gcheck, ;ap, ;assign, ;revive, ;locate <building>, ;location, ;travel <x,y>\r\n",
                msg->receiver);
        addMsg(out, strlen(out));
    }
}*/
