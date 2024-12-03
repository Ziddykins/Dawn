#ifndef PARSE_H_INCLUDED
#define PARSE_H_INCLUDED

#include <stddef.h>
#include "network.h"

int matches_regex(char *, const char *);
void handle_login(char *, char *, char *, char *);

void parse_private_message(struct Message *message);

void parse_room_message(struct Message *message);

int command_allowed(char *command, int pindex);

//char *nultrm   (char str[]);
char *to_lower (char *str);
char *xor_flip (char *str); //!DEPRECATED

void gen_salt(char * salt, size_t len);
void hash_pwd(unsigned char * digest, char const * salt, char const * password);
int hash_cmp(unsigned char const * s1, unsigned char const * s2);

extern char regex_group[15][MAX_MESSAGE_BUFFER];

#endif
