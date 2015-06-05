#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "status.h"
#include "network.h"
#include "stats.h"
#include "colors.h"

void check_alive (Bot *dawn, Message *message) {
    char hi[100];
    char hii[100];
    strcpy(hi, dawn->global_monster.name);
    strcpy(hii, message->sender_nick);
    printf("%s%s\n", hi, hii);
}

void player_attack (Bot *dawn, Message *message, int global) {
    int i;
    char out[MAX_MESSAGE_BUFFER];
    //hp, mana, str, int, mdef, def 
    printf("in combat\n");

    for (i=0; i<dawn->player_count; i++) {
        if (strcmp(dawn->players[i].username, message->sender_nick) == 0) {
            int ustats[6] = { dawn->players[i].max_health, dawn->players[i].max_mana,
                              dawn->players[i].strength, dawn->players[i].intelligence,
                              dawn->players[i].m_def, dawn->players[i].defense };
            get_stat(dawn, message, ustats);
            if (dawn->players[i].health <= 0) {
                sprintf(out, "PRIVMSG %s :%s, you are dead and can't make an attack; revive at a shrine"
                             ", use a potion, or have someone revive you!\r\n", message->receiver, message->sender_nick);
                send_socket(out);
                return;
            } else {
                if (global) {
                    if (!dawn->global_monster.active) {
                        sprintf(out, "PRIVMSG %s :%s, there is no monster in the room!\r\n",
                                     message->receiver, message->sender_nick);
                        send_socket(out);
                        return;
                    } else {
                        int player_attack, monster_defense, player_defense, monster_attack, critical;
                        monster_defense++;
                        player_defense++;
                        if ((rand() % 100) < 15) {
                            critical = 1;
                            player_attack = (rand() % ustats[2]) * 2;
                        } else {
                            critical = 0;
                            player_attack = rand() % ustats[2];
                        }

                        if (player_attack == 0) {
                            sprintf(out, "PRIVMSG %s :%s trips over his shoelace and misses\r\n", message->receiver,
                                         message->sender_nick);
                            send_socket(out);
                            monster_attack = rand() % dawn->global_monster.str;
                            if (monster_attack > 0) {
                                dawn->players[i].health -= monster_attack;
                                sprintf(out, "PRIVMSG %s :The %s attacks %s while they're off-guard,"
                                        " dealing %d damage! (Remaining: %ld)\r\n",
                                        message->receiver, dawn->global_monster.name, message->sender_nick,
                                        monster_attack, dawn->players[i].health);
                                send_socket(out);
                            //    check_alive(dawn, message);
                            }
                        } else {
                            dawn->global_monster.hp -= player_attack;
                            if (critical) {
                                sprintf(out, "PRIVMSG %s :%s attacks the %s for %d %sCRITICAL%s damage! "
                                        "Remaining (%d)\r\n",
                                        message->receiver, message->sender_nick, dawn->global_monster.name,
                                        player_attack, red, normal, dawn->global_monster.hp);
                            } else {
                                sprintf(out, "PRIVMSG %s :%s attacks the %s for %d damage! (Remaining: %d)\r\n",
                                        message->receiver, message->sender_nick, dawn->global_monster.name,
                                        player_attack, dawn->global_monster.hp);
                              //  check_alive(dawn, message);
                            }
                            send_socket(out);
                        }
                    }
                }
            }
        }
    }
}
