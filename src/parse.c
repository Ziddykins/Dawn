#include <stdio.h>
#include <pcre.h>
#include <string.h>
#include "network.h"
#include "parse.h"
#include "status.h"
#include "player.h"
#include "inventory.h"
#include "combat.h"
#include "items.h"

char regex_group[15][2048];

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

void parse_private_message (struct Message *message) {
    printf("<%s(%s)%s> %s\n",
            message->sender_nick, message->sender_ident, message->sender_hostmask, message->message);
}

void parse_room_message (struct Message *message, struct Bot *dawn) {
    char out[MAX_MESSAGE_BUFFER];
/*    printf("%s <%s(%s)%s> %s\n", message->receiver, 
            message->sender_nick, message->sender_ident, message->sender_hostmask, message->message);*/
    if (strcmp(message->message, ";new") == 0) {
        //TODO: Add a proper login system.
        //Thought: Add hostmask. If hostmask matches nick, we're good
        //If not : Give ability to add hostmask if password matches
        init_new_character(message->sender_nick, "temp", dawn);
    }
    //To avoid fiery death, check if user is sending a command,
    //and if so, check to see if the user exists. get_pindex() will
    //return -1 if the user isn't found, which, doesn't work well
    //for array bounds.
    if (check_if_matches_regex(message->message, "^;(.*)")) {
        if (get_pindex(dawn, message->sender_nick) == -1) {
            sprintf(out, "PRIVMSG %s :Please create a new account by issuing ';new'\r\n", message->receiver);
            send_socket(out);
            return;
        }
    }

    if (strcmp(message->message, ";sheet") == 0) {
        print_sheet(dawn, message);
    } else if (check_if_matches_regex(message->message, ";sheet (\\w+)")) {
        strcpy(message->sender_nick, regex_group[1]);
        if (get_pindex(dawn, regex_group[1]) != -1) {
            print_sheet(dawn, message);
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
    } else if (strcmp(message->message, ";rev ples") == 0) {
        int i = get_pindex(dawn, message->sender_nick);
        dawn->players[i].health = dawn->players[i].max_health;
        dawn->players[i].alive  = 1;
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
            int amount = atoi(regex_group[2]);
            int index;
            strcpy(username, regex_group[1]);
            index = get_pindex(dawn, username);
            dawn->players[index].experience += amount;
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
    } else if (strcmp(message->message, ";help") == 0) {
        sprintf(out, "PRIVMSG %s :;ghunt, ;hunt, ;gmelee, ;drop <slot>, ;inv, ;equip <slot>, ;unequip <slot>,"
                " ;info <slot>, ;sheet, ;sheet <user>, ;location, ;make snow angels, ;slay, ;gslay, ;check,"
                " ;gcheck\r\n",
                message->receiver);
        send_socket(out);
    }
}

char *nultrm (char string[100]) {
    size_t len = strlen(string);
    string[len+1] = '\0';
    return string;
}
