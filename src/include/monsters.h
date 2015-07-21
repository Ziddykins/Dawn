#ifndef MONSTERS_H_INCLUDED
#define MONSTERS_H_INCLUDED
struct Bot;
void slay_monster  (struct Bot *, const char [], int, int);
void call_monster  (struct Bot *, const char [], int);
void print_monster (struct Bot *, const char [], int);

enum MonsterType {BERSERKER, ELDER, THIEF, TANK, BOSS, DEFMON};

struct Monsters {
    char name[64];
    int hp, mhp, def, str, intel, mdef;
    int gold, exp, drop_level, active;
    int slay_cost, which;
    enum MonsterType type;
};

#endif
