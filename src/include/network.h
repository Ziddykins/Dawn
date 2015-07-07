#ifndef NETWORK_H_INCLUDED
#define NETWORK_H_INCLUDED

/*
 * The network header file is responsible for managing the IRC connection and
 * handling data that is received from or sent to the IRC server.
 * It's main task is to avoid getting kicked from the server due to spam.
 */

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include "limits.h"
#include "status.h"

struct Message {
    char sender_nick[64], sender_ident[64], sender_hostmask[64];
    char receiver[64], message[2048];
};

//Initializes both the Message List and Message History List
void init_send_queue(void);

/*
 * A Message History List keeps track of messages that have been sent in the past
 * SENDQ_INTERVAL seconds. Whenever a sent message becomes older than that interval
 * it'll be purged from the list thus making space for new sent messages. which the
 * Message List can add to the Message History List.
 */

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

/*
 * The Message List is responsible for keeping track of all messages that are to be
 * send but couldn't because the Message History List did not allow for any further
 * storage.
 * Upon removal of an element from the Message History List it is checked whether
 * there now is enough space to send the next message waiting in Queue.
 */

MsgHistoryList createMsgHistoryList(void);2
void deleteMsgHistoryList(void);
void addMsgHistory(size_t len);

struct msgNode {
    char * msg; //the message
    size_t len; //the length of the message without '\0'
    struct msgNode * next;
};

struct msgList {
    struct msgNode * head, * tail; //start and end
    size_t byteSize; //total size of the queue
    size_t msgs; //number of messages waiting to be sent
};

typedef void * MsgList;

MsgList createMsgList(void); //allocates a new message history list
void deleteMsgList(void); //frees all storage allocated
void addMsg(char * msg, size_t len); //deep copies msg

char * retrMsg(void); //returns pointer to msg, destroys node

void popMsgHist(void); //updates the message history
size_t peekMsgSize(void); //returns size of mlist's head

void processMessages(void); //will move messages from src to dest while allowed

extern char buffer[MAX_RECV_BUFFER + 1];
extern int con_socket;

int init_connect_server (const char *, const char *);
void close_socket (int);
void send_socket (char *);
void send_login (char *, char *, char *, char *);
void parse_room_message (struct Bot *, struct Message *);
void parse_private_message (struct Bot *, struct Message *);

#endif
