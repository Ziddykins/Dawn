#include "include/namegen.h"

NameGen createNameGen(unsigned int markovTier) {
    if(markovTier == 0)
        return 0;
    struct namegen * ng = malloc(sizeof *ng);
    ng->markov_tier = markovTier;
    ng->success = 0;
    ng->tiers = malloc(markovTier * sizeof *ng->tiers);
    for(size_t i = 0; i < markovTier; i++) {
        ng->tiers[i] = createAnalyzer(i+1);
    }
    return ng;
}

void freeNameGen(NameGen ng) {
    if(!ng)
        return;
    struct namegen * cng = ng;
    for(size_t i = 0; i < cng->markov_tier; i++) {
        freeAnalyzer(cng->tiers[i]);
    }
    free(cng->tiers);
    free(cng);
}

void addFile(NameGen ng, char const * fn) {
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

void genName(char * name, NameGen ng, size_t avg, double var) {
    if(!ng)
        return;
    struct namegen * cng = ng;
    assert(name && avg >= 0 && var >= 0);
    //assert((malloc_usable_size(name)/sizeof *name) >= avg + cng->markov_tier + avg*var);
    assert(cng->success);
    double gauss = gaussrand();
    gauss = gauss > 1 || gauss < -1 ? 0 : gauss;
    size_t min = avg + (size_t)(gauss * avg * var);
    /*do {
        name[0] = (char)(genNum() * 26 + 'A') //(char)(rand()%256-128);
    } while(!isProducible(ng, name[0], cng->markov_tier));*/

    size_t tier = (size_t)((1-ABS(gauss)) * (cng->markov_tier-1));
    char * startStr = genStart(cng->tiers[tier]);
    size_t len = 0;
    int ret = snprintf(name, avg*2+1, "%s", startStr);
    if(ret < 0 || ret == (int)(avg*2+1)) {
        perror("snprintf");
        return;
    }
    len += (size_t)ret;


    char last = name[tier+1];
    while(len < min) {
        while((tier = ((size_t)(rand()) % cng->markov_tier))+len+1 > min); //probability skew is okay
        char * res;
        while(!isProducible(ng, last, min-len)) {
            last = (char)(rand() % 256 - 128);
        }
        if(!(res = genToken(cng->tiers[tier], last, 0))) {
            continue;
        }
        snprintf(name+len, (size_t)(avg*2+1-len), "%s", res);
        len += tier+1;
        assert(len < avg*2+1);
        last = name[len-1];
    }

    size_t distributedLen = (size_t)((ABS(gaussrand())+(1-var))*cng->markov_tier);
    for(size_t i = distributedLen < cng->markov_tier-1 ? distributedLen : cng->markov_tier-1; i < cng->markov_tier; i--) {
        char * res;
        if((res = genToken(cng->tiers[i], last, 1)) != 0) {
            snprintf(name+len, (size_t)(avg*2+cng->markov_tier+1), "%s", res);
            len += i+1;
            assert(len <= avg*2+cng->markov_tier+1);
            break;
        }
    }
}

double ABS(double a) {
    return a > -a ? a : -a;
}

double gaussrand() {
	static double V1, V2, S;
	static int phase = 0;
	double X;

	if(phase == 0) {
		do {
			V1 = 2 * genNum() - 1;
			V2 = 2 * genNum() - 1;
			S = V1 * V1 + V2 * V2;
			} while(S >= 1 || S == 0);
		X = V1 * sqrt(-2 * log(S) / S);
	} else {
		X = V2 * sqrt(-2 * log(S) / S);
    }
	phase = !phase;

	return X;
}

int isProducible(NameGen ng, char src, size_t maxTier) {
    if(!ng || src == '\0')
        return 0;
    int valid = 0;
    struct namegen * cng = ng;
    for(size_t i = 0; i < maxTier && i < cng->markov_tier; i++) {
        if(hasRuleFor(cng->tiers[i], src)) {
            valid = 1;
            break;
        }
    }
    return valid;
}
