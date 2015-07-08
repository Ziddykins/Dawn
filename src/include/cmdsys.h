#ifndef CMDSYS_H_INCLUDED
#define CMDSYS_H_INCLUDED

#include <inttypes.h> //uint64_t
#include <stddef.h> //size_t
#include <assert.h>
#include "network.h"
#include "player.h"

struct cmdSys {
    size_t len, capacity;
    uint64_t * hashes;
    char ** cmds;
    char ** helptexts;
    int * auth_levels;
    void (*(*fn))(int pindex, struct Message *msg); //array of function pointers
    size_t flags; //alignment + safety checks
};

//void sample_function(struct Message *msg);

enum cmdSysFlags {
    CMD_UNUSED = 0,
    CMD_INITIALIZED = 1<<0,
    CMD_REGISTERED = 1<<1,
    CMD_FINALIZED = 1<<2
};

enum cmdMode {
    CMD_EXEC,
    CMD_HELP
};

typedef void * CmdSys;

extern CmdSys commands;

void init_cmdsys(void);

CmdSys createCmdSys(void);
void freeCmdSys(CmdSys);

void registerCmd(CmdSys, char * cmd, char * helptext, int auth_level, void (*fn)(int pindex, struct Message *msg));

void finalizeCmdSys(CmdSys);

void invokeCmd(CmdSys, int pindex, char * cmd, struct Message * msg, int mode);

//Internal functions
void sortCmds(CmdSys);
uint64_t hashCode(char *);
void trimArrays(CmdSys);
size_t getCmdID(CmdSys cs, char * cmd);


#endif // CMDSYS_H_INCLUDED
