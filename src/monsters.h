#ifndef MONSTERS_H_INCLUDED
#define MONSTERS_H_INCLUDED
typedef struct {
    char name[64];
    unsigned mhp, def, str, intel, mdef, gold, exp, range, told, active;
    int hp;
} Monsters;
#endif
