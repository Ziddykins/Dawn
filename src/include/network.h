#ifndef NETWORK_H_INCLUDED
#define NETWORK_H_INCLUDED
#include "limits.h"
#include "status.h"

struct Message {
    char sender_nick[64], sender_ident[64], sender_hostmask[64];
    char receiver[64], message[2048];
};

struct msgHistoryNode {
    size_t len;
    time_t date;
    struct msgHistoryNode * next;
};

struct msgHistoryList {
    struct msgHistoryNode * head, * tail;
    size_t byteSize;
    size_t msgs;
};

typedef void * MsgHistoryList;

MsgHistoryList createMsgHistoryList(void);
void addMsgHistory(size_t len);

struct msgNode {
    char * msg; //the message
    size_t len; //the length of the message without \0
    struct msgNode * next;
};

struct msgList {
    struct msgNode * head, * tail; //start and end
    size_t byteSize; //total size of the queue
    size_t msgs; //number of messages waiting to be sent
};

typedef void * MsgList;

MsgList createMsgList(void);
void addMsg(char * msg, size_t len); //deep copies msg

char * retrMsg(void); //returns pointer to msg, destroys node

void popMsgHist(void); //updates the message history
size_t peekMsgSize(void); //returns size of mlist's head


void processMessages(void); //will move messages from src to dest while allowed

void init_send_queue(void);

extern char buffer[MAX_RECV_BUFFER + 1];
extern int con_socket;

int init_connect_server (const char *, const char *);
void close_socket (int);
void send_socket (char *);
void send_login (char *, char *, char *, char *);
void parse_room_message (struct Bot *, struct Message *);
void parse_private_message (struct Bot *, struct Message *);

#endif
