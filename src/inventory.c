#include <string.h>
#include <stdio.h>
#include "include/status.h"
#include "include/player.h"
#include "include/network.h"
#include "include/parse.h"
#include "include/colors.h"
#include "include/limits.h"

void print_inventory (struct Bot *b, struct Message *message) {
    int i = get_pindex(b, message->sender_nick);
    int j;
    char out[MAX_MESSAGE_BUFFER];
    char temp[100];

    sprintf(out, "PRIVMSG %s :", message->receiver);
    for (j=0; j<(MAX_INVENTORY_SLOTS - b->players[i].available_slots); j++) {
        if (j % 9 == 0) {
            strcat(out, "\r\n");
            addMsg(out, strlen(out));
            sprintf(out, "PRIVMSG %s :", message->receiver);
        }
        int equipped = b->players[i].inventory[j].equipped;
        temp[0] = '\0';
        sprintf(temp, "[%s%d%s] - %s ", equipped ? IRC_GREEN : IRC_RED, j, IRC_NORMAL,
                b->players[i].inventory[j].name);
        strcat(out, temp);
    }
    sprintf(temp, " - Available slots: %d", b->players[i].available_slots);
    strcat(out, temp);
    strcat(out, "\r\n");
    addMsg(out, strlen(out));
}

void equip_inventory (struct Bot *b, struct Message *message, int slot, int unequip) {
    int i = get_pindex(b, message->sender_nick);
    char out[MAX_MESSAGE_BUFFER];

    if (slot > MAX_INVENTORY_SLOTS) return;

    if (slot >= (MAX_INVENTORY_SLOTS - b->players[i].available_slots)) {
        sprintf(out, "PRIVMSG %s :There is nothing in slot %d\r\n", message->receiver, slot);
        addMsg(out, strlen(out));
        return;
    }
    if (unequip && b->players[i].inventory[slot].equipped) {
        sprintf(out, "PRIVMSG %s :%s unequipped\r\n", message->receiver, b->players[i].inventory[slot].name);
        b->players[i].inventory[slot].equipped = 0;
        addMsg(out, strlen(out));
        return;
    } else if (unequip && !b->players[i].inventory[slot].equipped) {
        sprintf(out, "PRIVMSG %s :%s is not equipped\r\n", message->receiver,
                      b->players[i].inventory[slot].name);
        addMsg(out, strlen(out));
    }

    if (b->players[i].inventory[slot].equippable && !b->players[i].inventory[slot].equipped) {
        sprintf(out, "PRIVMSG %s :%s equipped\r\n", message->receiver, b->players[i].inventory[slot].name);
        b->players[i].inventory[slot].equipped = 1;
    } else {
        sprintf(out, "PRIVMSG %s :%s is either unequippable or already equipped\r\n",
                message->receiver,
                b->players[i].inventory[slot].name);
    }
    addMsg(out, strlen(out));
}
