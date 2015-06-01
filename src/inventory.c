#include <string.h>
#include <stdio.h>
#include "player.h"
#include "status.h"
#include "network.h"
#include "parse.h"

void print_inventory (Bot *dawn, Message *message) {
    int i, j;
    char out[MAX_MESSAGE_BUFFER];
    for (i=0; i<dawn->player_count; i++) {
        if (strcmp(message->sender_nick, dawn->players[i].username) == 0) {
            sprintf(out, "PRIVMSG %s :", message->receiver);
            for (j=0; j<=dawn->players[i].inventory.available_slots-24; j++) {
                char temp[100];
                sprintf(temp, "[%d] - %s ", j, dawn->players[i].inventory.equipment[j].name);
                printf("temp %s\n", temp);
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
            sprintf(out, "PRIVMSG %s :%d\r\n", message->receiver, slot);
            send_socket(out);
        }
    }
}
