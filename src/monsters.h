#ifndef MONSTERS_H_INCLUDED
#define MONSTERS_H_INCLUDED
typedef struct {
    char name[64];
    unsigned mhp, def, str, intel, mdef, gold, exp, drop_level, active;
    int hp;
} Monsters;
#endif
