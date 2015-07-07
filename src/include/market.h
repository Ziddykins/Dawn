#ifndef MARKET_H_INCLUDED
#define MARKET_H_INCLUDED
#include <stdlib.h>
#include <limits.h>
#include "status.h"
#include "limits.h"
#include "network.h"
void fluctuate_market (struct Bot *);
void print_market (struct Bot *);
void print_materials (struct Bot *, struct Message *);
void market_buysell (struct Bot *, struct Message *, int, char *, long);
enum Materials {WOOD, LEATHER, ORE, STONE, BRONZE, MAIL, STEEL, DIAMOND};
#endif
