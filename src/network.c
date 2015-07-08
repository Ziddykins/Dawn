#include "include/network.h"

//! Receive buffer
char buffer[MAX_RECV_BUFFER + 1];

//! Main IRC connection of the bot
int con_socket;

//! Singleton Message List
static MsgList mlist;
//! Singleton Message History List
static MsgHistoryList mhlist;

//! Information whether a Message List has already been created
static int msgQ_singleton = 0;

//! Information whether a Message History List has already been created
static int msgHQ_singleton = 0;

//! Creates both a Message and a Message History List
void init_send_queue() {
    mlist = createMsgList();
    mhlist = createMsgHistoryList();
}

/**
 * @brief Connects to given address, file descriptor stored in con_socket
 */
int init_connect_server (const char *ip_addr, const char *port) {
    errno = 0;
    struct addrinfo destination, *res;
    memset(&destination, 0, sizeof destination);
    destination.ai_family = AF_INET;
    destination.ai_socktype = SOCK_STREAM;
    getaddrinfo(ip_addr, port, &destination, &res);
    if((con_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1) {
        perror("socket");
        exit(1);
    }
    printf("[!] Trying to connect\n");
    if(connect(con_socket, res->ai_addr, res->ai_addrlen) == -1) {
        perror("connect");
        exit(1);
    }
    printf("[!] Connected\n");
    freeaddrinfo(res);
    return errno;
}

/**
 * Close existing connection
 */
void close_socket (int socket) {
    close(socket);
}

/**
 * @brief Will send a given character string to the IRC server
 * Internal function used by Message List.
 * upon write error program will terminate
 * @param out_buf character string to send
 * @see addMsg
 */
void send_socket (char * out_buf) {
    size_t len = strlen(out_buf);
    if(len > MAX_MESSAGE_BUFFER) {
        printf("Message too long: %lu", len);
        return;
    }

    if(!write(con_socket, out_buf, len)) {
        perror("write");
        exit(1);
    }
}

/**
 * @brief creates and allocates a Message History List
 * Internal function used by init_send_queue()
 * @see init_send_queue()
 * @return new Message History List
 */
MsgHistoryList createMsgHistoryList() {
    if(msgHQ_singleton)
        return 0;
    struct msgHistoryList * newList;
    if(!(newList = calloc(1, sizeof *newList))) {
        perror("calloc");
        exit(1);
    }
    msgHQ_singleton++;
    return newList;
}

/**
 * @brief frees all allocated storage of the Message History List
 * @see mhlist
 */
void freeMsgHistList() {
    if(!msgHQ_singleton || !mhlist)
        return;
    struct msgHistoryList * cmhlist = mhlist;
    struct msgHistoryNode * tmp = cmhlist->head;
    while(tmp) {
        struct msgHistoryNode * next = tmp->next;
        free(tmp);
        tmp = next;
    }
    free(cmhlist);
}

/**
 * @brief Enqueues a Messages Metadata into the Message History list
 * Internal function used by addMsg
 * @see addMsg
 */
void addMsgHistory(size_t len) {
    if(mhlist == 0)
        return;
    struct msgHistoryList * cmhlist = mhlist;

    time_t curTime = time(0);
    if(cmhlist->head == 0) {
        addEvent(MSGSEND, 0, SENDQ_INTERVAL, NORMAL); //should be unique, though there should also never be multiple MSGSEND events in the queue
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

/**
 * @brief allocates and initializes a Message List if none exists ye
 * Internal function used by init_send_queue()
 * @return new Message List
 * @see init_send_queue()
 */
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

/**
 * @brief frees all allocated storage of the Message List including the messages themselves
 */
void freeMsgList() {
    if(!mlist || !msgQ_singleton)
        return;
    struct msgList * cmlist = mlist;
    struct msgNode * tmp = cmlist->head;
    while(tmp) {
        struct msgNode * next = tmp->next;
        free(tmp->msg);
        free(tmp);
        tmp = next;
    }
    free(cmlist);
}

/**
 * @brief Enqueues a Message that is to be sent to the IRC server into the Message List
 * Upon insertion the function will send from the oldest Message in the queue on as many
 * Messages as it can (MAX_SENDQ_SIZE).
 * @param msg message
 * @param len length of given message without NUL terminator
 * @see SENDQ_INTERVAL, MAX_SENDQ_SIZE
 */
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

/**
 * @brief will pop the Message Qeue and delete the oldest Message
 * Internal function used by processMessages
 * @return char* character string to be freed by the callee
 * @see processMessages
 */
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

/**
 * @brief removes all messages that are older than SENDQ_INTERVAL from the Message History List
 * Internal function indirectly used by popMsgHist, addMsgHistory, addMsg via the Event Queue in status.h
 * @see SENDQ_INTERVAL, popMsgHist, addMsgHistory, addMsg
 */
void popMsgHist() {
    if(mhlist == 0)
        return;
    struct msgHistoryList * cmhlist = mhlist;
    if(cmhlist->head == 0)
        return;
    time_t curTime = time(0);
    while(cmhlist->head != 0 && curTime - SENDQ_INTERVAL >= cmhlist->head->date) {
        cmhlist->byteSize -= cmhlist->head->len;
        printf("MQueue: %zu of %d (POP)\n", cmhlist->byteSize, MAX_SENDQ_SIZE);
        cmhlist->msgs--;
        struct msgHistoryNode * newHead = cmhlist->head->next;
        free(cmhlist->head);
        cmhlist->head = newHead;
    }
    if(cmhlist->head != 0) {
        addEvent(MSGSEND, 1, (unsigned int)(SENDQ_INTERVAL - (curTime - cmhlist->head->date)), NORMAL);
    }
    processMessages();
}

/**
 * @brief Returns the size of the oldest message (head) in the Message List without removing said message
 * Internal function used by processMessages
 * @return size_t size in number of bytes of the message
 * @see processMessages
 */
size_t peekMsgSize() {
    if(mlist == 0)
        return 0;
    struct msgList * cmlist = mlist;
    if(cmlist->head == 0)
        return 0;
    return cmlist->head->len;
}

/**
 * @brief Will send messages as long as the Message History List allows for it
 * Internal function used by addMsg, popMsgHist
 * @see addMsg, popMsgHist
 */
void processMessages() {
    if(mlist == 0 || mhlist == 0)
        return;
    struct msgList * csrc = mlist;
    struct msgHistoryList * cdest = mhlist;

    size_t len;
    while(csrc->head != 0 && cdest->msgs < MAX_MSGS_IN_INTERVAL && cdest->byteSize + (len = peekMsgSize()) < MAX_SENDQ_SIZE) {
        char * msg = retrMsg();
        send_socket(msg);
        free(msg);
        addMsgHistory(len);
        printf("MQueue: %zu of %d (PUSH)\n", cdest->byteSize, MAX_SENDQ_SIZE);
    }
    if(cdest->msgs >= MAX_MSGS_IN_INTERVAL || cdest->byteSize + peekMsgSize() >= MAX_SENDQ_SIZE) {
        printf("MQeue: full - waiting for update.\n");
    }
}
