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
    struct priority_queue * rop;
    CALLEXIT(!(rop = malloc(sizeof *rop)))
    rop->head = NULL;
    rop->min = NULL;
    rop->min_prev = NULL;
    rop->size = 0;
    return rop;
}

static void free_tree(struct priority_tree * pt, int free_elem) {
    while(pt->children) {
        struct priority_node * next = pt->children->next;
        free_tree(pt->children->tree, free_elem);
        free(pt->children);
        pt->children = next;
    }
    if(free_elem) {
        free(pt->elem);
    }
    free(pt);
}

void free_priority_queue(PriorityQueue pq, int free_elem) {
    while(pq->head) {
        struct priority_node * next = pq->head->next;
        free_tree(pq->head->tree, free_elem);
        free(pq->head);
        pq->head = next;
    }
    free(pq);
}

static inline void priority_update_min(PriorityQueue pq) {
    if(!pq->min || pq->min->tree->priority > pq->head->tree->priority) {
        pq->min_prev = NULL;
        pq->min = pq->head;
    } else if(!pq->min_prev && pq->min) {
        pq->min_prev = pq->head;
    }
}

static void priority_insert_tree(PriorityQueue pq, struct priority_tree * pn) {
    struct priority_node * new_node;
    CALLEXIT(!(new_node = malloc(sizeof *new_node)))
    new_node->next = pq->head;
    new_node->tree = pn;
    pq->head = new_node;
    priority_update_min(pq);
    pq->size++;
}

void priority_insert(PriorityQueue pq, float priority, void * elem) {
    struct priority_tree * new_tree;
    CALLEXIT(!(new_tree = malloc(sizeof *new_tree)))
    new_tree->children = NULL;
    new_tree->last_child = NULL;
    new_tree->degree = 0;
    new_tree->elem = elem;
    new_tree->priority = priority;

    priority_insert_tree(pq, new_tree);
}

static inline int ceillog2ul(unsigned long n) {
#ifndef __has_builtin
#define __has_builtin(x) 0
#endif
#if __GNUC__ >= 4 \
 || __has_builtin(__builtin_clzl)
    return (int)(sizeof(n) * CHAR_BIT) - __builtin_clzl(n); //not exactly the same as below but good enough
#else
    return (int)(ceil(log2(n)));
#endif
}

static void priority_add_child(struct priority_tree * parent, struct priority_node * new_child) {
    new_child->next = parent->children;
    parent->children = new_child;
    parent->degree++;
    if(!parent->last_child) {
        parent->last_child = parent->children;
    }
}

void * priority_remove_min(PriorityQueue pq) {
    //save element to return for later use
    void *elem = pq->min->tree->elem;

    //remove minimum
    if (pq->min_prev) {
        pq->min_prev->next = pq->min->next;
    } else {
        pq->head = pq->min->next;
    }
    //split min tree into subtrees and insert them
    if(pq->min->tree->children) {
        pq->min->tree->last_child->next = pq->head;
        pq->head = pq->min->tree->children;
    }
    free(pq->min->tree);
    free(pq->min);

    struct priority_node **trees; //needed for cleanup algorithm
    size_t const len = (size_t)(2*ceillog2ul(pq->size)+1);
    CALLEXIT(!(trees = calloc(len, sizeof trees)));

    struct priority_node * current_node = pq->head;

    while(current_node) {
        struct priority_node * process_node = current_node;
        current_node = current_node->next;
        while(1) {
            int degree = process_node->tree->degree;
            if(trees[degree] == NULL) {
                trees[degree] = process_node;
                break;
            } else {
                if(trees[degree]->tree->priority > process_node->tree->priority) {
                    priority_add_child(process_node->tree, trees[degree]);
                } else {
                    priority_add_child(trees[degree]->tree, process_node);
                    process_node = trees[degree];
                }
                trees[degree] = NULL;
            }
        }
    }

    pq->size--;
    pq->head = NULL;
    pq->min = NULL;
    pq->min_prev = NULL;
    if(pq->size) {
        size_t i;
        for(i = 0; !trees[i]; i++);
        pq->head = trees[i];
        pq->head->next = NULL;
        pq->min = pq->head;

        for(i++; i < len; i++) {
            if(trees[i]) {
                trees[i]->next = pq->head;
                pq->head = trees[i];
                priority_update_min(pq);
            }
        }
    }

    free(trees);
    return elem;
}

int priority_empty(PriorityQueue pq) {
    return pq->head == NULL;
}
