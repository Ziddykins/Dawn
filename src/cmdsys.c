#include "include/cmdsys.h"

#include "include/player.h"
#include "include/util.h"
#include "include/status.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

CmdSys commands;

void init_global_cmd_sys() {
    commands = init_cmd_sys();
}

CmdSys init_cmd_sys() {
    struct cmd_sys * ccs;
    CALLEXIT(!(ccs = calloc(1, sizeof *ccs)))
    ccs->flags = CMD_INITIALIZED;
    return ccs;
}

void free_cmd_sys(CmdSys cs) {
    struct cmd_sys * ccs = cs ? cs : commands;
    assert(ccs);

    free(ccs->hashes);

    //free allocated character strings
    for(size_t i = 0; i < ccs->len; i++) {
        free(ccs->cmds[i]);
        free(ccs->helptexts[i]);
    }

    free(ccs->cmds);
    free(ccs->helptexts);
    free(ccs->auth_levels);
    free(ccs->fn);

    free(ccs);
}

void register_cmd(CmdSys cs, char * cmd, char * helptext, int auth_level, void (*fn)(int pindex, struct Message *msg)) {
    struct cmd_sys * ccs = cs ? cs : commands;
    assert(ccs && cmd && helptext && fn);
    ccs->flags = CMD_REGISTERED;

    if(ccs->capacity == 0) {
        CALLEXIT(!(ccs->hashes = calloc(1, sizeof *ccs->hashes)))
        CALLEXIT(!(ccs->cmds = calloc(1, sizeof *ccs->cmds)))
        CALLEXIT(!(ccs->helptexts = calloc(1, sizeof *ccs->helptexts)))
        CALLEXIT(!(ccs->auth_levels = calloc(1, sizeof *ccs->auth_levels)))
        CALLEXIT(!(ccs->fn = calloc(1, sizeof *ccs->fn)))
        ccs->capacity = 1;
    }

    if(ccs->len >= ccs->capacity) {
        ccs->capacity *= 2;
        trim_arrays(ccs);
    }

    ccs->hashes[ccs->len] = hash_code(cmd);

    size_t len = strlen(cmd);
    assert(len < MAX_MESSAGE_BUFFER);
    CALLEXIT(!(ccs->cmds[ccs->len] = calloc(len+1, 1)))
    strncpy(ccs->cmds[ccs->len], cmd, len);

    len = strlen(helptext);
    assert(len < MAX_MESSAGE_BUFFER);
    CALLEXIT(!(ccs->helptexts[ccs->len] = calloc(len+1, 1)))
    strncpy(ccs->helptexts[ccs->len], helptext, len);

    ccs->auth_levels[ccs->len] = auth_level;
    ccs->fn[ccs->len] = fn;

    ccs->len++;
}

void finalize_cmd_sys(CmdSys cs) {
    struct cmd_sys * ccs = cs ? cs : commands;
    assert(ccs);

    ccs->capacity = ccs->len;
    trim_arrays(ccs);

    sort_cmd_sys(ccs);
    ccs->flags = CMD_FINALIZED;
}

void invoke_cmd(CmdSys cs, int pindex, char * cmd, struct Message * msg, int mode) {
    struct cmd_sys * ccs = cs ? cs : commands;
    assert(ccs && msg && cmd);
    assert(ccs->flags == CMD_FINALIZED);

    char * out;
    CALLEXIT(!(out = malloc(MAX_MESSAGE_BUFFER)))
    if(pindex == -1 && strcmp(cmd, ";new") != 0) {
        sprintf(out, "PRIVMSG %s :Please create a new account by issuing ';new'\r\n", msg->receiver);
        add_msg(out, strlen(out));
    } else {
        size_t cmdID = get_cmd_id(ccs, cmd);
        if(cmdID == (size_t)(-1)) {
            if(mode == CMD_EXEC) {
                snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :Unknown command. Please use ';help'\r\n", msg->receiver);
            } else {
                snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :Command does not exist.\r\n", msg->receiver);
            }
            add_msg(out, strlen(out));
            //printf("%s", out);
        } else {
            if(mode == CMD_EXEC) {
                if(dawn->players[pindex].auth_level >= ccs->auth_levels[cmdID]) {
                    ccs->fn[cmdID](pindex, msg);
                } else {
                    snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :You are not authorized to issue this command!\r\n", msg->receiver);
                    add_msg(out, strlen(out));
                }
            } else if(mode == CMD_HELP) {
                snprintf(out, MAX_MESSAGE_BUFFER, "PRIVMSG %s :'%s': %s\r\n", msg->receiver, ccs->cmds[cmdID], ccs->helptexts[cmdID]);
                add_msg(out, strlen(out));
            }
        }
    }
    free(out);
}

//Internal functions
void sort_cmd_sys(CmdSys cs) {
    size_t const SHIFT = 4;
    uint64_t const BITMASK = (1<<SHIFT)-1;
    unsigned short const HASH_BITLEN = 64;

    struct cmd_sys * ccs = cs ? cs : commands;
    assert(ccs);
    assert(ccs->capacity == ccs->len);
    if(ccs->len <= 1) {
        return;
    }

    size_t * positions;
    CALLEXIT(!(positions = malloc(ccs->len * sizeof *positions)))
    for(size_t i = 0; i < ccs->len; i++) {
        positions[i] = i;
    }

    size_t * buckets;
    CALLEXIT(!(buckets = malloc(ccs->len * (BITMASK+1) * sizeof *buckets))) //actually a 2d array

    size_t * idx;
    CALLEXIT(!(idx = calloc(BITMASK+1, sizeof *idx)))

    size_t bitmask = BITMASK;
    for(size_t i = 0; i < HASH_BITLEN / SHIFT; i++) {

        for(size_t j = 0; j < ccs->len; j++) {
            size_t pos = positions[j];
            size_t bucket = (ccs->hashes[pos] & bitmask) >> (i*SHIFT);

            buckets[ccs->len * bucket + idx[bucket]] = pos;
            idx[bucket]++;
        }

        size_t c = 0;
        for(size_t j = 0; j <= BITMASK; j++) {
            for(size_t k = 0; k < idx[j]; k++) {
                positions[c] = buckets[ccs->len * j + k];
                c++;
            }
            idx[j] = 0;
        }
        bitmask <<= SHIFT;
    }

    free(buckets);
    free(idx);

    uint64_t * hashes;
    CALLEXIT(!(hashes = calloc(ccs->len, sizeof *ccs->hashes)))
    char ** cmds;
    CALLEXIT(!(cmds = calloc(ccs->len, sizeof *ccs->cmds)))
    char ** helptexts;
    CALLEXIT(!(helptexts = calloc(ccs->len, sizeof *ccs->helptexts)))
    int * auth_levels;
    CALLEXIT(!(auth_levels = calloc(ccs->len, sizeof *ccs->auth_levels)))
    cmd_func fn;
    CALLEXIT(!(fn = calloc(ccs->len, sizeof *ccs->fn)))

    for(size_t i = 0; i < ccs->len; i++) {
        hashes[i] = ccs->hashes[i];
        cmds[i] = ccs->cmds[i];
        helptexts[i] = ccs->helptexts[i];
        auth_levels[i] = ccs->auth_levels[i];
        fn[i] = ccs->fn[i];
    }

    for(size_t i = 0; i < ccs->len; i++) {
        ccs->hashes[i] = hashes[positions[i]];
        ccs->cmds[i] = cmds[positions[i]];
        ccs->helptexts[i] = helptexts[positions[i]];
        ccs->auth_levels[i] = auth_levels[positions[i]];
        ccs->fn[i] = fn[positions[i]];
    }

    free(hashes);
    free(cmds);
    free(helptexts);
    free(auth_levels);
    free(fn);
    free(positions);
}

size_t get_cmd_id(CmdSys cs, char * cmd) {
    struct cmd_sys * ccs = cs;
    assert(ccs);
    uint64_t hash = hash_code(cmd);
    size_t min = 0, max = ccs->len-1, mid = (size_t)(-1);
    int valid = 0;

    while(max >= min) {
        mid = (min+max)/2;
        if(ccs->hashes[mid] == hash) {
            valid = 1; break;
        } else if(ccs->hashes[mid] > hash) {
            max = mid-1;
        } else {
            min = mid+1;
        }
    }

    if(valid) {
        for(size_t iter = mid; iter < ccs->len && ccs->hashes[iter] == hash; iter++) {
            if(strcmp(cmd, ccs->cmds[iter]) == 0) {
                return iter;
            }
        }
        for(size_t iter = mid; iter < ccs->len && ccs->hashes[iter] == hash; iter--) {
            if(strcmp(cmd, ccs->cmds[iter]) == 0) {
                return iter;
            }
        }
    }
    return (size_t)-1;
}

void trim_arrays(CmdSys cs) {
    struct cmd_sys * ccs = cs;
    assert(ccs);
    assert(ccs->len != 0);
    ccs->hashes = realloc(ccs->hashes, ccs->capacity * sizeof *ccs->hashes);
    ccs->cmds = realloc(ccs->cmds, ccs->capacity * sizeof *ccs->cmds);
    ccs->helptexts = realloc(ccs->helptexts, ccs->capacity * sizeof *ccs->helptexts);
    ccs->auth_levels = realloc(ccs->auth_levels, ccs->capacity * sizeof *ccs->auth_levels);
    ccs->fn = realloc(ccs->fn, ccs->capacity * sizeof *ccs->fn);
}

uint64_t hash_code(char * str) {
    uint64_t hash = 1;
    uint64_t const prime = 13835058055282163729ull;
    size_t len = strlen(str);
    for(size_t i = 0; i < len; i++) {
        hash += hash * 31 + (unsigned char)(str[i]);
        hash %= prime;
    }
    return hash;
}
