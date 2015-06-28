#ifndef PARSE_H_INCLUDED
#define PARSE_H_INCLUDED

int check_if_matches_regex(char *, const char *);
void handle_login(char *, char *, char *, char *);

char *nultrm   (char str[]);
char *to_lower (char str[]);
char *xor_flip (char str[]);

extern char regex_group[15][2048];

#endif
