#include <limits.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "include/colors.h"
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

inline double absd(double a) {
    return a > -a ? a : -a;
}

inline float absf(float a) {
    return a > -a ? a : -a;
}

inline double remapd(double x, double lo1, double hi1, double lo2, double hi2) {
    return (x-lo1)/(hi1-lo1)*(hi2-lo2)+lo2;
}

inline float remapf(float x, float lo1, float hi1, float lo2, float hi2) {
    return (x-lo1)/(hi1-lo1)*(hi2-lo2)+lo2;
}

int compare_float_asc(void const *a, void const *b) { //for qsort during map generation
    return (*(float const*)a) > (*(float const*)b) ? 1 : (*(float const*)a) < (*(float const*)b) ? -1 : 0;
}

static inline float fade(float t) { return t * t * t * (t * (t * 6 - 15) + 10); }
static inline float lerp(float t, float a, float b) { return a + t * (b - a); }
static inline float grad(int hash, float x, float y, float z) {
    int h = hash & 15;                      // CONVERT LO 4 BITS OF HASH CODE
    float u = h<8 ? x : y,                 // INTO 12 GRADIENT DIRECTIONS.
            v = h<4 ? y : h==12||h==14 ? x : z;
    return ((h&1) == 0 ? u : -u) + ((h&2) == 0 ? v : -v);
}

static int *p_noise_arr = 0;

void perlin_init() { //this is a very wrong way to do this but it works for our purposes
    int permutation[256];
    for(int i = 0; i < 256; i++) {
        permutation[i] = i;
    }
    for(int i = 0; i < 256; i++) {
        unsigned pos = (unsigned)(i + randd()*(256-i));
        int tmp = permutation[i];
        permutation[i] = permutation[pos];
        permutation[pos] = tmp;
    }
    if(p_noise_arr) {
        free(p_noise_arr);
    }
    CALLEXIT(!(p_noise_arr = malloc(512 * sizeof *p_noise_arr)))
    for (int i=0; i < 256 ; i++) {
        p_noise_arr[256+i] = p_noise_arr[i] = permutation[i];
    }
}

void perlin_cleanup() {
    if(p_noise_arr) {
        free(p_noise_arr);
    }
    p_noise_arr = NULL;
}

//reference perlin noise implementation
float noise(float x, float y, float z) {
    int X = (int) (floor(x)) & 255,                  // FIND UNIT CUBE THAT
            Y = (int) (floor(y)) & 255,                  // CONTAINS POINT.
            Z = (int) (floor(z)) & 255;
    x -= floor(x);                                // FIND RELATIVE X,Y,Z
    y -= floor(y);                                // OF POINT IN CUBE.
    z -= floor(z);
    float u = fade(x),                                // COMPUTE FADE CURVES
            v = fade(y),                                // FOR EACH OF X,Y,Z.
            w = fade(z);
    int A = p_noise_arr[X] + Y;
    int AA = p_noise_arr[A] + Z;
    int AB = p_noise_arr[A + 1] + Z; // HASH COORDINATES OF
    int B = p_noise_arr[X + 1] + Y;
    int BA = p_noise_arr[B] + Z;
    int BB = p_noise_arr[B + 1] + Z; // THE 8 CUBE CORNERS,

    return lerp(w, lerp(v, lerp(u, grad(p_noise_arr[AA], x, y, z),  // AND ADD
                                grad(p_noise_arr[BA], x - 1, y, z)), // BLENDED
                        lerp(u, grad(p_noise_arr[AB], x, y - 1, z),  // RESULTS
                             grad(p_noise_arr[BB], x - 1, y - 1, z))),// FROM  8
                lerp(v, lerp(u, grad(p_noise_arr[AA + 1], x, y, z - 1),  // CORNERS
                             grad(p_noise_arr[BA + 1], x - 1, y, z - 1)), // OF CUBE
                     lerp(u, grad(p_noise_arr[AB + 1], x, y - 1, z - 1),
                          grad(p_noise_arr[BB + 1], x - 1, y - 1, z - 1))));
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
