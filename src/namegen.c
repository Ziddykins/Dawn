#include "include/namegen.h"

NameGen init_name_gen(unsigned int markov_tier) {
    if(markov_tier == 0)
        return 0;
    struct namegen * ng;
    CALLEXIT(!(ng = malloc(sizeof *ng)))
    ng->markov_tier = markov_tier;
    ng->success = 0;
    CALLEXIT(!(ng->tiers = malloc(markov_tier * sizeof *ng->tiers)))
    for(size_t i = 0; i < markov_tier; i++) {
        ng->tiers[i] = init_analyzer(i+1);
    }
    return ng;
}

void free_name_gen(NameGen ng) {
    if(!ng)
        return;
    struct namegen * cng = ng;
    for(size_t i = 0; i < cng->markov_tier; i++) {
        free_analyzer(cng->tiers[i]);
    }
    free(cng->tiers);
    free(cng);
}

void add_file(NameGen ng, char const * fn) {
    if(!ng || !fn || strlen(fn) == 0)
        return;
    struct namegen * cng = ng;
    for(size_t i = 0; i < cng->markov_tier; i++) {
        if(!analyze(cng->tiers[i], fn)) {
            cng->success = 0;
            return;
        }
    }
    cng->success = 1;
}

void gen_name(char * name, NameGen ng, size_t avg, double var) {
    if(!ng)
        return;
    struct namegen * cng = ng;
    assert(name && avg > 0 && var >= 0);
    assert(cng->markov_tier <= avg);
    //assert((malloc_usable_size(name)/sizeof *name) >= avg + cng->markov_tier + avg*var);
    assert(cng->success);
    double gauss = gaussrand();
    gauss = gauss > 1 || gauss < -1 ? 0 : gauss;
    size_t min = avg + (size_t)(gauss * avg * var);
    /*do {
        name[0] = (char)(genNum() * 26 + 'A') //(char)(rand()%256-128);
    } while(!isProducible(ng, name[0], cng->markov_tier));*/

    size_t tier = (size_t)((1-ABS(gauss)) * (cng->markov_tier-1));
    char * start_str = gen_start(cng->tiers[tier]);
    size_t len = 0;
    int ret = snprintf(name, avg*2+1, "%s", start_str);
    CALLEXIT(ret < 0 || ret == (int)(avg*2+1))
    len += (size_t)ret;


    char last = name[tier+1];
    while(len < min) {
        while((tier = ((size_t)(rand()) % cng->markov_tier))+len+1 > min); //probability skew is okay
        char * res;
        while(!is_producible(ng, last, min-len)) {
            last = (char)(rand() % 256 - 128);
        }
        if(!(res = gen_token(cng->tiers[tier], last, 0))) {
            continue;
        }
        snprintf(name+len, (size_t)(avg*2+1-len), "%s", res);
        len += tier+1;
        assert(len < avg*2+1);
        last = name[len-1];
    }

    size_t distrib_len = (size_t)((ABS(gaussrand())+(1-var))*cng->markov_tier);
    for(size_t i = distrib_len < cng->markov_tier-1 ? distrib_len : cng->markov_tier-1; i < cng->markov_tier; i--) {
        char * res;
        if((res = gen_token(cng->tiers[i], last, 1)) != 0) {
            snprintf(name+len, (size_t)(avg*2+cng->markov_tier+1), "%s", res);
            len += i+1;
            assert(len <= avg*2+cng->markov_tier+1);
            break;
        }
    }
}

int is_producible(NameGen ng, char src, size_t max_tier) {
    if(!ng || src == '\0')
        return 0;
    int valid = 0;
    struct namegen * cng = ng;
    for(size_t i = 0; i < max_tier && i < cng->markov_tier; i++) {
        if(has_rule_for(cng->tiers[i], src)) {
            valid = 1;
            break;
        }
    }
    return valid;
}
