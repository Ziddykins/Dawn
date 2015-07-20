#include "include/status.h"
#include "include/events.h"

struct Bot * dawn;
static EventList elist = 0; //currently selected list (there may only be one at a time)

void init_timers (struct Bot *b, char const * fn) {
    //Times are defined in limits.h and are in seconds
    struct sigaction act;
    memset(&act, '\0', sizeof act);

    act.sa_handler = event_handler; //use simple signal handler
    act.sa_flags = SA_RESTART; //use SA_SIGINFO for more advanced handler

    CALLEXIT(sigaction(SIGALRM, &act, NULL) < 0) //hook SIGALRM to call our message_handler function

    load_events(fn);
    dawn = b;
    add_event(HEALING, 0, HEALING_INTERVAL, UNIQUE | KEEP);
    add_event(SAVING, 0, SAVING_INTERVAL, UNIQUE | KEEP);
    add_event(HOURLY, 0, 3600, UNIQUE | KEEP);
    printf(INFO "Timers started\n");
}

static int eventQ_singleton = 0; //only one instance may be created at any time

EventList init_event_list() {
    if(eventQ_singleton) //if an instance already exists do not create a new one
        return 0;
    eventQ_singleton++;
    struct event_list * newlist;
    CALLEXIT(!(newlist = calloc(1, sizeof *newlist)))
    return newlist;
}

void free_event_list() {
    if(elist == 0)
        return;
    struct event_list * celist = (struct event_list *)elist;

    struct event_node * tmp = celist->head;
    while(tmp != 0) {
        struct event_node * next = tmp->next;
        free(tmp->elem);
        free(tmp);
        tmp = next;
    }
    free(elist);
    eventQ_singleton--;
    elist = 0;
}


void update_alarm() {
    if(elist == 0)
        return;
    struct event_list* celist = (struct event_list *)elist;
    if(celist->head == 0)
        return;

    time_t curtime = time(0);
    time_t event = celist->head->event_time;
    if(event < curtime)
        event_handler(SIGALRM);
    else
        alarm((unsigned int)(event - curtime));
}

void select_list(EventList x) {
    elist = x;
}

void print_next_event() {
    struct event_list * celist = elist;
    printf(" next in +%zu (%s)",
        celist != 0 ?
            celist->head != 0 ?
                celist->head->event_time-time(0)
            : 0
        : 0,

        celist != 0 ?
            celist->head != 0 ?
                celist->head->elem != 0 ?
                    event_to_str(celist->head->elem->event)
                : "NONE"
            : "NONE"
        : "NONE");
}

void remove_event(struct event_node * prev) { //requires the /PREVIOUS/ node or 0 for head
    struct event_list * celist = (struct event_list *)elist;
    if(elist == 0)
        return;
    if(prev == 0) {
        struct event_node * next = celist->head->next;
        free(celist->head->elem);
        free(celist->head);
        celist->head = next;
    } else {
        struct event_node * to_free = prev->next;
        prev->next = prev->next->next;
        free(to_free->elem);
        free(to_free);
    }
}

void add_event(enum Events event, int e_data, unsigned int offset, int flags) { //flags -> enum event_mode (status.h)
    if(elist == 0)
        return;
    struct event_list * celist = (struct event_list *)elist;

    time_t newtime = time(0) + offset;
    struct event_node * tmp = celist->head, * prev = 0;
    if(flags & UNIQUE) {
        while(tmp != 0 && tmp->event_time < newtime) { //go to the place where we need to insert the new event
            if(tmp->elem->event == event && tmp->elem->data == e_data) {
                if(flags & KEEP) {
                    return;
                } else {
                    tmp = prev; //move to previous event
                    remove_event(prev); //remove next event
                }
            }
            prev = tmp;
            if(tmp != 0)
                tmp = tmp->next;
        }
        struct event_node * scanner = tmp, * prev_scanner = prev;
        while(scanner != 0) {
            if(tmp->elem->event == event && tmp->elem->data == e_data) {
                if(flags & KEEP) {
                    return;
                } else {
                    scanner = prev_scanner;
                    remove_event(prev_scanner);
                }
            }
            prev_scanner = scanner;
            if(scanner != 0)
                scanner = scanner->next;
        }
    } else {
        while(tmp != 0 && tmp->event_time < newtime) { //go to the place where we need to insert the new event
            prev = tmp;
            tmp = tmp->next;
        }
    }

    if(prev == 0) { //create a new node from scratch
        struct event_node * prev_head = celist->head;
        CALLEXIT(!(celist->head = calloc(1, sizeof *celist->head)))
        celist->head->next = prev_head;
        tmp = celist->head;
    } else { //or insert it where it belongs
        struct event_node * prevnext = prev->next;
        CALLEXIT(!(prev->next = calloc(1, sizeof *prev)))
        tmp = prev->next;
        tmp->next = prevnext;
    }
    CALLEXIT(!(tmp->elem = malloc(sizeof *tmp->elem)))
    tmp->elem->event = event;
    tmp->elem->data = e_data;
    tmp->event_time = newtime;

    celist->len++;
    update_alarm(); //head may have been replaced so we reset the alarm to the next event in the queue
    return;
}

struct event * retr_event() { //callee must free the data himself
    if(elist == 0)
        return 0;
    struct event_list * celist = (struct event_list *)elist;
    if(celist->head == 0)
        return 0;

    struct event * ret = celist->head->elem;
    struct event_node * to_free = celist->head;
    celist->head = celist->head->next;
    free(to_free);
    celist->len--;
    update_alarm();
    return ret;
}

time_t time_to_next_msg() {
    if(elist == 0)
        return time(0);
    struct event_list * celist = (struct event_list *)elist;
    if(celist->head == 0)
        return time(0);
    return celist->head->event_time;
}

size_t event_list_len() {
    if(elist == 0)
        return 0;
    struct event_list * celist = (struct event_list *)elist;
    return celist->len;
}

int is_next_due() {
    if(elist == 0)
        return 0;
    struct event_list * celist = (struct event_list *)elist;
    if(celist->head == 0)
        return 0;
    return celist->head->event_time <= time(0);
}

void event_handler(int sig) {
    assert(sig == SIGALRM);
    struct event * e;
    do {
        e = retr_event(); //handle event
        switch (e->event) {
            case MSGSEND:
            {
                pop_hist_msg();
                process_messages();
                break;
            }
            case HEALING:
            {
                int j;
                for (j=0; j < dawn->player_count; j++) {
                    if ((dawn->players[j].health + 5) <= dawn->players[j].max_health) {
                        dawn->players[j].health += 5;
                    } else {
                        dawn->players[j].health = dawn->players[j].max_health;
                    }
                }
                add_event(HEALING, 0, HEALING_INTERVAL, NORMAL);
                break;
            }
            case SAVING:
            {
                persistent_save(dawn);
                add_event(SAVING, 0, SAVING_INTERVAL, NORMAL);
                break;
            }
            case HOURLY:
            {
                hourly_events(dawn);
                add_event(HOURLY, 0, 3600, NORMAL);
                break;
            }
            case TRAVEL:
            {
                if(dawn->players[e->data].travel_timer.active) {
                    char out[MAX_MESSAGE_BUFFER];
                    dawn->players[e->data].pos_x = dawn->players[e->data].travel_timer.x;
                    dawn->players[e->data].pos_y = dawn->players[e->data].travel_timer.y;
                    sprintf(out, "PRIVMSG %s :%s has arrived at %d,%d\r\n", dawn->active_room, dawn->players[e->data].username,
                            dawn->players[e->data].pos_x, dawn->players[e->data].pos_y);
                    add_msg(out, strlen(out));
                    dawn->players[e->data].travel_timer.active = 0;
                    //check_special_location(dawn, e->data); DEPRECATED
                }
                break;
            }
        }
        free(e);
    } while(is_next_due());
}

char * event_to_str(enum Events x) {
    switch(x) {
        case HEALING:
            return "HEALING";
        case SAVING:
            return "SAVING";
        case HOURLY:
            return "HOURLY";
        case SUNNY:
            return "SUNNY";
        case RAINING:
            return "RAINING";
        case SNOWING:
            return "SNOWING";
        case TRAVEL:
            return "TRAVEL";
        case MSGSEND:
            return "MSGSEND";
    }
    return "NONE";
}


void save_events(char const * fn) {
    FILE * file;
    size_t len = 0, ret;
    if(!(file = fopen(fn, "wb"))) {
        PRINTWARN("Could not save events")
        errno = 0;
    } else {
        if(elist == 0) {
            fprintf(stderr, WARN "status/save_events: tried to save but there was no event list\n");
            return;
        }
        struct event_list * celist = elist;
        struct event_node * tmp = celist->head;
        while(tmp != 0) {
            if(!(ret = fwrite(&tmp->event_time, sizeof tmp->event_time, 1, file))) {
                PRINTERR("fwrite")
                errno = 0;
                fprintf(stderr, ERR "status/save_events: event file corrupted while saving - continuing");
                break;
            }
            len += ret * sizeof tmp->event_time;
            if(!(ret = fwrite(&tmp->elem->event, sizeof tmp->elem->event, 1, file))) {
                PRINTERR("fwrite")
                errno = 0;
                fprintf(stderr, ERR "status/save_events: event file corrupted while saving - continuing");
                break;
            }
            len += ret * sizeof tmp->elem->event;
            if(!(ret = fwrite(&tmp->elem->data, sizeof tmp->elem->data, 1, file))) {
                PRINTERR("fwrite")
                errno = 0;
                fprintf(stderr, ERR "status/save_events: event file corrupted while saving - continuing");
                break;
            }
            len += ret * sizeof tmp->elem->data;
            tmp = tmp->next;
        }
        fclose(file);
    }
    printf(INFO "Saved events (%zu bytes)\n", len);
}

void load_events(char const * fn) {
    FILE * file;
    size_t len = 0;
    if(!(file = fopen(fn, "rb"))) {
        select_list(init_event_list());
        PRINTWARN("Could not load events")
        errno = 0;
    } else {
        assert(elist == 0 && !eventQ_singleton);
        select_list(init_event_list());
        struct event_list * celist = elist;

        time_t tmpT;
        enum Events tmpE;
        int tmpD;
        if(feof(file)) {
            fclose(file);
            return;
        }

        len += sizeof tmpT * fread(&tmpT, sizeof tmpT, 1, file);
        if(feof(file) || ferror(file)) {
            PRINTERR("fread")
            fclose(file);
            return;
        }

        len += sizeof tmpE * fread(&tmpE, sizeof tmpE, 1, file);
        if(feof(file) || ferror(file)) {
            PRINTERR("fread")
            fclose(file);
            return;
        }

        len += sizeof tmpD * fread(&tmpD, sizeof tmpD, 1, file);
        if(ferror(file)) {
            PRINTERR("fread")
            fclose(file);
            return;
        }
        celist->len++;

        if(!feof(file)) {
            struct event_node * tmp = 0;
            CALLEXIT(!(tmp = celist->head = calloc(1, sizeof *celist->head)))
            while(!feof(file)) {
                CALLEXIT(!(tmp->elem = malloc(sizeof *tmp->elem)))
                tmp->event_time = tmpT;
                tmp->elem->event = tmpE;
                tmp->elem->data = tmpD;

                len += sizeof tmpT * fread(&tmpT, sizeof tmpT, 1, file);
                if(feof(file)) {
                    break;
                }
                CALLEXIT(ferror(file))

                len += sizeof tmpE * fread(&tmpE, sizeof tmpE, 1, file);
                CALLEXIT(feof(file) || ferror(file))

                len += sizeof tmpD * fread(&tmpD, sizeof tmpD, 1, file);
                CALLEXIT(feof(file) || ferror(file))

                celist->len++;

                CALLEXIT(!(tmp->next = calloc(1, sizeof *tmp)))
                tmp = tmp->next;
            }
        }
        fclose(file);
        update_alarm();
    }
    printf(INFO "Events read (%zu bytes)\n", len);
}
