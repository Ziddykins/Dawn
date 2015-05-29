#ifndef STATUS_H_INCLUDED
#define STATUS_H_INCLUDED
#include "player.h"

typedef struct {
    char *nickname;
    char *realname;
    char *ident;
    char *password;
    int login_sent;
    int in_rooms;
    int player_count;
    Player players[100];
} Bot;

#endif
