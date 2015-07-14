#ifndef COMMANDS_H_INCLUDED
#define COMMANDS_H_INCLUDED

#include <stdlib.h>
#include <stddef.h>
#include <assert.h>
#include "parse.h"
#include "limits.h"
#include "network.h"
#include "cmdsys.h"
#include "util.h"

#define PREFIX ";"
#define PREFIX_C ';'
#define CMD_MATCH "[\\w[]+"
#define CMD_LIT "^" PREFIX CMD_MATCH
#define CMD_LIT_MID PREFIX CMD_MATCH

void init_cmds(void);
void free_cmds(void);

void cmd_help(int pindex, struct Message * msg);
void cmd_new(int pindex, struct Message * msg);
void cmd_auth(int pindex, struct Message * msg);
void cmd_stop(int pindex, struct Message * msg);
void cmd_sheet(int pindex, struct Message * msg);
void cmd_equip(int pindex, struct Message * msg);
void cmd_unequip(int pindex, struct Message * msg);
void cmd_gmelee(int pindex, struct Message * msg);
void cmd_hunt(int pindex, struct Message * msg);
void cmd_revive(int pindex, struct Message * msg);
void cmd_drop(int pindex, struct Message * msg);
void cmd_info(int pindex, struct Message * msg);
void cmd_givexp(int pindex, struct Message * msg);
void cmd_make(int pindex, struct Message * msg);
void cmd_location(int pindex, struct Message * msg);
void cmd_slay(int pindex, struct Message * msg);
void cmd_gslay(int pindex, struct Message * msg);
void cmd_check(int pindex, struct Message * msg);
void cmd_gcheck(int pindex, struct Message * msg);
void cmd_assign(int pindex, struct Message * msg);
void cmd_ap(int pindex, struct Message * msg);
void cmd_travel(int pindex, struct Message * msg);
void cmd_locate(int pindex, struct Message * msg);
void cmd_materials(int pindex, struct Message * msg);
void cmd_market(int pindex, struct Message * msg);
void cmd_save(int pindex, struct Message * msg);
void cmd_cry(int pindex, struct Message * msg);
void cmd_gib(int pindex, struct Message * msg);
void cmd_inv(int pindex, struct Message * msg);
void cmd_ghunt(int pindex, struct Message * msg);
void cmd_melee(int pindex, struct Message * msg);
void cmd_fluctuate(int pindex, struct Message * msg);
#endif // COMMANDS_H_INCLUDED
