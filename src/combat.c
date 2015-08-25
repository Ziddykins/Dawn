#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "include/status.h"
#include "include/network.h"
#include "include/stats.h"
#include "include/colors.h"
#include "include/player.h"
#include "include/items.h"
#include "include/combat.h"

void monster_attacks(struct Message *, int, int);

static void calc_contribution(int pindex, int amount, int monster_mhp, int monster_hp) {
    if (amount >= monster_mhp) {
        dawn->players[pindex].contribution = monster_mhp;
    } else {
        if (dawn->players[pindex].contribution + amount > monster_hp) {
            dawn->players[pindex].contribution += monster_hp;
        } else {
            dawn->players[pindex].contribution += amount;
        }
        if (dawn->players[pindex].contribution > monster_mhp) {
            dawn->players[pindex].contribution = monster_mhp;
        }
    }
}

int check_alive(struct Message *message) {
    int pindex = get_pindex(message->sender_nick);
    char out[MAX_MESSAGE_BUFFER];

    if (dawn->players[pindex].health <= 0 || dawn->players[pindex].fullness <= 0) {
        if (dawn->players[pindex].alive) {
            char reason[32];
            float newexp = (float) dawn->players[pindex].experience * 0.90f;
            strcpy(reason, dawn->players[pindex].health <= 0 ? "been killed" : "died from starvation");
            sprintf(out, "PRIVMSG %s :%s has %s! 10%% experience has been lost\r\n",
                    message->receiver, message->sender_nick, reason);
            add_msg(out, strlen(out));
            dawn->players[pindex].deaths++;
            dawn->players[pindex].alive = 0;
            dawn->players[pindex].experience = (unsigned long long) newexp;
            if (dawn->players[pindex].fullness <= 0)
                dawn->players[pindex].fullness = 100;
            return 0;
        }
    }

    if (dawn->global_monster.hp <= 0 && dawn->global_monster.active) {
        sprintf(out, "PRIVMSG %s :The %s has been killed!\r\n", message->receiver, dawn->global_monster.name);
        add_msg(out, strlen(out));
        for (int i = 0; i < dawn->player_count; i++) {
            if (dawn->players[i].contribution > 0) {
                int drop_chance = rand()%100;
                double percent = ((float) dawn->players[i].contribution / (float) dawn->global_monster.mhp);
                double expgain = percent * dawn->global_monster.exp;
                double goldgain = percent * dawn->global_monster.gold;
                //Need to change the sender_nick so the proper drops are awarded as well as level checking
                strcpy(message->sender_nick, dawn->players[i].username);
                dawn->players[i].experience += (unsigned long long) expgain;
                dawn->players[i].gold += (int) goldgain;
                sprintf(out, "PRIVMSG %s :%s has helped defeat the foe and receives %d experience and %d gold"
                                " for their efforts!\r\n", message->receiver, dawn->players[i].username, (int) expgain,
                        (int) goldgain);
                dawn->global_monster.active = 0;
                add_msg(out, strlen(out));
                dawn->players[i].kills++;
                check_levelup(message);
                if (drop_chance < 85) {
                    generate_drop(message);
                }
            }
        }
        return 0;
    }
    return 1;
}

void player_attacks(struct Message *message, int global) {
    int pindex = get_pindex(message->sender_nick);
    char out[MAX_MESSAGE_BUFFER];
    int player_attack, monster_defense, player_defense, critical;
    int ustats[6] = {dawn->players[pindex].max_health, dawn->players[pindex].max_mana,
                     dawn->players[pindex].strength, dawn->players[pindex].intelligence,
                     dawn->players[pindex].m_def, dawn->players[pindex].defense};

    get_stat(message, ustats);
    player_defense = rand() % dawn->players[pindex].defense;

    //15% chance to critical hit (double) attack
    if ((rand() % 100) < 15) {
        critical = 1;
        player_attack = (rand() % ustats[2]) * 2;
    } else {
        critical = 0;
        player_attack = rand() % ustats[2];
    }

    if (!dawn->players[pindex].alive) {
        sprintf(out, "PRIVMSG %s :%s, you are dead and can't make an attack; revive at a shrine"
                     ", use a potion, or have someone revive you!\r\n", message->receiver, message->sender_nick);
        add_msg(out, strlen(out));
        return;
    }

    if (global && dawn->global_monster.active) {
        if (player_attack == 0) {
            sprintf(out, "PRIVMSG %s :%s trips over his shoelace and misses\r\n", message->receiver,
                         message->sender_nick);
            add_msg(out, strlen(out));
            monster_attacks(message, player_defense, pindex);
        } else {
            //Avoid floating point exceptions
            if (!dawn->global_monster.def == 0) {
                monster_defense = rand() % dawn->global_monster.def;
            } else {
                monster_defense = 0;
            }
            if (player_attack - monster_defense > 0) {
                calc_contribution(pindex, (player_attack - monster_defense), dawn->global_monster.mhp,
                                  dawn->global_monster.hp);
                dawn->global_monster.hp -= (player_attack - monster_defense);
                if (critical) {
                    sprintf(out, "PRIVMSG %s :%s attacks the %s for %d %sCRITICAL%s damage! "
                            "Remaining (%d)\r\n",
                            message->receiver, message->sender_nick, dawn->global_monster.name,
                            player_attack - monster_defense, IRC_RED, IRC_NORMAL, dawn->global_monster.hp);
                } else {
                    sprintf(out, "PRIVMSG %s :%s attacks the %s for %d damage! (Remaining: %d)\r\n",
                            message->receiver, message->sender_nick, dawn->global_monster.name,
                            player_attack - monster_defense, dawn->global_monster.hp);
                }
                add_msg(out, strlen(out));
                if (check_alive(message)) {
                    monster_attacks(message, player_defense, pindex);
                }
            } else {
                sprintf(out, "PRIVMSG %s :The %s has blocked %s's attack!\r\n", message->receiver,
                        dawn->global_monster.name, message->sender_nick);
                monster_attacks(message, player_defense, pindex);
                add_msg(out, strlen(out));
            }
        }
    }
}

void monster_attacks(struct Message *message, int player_defense, int pindex) {
    int monster_attack = rand() % dawn->global_monster.str;
    char out[MAX_MESSAGE_BUFFER];

    if (monster_attack > 0 && (monster_attack - player_defense) > 0 && dawn->players[pindex].alive) {
        dawn->players[pindex].health -= monster_attack;
        sprintf(out, "PRIVMSG %s :The %s attacks %s viciously for %d damage! (Remaining: %ld)\r\n",
                message->receiver, dawn->global_monster.name, message->sender_nick,
                monster_attack, dawn->players[pindex].health);
        add_msg(out, strlen(out));
        check_alive(message);
    } else {
        if (dawn->global_monster.hp > 0) {
            sprintf(out, "PRIVMSG %s :%s has blocked the %s's attack!\r\n",
                    message->receiver, message->sender_nick, dawn->global_monster.name);
            add_msg(out, strlen(out));
        }
    }
}
