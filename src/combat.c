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

void monster_attacks (struct Bot *, struct Message *, int, int);

static void calc_contribution (struct Bot *b, int pindex, int amount, int monster_mhp, int monster_hp) {
    if (amount >= monster_mhp) {
        b->players[pindex].contribution = monster_mhp;
    } else {
        if (b->players[pindex].contribution + amount > monster_hp) {
            b->players[pindex].contribution += monster_hp;
        } else {
            b->players[pindex].contribution += amount;
        }
        if (b->players[pindex].contribution > monster_mhp) {
            b->players[pindex].contribution = monster_mhp;
        }
    }
}

int check_alive (struct Bot *b, struct Message *message) {
    int pindex = get_pindex(b, message->sender_nick);
    char out[MAX_MESSAGE_BUFFER];

    if (b->players[pindex].health <= 0 || b->players[pindex].fullness <= 0) {
        if (b->players[pindex].alive) {
            char reason[32];
            float newexp = (float)b->players[pindex].experience * 0.90f;
            strcpy(reason, b->players[pindex].health <= 0 ? "been killed" : "died from starvation");
            sprintf(out, "PRIVMSG %s :%s has %s! 10%% experience has been lost\r\n",
                    message->receiver, message->sender_nick, reason);
            addMsg(out, strlen(out));
            b->players[pindex].deaths++;
            b->players[pindex].alive = 0;
            b->players[pindex].experience = (unsigned long long)newexp;
            return 0;
        }
    }

    if (b->global_monster.hp <= 0 && b->global_monster.active) {
        sprintf(out, "PRIVMSG %s :The %s has been killed!\r\n", message->receiver, b->global_monster.name);
        addMsg(out, strlen(out));
        for (int i=0; i<b->player_count; i++) {
            if (b->players[i].contribution > 0) {
                int drop_chance = rand()%100;
                double percent  = ((float)b->players[i].contribution / (float)b->global_monster.mhp);
                double expgain  = percent * b->global_monster.exp;
                double goldgain = percent * b->global_monster.gold;
                //Need to change the sender_nick so the proper drops are awarded as well as level checking
                strcpy(message->sender_nick, b->players[i].username);
                b->players[i].experience += (unsigned long long)expgain;
                b->players[i].gold += (int)goldgain;
                sprintf(out, "PRIVMSG %s :%s has helped defeat the foe and receives %d experience and %d gold"
                        " for their efforts!\r\n", message->receiver, b->players[i].username, (int)expgain, (int)goldgain);
                b->global_monster.active = 0;
                addMsg(out, strlen(out));
                b->players[i].kills++;
                check_levelup(b, message);
                if (drop_chance < 85) {
                    generate_drop(b, message);
                }
            }
        }
        return 0;
    }
    return 1;
}

void player_attacks (struct Bot *b, struct Message *message, int global) {
    int pindex = get_pindex(b, message->sender_nick);
    char out[MAX_MESSAGE_BUFFER];
    int player_attack, monster_defense, player_defense, critical;
    int ustats[6] = { b->players[pindex].max_health, b->players[pindex].max_mana,
                      b->players[pindex].strength, b->players[pindex].intelligence,
                      b->players[pindex].m_def, b->players[pindex].defense };

    get_stat(b, message, ustats);
    player_defense = rand() % b->players[pindex].defense;

    //15% chance to critical hit (double) attack
    if ((rand() % 100) < 15) {
        critical = 1;
        player_attack = (rand() % ustats[2]) * 2;
    } else {
        critical = 0;
        player_attack = rand() % ustats[2];
    }

    if (!b->players[pindex].alive) {
        sprintf(out, "PRIVMSG %s :%s, you are dead and can't make an attack; revive at a shrine"
                     ", use a potion, or have someone revive you!\r\n", message->receiver, message->sender_nick);
        addMsg(out, strlen(out));
        return;
    }

    if (global && b->global_monster.active) {
        if (player_attack == 0) {
            sprintf(out, "PRIVMSG %s :%s trips over his shoelace and misses\r\n", message->receiver,
                         message->sender_nick);
            addMsg(out, strlen(out));
            monster_attacks(b, message, player_defense, pindex);
        } else {
            //Avoid floating point exceptions
            if (!b->global_monster.def == 0) {
                monster_defense = rand() % b->global_monster.def;
            } else {
                monster_defense = 0;
            }
            if (player_attack - monster_defense > 0) {
                calc_contribution(b, pindex, (player_attack - monster_defense), b->global_monster.mhp, b->global_monster.hp);
                b->global_monster.hp -= (player_attack - monster_defense);
                if (critical) {
                    sprintf(out, "PRIVMSG %s :%s attacks the %s for %d %sCRITICAL%s damage! "
                            "Remaining (%d)\r\n",
                            message->receiver, message->sender_nick, b->global_monster.name,
                            player_attack - monster_defense, red, normal, b->global_monster.hp);
                } else {
                    sprintf(out, "PRIVMSG %s :%s attacks the %s for %d damage! (Remaining: %d)\r\n",
                            message->receiver, message->sender_nick, b->global_monster.name,
                            player_attack - monster_defense, b->global_monster.hp);
                }
                addMsg(out, strlen(out));
                if (check_alive(b, message)) {
                    monster_attacks(b, message, player_defense, pindex);
                }
            } else {
                sprintf(out, "PRIVMSG %s :The %s has blocked %s's attack!\r\n", message->receiver,
                        b->global_monster.name, message->sender_nick);
                monster_attacks(b, message, player_defense, pindex);
                addMsg(out, strlen(out));
            }
        }
    }
}

void monster_attacks (struct Bot *b, struct Message *message, int player_defense, int pindex) {
    int monster_attack = rand() % b->global_monster.str;
    char out[MAX_MESSAGE_BUFFER];

    if (monster_attack > 0 && (monster_attack - player_defense) > 0 && b->players[pindex].alive) {
        b->players[pindex].health -= monster_attack;
        sprintf(out, "PRIVMSG %s :The %s attacks %s viciously for %d damage! (Remaining: %ld)\r\n",
                message->receiver, b->global_monster.name, message->sender_nick,
                monster_attack, b->players[pindex].health);
        addMsg(out, strlen(out));
        check_alive(b, message);
    } else {
        if (b->global_monster.hp > 0) {
            sprintf(out, "PRIVMSG %s :%s has blocked the %s's attack!\r\n",
                    message->receiver, message->sender_nick, b->global_monster.name);
            addMsg(out, strlen(out));
        }
    }
}
