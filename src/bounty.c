#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "include/status.h"
#include "include/bounty.h"
#include "include/monsters.h"
#include "include/limits.h"

void collect_bounty (int);

void gen_global_bounty (void) {
    char out[MAX_MESSAGE_BUFFER];
    if (dawn->gbounty.amount) return;
    struct Bounty global_bounty;
    dawn->gbounty.name[0] = '\0';
    strcpy(global_bounty.name, dawn->monsters[rand() % MAX_MONSTERS].name);
    global_bounty.amount = 1 + rand() % 25;
    global_bounty.multiplier = 1 + rand() % 5;
    dawn->gbounty = global_bounty;

    snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :New bounty! Kill %d %s's, (%dx reward)\r\n",
            dawn->active_room, dawn->gbounty.amount, dawn->gbounty.name, dawn->gbounty.multiplier);
    add_msg(out, strlen(out));
}

void check_bounty (struct Message *msg) {
    int pindex = get_pindex(msg->sender_nick);
    char out[MAX_MESSAGE_BUFFER];
    if (strcmp(dawn->gbounty.name, dawn->global_monster.name) == 0) {
        dawn->players[pindex].bounty++;
        snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :%s has taken down one more %s, %d to go until they win the bounty!\r\n",
                dawn->active_room, msg->sender_nick, dawn->gbounty.name, (dawn->gbounty.amount - dawn->players[pindex].bounty));
        add_msg(out, strlen(out));
        if (dawn->players[pindex].bounty == dawn->gbounty.amount) collect_bounty(pindex);
    }
}

void collect_bounty (int pindex) {
    char out[MAX_MESSAGE_BUFFER];
    int gold_reward = dawn->global_monster.gold * dawn->gbounty.multiplier * dawn->gbounty.amount;
    int exp_reward  = dawn->global_monster.exp  * dawn->gbounty.multiplier * dawn->gbounty.amount;

    dawn->players[pindex].gold += gold_reward;
    dawn->players[pindex].experience  += exp_reward;

    snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :%s has completed the bounty: Slay %d %s's! %s has been"
            " rewarded with %d gold and %d experience!\r\n", dawn->active_room, dawn->players[pindex].username,
            dawn->gbounty.amount, dawn->gbounty.name, dawn->players[pindex].username, gold_reward, exp_reward);
    add_msg(out, strlen(out));
    struct Bounty empty;
    empty.name[0] = '\0';
    empty.amount = 0;
    empty.multiplier = 0;
    dawn->gbounty = empty;
}
