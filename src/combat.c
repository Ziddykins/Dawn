#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "status.h"
#include "network.h"
#include "stats.h"
#include "colors.h"
#include "player.h"

void monster_attacks (Bot *, Message *, int, int);

void calc_contribution (Bot *dawn, int i, int amount, int monster_mhp, int monster_hp) {
    if (amount >= monster_mhp) {
        dawn->players[i].contribution = monster_mhp;
    } else {
        if ((int)dawn->players[i].contribution + amount > monster_hp) {
            dawn->players[i].contribution += monster_hp;
        } else {
            dawn->players[i].contribution += amount;
        }
        if ((int)dawn->players[i].contribution > monster_mhp) dawn->players[i].contribution = monster_mhp;
    }
}
 
void check_alive (Bot *dawn, Message *message) {
    int i = get_pindex(dawn, message->sender_nick);
    char out[MAX_MESSAGE_BUFFER];

    if (dawn->players[i].health <= 0) {
        sprintf(out, "PRIVMSG %s :%s has been killed by the %s\r\n", 
                message->receiver, message->sender_nick, dawn->global_monster.name);
        send_socket(out);
        dawn->players[i].deaths++;
    }

    if (dawn->global_monster.hp <= 0 && dawn->global_monster.active) {
        sprintf(out, "PRIVMSG %s :The %s has been killed!\r\n", message->receiver, dawn->global_monster.name);
        send_socket(out);
        int j;
        for (j=0; j<dawn->player_count; j++) {
            if (dawn->players[j].contribution > 0) {
                float percent  = ((float)dawn->players[j].contribution / (float)dawn->global_monster.mhp);
                float expgain  = percent * dawn->global_monster.exp;
                float goldgain = percent * dawn->global_monster.gold;
                dawn->players[j].experience += (int)expgain;
                dawn->players[j].gold += (int)goldgain;
                sprintf(out, "PRIVMSG %s :%s has helped defeat the foe and receives %d experience and %d gold"
                        " for their efforts!\r\n", message->receiver, dawn->players[j].username, (int)expgain, (int)goldgain);
                dawn->global_monster.active = 0;
                send_socket(out);
                dawn->players[j].kills++;
                check_levelup(dawn, message);
            }
        }
    }
}

void player_attacks (Bot *dawn, Message *message, int global) {
    int i = get_pindex(dawn, message->sender_nick);
    char out[MAX_MESSAGE_BUFFER];
    int player_attack, monster_defense, player_defense, critical;
    int ustats[6] = { dawn->players[i].max_health, dawn->players[i].max_mana,
                      dawn->players[i].strength, dawn->players[i].intelligence,
                      dawn->players[i].m_def, dawn->players[i].defense };

    get_stat(dawn, message, ustats);
    player_defense = rand() % dawn->players[i].defense;

    //15% chance to critical hit (double) attack
    if ((rand() % 100) < 15) {
        critical = 1;
        player_attack = (rand() % ustats[2]) * 2;
    } else {
        critical = 0;
        player_attack = rand() % ustats[2];
    }

    if (dawn->players[i].health <= 0) {
        sprintf(out, "PRIVMSG %s :%s, you are dead and can't make an attack; revive at a shrine"
                     ", use a potion, or have someone revive you!\r\n", message->receiver, message->sender_nick);
        send_socket(out);
        return;
    }

    if (global && dawn->global_monster.active) {
        if (player_attack == 0) {
            sprintf(out, "PRIVMSG %s :%s trips over his shoelace and misses\r\n", message->receiver,
                         message->sender_nick);
            send_socket(out);
            monster_attacks(dawn, message, player_defense, i); 
        } else {
            //Avoid floating point exceptions
            if (!dawn->global_monster.def == 0) {
                monster_defense = rand() % dawn->global_monster.def;
            } else {
                monster_defense = 0;
            }
            if (player_attack - monster_defense > 0) {
                calc_contribution(dawn, i, 
                            (player_attack - monster_defense), 
                            dawn->global_monster.mhp,
                            dawn->global_monster.hp);

                dawn->global_monster.hp -= (player_attack - monster_defense);

                if (critical) {
                    sprintf(out, "PRIVMSG %s :%s attacks the %s for %d %sCRITICAL%s damage! "
                            "Remaining (%d)\r\n",
                            message->receiver, message->sender_nick, dawn->global_monster.name,
                            player_attack - monster_defense, red, normal, dawn->global_monster.hp);
                } else {
                    sprintf(out, "PRIVMSG %s :%s attacks the %s for %d damage! (Remaining: %d)\r\n",
                            message->receiver, message->sender_nick, dawn->global_monster.name,
                            player_attack - monster_defense, dawn->global_monster.hp);
                }

                send_socket(out);
                check_alive(dawn, message);
                monster_attacks(dawn, message, player_defense, i);
            } else {
                sprintf(out, "PRIVMSG %s :The %s has blocked %s's attack!\r\n", message->receiver,
                        dawn->global_monster.name, message->sender_nick);
                monster_attacks(dawn, message, player_defense, i);
                send_socket(out);
            }
        }
    }
}

void monster_attacks (Bot *dawn, Message *message, int player_defense, int i) {
    int monster_attack = rand() % dawn->global_monster.str;
    char out[MAX_MESSAGE_BUFFER];

    if (monster_attack > 0 && (monster_attack - player_defense) > 0) {
        check_alive(dawn, message);
        dawn->players[i].health -= monster_attack;
        sprintf(out, "PRIVMSG %s :The %s attacks %s viciously for %d damage! (Remaining: %ld)\r\n",
                message->receiver, dawn->global_monster.name, message->sender_nick,
                monster_attack, dawn->players[i].health);
        send_socket(out);
        check_alive(dawn, message);
    } else {
        if (dawn->global_monster.hp > 0) {
            sprintf(out, "PRIVMSG %s :%s has blocked the %s's attack!\r\n", 
                    message->receiver, message->sender_nick, dawn->global_monster.name);
            send_socket(out);
        }
    }
}
