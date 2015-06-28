#include <stdio.h>
#include <pcre.h>
#include <string.h>
#include <stdlib.h>
#include "include/network.h"
#include "include/parse.h"
#include "include/status.h"
#include "include/player.h"
#include "include/inventory.h"
#include "include/combat.h"
#include "include/items.h"

//All groupers from the regular expression
//string will be stored here globally.
char regex_group[15][2048];

char *to_lower (char str[64]) {
    for (size_t i=0; i<strlen(str); i++) {
        if (str[i] >= 'A' && str[i] <= 'Z') {
            str[i] += 32;
        }
    }
    return str;
}

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
                            strlen(buf), 0, PCRE_NOTEOL, substring_vec, 30);
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
    send_socket(out);
    sprintf(out, "USER %s * * :%s\r\n", ident, real);
    send_socket(out);
    printf("%s\n", pass); //TODO:quieting down warnings for now but this will be nickserv
}

void parse_private_message (struct Bot *dawn, struct Message *message) {
    char out[MAX_MESSAGE_BUFFER];
    int pindex = get_pindex(dawn, message->sender_nick);

    if (pindex == -1) {
        sprintf(out, "PRIVMSG %s :You do not have an account\r\n", message->sender_nick);
        send_socket(out);
        return;
    }

    if (check_if_matches_regex(message->message, ";set password (\\w+)")) {
        if (strcmp(message->sender_hostmask, dawn->players[pindex].hostmask) == 0) {
            strcpy(dawn->players[pindex].password, regex_group[1]);
            sprintf(out, "PRIVMSG %s :Your password has been set\r\n", message->sender_nick);
        }
        send_socket(out);
    } else if (check_if_matches_regex(message->message, ";login (\\w+)")) {
        if (strcmp(dawn->players[pindex].hostmask, message->sender_hostmask) != 0) {
            if (strcmp(regex_group[1], dawn->players[pindex].password) == 0) {
                strcpy(dawn->players[pindex].hostmask, message->sender_hostmask);
                sprintf(out, "PRIVMSG %s :%s has been verified\r\n", dawn->active_room, message->sender_nick);
                send_socket(out);
                sprintf(out, "PRIVMSG %s :Password correct\r\n", message->sender_nick);
            } else {
                sprintf(out, "PRIVMSG %s :Incorrect password\r\n", message->sender_nick);
            }
        } else {
            sprintf(out, "PRIVMSG %s :I already recognize you, no need to log in\r\n", message->sender_nick);
        }
        send_socket(out);
    }
}

void parse_room_message (struct Bot *dawn, struct Message *message) {
    char out[MAX_MESSAGE_BUFFER];
    if (strcmp(message->message, ";new") == 0) {
        init_new_character(dawn, message);
    }

    //Check if a user has an account and is logged in from the correct host
    if (check_if_matches_regex(message->message, "^;(.*)")) {
        int pindex = get_pindex(dawn, message->sender_nick);
        if (pindex == -1) {
            sprintf(out, "PRIVMSG %s :Please create a new account by issuing ';new'\r\n", message->receiver);
            send_socket(out);
            return;
        }
        if (strcmp(dawn->players[pindex].hostmask, message->sender_hostmask) != 0) {
            sprintf(out, "PRIVMSG %s :%s, I seem to recall you connecting from a different host. Please login "
                    "by sending me a private message containing: ;login <your_password>\r\n", 
                    message->receiver, message->sender_nick);
            send_socket(out);
            return;
        }
    }

    if (strcmp(message->message, ";sheet") == 0) {
        print_sheet(dawn, message);
    } else if (check_if_matches_regex(message->message, ";sheet (\\w+)")) {
        strcpy(message->sender_nick, to_lower(regex_group[1]));
        if (get_pindex(dawn, regex_group[1]) != -1) {
            print_sheet(dawn, message);
        } else {
            printf("not found\n");
        }
    } else if (strcmp(message->message, ";inv") == 0) {
        print_inventory(dawn, message);
    } else if (check_if_matches_regex(message->message, ";equip (\\d+)")) {
        int slot = atoi(regex_group[1]);
        equip_inventory(dawn, message, slot, 0);
    } else if (check_if_matches_regex(message->message, ";unequip (\\d+)")) {
        int slot = atoi(regex_group[1]);
        equip_inventory(dawn, message, slot, 1);
    } else if (strcmp(message->message, ";gmelee") == 0) {
        player_attacks(dawn, message, 1);
    } else if (check_if_matches_regex(message->message, ";ghunt")) {
        if (!dawn->global_monster.active) {
            call_monster(dawn, message->sender_nick, 1);
        }
    } else if (strcmp(message->message, ";hunt") == 0) {
        int i = get_pindex(dawn, message->sender_nick);
        if (!dawn->players[i].personal_monster.active) {
            call_monster(dawn, message->sender_nick, 0);
        }
    } else if (strcmp(message->message, ";revive") == 0) {
        revive(dawn, message);
    } else if (check_if_matches_regex(message->message, ";drop (\\d+)")) {
        int slot = atoi(regex_group[1]);
        drop_item(dawn, message, slot);
    } else if (check_if_matches_regex(message->message, ";info (\\d+)")) {
        int slot = atoi(regex_group[1]);
        get_item_info(dawn, message, slot);
    } else if (check_if_matches_regex(message->message, ";givexp (\\w+) (\\d+)")) {
        //Just temp so I can level people back up who don't want to start over
        if (strcmp(message->sender_nick, "ziddy") == 0) {
            char username[64];
            char *eptr;
            int index;
            unsigned long long amount = strtoll(regex_group[2], &eptr, 10);

            strcpy(username, to_lower(regex_group[1]));
            index = get_pindex(dawn, username);
            dawn->players[index].experience += amount;
            if (index == -1) return;
            while (dawn->players[index].experience > get_nextlvl_exp(dawn, username)) {
                struct Message temp;
                strcpy(temp.sender_nick, username);
                strcpy(temp.receiver, dawn->active_room);
                check_levelup(dawn, &temp);
            }
        } else {
            sprintf(out, "PRIVMSG %s :You're not my real mother!\r\n", message->receiver);
            send_socket(out);
        }
    } else if (strcmp(message->message, ";make snow angels") == 0) {
        if (dawn->weather == SNOWING) {
            sprintf(out, "PRIVMSG %s :%s falls to the ground and begins making snow angels!\r\n",
                    message->receiver, message->sender_nick);
            send_socket(out);
        } else {
            sprintf(out, "PRIVMSG %s :There is no snow\r\n", message->receiver);
            send_socket(out);
        }
    } else if (strcmp(message->message, ";location") == 0) {
        print_location(dawn, get_pindex(dawn, message->sender_nick));
    } else if (strcmp(message->message, ";slay") == 0) {
        slay_monster(dawn, message->sender_nick, 0, 0);
    } else if (strcmp(message->message, ";gslay") == 0) {
        sprintf(out, "PRIVMSG %s :The %s will cost %d more gold to slay. Contribute to hiring a warrior "
                "with ;gslay <amount>\r\n", message->receiver, dawn->global_monster.name,
                dawn->global_monster.slay_cost);
        send_socket(out);
    } else if (check_if_matches_regex(message->message, ";gslay (\\d+)")) {
        slay_monster(dawn, message->sender_nick, 1, atoi(regex_group[1]));
    } else if (strcmp(message->message, ";check") == 0) {
        print_monster(dawn, message->sender_nick, 0);
    } else if (strcmp(message->message, ";gcheck") == 0) {
        print_monster(dawn, message->sender_nick, 1);
    } else if (strcmp(message->message, ";assign") == 0) {
        int pindex = get_pindex(dawn, message->sender_nick);
        sprintf(out, "PRIVMSG %s :To assign your attribute points, use \";assign <type> <amount>\"; type can be "
                "str, def, int or mdef. You have %d attribute points left which you can assign\r\n",
                message->receiver, dawn->players[pindex].attr_pts);
        send_socket(out);
    } else if (strcmp(message->message, ";ap") == 0) {
        int pindex = get_pindex(dawn, message->sender_nick);
        sprintf(out, "PRIVMSG %s :%s, you have %d attribute points which you can assign\r\n", 
                message->receiver, message->sender_nick, dawn->players[pindex].attr_pts);
        send_socket(out);
    } else if (check_if_matches_regex(message->message, ";assign (\\w+) (\\d+)")) {
        assign_attr_points(dawn, message, to_lower(regex_group[1]), atoi(regex_group[2]));
    } else if (check_if_matches_regex(message->message, ";travel (\\d+),(\\d+)")) {
        move_player(dawn, message, atoi(regex_group[1]), atoi(regex_group[2]));
    } else if (check_if_matches_regex(message->message, ";locate (\\w+)")) {
        find_building(dawn, message, to_lower(regex_group[1]));
    } else if (strcmp(message->message, ";materials") == 0) {
        int pindex = get_pindex(dawn, message->sender_nick);
        sprintf(out, "PRIVMSG %s :%s, [Materials: - Wood: %ld - Leather %ld - Stone: %ld - Ore: %ld - "
               " Bronze: %ld - Mail: %ld - Steel: %ld - Diamond: %ld]\r\n", 
               message->receiver, message->sender_nick, dawn->players[pindex].wood,
               dawn->players[pindex].leather, dawn->players[pindex].stone, dawn->players[pindex].ore,
               dawn->players[pindex].bronze, dawn->players[pindex].mail, dawn->players[pindex].steel,
               dawn->players[pindex].diamond);
        send_socket(out);
    } else if (strcmp(message->message, ";help") == 0) {
        sprintf(out, "PRIVMSG %s :;ghunt, ;hunt, ;gmelee, ;drop <slot>, ;inv, ;equip <slot>, ;unequip <slot>,"
                " ;info <slot>, ;sheet, ;sheet <user>, ;location, ;make snow angels, ;slay, ;gslay, ;check,"
                " ;gcheck, ;ap, ;assign, ;revive, ;locate <building>, ;location, ;travel <x,y>\r\n",
                message->receiver);
        send_socket(out);
    }
}

char *nultrm (char string[100]) {
    size_t len = strlen(string);
    string[len+1] = '\0';
    return string;
}
