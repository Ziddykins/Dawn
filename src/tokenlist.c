#include "include/tokenlist.h"

TokenList init_token_list() {
    struct token_list * new_list;
    CALLEXIT(!(new_list = malloc(sizeof *new_list)))
    new_list->head = 0;
    new_list->len = 0;
    new_list->num_total = 0;
    return new_list;
}

void free_token_list(TokenList l) {
    if(!l) {
        return;
    }
    struct token_list * cl = l;
    struct token_node * tmp = cl->head;

    while(tmp) {
        struct token_node * next = tmp->next;
        free(tmp->elem);
        free(tmp);
        tmp = next;
    }
    free(cl);
}

void inc_token(TokenList l, char * c, int end) {
    if(!l) {
        return;
    }
    struct token_list * cl = l;
    struct token_node * tmp = cl->head;
    if(!tmp) { //list is empty
        CALLEXIT(!(cl->head = calloc(1, sizeof *cl->head)))
        cl->head->next = cl->head->prev = 0;
        CALLEXIT(!(cl->head->elem = malloc(strlen(c)+1)))
        strcpy(cl->head->elem, c);
        cl->head->num = 1; //c has one occurrence
        cl->head->end = end;
        cl->len = 1; //and the list is now of length 1
    } else { //list is not empty
        //search for the item
        struct token_node * prev = 0;
        while(tmp && strcmp(tmp->elem, c) != 0) {
            prev = tmp;
            tmp = tmp->next;
        }
        if(!tmp) { //c was not found, we've reached the end of the list
            //insert new element at the end of the list
            CALLEXIT(!(prev->next = calloc(1, sizeof *prev)))

            tmp = prev->next;
            CALLEXIT(!(tmp->elem = malloc(strlen(c)+1)))
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
    cl->num_total++;
}

size_t get_len(TokenList l) {
    if(!l)
        return 0;
    struct token_list * cl = l;
    return cl->len;
}

size_t get_num_total(TokenList l) {
    if(!l)
        return 0;
    struct token_list * cl = l;
    return cl->num_total;
}

TokenIterator get_iterator(TokenList l) {
    if(!l)
        return 0;
    struct token_list * cl = l;
    struct token_iterator * it;
    CALLEXIT(!(it = malloc(sizeof *it)))
    it->cur_node = cl->head;
    it->consumed = cl->head ? cl->head->num : 0;
    return it;
}

void free_iterator(TokenIterator it) {
    free(it);
}

int has_elem(TokenIterator it) {
    if(!it)
        return 0;
    struct token_iterator * cit = it;
    return cit->cur_node ? 1 : 0;
}

void next(TokenIterator it) {
    struct token_iterator * cit = it;
    cit->cur_node = cit->cur_node->next;
    cit->consumed += cit->cur_node ? cit->cur_node->num : 0;
}

char * get_str(TokenIterator it) {
    return ((struct token_iterator *)it)->cur_node->elem;
}

unsigned int get_num_str(TokenIterator it) {
    return ((struct token_iterator *)it)->cur_node->num;
}

int get_is_end(TokenIterator it) {
    return ((struct token_iterator *)it)->cur_node->end;
}

size_t get_num_consumed(TokenIterator it) {
    return ((struct token_iterator *)it)->consumed;
}

//swaps the /data/ of two nodes
void swap(struct token_node * a, struct token_node * b) {
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
