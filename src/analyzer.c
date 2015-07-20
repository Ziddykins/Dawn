#include "include/analyzer.h"

Analyzer init_analyzer(size_t markov_tier) {
    if(markov_tier == 0)
        return 0;
    struct analyzer * a;
    CALLEXIT(!(a = malloc(sizeof *a)))

    a->tlists = 0;
    a->markov_tier = markov_tier;
    a->tokens = 0;
    a->start = 0;
    return a;
}

void free_analyzer(Analyzer a) {
    if(!a)
        return;
    struct analyzer * ca = a;
    if(ca->tlists) {
        for(size_t i = 0; i < CHAR_LEN; i++) {
            free_token_list(ca->tlists[i]);
        }
    }
    free(ca->tlists);
    free_token_list(ca->start);
    free(ca);
}

int analyze(Analyzer a, char const * fn) {
    if(!a)
        return 0;
    struct analyzer * ca = a;
    FILE * file;
    CALLEXIT(!(file = fopen(fn, "r")))

    char * word = 0; //malloc(MAX_WORD_LEN)
    char * prediction;
    CALLEXIT(!(prediction = malloc(ca->markov_tier+1))) //next n characters, \0 terminated
    if(!ca->tlists) {
        CALLEXIT(!(ca->tlists = malloc(CHAR_LEN * sizeof *ca->tlists)))
        for(size_t i = 0; i < CHAR_LEN; i++) {
            ca->tlists[i] = init_token_list();
        }
    }
    if(!ca->start) {
        ca->start = init_token_list();
    }
    size_t len = 0;
    char * start;
    CALLEXIT(!(start = calloc(1, ca->markov_tier+2)))
    while(getline(&word, &len, file) != -1) { //fscanf(file, "%"MAX_WORD_LEN_LITERAL"s", word) != EOF
        word[strlen(word)-1] = '\0';
        char cur_char = word[0];

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
        inc_token(ca->start, start, 0);

        for(i = 1; prediction[ca->markov_tier-1] != '\0' && cur_char != '\0'; i++) {
            //add markov prediction
            int end = 0;

            if(word[i+ca->markov_tier] == '\0') {
                end = 1;
            }
            inc_token(ca->tlists[(unsigned char)cur_char], prediction, end);
            ca->tokens++;
            //shift prediction
            for(size_t pos = 1; pos < ca->markov_tier; pos++) {
                prediction[pos-1] = prediction[pos];
            }
            prediction[ca->markov_tier-1] = word[i+ca->markov_tier];

            //get next character
            cur_char = word[i];
        }
    }
    fclose(file);
    free(word);
    free(start);
    free(prediction);
    return 1;
}

char * gen_token(Analyzer a, char src, int end) {
    if(!a) {
        return 0;
    }

    struct analyzer * ca = a;
    if(!ca->tlists || ca->tokens == 0) {
        return 0;
    }

    if(get_len(ca->tlists[(unsigned char)src]) == 0) {
        return 0;
    }

    double prob = randd();
    TokenIterator it = get_iterator(ca->tlists[(unsigned char)src]);

    while(has_elem(it) && (prob > ((double)(get_num_consumed(it)))/get_num_total(ca->tlists[(unsigned char)src]) || end ? !get_is_end(it) : 0)) {
        next(it);
    }

    if(!has_elem(it) || (end && !get_is_end(it))) {
        free_iterator(it);
        return 0;
    }

    char * res = get_str(it);
    free_iterator(it);

    return res;
}

char * gen_start(Analyzer a) {
    if(!a) {
        return 0;
    }

    struct analyzer * ca = a;
    if(!ca->start || ca->tokens == 0) {
        return 0;
    }

    if(get_len(ca->start) == 0) {
        return 0;
    }

    double prob = randd();
    TokenIterator it = get_iterator(ca->start);

    while(has_elem(it) && (prob > ((double)(get_num_consumed(it)))/get_num_total(ca->start))) {
        next(it);
    }

    if(!has_elem(it)) {
        free_iterator(it);
        return 0;
    }

    char * res = get_str(it);
    free_iterator(it);

    return res;
}

int has_rule_for(Analyzer a, char src) {
    if(!a) {
        return 0;
    }

    struct analyzer * ca = a;
    if(!ca->tlists) {
        return 0;
    }
    return get_len(ca->tlists[(unsigned char)src]) != 0;
}
