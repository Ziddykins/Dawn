#ifndef NAMEGEN_H_INCLUDED
#define NAMEGEN_H_INCLUDED

#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <assert.h>
#include <math.h>
#include "analyzer.h"
#include "colors.h"

struct namegen {
    Analyzer * tiers;
    unsigned int markov_tier;
    int success;
};

typedef void * NameGen;

NameGen createNameGen(unsigned int markovTier);
void freeNameGen(NameGen);

void addFile(NameGen, char const *);

//name must have avg*2+markovTier+1 space
void genName(char * name, NameGen, size_t avg, double var);

//internal functions
int isProducible(NameGen ng, char src, size_t maxTier);
double gaussrand(void);
double ABS(double);

#endif // NAMEGEN_H_INCLUDED
