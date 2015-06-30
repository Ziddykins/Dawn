#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include "include/network.h"
#include "include/limits.h"

char buffer[MAX_RECV_BUFFER + 1];
int con_socket;

static MsgList mlist;
static MsgHistoryList mhlist;
static int msgQ_singleton = 0;
static int msgHQ_singleton = 0;

void init_send_queue() {
    mlist = createMsgList();
    mhlist = createMsgHistoryList();
}

int init_connect_server (const char *ip_addr, const char *port) {
    struct addrinfo destination, *res;
    memset(&destination, 0, sizeof destination);
    destination.ai_family = AF_INET;
    destination.ai_socktype = SOCK_STREAM;
    getaddrinfo(ip_addr, port, &destination, &res);
    con_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    printf("trying to connect\n");
    connect(con_socket, res->ai_addr, res->ai_addrlen);
    printf("gud\n");
    freeaddrinfo(res);
    return errno;
}

void close_socket (int socket) {
    close(socket);
}

void send_socket (char * out_buf) {
    size_t len = strlen(out_buf);
    if(len > MAX_MESSAGE_BUFFER) {
        printf("Message too long: %lu", len);
        return;
    }
    write(con_socket, out_buf, len);
}

MsgHistoryList createMsgHistoryList() {
    if(msgHQ_singleton)
        return 0;
    struct msgList * newList;
    if(!(newList = calloc(1, sizeof *newList))) {
        perror("calloc");
        exit(1);
    }
    msgHQ_singleton++;
    return newList;
}

void addMsgHistory(size_t len) {
    if(mhlist == 0)
        return;
    struct msgHistoryList * cmhlist = mhlist;

    time_t curTime = time(0);
    if(cmhlist->head == 0) {
        addEvent(MSGSEND, 0, SENDQ_INTERVAL, 0); //should be unique, though there should also never be multiple MSGSEND events in the queue
        if(!(cmhlist->head = cmhlist->tail = calloc(1, sizeof *cmhlist->head))) {
            perror("malloc");
            exit(1);
        }
    } else {
        if(!(cmhlist->tail->next = calloc(1, sizeof *cmhlist->tail))) {
            perror("malloc");
            exit(1);
        }
        cmhlist->tail = cmhlist->tail->next;
    }
    cmhlist->tail->date = curTime;
    cmhlist->tail->len = len;

    cmhlist->byteSize += len;
    cmhlist->msgs++;
}

MsgList createMsgList() {
    if(msgQ_singleton)
        return 0;
    struct msgList * newList;
    if(!(newList = calloc(1, sizeof *newList))) {
        perror("calloc");
        exit(1);
    }
    msgQ_singleton++;
    return newList;
}

void addMsg(char * msg, size_t len) {
    if(mlist == 0)
        return;
    struct msgList * cmlist = mlist;

    if(cmlist->head == 0) {
        if(!(cmlist->head = cmlist->tail = calloc(1, sizeof *cmlist->head))) {
            perror("calloc");
            exit(1);
        }
    } else {
        if(!(cmlist->tail->next = calloc(1, sizeof *cmlist->tail))) {
            perror("calloc");
            exit(1);
        }
        cmlist->tail = cmlist->tail->next;
    }
    if(!(cmlist->tail->msg = malloc(len+1))) {
        perror("malloc");
        exit(1);
    }
    strcpy(cmlist->tail->msg, msg);
    cmlist->tail->len = len;
    cmlist->byteSize += len;
    cmlist->msgs++;
    processMessages();
}

char * retrMsg() {
    if(mlist == 0)
        return 0;
    struct msgList * cmlist = mlist;
    if(cmlist->head == 0)
        return 0;

    char * data = cmlist->head->msg;

    cmlist->byteSize -= cmlist->head->len;
    cmlist->msgs--;

    struct msgNode * newHead = cmlist->head->next;
    free(cmlist->head);
    cmlist->head = newHead;
    if(cmlist->head == 0)
        cmlist->tail = 0;

    return data;
}

void popMsgHist() { //called when a history message reaches it's destruction time
    if(mhlist == 0)
        return;
    struct msgHistoryList * cmhlist = mhlist;
    if(cmhlist->head == 0)
        return;
    time_t curTime = time(0);
    while(cmhlist->head != 0 && cmhlist->head->date <= curTime - SENDQ_INTERVAL) {
        cmhlist->byteSize -= cmhlist->head->len;
        struct msgHistoryNode * newHead = cmhlist->head->next;
        free(cmhlist->head);
        cmhlist->head = newHead;
    }
    if(cmhlist->head != 0)
        addEvent(MSGSEND, 0, (unsigned int)(curTime - cmhlist->head->date), 1);
}

size_t peekMsgSize() {
    if(mlist == 0)
        return (size_t)-1;
    struct msgList * cmlist = mlist;
    if(cmlist->head == 0)
        return (size_t)-1;
    return cmlist->head->len;
}

void processMessages() {
    if(mlist == 0 || mhlist == 0)
        return;
    struct msgList * csrc = mlist;
    struct msgHistoryList * cdest = mhlist;

    if(csrc->head == 0) {
        return;
    }

    size_t len;
    while(cdest->byteSize + (len = peekMsgSize()) < MAX_SENDQ_SIZE) {
        send_socket(retrMsg());
        addMsgHistory(len);
    }
}
