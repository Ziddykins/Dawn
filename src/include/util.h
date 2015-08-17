#ifndef UTIL_H_INCLUDED
#define UTIL_H_INCLUDED

#include "colors.h"
#include <stdio.h>
#include <stddef.h>

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
double remapd(double x, double lo1, double hi1, double lo2, double hi2);
float remapf(float x, float lo1, float hi1, float lo2, float hi2);
int compare_float_asc(void const *a, void const *b);

float noise(float x, float y, float z);
void perlin_init(void);
void perlin_cleanup(void);

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
