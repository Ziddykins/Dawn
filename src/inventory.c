#include <string.h>
#include <stdio.h>
#include "include/status.h"
#include "include/player.h"
#include "include/network.h"
#include "include/parse.h"
#include "include/colors.h"
#include "include/limits.h"

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
        int equipped = dawn->players[i].inventory[j].equipped;
        temp[0] = '\0';
        sprintf(temp, "[%s%d%s] - %s ", equipped ? IRC_GREEN : IRC_RED, j, IRC_NORMAL,
                dawn->players[i].inventory[j].name);
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
            if (!unequip && !dawn->players[i].inventory[j].equipped) {
                dawn->players[i].inventory[j].equipped = 1;
                count++;
            } else if (unequip && dawn->players[i].inventory[j].equipped) {
                dawn->players[i].inventory[j].equipped = 0;
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

    if (unequip && dawn->players[i].inventory[slot].equipped) {
        sprintf(out, "PRIVMSG %s :%s unequipped\r\n", message->receiver, dawn->players[i].inventory[slot].name);
        dawn->players[i].inventory[slot].equipped = 0;
        add_msg(out, strlen(out));
        return;
    } else if (unequip && !dawn->players[i].inventory[slot].equipped) {
        sprintf(out, "PRIVMSG %s :%s is not equipped\r\n", message->receiver,
                dawn->players[i].inventory[slot].name);
        add_msg(out, strlen(out));
    }

    if (dawn->players[i].inventory[slot].equippable && !dawn->players[i].inventory[slot].equipped) {
        sprintf(out, "PRIVMSG %s :%s equipped\r\n", message->receiver, dawn->players[i].inventory[slot].name);
        dawn->players[i].inventory[slot].equipped = 1;
    } else {
        sprintf(out, "PRIVMSG %s :%s is either unequippable or already equipped\r\n",
                message->receiver,
                dawn->players[i].inventory[slot].name);
    }
    add_msg(out, strlen(out));
}
