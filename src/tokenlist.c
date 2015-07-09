#include "include/tokenlist.h"

TokenList createTokenList() {
    struct tokenList * newList;
    CALLEXIT(newList = malloc(sizeof *newList))
    newList->head = 0;
    newList->len = 0;
    newList->totalNum = 0;
    return newList;
}

void freeTokenList(TokenList l) {
    if(!l) {
        return;
    }
    struct tokenList * cl = l;
    struct tokenNode * tmp = cl->head;

    while(tmp) {
        struct tokenNode * next = tmp->next;
        free(tmp->elem);
        free(tmp);
        tmp = next;
    }
    free(cl);
}

void incToken(TokenList l, char * c, int end) {
    if(!l) {
        return;
    }
    struct tokenList * cl = l;
    struct tokenNode * tmp = cl->head;
    if(!tmp) { //list is empty
        CALLEXIT(cl->head = calloc(1, sizeof *cl->head))
        cl->head->next = cl->head->prev = 0;
        CALLEXIT(cl->head->elem = malloc(strlen(c)+1))
        strcpy(cl->head->elem, c);
        cl->head->num = 1; //c has one occurrence
        cl->head->end = end;
        cl->len = 1; //and the list is now of length 1
    } else { //list is not empty
        //search for the item
        struct tokenNode * prev = 0;
        while(tmp && strcmp(tmp->elem, c) != 0) {
            prev = tmp;
            tmp = tmp->next;
        }
        if(!tmp) { //c was not found, we've reached the end of the list
            //insert new element at the end of the list
            CALLEXIT(prev->next = calloc(1, sizeof *prev));

            tmp = prev->next;
            CALLEXIT(tmp->elem = malloc(strlen(c)+1))
            strcpy(tmp->elem, c);
            tmp->next = 0;
            tmp->prev = prev;
            tmp->num = 1;
            tmp->end = end,
            cl->len++; //list is now 1 entry longer
        } else {
            //c has been found, increment its value
            tmp->num++;
            tmp->end |= end;
            //swap with predecessors till sorted
            while(tmp->prev && tmp->prev->num < tmp->num) {
                swap(tmp->prev, tmp);
            }
        }
    }
    cl->totalNum++;
}

size_t getLen(TokenList l) {
    if(!l)
        return 0;
    struct tokenList * cl = l;
    return cl->len;
}

size_t getTotalNum(TokenList l) {
    if(!l)
        return 0;
    struct tokenList * cl = l;
    return cl->totalNum;
}

TokenIterator getIterator(TokenList l) {
    if(!l)
        return 0;
    struct tokenList * cl = l;
    struct tokenIterator * it;
    CALLEXIT(it = malloc(sizeof *it))
    it->curNode = cl->head;
    it->consumed = cl->head ? cl->head->num : 0;
    return it;
}

void freeIterator(TokenIterator it) {
    free(it);
}

int hasElem(TokenIterator it) {
    if(!it)
        return 0;
    struct tokenIterator * cit = it;
    return cit->curNode ? 1 : 0;
}

void next(TokenIterator it) {
    struct tokenIterator * cit = it;
    cit->curNode = cit->curNode->next;
    cit->consumed += cit->curNode ? cit->curNode->num : 0;
}

char * getStr(TokenIterator it) {
    return ((struct tokenIterator *)it)->curNode->elem;
}

unsigned int getStrNum(TokenIterator it) {
    return ((struct tokenIterator *)it)->curNode->num;
}

int getIsEnd(TokenIterator it) {
    return ((struct tokenIterator *)it)->curNode->end;
}

size_t getConsumed(TokenIterator it) {
    return ((struct tokenIterator *)it)->consumed;
}

//swaps the /data/ of two nodes
void swap(struct tokenNode * a, struct tokenNode * b) {
    assert(a && b);

    char * elemA = a->elem;
    unsigned int numA = a->num;
    int endA = a->end;

    a->elem = b->elem;
    a->num = b->num;
    a->end = b->end;

    b->elem = elemA;
    b->num = numA;
    b->end = endA;
}
