#ifndef NETWORK_H_INCLUDED
#define NETWORK_H_INCLUDED
#include "limits.h"
#include "status.h"

struct Message {
    char sender_nick[64], sender_ident[64], sender_hostmask[64];
    char receiver[64], message[2048];
};

struct msgNode {
    char * msg; //the message
    size_t len; //the length of the message without \0
    time_t date; //when the message was enqueued
    struct msgNode * next;
};

struct msgList {
    struct msgNode * head, * tail; //start and end
    size_t byteSize; //total size of the queue
    size_t msgs; //number of messages waiting to be sent
};

typedef void * msgList;

msgList createMsgList(void);
void addMsg(msgList mlist, char * msg, size_t len);

extern char buffer[MAX_RECV_BUFFER + 1];
extern int con_socket;

int init_connect_server (const char *, const char *);
void close_socket (int);
void send_socket (char *);
void send_login (char *, char *, char *, char *);
void parse_room_message (struct Bot *, struct Message *);
void parse_private_message (struct Bot *, struct Message *);

#endif
