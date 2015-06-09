#include <string.h>
#include <stdio.h>
#include "status.h"
#include "player.h"
#include "network.h"
#include "parse.h"
#include "colors.h"
#include "limits.h"

void print_inventory (Bot *dawn, Message *message) {
    int i = get_pindex(dawn, message->sender_nick);
    unsigned int j;
    char out[MAX_MESSAGE_BUFFER];
    char temp[100];

    sprintf(out, "PRIVMSG %s :", message->receiver);
    for (j=0; j<(MAX_INVENTORY_SLOTS - dawn->players[i].available_slots); j++) {
        int equipped = dawn->players[i].inventory[j].equipped;
        temp[0] = '\0';
        sprintf(temp, "[%s%d%s] - %s ", equipped ? green : red, j, normal,
                                        dawn->players[i].inventory[j].name);
        strcat(out, temp);
    }
    sprintf(temp, " - Available slots: %d", dawn->players[i].available_slots);
    strcat(out, temp);
    strcat(out, "\r\n");
    send_socket(out);
}

void equip_inventory (Bot *dawn, Message *message, unsigned int slot, int unequip) {
    int i = get_pindex(dawn, message->sender_nick);
    char out[MAX_MESSAGE_BUFFER];
    
    if (slot > 25) return;

    if (slot >= (MAX_INVENTORY_SLOTS - dawn->players[i].available_slots)) {
        sprintf(out, "PRIVMSG %s :There is nothing in slot %d\r\n", message->receiver, slot);
        send_socket(out);
        return;
    }
    if (unequip && dawn->players[i].inventory[slot].equipped) {
        sprintf(out, "PRIVMSG %s :%s unequipped\r\n", message->receiver, dawn->players[i].inventory[slot].name);
        dawn->players[i].inventory[slot].equipped = 0;
        send_socket(out);
        return;
    } else if (unequip && !dawn->players[i].inventory[slot].equipped) {
        sprintf(out, "PRIVMSG %s :%s is not equipped\r\n", message->receiver, 
                      dawn->players[i].inventory[slot].name);
        send_socket(out);
    }

    if (dawn->players[i].inventory[slot].equippable && !dawn->players[i].inventory[slot].equipped) {
        sprintf(out, "PRIVMSG %s :%s equipped\r\n", message->receiver, dawn->players[i].inventory[slot].name);
        dawn->players[i].inventory[slot].equipped = 1;
    } else {
        sprintf(out, "PRIVMSG %s :%s is either unequippable or already equipped\r\n",
                message->receiver,
                dawn->players[i].inventory[slot].name);
    }
    send_socket(out);
}
