#ifndef COMMANDS_H_INCLUDED
#define COMMANDS_H_INCLUDED

#include <stdlib.h>
#include <stddef.h>
#include <assert.h>
#include "parse.h"
#include "limits.h"
#include "network.h"
#include "cmdsys.h"

#define CMD_LIT ";\\w+ "

void cmd_help(int pindex, struct Message * msg);
void cmd_sheet(int pindex, struct Message * msg);
void cmd_equip(int pindex, struct Message * msg);
void cmd_unequip(int pindex, struct Message * msg);
void cmd_gmelee(int pindex, struct Message * msg);
void cmd_hunt(int pindex, struct Message * msg);
void cmd_revive(int pindex, struct Message * msg);
void cmd_drop(int pindex, struct Message * msg);
void cmd_info(int pindex, struct Message * msg);

#endif // COMMANDS_H_INCLUDED
