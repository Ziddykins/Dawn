#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <time.h>
#include "status.h"
#include "player.h"
#include "network.h"
#include "limits.h"
#include "parse.h"

int main (void) {
    FILE *urandom = fopen("/dev/urandom", "r");
    unsigned int seed;
    if (urandom == NULL) {
        fprintf (stderr, "Cannot open /dev/urandom!\n");
        exit (1);
    }
    fread(&seed, sizeof (seed), 1, urandom);
    srand(seed);
    fclose(urandom);

    int match;
    ssize_t len;
    char dalnet[]  = "67.198.195.194";
    char port[]    = "6667";

    //Keep a NULL at the end
    char *rooms[]  = { "#stacked", NULL }, **n;
    n = rooms;
    
    //The bot structure it self
    struct Bot dawn;

    //Load characters
    if (access("players.db", F_OK) != -1) {
        FILE *file = fopen("players.db", "r");
        fseek(file, 0L, SEEK_END);
        ssize_t sz = ftell(file);
        fseek(file, 0L, SEEK_SET);
        if (sz > 0) {
            load_players(&dawn, sizeof(dawn));
        }
        fclose(file);
    } else {
        FILE *file = fopen("players.db", "w+");
        fclose(file);
        printf("Player database not found, creating\nPlease rerun\n");
        exit(1);
    }

    if (access("monsters.raw", F_OK) != -1) {
        FILE *file = fopen("monsters.raw", "r");
        char line[1024];
        char name[100];
        int count = 0;
        unsigned int hp, str, def, intel, mdef, gold, exp, mhp, drop_level;
        if (file != NULL) {
            while (fgets(line, sizeof(line), file)) {
                //TODO: Add ranges to raw files
                if (sscanf(line, "%[^:]:%u:%u:%u:%u:%u:%u:%u:%u:%u",
                        name, &hp, &str, &def, &intel, &mdef, &gold, &exp, &mhp, &drop_level) != 10) {
                    printf("Malformed monster file at line %d\n", count);
                    exit(1);
                }
                strcpy(dawn.monsters[count].name, name);
                dawn.monsters[count].hp     = hp;
                dawn.monsters[count].str    = str;
                dawn.monsters[count].def    = def;
                dawn.monsters[count].intel  = intel;
                dawn.monsters[count].mdef   = mdef;
                dawn.monsters[count].gold   = gold;
                dawn.monsters[count].exp    = exp;
                dawn.monsters[count].mhp    = mhp;
                dawn.monsters[count].drop_level = drop_level;
                dawn.monsters[count].active = 0;
                count++;
            }
            fclose(file);
        } else {
            printf("Error opening raw monsters\n");
            exit(1);
        }
    }

    //Initial settings
    strcpy(dawn.nickname, "WellFuk");
    strcpy(dawn.realname, "Helo");
    strcpy(dawn.ident,    "hehe");
    strcpy(dawn.password, "none");
    strcpy(dawn.active_room, rooms[0]);

    dawn.login_sent = 0;
    dawn.in_rooms   = 0;

    init_timers(&dawn);

    if (init_connect_server(dalnet, port) == 0) { 
        printf("[!] Connected to server %s\n", dalnet);
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
                    handle_login(dawn.nickname, dawn.password, dawn.realname, dawn.ident);
                    dawn.login_sent = 1;
                }
            }

            //If we're logged in and we're received a
            //welcome message, join the rooms
            if (!dawn.in_rooms && dawn.login_sent == 1) {
                match = check_if_matches_regex(buffer, ":(.*?)\\s001(.*)");
                if (match) {
                    printf("[!] Got welcome message from server, joining rooms\n");
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
            //Also checks for kicks, parts and joins and quits to control availability of users
            //Upon joining rooms the bot will search for status 353, which is names, and will
            //iterate through the online people, setting their status to available if they
            //have an account on the bot
            if (dawn.in_rooms) {
                struct Message message;
                //NAMES (Status 353)
                if (check_if_matches_regex(buffer, ":(.*?)\\s353\\s(.*?)\\s@\\s(.*?)\\s:(.*)")) {
                    char *ch_ptr;
                    ch_ptr = strtok(regex_group[4], " @&+");
                    while (ch_ptr != NULL) {
                        int index = get_pindex(&dawn, ch_ptr);
                        if (index != -1){
                            dawn.players[index].available = 1;
                        }
                        ch_ptr = strtok(NULL, " @&+\r\n");
                    }
                }
                //Kicks
                if (check_if_matches_regex(buffer, ":(.*?)!~?(.*?)@(.*?)\\s(\\w+)\\s(.*?)\\s(.*?)\\s:(.*)")) {
                    if (strcmp(regex_group[4], "KICK") == 0) {
                        int index = get_pindex(&dawn, regex_group[6]);
                        if (index != -1) {
                            dawn.players[index].available = 0;
                        }
                    }
                }
                //Parting and joining
                if (check_if_matches_regex(buffer, ":(.*?)!~?(.*?)@(.*?)\\s(.*?)\\s(.*?)")) {
                    int index = get_pindex(&dawn, regex_group[1]);
                    if (index != -1) {
                        if (strcmp(regex_group[4], "PART") == 0) {
                            dawn.players[index].available = 0;
                        } else if (strcmp(regex_group[4], "JOIN") == 0) {
                            dawn.players[index].available = 1;
                        } else if (strcmp(regex_group[4], "QUIT") == 0) {
                            dawn.players[index].available = 0;
                        }
                    }
                }
                //Regular messages and notices
                if (check_if_matches_regex(buffer, ":(.*?)!~?(.*?)@(.*?)\\s(.*?)\\s(.*?)\\s:(.*)\r\n")) {
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
        printf("[*] Could not connect, error: %d\n", errno);
        close(con_socket);
        return 1;
    }
    close(con_socket);
    printf("[!] Program exiting normally\n");
    return 0;
}
