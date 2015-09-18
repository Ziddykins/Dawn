#ifndef SPELLS_H_INCLUDED
#define SPELLS_H_INCLUDED

void cast_heal(const char *, const char *);
void cast_rain(const char *);
void cast_fireball(const char *, const char *);
void cast_revive(const char *, const char *);
void cast_teleport(const char *, int, int);
void check_learn_spells(const char *);

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

struct SpellRevive {
    int level, experience, learned;
};

struct SpellTeleport {
    int level, experiece, learned;
};
    
struct SpellBook {
    struct SpellHeal heal;
    struct SpellRain rain;
    struct SpellFrost frost;
    struct SpellFireball fireball;
    struct SpellRevive revive;
    struct SpellTeleport teleport;
};

#endif
