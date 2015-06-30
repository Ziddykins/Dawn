#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <time.h>
#include "include/status.h"
#include "include/player.h"
#include "include/network.h"
#include "include/limits.h"
#include "include/parse.h"

//External global defined in limit.h
//Determined by server - if values not received on connect, default to 64
int MAX_CHANNEL_LENGTH = 64;
int MAX_NICK_LENGTH    = 64;

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
    const char dalnet[]  = "154.35.175.101";
    const char port[]    = "6667";

    //Keep a NULL at the end
    const char *rooms[]  = { "#stacked", NULL }, **n;
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
        dawn.global_monster.active = 0;
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
        int hp, str, def, intel, mdef, gold, exp, mhp, drop_level;
        if (file != NULL) {
            while (fgets(line, sizeof(line), file)) {
                //TODO: Add ranges to raw files
                if (sscanf(line, "%[^:]:%d:%d:%d:%d:%d:%d:%d:%d:%d",
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
                dawn.monsters[count].slay_cost  = gold;
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
    strcpy(dawn.nickname, "Ziddy");
    strcpy(dawn.realname, "Helo");
    strcpy(dawn.ident,    "hehe");
    strcpy(dawn.password, "none");
    strcpy(dawn.active_room, rooms[0]);

    dawn.login_sent = 0;
    dawn.in_rooms   = 0;
    init_send_queue();
    if (init_connect_server(dalnet, port) == 0) {
        printf("[!] Connected to server %s\n", dalnet);
        init_timers(&dawn);
        while ((len = recv(con_socket, buffer, MAX_RECV_BUFFER, 0))) {
            char out[MAX_MESSAGE_BUFFER];
            buffer[len] = '\0';

            //Handle keepalive pings from the server
            if (check_if_matches_regex(buffer, "PING :(.*)")) {
                sprintf(out, "PONG :%s\r\n", regex_group[1]);
                addMsg(out, strlen(out));
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

            //Nickname is in use, add a random suffix
            if (check_if_matches_regex(buffer, ":.*?\\s433\\s*\\s.*")) {
                int rand_suffix = rand() % 5000;
                printf("Username %s in use\n", dawn.nickname);
                sprintf(dawn.nickname, "%s%d", dawn.nickname, rand_suffix);
                handle_login(dawn.nickname, dawn.password, dawn.realname, dawn.ident);
            }


            //If we're logged in and we're received a
            //welcome message, join the rooms
            if (!dawn.in_rooms && dawn.login_sent == 1) {
                match = check_if_matches_regex(buffer, ":(.*?)\\s001(.*)");
                if (match) {
                    printf("[!] Got welcome message from server, joining rooms\n");
                    while (*n != NULL) {
                        sprintf(out, "JOIN %s\r\n", *n++);
                        addMsg(out, strlen(out));
                    }
                    n = NULL;
                    dawn.in_rooms = 1;
                }
            }

            //Upon joining rooms the bot will search for status 353, which is names, and will
            //iterate through the online people, setting their status to available if they
            //have an account on the bot
            if (dawn.in_rooms) {
                struct Message message;

                //Set lengths based on server rather than assuming
                if (check_if_matches_regex(buffer, ":.*?005.*?CHANNELLEN=(\\d+)\\s.*?NICKLEN=(\\d+)")) {
                    MAX_CHANNEL_LENGTH = atoi(regex_group[1]) + 1;
                    MAX_NICK_LENGTH    = atoi(regex_group[2]) + 1;
                }

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
                    int index = get_pindex(&dawn, to_lower(regex_group[1]));
                    if (index != -1) {
                        if (strcmp(regex_group[4], "PART") == 0) {
                            dawn.players[index].available = 0;
                        } else if (strcmp(regex_group[4], "JOIN") == 0) {
                            dawn.players[index].available = 1;
                        } else if (strcmp(regex_group[4], "QUIT") == 0) {
                            dawn.players[index].available = 0;
                        }
                    }
                    if (dawn.player_count == 0) {
                        struct Message temp;
                        strcpy(temp.sender_nick, to_lower(dawn.nickname));
                        strcpy(temp.sender_hostmask, nultrm(regex_group[3]));
                        init_new_character(&dawn, &temp);
                    }
                }
                //Regular messages and notices
                if (check_if_matches_regex(buffer, ":(.*?)!~?(.*?)@(.*?)\\s(.*?)\\s(.*?)\\s:(.*)\r\n")) {
                    strncpy(message.sender_nick,     to_lower(regex_group[1]), 64);
                    strncpy(message.sender_ident,    regex_group[2], 64);
                    strncpy(message.sender_hostmask, regex_group[3], 64);
                    strncpy(message.receiver,        regex_group[5], 64);
                    strncpy(message.message,         regex_group[6], MAX_MESSAGE_BUFFER);
                    if (strcmp(regex_group[4], "PRIVMSG") == 0) {
                        if (message.receiver[0] == '#') {
                            parse_room_message(&dawn, &message);
                        } else {
                            parse_private_message(&dawn, &message);
                        }
                    } else if (strcmp(regex_group[4], "NOTICE") == 0) {
                    //    parse_notice_message(&message);
                    }
                }
            }
        }
    } else {
        perror("Failed to connect");
        close(con_socket);
        return 1;
    }
    close(con_socket);
    printf("[!] Program exiting normally\n");
    return 0;
}
