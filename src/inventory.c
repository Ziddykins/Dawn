#include <string.h>
#include <stdio.h>
#include "status.h"
#include "player.h"
#include "network.h"
#include "parse.h"

void print_inventory (Bot *dawn, Message *message) {
    int i;
    unsigned int j;
    char out[MAX_MESSAGE_BUFFER];
    for (i=0; i<dawn->player_count; i++) {
        if (strcmp(message->sender_nick, dawn->players[i].username) == 0) {
            sprintf(out, "PRIVMSG %s :", message->receiver);
            for (j=0; j<=dawn->players[i].inventory.available_slots-24; j++) {
                char temp[100];
                int equipped = dawn->players[i].inventory.equipment[j].equipped;
                //Ick
                sprintf(temp, "[%c%d ] - %s ", equipped ? '*' : ' ', j,
                        dawn->players[i].inventory.equipment[j].name);
                strcat(out, temp);
            }
            strcat(out, "\r\n");
            send_socket(out);
        }
    }
}

void equip_inventory (Bot *dawn, Message *message, int slot) {
    int i;
    char out[MAX_MESSAGE_BUFFER];
    if (slot < 0 || slot > 25) return;
    for (i=0; i<=dawn->player_count; i++) {
        if (strcmp(message->sender_nick, dawn->players[i].username) == 0) {
            if (dawn->players[i].inventory.equipment[slot].equippable
                    && !dawn->players[i].inventory.equipment[slot].equipped) {
                sprintf(out, "PRIVMSG %s :%s equipped\r\n",
                        message->receiver,
                        dawn->players[i].inventory.equipment[slot].name);
                dawn->players[i].inventory.equipment[slot].equipped = 1;
            } else {
                sprintf(out, "PRIVMSG %s :%s is either unequippable or already equipped\r\n",
                        message->receiver,
                        dawn->players[i].inventory.equipment[slot].name);
            }
            send_socket(out);
        }
    }
}
