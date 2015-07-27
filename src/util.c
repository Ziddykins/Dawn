#include "include/util.h"

double randd() {
    return ((double)(rand()))/INT_MAX;
}

double gaussrand() {
	static double V1, V2, S;
	static int phase = 0;
	double X;

	if(phase == 0) {
		do {
			V1 = 2 * randd() - 1;
			V2 = 2 * randd() - 1;
			S = V1 * V1 + V2 * V2;
			} while(S >= 1 || !(S < 0 || S > 0)); //S must be != 0
		X = V1 * sqrt(-2 * log(S) / S);
	} else {
		X = V2 * sqrt(-2 * log(S) / S);
    }
	phase = !phase;

	return X;
}

double absd(double a) {
    return a > -a ? a : -a;
}

float absf(float a) {
    return a > -a ? a : -a;
}

int compareFloatAsc(void const * a, void const * b) {
    return (*(float const*)a) > (*(float const*)b) ? 1 : (*(float const*)a) < (*(float const*)b) ? -1 : 0;
}

PriorityQueue init_priority_queue(void) {
    PriorityQueue newpq;
    CALLEXIT(!(newpq = malloc(sizeof *newpq)))
    newpq->head = 0;
    return newpq;
}

void free_priority_queue(PriorityQueue pq, int free_elem) {
    while(pq->head) {
        struct priority_node * nextHead = pq->head->next;
        if(free_elem) {
            free(pq->head->elem); //CAUTION!
        }
        free(pq->head);
        pq->head = nextHead;
    }
    free(pq);
}

void priority_insert(PriorityQueue pq, float priority, void * elem) {
    struct priority_node *newpn;
    CALLEXIT(!(newpn = malloc(sizeof *newpn)))
    if(!pq->head) {
        pq->head = newpn;
        newpn->next = 0;
    } else {
        struct priority_node *prevpn = pq->head;
        while(prevpn->next && prevpn->priority < priority) {
            prevpn = prevpn->next;
        }
        newpn->next = prevpn->next;
        prevpn->next = newpn;
    }
    newpn->elem = elem;
    newpn->priority = priority;
}

void* priority_remove_min(PriorityQueue pq) {
    if(!pq || !pq->head) {
        return 0;
    }

    void * ret = pq->head->elem;
    struct priority_node * next = pq->head->next;
    free(pq->head);
    pq->head = next;

    return ret;
}

int priority_empty(PriorityQueue pq) {
    return pq->head == 0;
}


void print_priorities(PriorityQueue pq) {
    if(!pq) {
        printf(INFO "[-]");
    }
    printf(INFO);
    struct priority_node * cur = pq->head;
    while(cur) {
        printf("[%.3f]->", cur->priority);
        cur = cur->next;
    }
    printf("[-]\n");
}

