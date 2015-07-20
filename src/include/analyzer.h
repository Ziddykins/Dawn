#ifndef ANALYZER_H_INCLUDED
#define ANALYZER_H_INCLUDED

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include "tokenlist.h"
#include "colors.h"
#include "util.h"

#define CHAR_LEN 256
//#define MAX_WORD_LEN 1024
//#define MAX_WORD_LEN_LITERAL "1024"

struct analyzer {
    size_t markov_tier;
    TokenList * tlists;
    TokenList start;
    size_t tokens;
};

typedef void * Analyzer;

Analyzer init_analyzer(size_t markov_tier);
void free_analyzer(Analyzer);

int analyze(Analyzer, char const *);

//uses rand without calling srand!
char * gen_token(Analyzer, char src, int end);
char * gen_start(Analyzer);
int has_rule_for(Analyzer, char src);

#endif // ANALYZER_H_INCLUDED
