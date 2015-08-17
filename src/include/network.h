#ifndef NETWORK_H_INCLUDED
#define NETWORK_H_INCLUDED

/*
 * The network header file is responsible for managing the IRC connection and
 * handling data that is received from or sent to the IRC server.
 * It's main task is to avoid getting kicked from the server due to spam.
 */

#include "limits.h"

#include <stddef.h>
#include <time.h>

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

struct msg_hist_node {
    size_t len;
    time_t date;
    struct msg_hist_node * next;
};

struct msg_hist_list {
    struct msg_hist_node * head, * tail;
    size_t byte_size;
    size_t msgs;
};

typedef struct msg_hist_list * MsgHistoryList;

/*
 * The Message List is responsible for keeping track of all messages that are to be
 * send but couldn't because the Message History List did not allow for any further
 * storage.
 * Upon removal of an element from the Message History List it is checked whether
 * there now is enough space to send the next message waiting in Queue.
 */

MsgHistoryList init_msg_hist_list(void);
void free_msg_hist_list(void);
void add_hist_msg(size_t len);

struct msg_node {
    char * msg; //the message
    size_t len; //the length of the message without '\0'
    struct msg_node * next;
};

struct msg_list {
    struct msg_node * head, * tail; //start and end
    size_t byte_size; //total size of the queue
    size_t msgs; //number of messages waiting to be sent
};

typedef struct msg_list * MsgList;

MsgList init_msg_list(void); //allocates a new message history list
void free_msg_list(void); //frees all storage allocated
void add_msg(char * msg, size_t len); //deep copies msg

void pop_hist_msg(void); //updates the message history
size_t peek_msg_size(void); //returns size of mlist's head

void process_messages(void); //will move messages from src to dest while allowed

extern char buffer[MAX_RECV_BUFFER + 1];
extern int con_socket;

int init_connect_server (const char *, const char *);
int hostname_to_ip (char *, char *); //Returns an IP from a hostname
void close_socket (int);
void send_socket (char *,size_t);
void send_login (char *, char *, char *, char *);
#endif
