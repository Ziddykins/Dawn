#ifndef NAMEGEN_H_INCLUDED
#define NAMEGEN_H_INCLUDED

#include "analyzer.h"

struct namegen {
    Analyzer * tiers;
    unsigned int markov_tier;
    int success;
};

typedef struct namegen * NameGen;

NameGen init_name_gen(unsigned int markov_tier);
void free_name_gen(NameGen);

void add_file(NameGen, char const *);

//name must have avg*2+markovTier+1 space
void gen_name(char * name, NameGen, size_t avg, double var);

//internal functions
int is_producible(NameGen ng, char src, size_t max_tier);

#endif // NAMEGEN_H_INCLUDED
