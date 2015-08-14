#include <string.h>
#include "include/status.h"
#include "include/spells.h"
#include "include/limits.h"

void cast_heal (struct Bot *b, const char *username, const char *target) {
    int pindex = get_pindex(b, username);
    int tindex = get_pindex(b, target);
    char out[MAX_MESSAGE_BUFFER];

    if (tindex == -1) {
        snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :Invalid target\r\n", b->active_room);
        add_msg(out, strlen(out));
        return;
    }

    if (!b->players[pindex].spellbook.heal.learned) {
        snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :%s, you do not know this spell\r\n",
                b->active_room, username);
        add_msg(out, strlen(out));
        return;
    }

    if (b->players[tindex].health <= 0) {
        snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :%s, your magic powers are not yet strong enough "
                "to revive the dead\r\n", b->active_room, username);
        add_msg(out, strlen(out));
        return;
    }

    if (b->players[tindex].health == b->players[tindex].max_health) {
        snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :%s is already at full health\r\n",
                b->active_room, target);
        add_msg(out, strlen(out));
        return;
    }

    int heal_cost   = 1 + b->players[pindex].spellbook.heal.level * 5;
    int heal_amount = 1 + rand() % (b->players[pindex].spellbook.heal.level * 5);

    if (b->players[pindex].mana >= heal_cost) {
        b->players[tindex].health += heal_amount;
        if (b->players[tindex].health >= b->players[tindex].max_health)
            b->players[tindex].health = b->players[tindex].max_health;
        sprintf(out, "PRIVMSG %s :%s has healed %s for %d HP!\r\n", b->active_room, username, target, heal_amount);
    } else {
        sprintf(out, "PRIVMSG %s :%s, you don't have enough mana to cast this spell\r\n", b->active_room, username);
    }

    add_msg(out, strlen(out));
}

void cast_fireball (struct Bot *b, const char *username, const char *target) {
    int pindex = get_pindex(b, username);
    int tindex = get_pindex(b, target);
    char out[MAX_MESSAGE_BUFFER];

    if (tindex == -1) {
        snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :Invalid target\r\n", b->active_room);
        add_msg(out, strlen(out));
        return;
    }

    if (!b->players[pindex].spellbook.fireball.learned) {
        snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :%s, you do not know this spell\r\n",
                b->active_room, username);
        add_msg(out, strlen(out));
        return;
    }

    if (b->players[tindex].health <= 0) {
        snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :%s, %s is dead\r\n",
                b->active_room, username, target);
        add_msg(out, strlen(out));
        return;
    }

    if (b->players[pindex].level >= b->players[tindex].level * 3) {
        snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :%s, pick on someone your own size!\r\n",
                b->active_room, username);
        add_msg(out, strlen(out));
        return;
    }

    if (strcmp(username, target) == 0) {
        snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :%s whips a fireball at a mirror; it bounces back"
                ", hitting themselves for 1 point of damage. Well done.\r\n", b->active_room, username);
        b->players[pindex].health--;
        add_msg(out, strlen(out));
        return;
    }
    
    int fireball_cost   = 1 + b->players[pindex].spellbook.fireball.level * 7;
    int fireball_damage = 1 + rand() % b->players[pindex].intelligence * b->players[pindex].spellbook.fireball.level;

    if (b->players[pindex].mana >= fireball_cost) {
        int target_save = 1 + rand() % b->players[tindex].m_def;
        char result[128];
        b->players[pindex].mana -= fireball_cost;
        snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :%s hurls a fireball at %s! ",
                b->active_room, username, target);
        if (target_save == fireball_damage) {
            snprintf(result, 128, "%s's shields had just enough energy to absorb the entire fireball!\r\n", target);
        } else if (target_save < fireball_damage) {
            struct Message tmpmsg;
            strcpy(tmpmsg.sender_nick, target);
            strcpy(tmpmsg.receiver, b->active_room);
            snprintf(result, 128, "%s is hit for %d damage!\r\n", target, fireball_damage - target_save);
            b->players[tindex].health -= fireball_damage - target_save;
            check_alive(b, &tmpmsg);
        } else if (target_save > fireball_damage) {
            snprintf(result, 128, "%s swats the fireball out of the sky with ease\r\n", target);
        }
        strncat(out, result, MAX_MESSAGE_BUFFER - strlen(result));
    }
    add_msg(out, strlen(out));
}

void cast_rain (struct Bot *b, const char *username) {
    int pindex = get_pindex(b, username);
    char out[MAX_MESSAGE_BUFFER];

    if (!b->players[pindex].spellbook.rain.learned) {
        snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :%s, you do not know this spell\r\n",
                b->active_room, username);
        add_msg(out, strlen(out));
        return;
    }
    
    int rain_cost   = 1 + b->players[pindex].spellbook.rain.level * 6;
    int rain_amount = 1 + rand() % (b->players[pindex].spellbook.rain.level * 6);

    if (b->players[pindex].mana >= rain_cost) {
        b->players[pindex].mana -= rain_cost;

        for (int i=0; i<b->player_count; i++) {
            b->players[i].fullness += rain_amount;
            if (b->players[i].fullness >= 100) b->players[i].fullness = 100;
        }

        snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :%s has conjured up a storm. Rain pours down "
                "upon the town, allowing the crops to flourish! Everyone's fullness +%d!\r\n",
                b->active_room, username, rain_amount);
    } else {
        snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :%s, you do not have enough mana to cast this spell\r\n",
                b->active_room, username);
    }
    add_msg(out, strlen(out));
}

void check_learn_spells (struct Bot *b, const char *username) {
    int pindex = get_pindex(b, username);
    int plevel = b->players[pindex].level;
    char out[MAX_MESSAGE_BUFFER];
    char spell[20];

    switch (plevel) {
        case 3:
            b->players[pindex].spellbook.heal.learned = 1;
            b->players[pindex].spellbook.heal.level   = 1;
            strcpy(spell, "Heal");
            break;
        case 5:
            b->players[pindex].spellbook.frost.learned = 1;
            b->players[pindex].spellbook.frost.level   = 1;
            b->players[pindex].spellbook.frost.element = ICE;
            strcpy(spell, "Frost");
        case 8:
            b->players[pindex].spellbook.rain.learned = 1;
            b->players[pindex].spellbook.rain.level   = 1;
            strcpy(spell, "Rain");
            break;
        case 17:
            b->players[pindex].spellbook.fireball.learned = 1;
            b->players[pindex].spellbook.fireball.level   = 1;
            strcpy(spell, "Fireball");
            break;
        default:
            return;
    }
    snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :%s has learned the spell %s!\r\n",
            b->active_room, username, spell);
    add_msg(out, strlen(out));
}
