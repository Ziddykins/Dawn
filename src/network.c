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

msgList createMsgList() {
    struct msgList * newList;
    if(!(newList = calloc(1, sizeof *newList))) {
        perror("calloc");
        exit(1);
    }
    return newList;
}

void addMsg(msgList mlist, char * msg, size_t len) {
    if(mlist == 0)
        return;
    struct msgList * cmlist = mlist;

    time_t curTime = time(0);
    if(cmlist->head == 0) {
        if(!(cmlist->head = cmlist->tail = malloc(sizeof *cmlist->head))) {
            perror("malloc");
            exit(1);
        }
    } else {
        if(!(cmlist->tail->next = malloc(sizeof *cmlist->tail))) {
            perror("malloc");
            exit(1);
        }
        cmlist->tail = cmlist->tail->next;
    }
    if(!(cmlist->tail->msg = malloc(len+1))) {
        perror("malloc");
        exit(1);
    }
    strcpy(cmlist->tail->msg, msg);
    cmlist->tail->date = curTime;
    cmlist->byteSize += len;
    cmlist->msgs++;
}
