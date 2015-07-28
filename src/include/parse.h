#ifndef PARSE_H_INCLUDED
#define PARSE_H_INCLUDED

#include <stdio.h>
#include <pcre.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <openssl/sha.h>
#include "network.h"
#include "status.h"
#include "player.h"
#include "inventory.h"
#include "combat.h"
#include "items.h"
#include "market.h"

int check_if_matches_regex(char *, const char *);
void handle_login(char *, char *, char *, char *);

int command_allowed (struct Bot *b, char * command, int pindex);

//char *nultrm   (char str[]);
char *to_lower (char *str);
char *xor_flip (char *str); //!DEPRECATED

void gen_salt(char * salt, size_t len);
void hash_pwd(unsigned char * digest, char const * salt, char const * password);
int hash_cmp(unsigned char const * s1, unsigned char const * s2);

extern char regex_group[15][2048];

#endif
