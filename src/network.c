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
    mlist = init_msg_list();
    mhlist = init_msg_hist_list();
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
    CALLEXIT((con_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1)
    printf(INFO "Trying to connect\n");
    CALLEXIT(connect(con_socket, res->ai_addr, res->ai_addrlen) == -1)
    printf(INFO "Connected to %s:%s\n", ip_addr, port);
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
 * @see add_msg
 */
void send_socket (char * out_buf) {
    size_t len = strlen(out_buf);
    if(len > MAX_MESSAGE_BUFFER) {
        printf(WARN "network/send_socket: Message too long: %lu", len);
        return;
    }

    CALLEXIT(write(con_socket, out_buf, len) == -1)
}

/**
 * @brief creates and allocates a Message History List
 * Internal function used by init_send_queue()
 * @see init_send_queue()
 * @return new Message History List
 */
MsgHistoryList init_msg_hist_list() {
    if(msgHQ_singleton)
        return 0;
    struct msg_hist_list * new_list;
    CALLEXIT(!(new_list = calloc(1, sizeof *new_list)))
    msgHQ_singleton++;
    return new_list;
}

/**
 * @brief frees all allocated storage of the Message History List
 * @see mhlist
 */
void free_msg_hist_list() {
    if(!msgHQ_singleton || !mhlist)
        return;
    struct msg_hist_list * cmhlist = mhlist;
    struct msg_hist_node * tmp = cmhlist->head;
    while(tmp) {
        struct msg_hist_node * next = tmp->next;
        free(tmp);
        tmp = next;
    }
    free(cmhlist);
}

/**
 * @brief Enqueues a Messages Metadata into the Message History list
 * Internal function used by add_msg
 * @see add_msg
 */
void add_hist_msg(size_t len) {
    if(mhlist == 0)
        return;
    struct msg_hist_list * cmhlist = mhlist;

    time_t cur_time = time(0);
    if(cmhlist->head == 0) {
        add_event(MSGSEND, 0, SENDQ_INTERVAL, NORMAL); //should be unique, though there should also never be multiple MSGSEND events in the queue
        CALLEXIT(!(cmhlist->head = cmhlist->tail = calloc(1, sizeof *cmhlist->head)))
    } else {
        CALLEXIT(!(cmhlist->tail->next = calloc(1, sizeof *cmhlist->tail)))
        cmhlist->tail = cmhlist->tail->next;
    }
    cmhlist->tail->date = cur_time;
    cmhlist->tail->len = len;

    cmhlist->byte_size += len;
    cmhlist->msgs++;
}

/**
 * @brief allocates and initializes a Message List if none exists ye
 * Internal function used by init_send_queue()
 * @return new Message List
 * @see init_send_queue()
 */
MsgList init_msg_list() {
    if(msgQ_singleton)
        return 0;
    struct msg_list * new_list;
    CALLEXIT(!(new_list = calloc(1, sizeof *new_list)))
    msgQ_singleton++;
    return new_list;
}

/**
 * @brief frees all allocated storage of the Message List including the messages themselves
 */
void free_msg_list() {
    if(!mlist || !msgQ_singleton)
        return;
    struct msg_list * cmlist = mlist;
    struct msg_node * tmp = cmlist->head;
    while(tmp) {
        struct msg_node * next = tmp->next;
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
void add_msg(char * msg, size_t len) {
    if(mlist == 0)
        return;
    struct msg_list * cmlist = mlist;

    if(cmlist->head == 0) {
        CALLEXIT(!(cmlist->head = cmlist->tail = calloc(1, sizeof *cmlist->head)))
    } else {
        CALLEXIT(!(cmlist->tail->next = calloc(1, sizeof *cmlist->tail)))
        cmlist->tail = cmlist->tail->next;
    }
    CALLEXIT(!(cmlist->tail->msg = malloc(len+1)))
    strcpy(cmlist->tail->msg, msg);
    cmlist->tail->len = len;
    cmlist->byte_size += len;
    cmlist->msgs++;
    process_messages();
}

/**
 * @brief will pop the Message Qeue and delete the oldest Message
 * Internal function used by process_messages
 * @return char* character string to be freed by the callee
 * @see process_messages
 */
char * retr_msg() {
    if(mlist == 0)
        return 0;
    struct msg_list * cmlist = mlist;
    if(cmlist->head == 0)
        return 0;

    char * data = cmlist->head->msg;

    cmlist->byte_size -= cmlist->head->len;
    cmlist->msgs--;

    struct msg_node * new_head = cmlist->head->next;
    free(cmlist->head);
    cmlist->head = new_head;
    if(cmlist->head == 0)
        cmlist->tail = 0;
    return data;
}

/**
 * @brief removes all messages that are older than SENDQ_INTERVAL from the Message History List
 * Internal function indirectly used by pop_hist_msg, add_hist_msg, add_msg via the Event Queue in status.h
 * @see SENDQ_INTERVAL, pop_hist_msg, add_hist_msg, add_msg
 */
void pop_hist_msg() {
    if(mhlist == 0)
        return;
    struct msg_hist_list * cmhlist = mhlist;
    if(cmhlist->head == 0)
        return;
    time_t cur_time = time(0);
    while(cmhlist->head != 0 && cur_time - SENDQ_INTERVAL >= cmhlist->head->date) {
        cmhlist->byte_size -= cmhlist->head->len;
        //printf("MQueue: %zu of %d (POP)\n", cmhlist->byte_size, MAX_SENDQ_SIZE);
        cmhlist->msgs--;
        struct msg_hist_node * new_head = cmhlist->head->next;
        free(cmhlist->head);
        cmhlist->head = new_head;
    }
    if(cmhlist->head != 0) {
        add_event(MSGSEND, 1, (unsigned int)(SENDQ_INTERVAL - (cur_time - cmhlist->head->date)), NORMAL);
    }
    process_messages();
}

/**
 * @brief Returns the size of the oldest message (head) in the Message List without removing said message
 * Internal function used by process_messages
 * @return size_t size in number of bytes of the message
 * @see process_messages
 */
size_t peek_msg_size() {
    if(mlist == 0)
        return 0;
    struct msg_list * cmlist = mlist;
    if(cmlist->head == 0)
        return 0;
    return cmlist->head->len;
}

/**
 * @brief Will send messages as long as the Message History List allows for it
 * Internal function used by add_msg, pop_hist_msg
 * @see add_msg, pop_hist_msg
 */
void process_messages() {
    if(mlist == 0 || mhlist == 0)
        return;
    struct msg_list * csrc = mlist;
    struct msg_hist_list * cdest = mhlist;

    size_t len;
    while(csrc->head != 0 && cdest->msgs < MAX_MSGS_IN_INTERVAL && cdest->byte_size + (len = peek_msg_size()) < MAX_SENDQ_SIZE) {
        char * msg = retr_msg();
        send_socket(msg);
        free(msg);
        add_hist_msg(len);
        //printf("MQueue: %zu of %d (PUSH)\n", cdest->byte_size, MAX_SENDQ_SIZE);

    }
    if(cdest->msgs >= MAX_MSGS_IN_INTERVAL || cdest->byte_size + peek_msg_size() >= MAX_SENDQ_SIZE) {
        printf(INFO "Send queue full\n");
    }
}
