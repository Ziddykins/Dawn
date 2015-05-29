#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include "network.h"
#include "limits.h"
 
char buffer[MAX_RECV_BUFFER + 1];
int con_socket;

int init_connect_server (char *ip_addr, char *port) {
    struct addrinfo destination, *res;
    memset(&destination, 0, sizeof destination);
    destination.ai_family = AF_INET;
    destination.ai_socktype = SOCK_STREAM;
    getaddrinfo(ip_addr, port, &destination, &res);
    con_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    printf("trying to connect\n");
    connect(con_socket, res->ai_addr, res->ai_addrlen);
    printf("gud\n");
    free(res);
    return errno;
}

void close_socket (int con_socket) {
    close(con_socket);
}

void send_socket (char *out_buf) {
    int bytes = 0;
    bytes = write(con_socket, out_buf, strlen(out_buf));
    printf("Sending (%4d bytes): %s", bytes, out_buf);
}
