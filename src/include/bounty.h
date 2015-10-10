#ifndef BOUNTY_H_INCLUDED
#define BOUNTY_H_INCLUDED

//Prototypes
void check_bounty (struct Message *msg);
void gen_global_bounty (void);

struct Bounty {
    char name[64];
    int amount;
    int multiplier;
};

#endif
