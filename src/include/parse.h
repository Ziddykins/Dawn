#ifndef PARSE_H_INCLUDED
#define PARSE_H_INCLUDED

#include <stdio.h>
#include <pcre.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "network.h"
#include "status.h"
#include "player.h"
#include "inventory.h"
#include "combat.h"
#include "items.h"

int check_if_matches_regex(char *, const char *);
void handle_login(char *, char *, char *, char *);

int command_allowed (struct Bot *b, char * command, int pindex);

char *nultrm   (char str[]);
char *to_lower (char str[]);
char *xor_flip (char str[]); //!DEPRECATED

void genSalt(char * salt, size_t len);
uint64_t hashPwd(char const * username, char const * password);
uint64_t hash(char const *);

extern char regex_group[15][2048];

#endif
