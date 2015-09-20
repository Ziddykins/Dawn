#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "include/status.h"
#include "include/spells.h"
#include "include/limits.h"
#include "include/combat.h"

void cast_heal(const char *username, const char *target) {
    int pindex = get_pindex(username);
    int tindex = get_pindex(target);
    char out[MAX_MESSAGE_BUFFER];

    if (tindex == -1) {
        snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :Invalid target\r\n", dawn->active_room);
        add_msg(out, strlen(out));
        return;
    }

    if (!dawn->players[pindex].spellbook.heal.learned) {
        snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :%s, you do not know this spell\r\n",
                 dawn->active_room, username);
        add_msg(out, strlen(out));
        return;
    }

    if (dawn->players[tindex].health <= 0) {
        snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :%s, your magic powers are not yet strong enough "
                "to revive the dead\r\n", dawn->active_room, username);
        add_msg(out, strlen(out));
        return;
    }

    if (dawn->players[tindex].health == dawn->players[tindex].max_health) {
        snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :%s is already at full health\r\n",
                 dawn->active_room, target);
        add_msg(out, strlen(out));
        return;
    }

    int heal_cost = 1 + dawn->players[pindex].spellbook.heal.level * 5;
    int heal_amount = 1 + rand() % (dawn->players[pindex].spellbook.heal.level * 5);

    if (dawn->players[pindex].mana >= heal_cost) {
        dawn->players[tindex].health += heal_amount;
        if (dawn->players[tindex].health >= dawn->players[tindex].max_health)
            dawn->players[tindex].health = dawn->players[tindex].max_health;
        sprintf(out, "PRIVMSG %s :%s has healed %s for %d HP!\r\n", dawn->active_room, username, target, heal_amount);
    } else {
        sprintf(out, "PRIVMSG %s :%s, you don't have enough mana to cast this spell\r\n", dawn->active_room, username);
    }

    add_msg(out, strlen(out));
}

void cast_revive(const char *username, const char *target) {
    int pindex = get_pindex(username);
    int tindex = get_pindex(target);
    char out[MAX_MESSAGE_BUFFER];

    if (tindex == -1) {
        snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :Invalid target\r\n", dawn->active_room);
        add_msg(out, strlen(out));
        return;
    }

    if (!dawn->players[pindex].spellbook.revive.learned) {
        snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :%s, you do not know this spell\r\n",
                dawn->active_room, username);
        add_msg(out, strlen(out));
        return;
    }

    if (dawn->players[tindex].alive) {
        snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :%s, %s is already alive\r\n",
                dawn->active_room, username, target);
        add_msg(out, strlen(out));
        return;
    }

    if (strcmp(username, target) == 0) {
        snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :%s, you can't revive yourself... You're dead.\r\n",
                dawn->active_room, username);
        add_msg(out, strlen(out));
        return;
    }

    int revive_cost = 1 + dawn->players[pindex].spellbook.revive.level * 6;

    if (dawn->players[pindex].mana >= revive_cost) {
        dawn->players[pindex].mana -= revive_cost;
        dawn->players[tindex].alive = 1;
        snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :%s has revived %s from the dead!\r\n",
                dawn->active_room, username, target);
        add_msg(out, strlen(out));
    } else {
        snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :%s, you do not have the required %d mana to cast this spell\r\n",
                dawn->active_room, username, revive_cost);
        add_msg(out, strlen(out));
    }
}



void cast_fireball(const char *username, const char *target) {
    int pindex = get_pindex(username);
    int tindex = get_pindex(target);
    char out[MAX_MESSAGE_BUFFER];

    if (tindex == -1) {
        snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :Invalid target\r\n", dawn->active_room);
        add_msg(out, strlen(out));
        return;
    }

    if (!dawn->players[pindex].spellbook.fireball.learned) {
        snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :%s, you do not know this spell\r\n",
                 dawn->active_room, username);
        add_msg(out, strlen(out));
        return;
    }

    if (dawn->players[tindex].health <= 0) {
        snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :%s, %s is dead\r\n",
                 dawn->active_room, username, target);
        add_msg(out, strlen(out));
        return;
    }

    if (dawn->players[pindex].level >= dawn->players[tindex].level * 3) {
        snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :%s, pick on someone your own size!\r\n",
                 dawn->active_room, username);
        add_msg(out, strlen(out));
        return;
    }

    if (strcmp(username, target) == 0) {
        snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :%s whips a fireball at a mirror; it bounces back"
                ", hitting themselves for 1 point of damage. Well done.\r\n", dawn->active_room, username);
        dawn->players[pindex].health--;
        add_msg(out, strlen(out));
        return;
    }

    int fireball_cost = 1 + dawn->players[pindex].spellbook.fireball.level * 7;
    int fireball_damage =
            1 + rand() % dawn->players[pindex].intelligence * dawn->players[pindex].spellbook.fireball.level;

    if (dawn->players[pindex].mana >= fireball_cost) {
        int target_save = 1 + rand() % dawn->players[tindex].m_def;
        char result[128];
        dawn->players[pindex].mana -= fireball_cost;
        snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :%s hurls a fireball at %s! ",
                 dawn->active_room, username, target);
        if (target_save == fireball_damage) {
            snprintf(result, 128, "%s's shields had just enough energy to absorb the entire fireball!\r\n", target);
        } else if (target_save < fireball_damage) {
            struct Message tmpmsg;
            strcpy(tmpmsg.sender_nick, target);
            strcpy(tmpmsg.receiver, dawn->active_room);
            snprintf(result, 128, "%s is hit for %d damage!\r\n", target, fireball_damage - target_save);
            dawn->players[tindex].health -= fireball_damage - target_save;
            check_alive(&tmpmsg);
        } else if (target_save > fireball_damage) {
            snprintf(result, 128, "%s swats the fireball out of the sky with ease\r\n", target);
        }
        strncat(out, result, MAX_MESSAGE_BUFFER - strlen(result));
    }
    add_msg(out, strlen(out));
}

void cast_teleport(const char *username, int x, int y) {
    struct Message temp;
    strcpy(temp.sender_nick, username);
    strcpy(temp.receiver, dawn->active_room);
    move_player(&temp, x, y, 1);
}

void cast_rain(const char *username) {
    int pindex = get_pindex(username);
    char out[MAX_MESSAGE_BUFFER];

    if (!dawn->players[pindex].spellbook.rain.learned) {
        snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :%s, you do not know this spell\r\n",
                 dawn->active_room, username);
        add_msg(out, strlen(out));
        return;
    }

    int rain_cost = 1 + dawn->players[pindex].spellbook.rain.level * 6;
    int rain_amount = 1 + rand() % (dawn->players[pindex].spellbook.rain.level * 6);

    if (dawn->players[pindex].mana >= rain_cost) {
        dawn->players[pindex].mana -= rain_cost;

        for (int i = 0; i < dawn->player_count; i++) {
            dawn->players[i].fullness += rain_amount;
            if (dawn->players[i].fullness >= 100) dawn->players[i].fullness = 100;
        }

        snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :%s has conjured up a storm. Rain pours down "
                "upon the town, allowing the crops to flourish! Everyone's fullness +%d!\r\n",
                 dawn->active_room, username, rain_amount);
    } else {
        snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :%s, you do not have enough mana to cast this spell\r\n",
                 dawn->active_room, username);
    }
    add_msg(out, strlen(out));
}

void check_learn_spells(const char *username) {
    int pindex = get_pindex(username);
    int plevel = dawn->players[pindex].level;
    char out[MAX_MESSAGE_BUFFER];
    char spell[20];

    switch (plevel) {
        case 3:
            dawn->players[pindex].spellbook.heal.learned = 1;
            dawn->players[pindex].spellbook.heal.level = 1;
            strcpy(spell, "Heal");
            break;
        case 5:
            dawn->players[pindex].spellbook.frost.learned = 1;
            dawn->players[pindex].spellbook.frost.level = 1;
            dawn->players[pindex].spellbook.frost.element = ICE;
            strcpy(spell, "Frost");
        case 8:
            dawn->players[pindex].spellbook.rain.learned = 1;
            dawn->players[pindex].spellbook.rain.level = 1;
            strcpy(spell, "Rain");
            break;
        case 10:
            dawn->players[pindex].spellbook.revive.learned = 1;
            dawn->players[pindex].spellbook.revive.level = 1;
            strcpy(spell, "Revive");
            break;
        case 17:
            dawn->players[pindex].spellbook.fireball.learned = 1;
            dawn->players[pindex].spellbook.fireball.level = 1;
            strcpy(spell, "Fireball");
            break;
        case 21:
            dawn->players[pindex].spellbook.teleport.learned = 1;
            dawn->players[pindex].spellbook.teleport.level = 1;
            strcpy(spell, "Teleport");
            break;
        default:
            return;
    }
    snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :%s has learned the spell %s!\r\n",
             dawn->active_room, username, spell);
    add_msg(out, strlen(out));
}
