#ifndef MONSTERS_H_INCLUDED
#define MONSTERS_H_INCLUDED
typedef struct {
    char name[64];
    int hp, mhp, def, str, intel, mdef, gold, exp, range;
    int active, told;
} Monsters;
#endif
