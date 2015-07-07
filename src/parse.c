#include "include/parse.h"

//All groupers from the regular expression
//string will be stored here globally.
char regex_group[15][2048];

char *to_lower (char str[MAX_MESSAGE_BUFFER]) {
    for (size_t i=0; i<strlen(str); i++) {
        if (str[i] >= 'A' && str[i] <= 'Z') {
            str[i] += 32;
        }
    }
    return str;
}

char *xor_flip (char * password) { //password -> 64
    unsigned int key = 0xDEADBEBE;
    for (size_t i=0; i<strlen(password); i++)
        password[i] ^= key;
    return password;
}

void genSalt(char * salt, size_t len) {
    size_t pos;
    for(pos = 0; pos < len-1; pos++) {
        do {
            salt[pos] = (char)(rand() % 256 - 128); //char -> (-128...127); mod 2^x does not skew random distribution
        } while(salt[pos] != 0);
    }
    salt[pos] = '\0';
}

void hashPwd(unsigned char * digest, char const * salt, char const * password) {
    size_t concatlen = strnlen(salt, 16) + strnlen(password, 64) + 2;
    char * concat = malloc(concatlen);
    snprintf(concat, concatlen, "!%s%s", password, salt);
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, concat, strlen(concat));
    SHA256_Final(digest, &sha256);
    free(concat);
}

int hashcmp(unsigned char const * s1, unsigned char const * s2) {
    int i;
    for(i = 0; i < SHA256_DIGEST_LENGTH && s1 && *s1 && s2 && *s2 && *s1 == *s2; i++) {
        s1++;
        s2++;
    }
    return i == SHA256_DIGEST_LENGTH;
}

/*char *nultrm (char str[MAX_MESSAGE_BUFFER]) {
    size_t len = strlen(str);
    str[len+1] = '\0';
    return str;
}*/

int check_if_matches_regex (char *buf, const char *regular_expression) {
    pcre *regex_compiled;
    pcre_extra *pcre_optimized;
    int pcre_return;
    int pcre_error_offset;
    int substring_vec[30];
    const char *psubStrMatchStr;
    const char *pcre_error;

    regex_compiled = pcre_compile(regular_expression, PCRE_MULTILINE,
                                  &pcre_error, &pcre_error_offset, NULL);

    if (regex_compiled == NULL) {
        printf("Could not compile regular expression: %s (%s)\n",
                regular_expression, pcre_error);
        exit(1);
    }

    pcre_optimized = pcre_study(regex_compiled, 0, &pcre_error);

    if (pcre_error != NULL) {
        printf("Could not optimize regular expression: %s (%s)\n", regular_expression, pcre_error);
        exit(1);
    }

    pcre_return = pcre_exec(regex_compiled, pcre_optimized, buf,
                            (int)strlen(buf), 0, PCRE_NOTEOL, substring_vec, 30);
    pcre_free(regex_compiled);
    pcre_free(pcre_optimized);

    if (pcre_return > 0) {
        int j;
        for (j=0; j<pcre_return; j++) {
            pcre_get_substring(buf, substring_vec, pcre_return, j, &(psubStrMatchStr));
            strcpy(regex_group[j], psubStrMatchStr);
            pcre_free_substring(psubStrMatchStr);
        }
        //We've found a match
        return 1;
    } else {
        //We've not found a match
        return 0;
    }
}

void handle_login (char *nick, char *pass, char *real, char *ident) {
    char out[MAX_MESSAGE_BUFFER];
    sprintf(out, "NICK %s\r\n", nick);
    addMsg(out, strlen(out));
    sprintf(out, "USER %s * * :%s\r\n", ident, real);
    addMsg(out, strlen(out));
    printf("%s\n", pass); //TODO:quieting down warnings for now but this will be nickserv
}

void parse_private_message (struct Bot *b, struct Message *message) {
    char out[MAX_MESSAGE_BUFFER];
    int pindex = get_pindex(b, message->sender_nick);

    if (pindex == -1) {
        sprintf(out, "PRIVMSG %s :You do not have an account\r\n", message->sender_nick);
        addMsg(out, strlen(out));
        return;
    }

    if (check_if_matches_regex(message->message, ";set password (\\w+)")) {
        if (strcmp(message->sender_hostmask, b->players[pindex].hostmask) == 0) {
            hashPwd(b->players[pindex].pwd, b->players[pindex].salt, regex_group[1]);
            sprintf(out, "PRIVMSG %s :Your password has been set\r\n", message->sender_nick);
        }
        addMsg(out, strlen(out));
    } else if (check_if_matches_regex(message->message, ";login (\\w+)")) {
        if (strcmp(b->players[pindex].hostmask, message->sender_hostmask) != 0) {
            unsigned char hash[SHA256_DIGEST_LENGTH];
            hashPwd(hash, b->players[pindex].salt, regex_group[1]);
            if (hashcmp(hash, b->players[pindex].pwd)) {
                strcpy(b->players[pindex].hostmask, message->sender_hostmask);
                sprintf(out, "PRIVMSG %s :%s has been verified\r\n", b->active_room, message->sender_nick);
                addMsg(out, strlen(out));
                sprintf(out, "PRIVMSG %s :Password correct\r\n", message->sender_nick);
            } else {
                sprintf(out, "PRIVMSG %s :Incorrect password\r\n", message->sender_nick);
            }
        } else {
            sprintf(out, "PRIVMSG %s :I already recognize you, no need to log in\r\n", message->sender_nick);
        }
        addMsg(out, strlen(out));
    }
}

int command_allowed (struct Bot *b, char * command, int pindex) { //command -> 16
    if (b->players[pindex].travel_timer.active) {
        char disallowed[2][16] = {"melee", "gmelee"}; //remember to change array limits
        for (int i=0; i<2; i++) {
            printf("recv '%s' iter '%s'\n", command, disallowed[i]);
            if (strcmp(command, disallowed[i]) == 0) return 0;
        }
    }
    return 1;
}

void parse_room_message (struct Bot *b, struct Message *message) {
    char out[MAX_MESSAGE_BUFFER];
    if (strcmp(message->message, ";new") == 0) {
        init_new_character(b, message);
    }

    //Check if a user has an account and is logged in from the correct host
    if (check_if_matches_regex(message->message, "^;(.*)")) {
        int pindex = get_pindex(b, message->sender_nick);

        //store command to check if user can execute it in current state
        check_if_matches_regex(message->message, "^;(.*?)\\s");

        if (pindex == -1) {
            sprintf(out, "PRIVMSG %s :Please create a new account by issuing ';new'\r\n", message->receiver);
            addMsg(out, strlen(out));
            return;
        }
        if (strcmp(b->players[pindex].hostmask, message->sender_hostmask) != 0) {
            sprintf(out, "PRIVMSG %s :%s, I seem to recall you connecting from a different host. Please login "
                    "by sending me a private message containing: ;login <your_password>\r\n",
                    message->receiver, message->sender_nick);
            addMsg(out, strlen(out));
            return;
        }
        if (!command_allowed(b, regex_group[1], pindex)) {
            sprintf(out, "PRIVMSG %s :This command cannot be used at this time\r\n",  message->receiver);
            addMsg(out, strlen(out));
            return;
        }
    }

    if (strcmp(message->message, ";sheet") == 0) {
        print_sheet(b, message);
    } else if (check_if_matches_regex(message->message, ";sheet (\\w+)")) {
        strncpy(message->sender_nick, to_lower(regex_group[1]), MAX_NICK_LENGTH);
        if (get_pindex(b, regex_group[1]) != -1) {
            print_sheet(b, message);
        }
    } else if (strcmp(message->message, ";inv") == 0) {
        print_inventory(b, message);
    } else if (check_if_matches_regex(message->message, ";equip (\\d+)")) {
        int slot = atoi(regex_group[1]);
        equip_inventory(b, message, slot, 0);
    } else if (check_if_matches_regex(message->message, ";unequip (\\d+)")) {
        int slot = atoi(regex_group[1]);
        equip_inventory(b, message, slot, 1);
    } else if (strcmp(message->message, ";gmelee") == 0) {
        player_attacks(b, message, 1);
    } else if (check_if_matches_regex(message->message, ";ghunt")) {
        if (!b->global_monster.active) {
            call_monster(b, message->sender_nick, 1);
        }
    } else if (strcmp(message->message, ";hunt") == 0) {
        int i = get_pindex(b, message->sender_nick);
        if (!b->players[i].personal_monster.active) {
            call_monster(b, message->sender_nick, 0);
        }
    } else if (strcmp(message->message, ";revive") == 0) {
        revive(b, message);
    } else if (check_if_matches_regex(message->message, ";drop (\\d+)")) {
        int slot = atoi(regex_group[1]);
        drop_item(b, message, slot);
    } else if (check_if_matches_regex(message->message, ";info (\\d+)")) {
        int slot = atoi(regex_group[1]);
        get_item_info(b, message, slot);
    } else if (check_if_matches_regex(message->message, ";givexp (\\w+) (\\d+)")) {
        //Just temp so I can level people back up who don't want to start over
        if (strcmp(message->sender_nick, "ziddy") == 0) {
            char * username = calloc(MAX_NICK_LENGTH, 1);
            char *eptr;
            int index;
            unsigned long long amount = strtoull(regex_group[2], &eptr, 10);

            strncpy(username, to_lower(regex_group[1]), MAX_NICK_LENGTH);
            index = get_pindex(b, username);
            b->players[index].experience += amount;

            if (index == -1) return;

            while (b->players[index].experience > get_nextlvl_exp(b, username)) {
                struct Message temp;
                strcpy(temp.sender_nick, username);
                strcpy(temp.receiver, b->active_room);
                check_levelup(b, &temp);
            }
        } else {
            sprintf(out, "PRIVMSG %s :You're not my real mother!\r\n", message->receiver);
            addMsg(out, strlen(out));
        }
    } else if (strcmp(message->message, ";make snow angels") == 0) {
        if (b->weather == SNOWING) {
            sprintf(out, "PRIVMSG %s :%s falls to the ground and begins making snow angels!\r\n",
                    message->receiver, message->sender_nick);
            addMsg(out, strlen(out));
        } else {
            sprintf(out, "PRIVMSG %s :There is no snow\r\n", message->receiver);
            addMsg(out, strlen(out));
        }
    } else if (strcmp(message->message, ";location") == 0) {
        print_location(b, get_pindex(b, message->sender_nick));
    } else if (strcmp(message->message, ";slay") == 0) {
        slay_monster(b, message->sender_nick, 0, 0);
    } else if (strcmp(message->message, ";gslay") == 0) {
        sprintf(out, "PRIVMSG %s :The %s will cost %d more gold to slay. Contribute to hiring a warrior "
                "with ;gslay <amount>\r\n", message->receiver, b->global_monster.name,
                b->global_monster.slay_cost);
        addMsg(out, strlen(out));
    } else if (check_if_matches_regex(message->message, ";gslay (\\d+)")) {
        slay_monster(b, message->sender_nick, 1, atoi(regex_group[1]));
    } else if (strcmp(message->message, ";check") == 0) {
        print_monster(b, message->sender_nick, 0);
    } else if (strcmp(message->message, ";gcheck") == 0) {
        print_monster(b, message->sender_nick, 1);
    } else if (strcmp(message->message, ";assign") == 0) {
        int pindex = get_pindex(b, message->sender_nick);
        sprintf(out, "PRIVMSG %s :To assign your attribute points, use \";assign <type> <amount>\"; type can be "
                "str, def, int or mdef. You have %d attribute points left which you can assign\r\n",
                message->receiver, b->players[pindex].attr_pts);
        addMsg(out, strlen(out));
    } else if (strcmp(message->message, ";ap") == 0) {
        int pindex = get_pindex(b, message->sender_nick);
        sprintf(out, "PRIVMSG %s :%s, you have %d attribute points which you can assign\r\n",
                message->receiver, message->sender_nick, b->players[pindex].attr_pts);
        addMsg(out, strlen(out));
    } else if (check_if_matches_regex(message->message, ";assign (\\w+) (\\d+)")) {
        assign_attr_points(b, message, to_lower(regex_group[1]), atoi(regex_group[2]));
    } else if (check_if_matches_regex(message->message, ";travel (\\d+),(\\d+)")) {
        move_player(b, message, atoi(regex_group[1]), atoi(regex_group[2]));
    } else if (check_if_matches_regex(message->message, ";locate (\\w+)")) {
        find_building(b, message, to_lower(regex_group[1]));
    } else if (strcmp(message->message, ";materials") == 0) {
        print_materials(b, message);
    } else if (check_if_matches_regex(message->message, ";market sell (\\w+) (\\d+)")) {
        char *eptr;
        long amount = strtol(regex_group[2], &eptr, 10);
        market_buysell(b, message, 0, regex_group[1], amount);
    } else if (check_if_matches_regex(message->message, ";market buy (\\w+) (\\d+)")) {
        char *eptr;
        long amount = strtol(regex_group[2], &eptr, 10);
        market_buysell(b, message, 1, regex_group[1], amount);
    } else if (strcmp(message->message, ";save") == 0) {
        persistent_save(dawn);
        sprintf(out, "PRIVMSG %s :Saved.\r\n", message->receiver);
        addMsg(out, strlen(out));
    } else if (strcmp(message->message, ";market") == 0) {
        print_market(b);
    } else if (strcmp(message->message, ";gib gold") == 0) {
        int pindex = get_pindex(b, message->sender_nick);
        b->players[pindex].gold = INT_MAX;
    } else if (strcmp(message->message, ";help") == 0) {
        sprintf(out, "PRIVMSG %s :;ghunt, ;hunt, ;gmelee, ;drop <slot>, ;inv, ;equip <slot>, ;unequip <slot>,"
                " ;info <slot>, ;sheet, ;sheet <user>, ;location, ;make snow angels, ;slay, ;gslay, ;check,"
                " ;gcheck, ;ap, ;assign, ;revive, ;locate <building>, ;location, ;travel <x,y>\r\n",
                message->receiver);
        addMsg(out, strlen(out));
    }
}
