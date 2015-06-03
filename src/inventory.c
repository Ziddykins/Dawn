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
            for (j=0; j<=dawn->players[i].available_slots-24; j++) {
                char temp[100];
                int equipped = dawn->players[i].inventory[j].equipped;
                sprintf(temp, "[%c%d ] - %s ", equipped ? '*' : ' ', j, dawn->players[i].inventory[j].name);
                strcat(out, temp);
            }
            strcat(out, "\r\n");
            send_socket(out);
        }
    }
}

void equip_inventory (Bot *dawn, Message *message, int slot, int unequip) {
    int i;
    char out[MAX_MESSAGE_BUFFER];
    if (slot < 0 || slot > 25) return;
    for (i=0; i<=dawn->player_count; i++) {
        if (strcmp(message->sender_nick, dawn->players[i].username) == 0) {
            if (unequip && dawn->players[i].inventory[slot].equipped) {
                sprintf(out, "PRIVMSG %s :%s unequipped\r\n", message->receiver, dawn->players[i].inventory[slot].name);
                dawn->players[i].inventory[slot].equipped = 0;
                dawn->players[i].strength     -= dawn->players[i].inventory[slot].attr_strength;
                dawn->players[i].defense      -= dawn->players[i].inventory[slot].attr_defense;
                dawn->players[i].intelligence -= dawn->players[i].inventory[slot].attr_intelligence;
                dawn->players[i].m_def        -= dawn->players[i].inventory[slot].attr_mdef;
                send_socket(out);
                return;
            } else if (unequip && !dawn->players[i].inventory[slot].equipped) {
                sprintf(out, "PRIVMSG %s :%s is not equipped\r\n", message->receiver, 
                              dawn->players[i].inventory[slot].name);
                send_socket(out);
            }

            if (dawn->players[i].inventory[slot].equippable 
                    && !dawn->players[i].inventory[slot].equipped) {
                sprintf(out, "PRIVMSG %s :%s equipped\r\n", message->receiver, dawn->players[i].inventory[slot].name);
                dawn->players[i].inventory[slot].equipped = 1;
                //TODO:Going to have to be moved to stats.c
                //Make new functions to return user stats taking into account all
                //of the boosts, such as equipment boosts, boosts from sockets, etc
                dawn->players[i].strength     += dawn->players[i].inventory[slot].attr_strength;
                dawn->players[i].defense      += dawn->players[i].inventory[slot].attr_defense;
                dawn->players[i].intelligence += dawn->players[i].inventory[slot].attr_intelligence;
                dawn->players[i].m_def        += dawn->players[i].inventory[slot].attr_mdef;
            } else {
                sprintf(out, "PRIVMSG %s :%s is either unequippable or already equipped\r\n",
                        message->receiver,
                        dawn->players[i].inventory[slot].name);
            }
            send_socket(out);
        }
    }
}
