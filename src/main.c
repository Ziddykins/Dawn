#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include "network.h"
#include "limits.h"
#include "parse.h"
#include "status.h"
#include "player.h"

int main (void) {
    int len, match;
    char dalnet[]  = "108.61.240.240";
    char port[]    = "6667";

    //Keep a NULL at the end
    char *rooms[]  = { "#stacked", NULL }, **n;
    n = rooms;

    Bot dawn;

    //Load characters
    //File must exist otherwise errno gets sad and it won't connect
    //to the server; touch players.db - if file is empty, it won't load it.
    if (access("players.db", F_OK) != -1) {
        FILE *file = fopen("players.db", "r");
        fseek(file, 0L, SEEK_END);
        int sz = ftell(file);
        fseek(file, 0L, SEEK_SET);
        if (sz > 0) {
            load_players(&dawn, sizeof(dawn));
        }
        fclose(file);
    }

    //Initial settings
    strcpy(dawn.nickname, "WellFuk");
    strcpy(dawn.realname, "Helo");
    strcpy(dawn.ident, "hehe");
    strcpy(dawn.password, "none");
    strcpy(dawn.active_room, rooms[0]);
    dawn.login_sent = 0;
    dawn.in_rooms = 0;
    if (!dawn.player_count) dawn.player_count = 0;

    init_timers(&dawn);

    if (init_connect_server(dalnet, port) == 0) { 
        printf("ok\n");
        while ((len = recv(con_socket, buffer, MAX_RECV_BUFFER, 0))) {
            char out[MAX_MESSAGE_BUFFER];
            buffer[len] = '\0';
            check_timers(&dawn);

            //Handle keepalive pings from the server
            if (check_if_matches_regex(buffer, "PING :(.*)")) {
                sprintf(out, "PONG: %s\r\n", regex_group[1]);
                send_socket(out);
            }

            //Connected to the server, send nick/user details
            if (dawn.login_sent == 0) {
                match = check_if_matches_regex(buffer, "AUTH");
                if (match) {
                    printf("[!] Server checking for AUTH, sending nick/user\n");
                    handle_login(dawn.nickname, dawn.password,
                                 dawn.realname, dawn.ident);
                    dawn.login_sent = 1;
                }
            }

            //If we're logged in and we're received a
            //welcome message, join the rooms
            if (!dawn.in_rooms && dawn.login_sent == 1) {
                match = check_if_matches_regex(buffer, ":(.*?)\\s001(.*)");
                if (match) {
                    printf("[!] Got welcome message from server\n");
                    while (*n != NULL) {
                        sprintf(out, "JOIN %s\r\n", *n++);
                        send_socket(out);
                    }
                    n = NULL;
                    dawn.in_rooms = 1;
                }
            }

            //Regular messages following the format:
            //nick!~ident@hostname receiver :message
            //Since this will match regular and private message, we should check
            //to see if the first character is an octothorpe (#)
            //In regex_group[4], we also check to see if this was a notice
            //or a privmsg, since they follow the same format
            if (dawn.in_rooms) {
                Message message;
                match = check_if_matches_regex(buffer, 
                        ":(.*?)!~?(.*?)@(.*?)\\s(.*?)\\s(.*?)\\s:(.*)\r\n");
                if (match) {
                    strncpy(message.sender_nick,     regex_group[1], 64);
                    strncpy(message.sender_ident,    regex_group[2], 64);
                    strncpy(message.sender_hostmask, regex_group[3], 64);
                    strncpy(message.receiver,        regex_group[5], 64);
                    strncpy(message.message,         regex_group[6], MAX_MESSAGE_BUFFER);
                    if (strcmp(regex_group[4], "PRIVMSG") == 0) {
                        if (message.receiver[0] == '#') {
                            parse_room_message(&message, &dawn);
                        } else {
                            parse_private_message(&message);
                        }
                    } else if (strcmp(regex_group[4], "NOTICE") == 0) {
                        printf("notice\n");
                        //handle notices
                    }
                }
            }
        }
    } else {
        printf("didn't connect: %d\n", errno);
        close(con_socket);
        return 1;
    }
    close(con_socket);
    return 0;
}
