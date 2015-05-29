#ifndef PARSE_H_INCLUDED
#define PARSE_H_INCLUDED

int check_if_matches_regex(char *, char *);
void handle_login(char *, char *, char *, char *);

extern char regex_group[15][2048];

#endif
