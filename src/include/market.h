#ifndef MARKET_H_INCLUDED
#define MARKET_H_INCLUDED
#include <stdlib.h>
#include <limits.h>
#include "status.h"
#include "limits.h"
#include "network.h"

void fluctuate_market(void);

void print_market(void);

void print_materials(struct Message *);

void market_buysell(struct Message *, int, char *, long);
enum Materials {WOOD, LEATHER, ORE, STONE, BRONZE, MAIL, STEEL, DIAMOND}; //update MAT_COUNT in limits.h
#endif
