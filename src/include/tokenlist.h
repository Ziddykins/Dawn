#ifndef LINKEDLIST_H_INCLUDED
#define LINKEDLIST_H_INCLUDED

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include "colors.h"

struct tokenNode {
    struct tokenNode * next, * prev;
    char * elem; //'\0' denoted string of characters
    unsigned int num; //number of occurences of the character string
    int end; //you can end with this node or not
};

struct tokenList {
    struct tokenNode * head;
    size_t len;
    size_t totalNum;
};

struct tokenIterator {
    struct tokenNode * curNode;
    size_t consumed;
};

typedef void * TokenList;
typedef void * TokenIterator;

TokenList createTokenList(void);
void freeTokenList(TokenList l);

void incToken(TokenList l, char * c, int end);
size_t getLen(TokenList l);
size_t getTotalNum(TokenList l);

TokenIterator getIterator(TokenList l);
void freeIterator(TokenIterator it);

int hasElem(TokenIterator it);
void next(TokenIterator it);

char * getStr(TokenIterator it);
unsigned int getStrNum(TokenIterator it);
int getIsEnd(TokenIterator it);
size_t getConsumed(TokenIterator it);

//Internal functions
void swap(struct tokenNode *, struct tokenNode *);

#endif // LINKEDLIST_H_INCLUDED
