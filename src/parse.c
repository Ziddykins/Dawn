#include <stdio.h>
#include <pcre.h>
#include <string.h>
#include "network.h"
#include "parse.h"
#include "status.h"
#include "player.h"
#include "inventory.h"
#include "combat.h"

char regex_group[15][2048];

int check_if_matches_regex (char *buffer, char *regular_expression) {
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

    pcre_return = pcre_exec(regex_compiled, pcre_optimized, buffer,
                            strlen(buffer), 0, PCRE_NOTEOL, substring_vec, 30);
    pcre_free(regex_compiled);
    pcre_free(pcre_optimized);

    if (pcre_return > 0) {
        int j;
        for (j=0; j<pcre_return; j++) {
            pcre_get_substring(buffer, substring_vec, pcre_return, j, &(psubStrMatchStr));
            strcpy(regex_group[j], psubStrMatchStr);
            pcre_free_substring(psubStrMatchStr);
        }
        return 1;
    } else {
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

void parse_private_message (Message *message) {
    printf("<%s(%s)%s> %s\n",
            message->sender_nick, message->sender_ident, message->sender_hostmask, message->message);
}

void parse_room_message (Message *message, Bot *dawn) {
/*    printf("%s <%s(%s)%s> %s\n", message->receiver, 
            message->sender_nick, message->sender_ident, message->sender_hostmask, message->message);*/

    //To avoid fiery death, check if user is sending a command,
    //and if so, check to see if the user exists. get_pindex() will
    //return -1 if the user isn't found, which, doesn't work well
    //for array bounds.
    if (check_if_matches_regex(message->message, "^;(.*)")) {
        if (get_pindex(dawn, message->sender_nick) == -1) {
            char out[MAX_MESSAGE_BUFFER];
            sprintf(out, "PRIVMSG %s :Please create a new account by issuing ';new'\r\n", message->receiver);
            send_socket(out);
            return;
        }
    }

    if (strcmp(message->message, ";new") == 0) {
        init_new_character(message->sender_nick, "temp", dawn, message);
    } else if (strcmp(message->message, ";sheet") == 0) {
        print_sheet(dawn, message);
    } else if (check_if_matches_regex(message->message, ";sheet (\\w+)")) {
        strcpy(message->sender_nick, regex_group[1]);
        print_sheet(dawn, message);
    } else if (strcmp(message->message, ";inv") == 0) {
        print_inventory(dawn, message);
    } else if (check_if_matches_regex(message->message, ";equip (\\d+)")) {
        int slot = atoi(regex_group[1]);
        equip_inventory(dawn, message, slot, 0);
    } else if (check_if_matches_regex(message->message, ";unequip (\\d+)")) {
        int slot = atoi(regex_group[1]);
        equip_inventory(dawn, message, slot, 1);
    } else if (strcmp(message->message, ";gmelee") == 0) {
        player_attacks(dawn, message, 1, 0);
    } else if (check_if_matches_regex(message->message, ";gib (\\d+)")) {
        call_monster(dawn, atoi(regex_group[1]));
        set_timer(BATTLE, dawn, BATTLE_INTERVAL);
    } else if (strcmp(message->message, ";rev ples") == 0) {
        int i = get_pindex(dawn, message->sender_nick);
        dawn->players[i].health = 100;
    }
}
