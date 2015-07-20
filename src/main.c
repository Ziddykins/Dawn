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
#include "include/commands.h"
#include "include/util.h"

//External global defined in limit.h
//Determined by server - if values not received on connect, default to 64
unsigned int MAX_CHANNEL_LENGTH = 64;
unsigned int MAX_NICK_LENGTH    = 64;
char * auth_key;

int main (void) {
    FILE *urandom;
    CALLEXIT(!(urandom = fopen("/dev/urandom", "r")))

    unsigned int seed;
    CALLEXIT(!(fread(&seed, sizeof (seed), 1, urandom)))
    srand(seed);
    fclose(urandom);

    FILE *auth_key_file;
    CALLEXIT(!(auth_key_file = fopen("auth_key.txt", "w")))

    CALLEXIT(!(auth_key = calloc(AUTH_KEY_LEN+1, 1)))
    for(size_t i = 0; i < AUTH_KEY_LEN; i++) {
        do {
            auth_key[i] = rand() % 32;
        } while(auth_key[i] >= 26);
        if(rand()%2) {
            auth_key[i] += 'a';
        } else {
            auth_key[i] += 'A';
        }
    }

    CALLEXIT(!(fwrite(auth_key, AUTH_KEY_LEN, sizeof *auth_key, auth_key_file)))

    fclose(auth_key_file);

    printf(INFO "auth_key: %s\n", auth_key);
    auth_key_valid = 1;

    int match;
    ssize_t len;
    const char dalnet[]  = "154.35.175.101";
    const char port[]    = "6667";

    //Keep a NULL at the end
    const char *rooms[]  = { "#stacked", NULL }, **n;
    n = rooms;

    //The bot structure it self
    CALLEXIT(!(dawn = calloc(1, sizeof *dawn)))

    //Load characters
/*
    if (access("players.db", F_OK) != -1) {
        FILE *file = fopen("players.db", "r");
        fseek(file, 0L, SEEK_END);
        ssize_t sz = ftell(file);
        fseek(file, 0L, SEEK_SET);
        if (sz > 0) {
            load_players(&dawn);
        }
        fclose(file);
        dawn->global_monster.active = 0;
    } else {
        FILE *file = fopen("players.db", "w+");
        fclose(file);
        exit(1);
    }*/
    persistent_load(dawn);

    if (access("monsters.raw", F_OK) != -1) {
        FILE *file = fopen("monsters.raw", "r");
        char line[1024];
        char name[100];
        int count = 0;
        int hp, str, def, intel, mdef, gold, exp, mhp, drop_level;
        if (!file) {
            fprintf(stderr, ERR "main: Error opening raw monsters\n");
            return 1;
        }
        while (fgets(line, sizeof(line), file)) {
            //TODO: Add ranges to raw files
            if (sscanf(line, "%[^:]:%d:%d:%d:%d:%d:%d:%d:%d:%d",
                    name, &hp, &str, &def, &intel, &mdef, &gold, &exp, &mhp, &drop_level) != 10) {
                fprintf(stderr, ERR "main: Malformed monster file at line %d\n", count);
                return 1;
            }
            strcpy(dawn->monsters[count].name, name);
            dawn->monsters[count].hp     = hp;
            dawn->monsters[count].str    = str;
            dawn->monsters[count].def    = def;
            dawn->monsters[count].intel  = intel;
            dawn->monsters[count].mdef   = mdef;
            dawn->monsters[count].gold   = gold;
            dawn->monsters[count].exp    = exp;
            dawn->monsters[count].mhp    = mhp;
            dawn->monsters[count].drop_level = drop_level;
            dawn->monsters[count].slay_cost  = gold;
            dawn->monsters[count].active = 0;
            count++;
        }
        fclose(file);
    }

    //Initial settings
    strcpy(dawn->nickname, "Dawn-18");
    strcpy(dawn->realname, "Helo");
    strcpy(dawn->ident,    "hehe");
    strcpy(dawn->password, "none");
    strcpy(dawn->active_room, rooms[0]);
    dawn->login_sent = 0;
    dawn->in_rooms   = 0;

    init_send_queue();
    init_cmds();
    init_map();

    if (init_connect_server(dalnet, port) == 0) {
        while ((len = recv(con_socket, buffer, MAX_RECV_BUFFER, 0)) != -1) {
            char out[MAX_MESSAGE_BUFFER];
            buffer[len] = '\0';

            //Handle keepalive pings from the server
            if (check_if_matches_regex(buffer, "PING :(.*)")) {
                sprintf(out, "PONG :%s\r\n", regex_group[1]);
                add_msg(out, strlen(out));
            }

            //Connected to the server, send nick/user details
            if (dawn->login_sent == 0) {
                match = check_if_matches_regex(buffer, "AUTH");
                if (match) {
                    printf(INFO "Server checking for AUTH, sending nick/user\n");
                    handle_login(dawn->nickname, dawn->password, dawn->realname, dawn->ident);
                    dawn->login_sent = 1;
                }
            }

            //Nickname is in use, add a random suffix
            if (check_if_matches_regex(buffer, ":.*?\\s433\\s*\\s.*")) {
                int rand_suffix = rand() % 10;
                fprintf(stderr, WARN "Username %s in use\n", dawn->nickname);
                size_t nicklen = strlen(dawn->nickname);
                snprintf(dawn->nickname+nicklen, MAX_NICK_LENGTH-nicklen, "%d", rand_suffix);
                handle_login(dawn->nickname, dawn->password, dawn->realname, dawn->ident);
            }


            //If we're logged in and we're received a
            //welcome message, join the rooms
            if (!dawn->in_rooms && dawn->login_sent == 1) {
                match = check_if_matches_regex(buffer, ":(.*?)\\s001(.*)");
                if (match) {
                    printf(INFO "Got welcome message from server, joining rooms\n");
                    while (*n != NULL) {
                        sprintf(out, "JOIN %s\r\n", *n++);
                        fluctuate_market(dawn);
                        add_msg(out, strlen(out));
                    }
                    n = NULL;
                    dawn->in_rooms = 1;
                }
            }

            //Upon joining rooms the bot will search for status 353, which is names, and will
            //iterate through the online people, setting their status to available if they
            //have an account on the bot
            if (dawn->in_rooms) {
                struct Message message;

                //Set lengths based on server rather than assuming
                if (check_if_matches_regex(buffer, ":.*?005.*?CHANNELLEN=(\\d+)\\s.*?NICKLEN=(\\d+)")) {
                    MAX_CHANNEL_LENGTH = (unsigned int)strtoul(regex_group[1], 0, 10) + 1;
                    MAX_NICK_LENGTH    = (unsigned int)strtoul(regex_group[2], 0, 10) + 1;
                }

                //Check if user is identified
                //:punch.wa.us.dal.net 307 jkjff ziddy :has auth_level for this nick
                if (check_if_matches_regex(buffer, ":(.*?)\\s307\\s(.*?)\\s(.*?)\\s:")) {
                    int pindex = get_pindex(dawn, regex_group[3]);
                    if(pindex != -1 && (dawn->players[pindex].max_auth > AL_USER || dawn->players[pindex].auth_level > AL_NOAUTH)) {
                        if(dawn->players[pindex].max_auth < AL_REG) { //upgrade
                            dawn->players[pindex].max_auth = AL_REG;
                        }
                        if(dawn->players[pindex].auth_level < dawn->players[pindex].max_auth) {
                            dawn->players[pindex].auth_level = dawn->players[pindex].max_auth;
                            snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :%s has been verified. (%s)\r\n",
                                dawn->active_room,
                                dawn->players[pindex].username,
                                auth_level_to_str(dawn->players[pindex].auth_level));
                            add_msg(out, strlen(out));
                        } else {
                            snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :%s is already verified. (%s)\r\n",
                                dawn->active_room,
                                dawn->players[pindex].username,
                                auth_level_to_str(dawn->players[pindex].auth_level));
                            add_msg(out, strlen(out));
                        }
                    }
                }

                //NAMES (Status 353)
                //This will split up the received users in the room and set
                //their status availability to 1, stripping their name of status symbols
                //and converting their username to lowercase
                if (check_if_matches_regex(buffer, ":(.*?)\\s353\\s(.*?)\\s@\\s(.*?)\\s:(.*)")) {
                    char *ch_ptr;
                    ch_ptr = strtok(regex_group[4], " @&+");
                    while (ch_ptr != NULL) {
                        int index = get_pindex(dawn, to_lower(ch_ptr));
                        if (index != -1){
                            dawn->players[index].available = 1;
                            dawn->players[index].auth_level = AL_NOAUTH;
                            sprintf(out, "WHOIS %s\r\n", dawn->players[index].username);
                            add_msg(out, strlen(out));
                        }
                        ch_ptr = strtok(NULL, " @&+\r\n");
                    }
                }

                //Kicks
                if (check_if_matches_regex(buffer, ":(.*?)!~?(.*?)@(.*?)\\s(\\w+)\\s(.*?)\\s(.*?)\\s:(.*)")) {
                    if (strcmp(regex_group[4], "KICK") == 0) {
                        int index = get_pindex(dawn, to_lower(regex_group[6]));
                        if (index != -1) {
                            dawn->players[index].available = 0;
                            dawn->players[index].auth_level = AL_NOAUTH;
                        }
                    }
                }

                //Nick changes
                if (check_if_matches_regex(buffer, ":(.*?)!~?.*?@.*?\\sNICK\\s:(.*?)\r\n")) {
                    int oldindex = get_pindex(dawn, to_lower(regex_group[1]));
                    int newindex = get_pindex(dawn, to_lower(regex_group[2]));
                    if (oldindex != -1) {
                        dawn->players[oldindex].available = 0;
                        dawn->players[oldindex].auth_level = AL_NOAUTH;
                    }
                    if (newindex != -1) {
                        dawn->players[newindex].available = 1;
                        dawn->players[newindex].auth_level = AL_NOAUTH;
                    }
                }

                //Parting and joining
                if (check_if_matches_regex(buffer, ":(.*?)!~?(.*?)@(.*?)\\s(.*?)\\s(.*?)")) {
                    int pindex = get_pindex(dawn, to_lower(regex_group[1]));
                    if (pindex != -1) {
                        if (strcmp(regex_group[4], "PART") == 0) {
                            dawn->players[pindex].available = 0;
                            dawn->players[pindex].auth_level = AL_NOAUTH;
                        } else if (strcmp(regex_group[4], "JOIN") == 0) {
                            dawn->players[pindex].available = 1;
                            dawn->players[pindex].auth_level = AL_NOAUTH;
                            sprintf(out, "WHOIS %s\r\n", regex_group[1]);
                            add_msg(out, strlen(out));
                        } else if (strcmp(regex_group[4], "QUIT") == 0) {
                            dawn->players[pindex].available = 0;
                        }
                    }
                    if (dawn->player_count == 0) {
                        struct Message temp;
                        strcpy(temp.sender_nick, to_lower(dawn->nickname));
                        strcpy(temp.sender_hostmask, regex_group[3]);
                        init_new_character(dawn, &temp);
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
                        if(check_if_matches_regex(message.message, CMD_LIT)) {
                            if (message.receiver[0] == '#') {
                                invoke_cmd(0, get_pindex(dawn, message.sender_nick),  regex_group[0], &message, CMD_EXEC);
                            } else {
                                parse_private_message(dawn, &message);
                            }
                        }
                    } else if (strcmp(regex_group[4], "NOTICE") == 0) {
                    //    parse_notice_message(&message);
                    }
                }
            }
        }
    } else {
        perror(ERR "main: Failed to connect");
        close(con_socket);
        return 1;
    }
    if(len != -1)
        close(con_socket);
    free_msg_hist_list();
    free_msg_list();
    free_event_list();
    free_cmds();
    free_map();
    free(dawn);
    if(auth_key_valid || auth_key)
        free(auth_key);
    printf(INFO "Program exiting normally\n");
    return 0;
}
