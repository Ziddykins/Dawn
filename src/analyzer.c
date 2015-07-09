#include "include/analyzer.h"

Analyzer createAnalyzer(size_t markovTier) {
    if(markovTier == 0)
        return 0;
    struct analyzer * a;
    CALLEXIT(a = malloc(sizeof *a))

    a->tlists = 0;
    a->markov_tier = markovTier;
    a->tokens = 0;
    a->start = 0;
    return a;
}

void freeAnalyzer(Analyzer a) {
    if(!a)
        return;
    struct analyzer * ca = a;
    if(ca->tlists) {
        for(size_t i = 0; i < CHAR_LEN; i++) {
            freeTokenList(ca->tlists[i]);
        }
    }
    free(ca->tlists);
    freeTokenList(ca->start);
    free(ca);
}

int analyze(Analyzer a, char const * fn) {
    if(!a)
        return 0;
    struct analyzer * ca = a;
    FILE * file;
    CALLEXIT(file = fopen(fn, "r"))

    char * word = 0; //malloc(MAX_WORD_LEN)
    char * prediction;
    CALLEXIT(prediction = malloc(ca->markov_tier+1)) //next n characters, \0 terminated
    if(!ca->tlists) {
        CALLEXIT(ca->tlists = malloc(CHAR_LEN * sizeof *ca->tlists))
        for(size_t i = 0; i < CHAR_LEN; i++) {
            ca->tlists[i] = createTokenList();
        }
    }
    if(!ca->start) {
        ca->start = createTokenList();
    }
    size_t len = 0;
    char * start;
    CALLEXIT(start = calloc(1, ca->markov_tier+2))
    while(getline(&word, &len, file) != -1) { //fscanf(file, "%"MAX_WORD_LEN_LITERAL"s", word) != EOF
        word[strlen(word)-1] = '\0';
        char curChar = word[0];

        //initialize prediction
        size_t i;
        for(i = 0; i < ca->markov_tier && word[i+1] != '\0'; i++) {
            prediction[i] = word[i+1];
        }
        prediction[ca->markov_tier] = '\0';

        //word too short?
        if(i < ca->markov_tier) {
            continue;
        }
        start[0] = word[0];
        strcpy(start+1, prediction);
        incToken(ca->start, start, 0);

        for(i = 1; prediction[ca->markov_tier-1] != '\0' && curChar != '\0'; i++) {
            //add markov prediction
            int end = 0;

            if(word[i+ca->markov_tier] == '\0') {
                end = 1;
            }
            incToken(ca->tlists[(unsigned char)curChar], prediction, end);
            ca->tokens++;
            //shift prediction
            for(size_t pos = 1; pos < ca->markov_tier; pos++) {
                prediction[pos-1] = prediction[pos];
            }
            prediction[ca->markov_tier-1] = word[i+ca->markov_tier];

            //get next character
            curChar = word[i];
        }
    }
    fclose(file);
    free(word);
    free(start);
    free(prediction);
    return 1;
}

char * genToken(Analyzer a, char src, int end) {
    if(!a) {
        return 0;
    }

    struct analyzer * ca = a;
    if(!ca->tlists || ca->tokens == 0) {
        return 0;
    }

    if(getLen(ca->tlists[(unsigned char)src]) == 0) {
        return 0;
    }

    double prob = genNum();
    TokenIterator it = getIterator(ca->tlists[(unsigned char)src]);

    while(hasElem(it) && (prob > ((double)(getConsumed(it)))/getTotalNum(ca->tlists[(unsigned char)src]) || end ? !getIsEnd(it) : 0)) {
        next(it);
    }

    if(!hasElem(it) || (end && !getIsEnd(it))) {
        freeIterator(it);
        return 0;
    }

    char * res = getStr(it);
    freeIterator(it);

    return res;
}

char * genStart(Analyzer a) {
    if(!a) {
        return 0;
    }

    struct analyzer * ca = a;
    if(!ca->start || ca->tokens == 0) {
        return 0;
    }

    if(getLen(ca->start) == 0) {
        return 0;
    }

    double prob = genNum();
    TokenIterator it = getIterator(ca->start);

    while(hasElem(it) && (prob > ((double)(getConsumed(it)))/getTotalNum(ca->start))) {
        next(it);
    }

    if(!hasElem(it)) {
        freeIterator(it);
        return 0;
    }

    char * res = getStr(it);
    freeIterator(it);

    return res;
}

int hasRuleFor(Analyzer a, char src) {
    if(!a) {
        return 0;
    }

    struct analyzer * ca = a;
    if(!ca->tlists) {
        return 0;
    }
    return getLen(ca->tlists[(unsigned char)src]) != 0;
}
