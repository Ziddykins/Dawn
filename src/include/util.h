#ifndef UTIL_H_INCLUDED
#define UTIL_H_INCLUDED

#include "colors.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define STR(x) #x
#define XSTR(x) STR(x)
#define AT __FILE__ ":" XSTR(__LINE__)

#define PRINTERR(str) \
    perror(ERR AT ": " str);
#define PRINTWARN(str) \
    perror(WARN AT ": " str);

#define CALLEXIT(func) \
    if((func)) { \
        PRINTERR(#func) \
        exit(1); \
    }

#define CALLRSME(func) \
    if(func) { \
        PRINTERR(#func) \
    }

double randd(void);
double gaussrand(void);
double absd(double);
float absf(float);
int compareFloatAsc(void const * a, void const * b);

struct priority_node;

struct priority_tree {
    void *elem;
    struct priority_node * children, * last_child;
    int degree;
    float priority;
};

struct priority_node {
    struct priority_tree * tree;
    struct priority_node * next;
};

struct priority_queue {
    struct priority_node * head;
    struct priority_node * min, * min_prev;
    size_t size;
};

typedef struct priority_queue * PriorityQueue;

PriorityQueue init_priority_queue(void);
void free_priority_queue(PriorityQueue, int free_elem);

void priority_insert(PriorityQueue, float priority, void * elem);
void* priority_remove_min(PriorityQueue);
int priority_empty(PriorityQueue);

#endif // UTIL_H_INCLUDED
