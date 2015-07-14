#ifndef CMDSYS_H_INCLUDED
#define CMDSYS_H_INCLUDED

#include <inttypes.h> //uint64_t
#include <stddef.h> //size_t
#include <assert.h>
#include "network.h"
#include "player.h"

struct cmdSys {
    size_t len, capacity; //info for array list
    uint64_t * hashes;
    char ** cmds; //command strings e.g. ";help"
    char ** helptexts;
    int * auth_levels;
    void (*(*fn))(int pindex, struct Message *msg); //array of function pointers
    size_t flags; //alignment + safety checks
};

//void sample_function(struct Message *msg);

enum cmdSysFlags {
    CMD_UNUSED = 0,
    CMD_INITIALIZED = 1<<0,
    CMD_REGISTERED = 1<<1, //required for finalizing
    CMD_FINALIZED = 1<<2 //required for use
};

enum cmdMode {
    CMD_EXEC, //execute a command
    CMD_HELP //send the helptext publicly
};

typedef void * CmdSys;

extern CmdSys commands;

/**
 * @brief initializes the default command system (0)
 * reference to the default command system is stored in the commands variable
 * however you can also omit a command system with any of the command functions
 * to simply select the default one
 */
void init_cmdsys(void);

/**
 * @brief returns a new command system
 * @return new command system
 * @see init_cmdsys
 */
CmdSys createCmdSys(void);
/**
 * @brief deallocate all storage of a command system
 * @param cs command system or 0 for default
 */
void freeCmdSys(CmdSys cs);
/**
 * @brief register a function as a command with a command system
 * @param cs command system or 0 for default
 * @param cmd command string such as ";help"
 * @param helptext command help such as "Prints help"
 * @param auth_level see player.h for authentication levels
 * @param fn the function to be called for this command
 */
void registerCmd(CmdSys cs, char * cmd, char * helptext, int auth_level, void (*fn)(int pindex, struct Message *msg));

/**
 * @brief call this when done adding commands
 * this will sort the commands internally to allow for faster searching when invoking
 * @param cs command system or 0 for default
 */
void finalizeCmdSys(CmdSys cs);

/**
 * @brief call the function associated to a string or send its helptext
 * @param cs command system or 0 for default
 * @param pindex player who is invoking this command
 * @param cmd the command this player is invoking
 * @param msg the full message that was received
 * @param mode CMD_EXEC or CMD_HELP
 */
void invokeCmd(CmdSys cs, int pindex, char * cmd, struct Message * msg, int mode);

//Internal functions
void sortCmds(CmdSys);
uint64_t hashCode(char *);
void trimArrays(CmdSys);
size_t getCmdID(CmdSys cs, char * cmd);

#endif // CMDSYS_H_INCLUDED
