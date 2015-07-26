#ifndef LINKEDLIST_H_INCLUDED
#define LINKEDLIST_H_INCLUDED

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include "colors.h"
#include "util.h"

struct token_node {
    struct token_node * next, * prev;
    char * elem; //'\0' denoted string of characters
    unsigned int num; //number of occurences of the character string
    int end; //you can end with this node or not
};

struct token_list {
    struct token_node * head;
    size_t len;
    size_t num_total;
};

struct token_iterator {
    struct token_node * cur_node;
    size_t consumed;
};

typedef struct token_list * TokenList;
typedef struct token_iterator * TokenIterator;

TokenList init_token_list(void);
void free_token_list(TokenList l);

void inc_token(TokenList l, char * c, int end);
size_t get_len(TokenList l);
size_t get_num_total(TokenList l);

TokenIterator get_iterator(TokenList l);
void free_iterator(TokenIterator it);

int has_elem(TokenIterator it);
void next(TokenIterator it);

char * get_str(TokenIterator it);
unsigned int get_num_str(TokenIterator it);
int get_is_end(TokenIterator it);
size_t get_num_consumed(TokenIterator it);

//Internal functions
void swap(struct token_node *, struct token_node *);

#endif // LINKEDLIST_H_INCLUDED
