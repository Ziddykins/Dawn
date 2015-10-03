#include <string.h>
#include <stdio.h>
#include "include/status.h"
#include "include/player.h"
#include "include/network.h"
#include "include/parse.h"
#include "include/colors.h"
#include "include/limits.h"

int is_equipped (int pindex, int slot) {
    return ((dawn->players[pindex].inventory[slot].bitfield >> EQUIPPED) & 1);
}

void equip_toggle (int pindex, int slot) {
    dawn->players[pindex].inventory[slot].bitfield ^= (1 << EQUIPPED);
}

int is_equippable (int pindex, int slot) {
    return ((dawn->players[pindex].inventory[slot].bitfield >> EQUIPPABLE) & 1);
}

void acknowledge_new (int pindex, int slot) {
    dawn->players[pindex].inventory[slot].bitfield &= ~(1 << NEW);
}

void favorite_toggle (int pindex, int slot) {
    dawn->players[pindex].inventory[slot].bitfield ^= (1 << FAVORITE);
}

int is_favorite (int pindex, int slot) {
    return ((dawn->players[pindex].inventory[slot].bitfield >> FAVORITE) & 1);
}

int is_new (int pindex, int slot) {
    return ((dawn->players[pindex].inventory[slot].bitfield >> NEW) & 1);
}
        
void print_inventory(struct Message *message) {
    int i = get_pindex(message->sender_nick);
    int j;
    char out[MAX_MESSAGE_BUFFER];
    char temp[100];

    sprintf(out, "PRIVMSG %s :", message->receiver);

    for (j = 0; j < (MAX_INVENTORY_SLOTS - dawn->players[i].available_slots); j++) {
        if (j % 9 == 0) {
            strcat(out, "\r\n");
            add_msg(out, strlen(out));
            sprintf(out, "PRIVMSG %s :", message->receiver);
        }
        temp[0] = '\0';

        sprintf(temp, "[%s%d%s%s] - %s%s%s ", 
                is_equipped(i, j) ? IRC_GREEN : IRC_RED,
                j,
                IRC_NORMAL,
                is_favorite(i, j) ? IRC_PURPLE"*"IRC_NORMAL : "",
                is_new(i, j) ? IRC_RED"["IRC_NORMAL : "",
                dawn->players[i].inventory[j].name,
                is_new(i, j) ? IRC_RED"]"IRC_NORMAL : "");
        strcat(out, temp);
    }
    sprintf(temp, " - Available slots: %d", dawn->players[i].available_slots);
    strcat(out, temp);
    strcat(out, "\r\n");
    add_msg(out, strlen(out));
}

void equip_inventory(struct Message *message, int slot, int unequip, int all) {
    int i = get_pindex(message->sender_nick);
    char out[MAX_MESSAGE_BUFFER];

    if (all) {
        int count = 0;
        for (int j = 0; j < MAX_INVENTORY_SLOTS - dawn->players[i].available_slots; j++) {
            if (!unequip && !is_equipped(i, j)) {
                equip_toggle(i, j);
                acknowledge_new(i, j);
                count++;
            } else if (unequip && is_equipped(i, j)) {
                equip_toggle(i, j);
                count++;
            }
        }

        sprintf(out, "PRIVMSG %s :%s, %d pieces of equipment have been %s\r\n",
                message->receiver, message->sender_nick, count, unequip ? "unequipped" : "equipped");
        add_msg(out, strlen(out));
        return;
    }

    if (slot > MAX_INVENTORY_SLOTS) return;

    if (slot >= (MAX_INVENTORY_SLOTS - dawn->players[i].available_slots)) {
        sprintf(out, "PRIVMSG %s :There is nothing in slot %d\r\n", message->receiver, slot);
        add_msg(out, strlen(out));
        return;
    }

    if (unequip && is_equipped(i, slot)) {
        sprintf(out, "PRIVMSG %s :%s unequipped\r\n", message->receiver, dawn->players[i].inventory[slot].name);
        equip_toggle(i, slot);
        add_msg(out, strlen(out));
        return;
    } else if (unequip && !is_equipped(i, slot)) {
        sprintf(out, "PRIVMSG %s :%s is not equipped\r\n", message->receiver,
                dawn->players[i].inventory[slot].name);
        add_msg(out, strlen(out));
        return;
    }

    if (is_equippable(i, slot) && !is_equipped(i, slot)) {
        sprintf(out, "PRIVMSG %s :%s equipped\r\n", message->receiver, dawn->players[i].inventory[slot].name);
        equip_toggle(i, slot);
        acknowledge_new(i, slot);
    } else {
        sprintf(out, "PRIVMSG %s :%s is either unequippable or already equipped\r\n",
                message->receiver,
                dawn->players[i].inventory[slot].name);
    }
    add_msg(out, strlen(out));
}
