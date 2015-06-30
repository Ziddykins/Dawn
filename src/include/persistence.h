#ifndef PERSISTENCE_H_INCLUDED
#define PERSISTENCE_H_INCLUDED

#include "status.h"
#include "player.h"

void persistent_save(struct Bot *);
void persistent_load(struct Bot *);

#endif // PERSISTENCE_H_INCLUDED
