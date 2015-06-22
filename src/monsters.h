#ifndef MONSTERS_H_INCLUDED
#define MONSTERS_H_INCLUDED
struct Monsters {
    char name[64];
    unsigned mhp, def, str, intel, mdef, gold, exp, drop_level, active;
    int hp;
};
#endif
