#include <string.h>
#include <stdio.h>
#include "player.h"
#include "status.h"
#include "network.h"

void print_inventory (Bot *dawn, Message *message) {
    int i, j;
    char out[1024];
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
