#ifndef SPELLS_H_INCLUDED
#define SPELLS_H_INCLUDED

void cast_heal (struct Bot *, const char *, const char *);
void cast_rain (struct Bot *, const char *);
void cast_fireball (struct Bot *, const char *, const char *);
void check_learn_spells (struct Bot *, const char *);

enum Element {FIRE, ICE, EARTH, WATER};

struct SpellHeal {
    int level, experience, learned;
};

struct SpellRain {
    int level, experience, learned;
};

struct SpellFireball {
    int level, experience, learned;
};

struct SpellFrost {
    int level, experience, learned;
    enum Element element;
};
    
struct SpellBook {
    struct SpellHeal heal;
    struct SpellRain rain;
    struct SpellFrost frost;
    struct SpellFireball fireball;
};

#endif
