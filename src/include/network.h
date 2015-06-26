#ifndef NETWORK_H_INCLUDED
#define NETWORK_H_INCLUDED
#include "limits.h"
#include "status.h"

struct Message {
    char sender_nick[64], sender_ident[64], sender_hostmask[64];
    char receiver[64], message[2048];
};

extern char buffer[MAX_RECV_BUFFER + 1];
extern int con_socket;

int init_connect_server (const char *, const char *);
void close_socket (int);
void send_socket (char *);
void send_login (char *, char *, char *, char *);
void parse_room_message (struct Message *, struct Bot *);
void parse_private_message (struct Message *);

#endif