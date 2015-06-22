#ifndef PLAYER_H_INCLUDED
#define PLAYER_H_INCLUDED
#include "status.h"
#include "network.h"

//Prototypes
void save_players (struct Bot *, size_t);
void load_players (struct Bot *, size_t);
void print_sheet (struct Bot *, struct Message *);
void init_new_character (char [], char [], struct Bot *, struct Message *);
void check_levelup (struct Bot *, struct Message *);
long get_nextlvl_exp (struct Bot *, char []);
int  get_pindex (struct Bot *, char []);

#endif
