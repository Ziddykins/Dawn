#ifndef PLAYER_H_INCLUDED
#define PLAYER_H_INCLUDED
#include "status.h"
#include "network.h"

//Prototypes
void save_players (Bot *, size_t);
void load_players (Bot *, size_t);
void print_sheet (Bot *, Message *);
void init_new_character (char [], char [], Bot *, Message *);
void check_levelup (Bot *, Message *);
int get_pindex (Bot *, char []);
#endif
