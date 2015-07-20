#include "include/status.h"
#include "include/events.h"

struct Bot * dawn;
static EventList elist = 0; //currently selected list (there may only be one at a time)

void init_timers (struct Bot *b, char const * fn) {
    //Times are defined in limits.h and are in seconds
    struct sigaction act;
    memset(&act, '\0', sizeof act);

    act.sa_handler = eventHandler; //use simple signal handler
    act.sa_flags = SA_RESTART; //use SA_SIGINFO for more advanced handler

    CALLEXIT(sigaction(SIGALRM, &act, NULL) < 0) //hook SIGALRM to call our messageHandler function

    load_events(fn);
    dawn = b;
    addEvent(HEALING, 0, HEALING_INTERVAL, UNIQUE | KEEP);
    addEvent(SAVING, 0, SAVING_INTERVAL, UNIQUE | KEEP);
    addEvent(HOURLY, 0, 3600, UNIQUE | KEEP);
    printf(INFO "Timers started\n");
}

static int eventQ_singleton = 0; //only one instance may be created at any time

EventList createEventList() {
    if(eventQ_singleton) //if an instance already exists do not create a new one
        return 0;
    eventQ_singleton++;
    struct eventList * newlist;
    CALLEXIT(!(newlist = calloc(1, sizeof *newlist)))
    return newlist;
}

void freeEventList() {
    if(elist == 0)
        return;
    struct eventList * celist = (struct eventList *)elist;

    struct eventNode * tmp = celist->head;
    while(tmp != 0) {
        struct eventNode * next = tmp->next;
        free(tmp->elem);
        free(tmp);
        tmp = next;
    }
    free(elist);
    eventQ_singleton--;
    elist = 0;
}


void updateAlarm() {
    if(elist == 0)
        return;
    struct eventList* celist = (struct eventList *)elist;
    if(celist->head == 0)
        return;

    time_t curtime = time(0);
    time_t event = celist->head->event_time;
    if(event < curtime)
        eventHandler(SIGALRM);
    else
        alarm((unsigned int)(event - curtime));
}

void selectList(EventList x) {
    elist = x;
}

void printNextEvent() {
    struct eventList * celist = elist;
    printf(" next in +%zu (%s)",
        celist != 0 ?
            celist->head != 0 ?
                celist->head->event_time-time(0)
            : 0
        : 0,

        celist != 0 ?
            celist->head != 0 ?
                celist->head->elem != 0 ?
                    eventToStr(celist->head->elem->event)
                : "NONE"
            : "NONE"
        : "NONE");
}

void removeEvent(struct eventNode * prev) { //requires the /PREVIOUS/ node or 0 for head
    struct eventList * celist = (struct eventList *)elist;
    if(elist == 0)
        return;
    if(prev == 0) {
        struct eventNode * next = celist->head->next;
        free(celist->head->elem);
        free(celist->head);
        celist->head = next;
    } else {
        struct eventNode * toFree = prev->next;
        prev->next = prev->next->next;
        free(toFree->elem);
        free(toFree);
    }
}

/*void debugPrint() {
    if(!elist) {
        printf("[NONE]\n");
        return;
    }
    struct eventList * celist = (struct eventList *)elist;
    struct eventNode * tmp = celist->head;
    do {
        printf("[%s]->", eventToStr(tmp ? tmp->elem->event : -1));
        tmp = tmp ? tmp->next : 0;
    } while(tmp);
    printf("\n");
}*/

void addEvent(enum Events event, int eData, unsigned int offset, int flags) { //flags -> enum eventMode (status.h)
    if(elist == 0)
        return;
    struct eventList * celist = (struct eventList *)elist;

    time_t newtime = time(0) + offset;
    struct eventNode * tmp = celist->head, * prev = 0;
    if(flags & UNIQUE) {
        while(tmp != 0 && tmp->event_time < newtime) { //go to the place where we need to insert the new event
            if(tmp->elem->event == event && tmp->elem->data == eData) {
                if(flags & KEEP) {
                    /*printf("STATUS: Did not replace existing event of type %s;", eventToStr(event));
                    printNextEvent();
                    putchar('\n');*/
                    return;
                } else {
                    tmp = prev; //move to previous event
                    removeEvent(prev); //remove next event
                }
            }
            prev = tmp;
            if(tmp != 0)
                tmp = tmp->next;
        }
        struct eventNode * scanner = tmp, * prevScanner = prev;
        while(scanner != 0) {
            if(tmp->elem->event == event && tmp->elem->data == eData) {
                if(flags & KEEP) {
                    /*printf("STATUS: Did not replace existing event of type %s;", eventToStr(event));
                    printNextEvent();
                    putchar('\n');*/
                    return;
                } else {
                    scanner = prevScanner;
                    removeEvent(prevScanner);
                }
            }
            prevScanner = scanner;
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
        struct eventNode * prev_head = celist->head;
        CALLEXIT(!(celist->head = calloc(1, sizeof *celist->head)))
        celist->head->next = prev_head;
        tmp = celist->head;
    } else { //or insert it where it belongs
        struct eventNode * prevnext = prev->next;
        CALLEXIT(!(prev->next = calloc(1, sizeof *prev)))
        tmp = prev->next;
        tmp->next = prevnext;
    }
    CALLEXIT(!(tmp->elem = malloc(sizeof *tmp->elem)))
    tmp->elem->event = event;
    tmp->elem->data = eData;
    tmp->event_time = newtime;

    celist->len++;
    updateAlarm(); //head may have been replaced so we reset the alarm to the next event in the queue
    /*printf("STATUS: Added Event %s with data %d, %zu(+%u);", eventToStr(event), eData, time(0), offset);
    printNextEvent();
    putchar('\n');*/
    return;
}
/*
void printFromNode(struct eventNode * x) {
    if(x == 0) {
        printf("[-]");
        return;
    }
    printf("[%d]->", x->elem->event);
    printFromNode(x->next);
}


void printList() {
    if(elist == 0)
        return;
    struct eventList * celist = (struct eventList *)elist;
    printFromNode(celist->head);
    printf("\n");
}
*/

struct event * retrEvent() { //callee must free the data himself
    if(elist == 0)
        return 0;
    struct eventList * celist = (struct eventList *)elist;
    if(celist->head == 0)
        return 0;

    struct event * ret = celist->head->elem;
    struct eventNode * toFree = celist->head;
    celist->head = celist->head->next;
    free(toFree);
    celist->len--;
    updateAlarm();
    return ret;
}

time_t timeToNextMsg() {
    if(elist == 0)
        return time(0);
    struct eventList * celist = (struct eventList *)elist;
    if(celist->head == 0)
        return time(0);
    return celist->head->event_time;
}

size_t listLen() {
    if(elist == 0)
        return 0;
    struct eventList * celist = (struct eventList *)elist;
    return celist->len;
}

int nextIsDue() {
    if(elist == 0)
        return 0;
    struct eventList * celist = (struct eventList *)elist;
    if(celist->head == 0)
        return 0;
    return celist->head->event_time <= time(0);
}

void eventHandler(int sig) {
    assert(sig == SIGALRM);
    struct event * e;
    do {
        e = retrEvent(); //handle event
        /*printf("STATUS: Received %s;", eventToStr(e->event));
        printNextEvent();
        putchar('\n');*/
        switch (e->event) {
            case MSGSEND:
            {
                popMsgHist();
                processMessages();
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
                addEvent(HEALING, 0, HEALING_INTERVAL, NORMAL);
                break;
            }
            case SAVING:
            {
                persistent_save(dawn);
                addEvent(SAVING, 0, SAVING_INTERVAL, NORMAL);
                break;
            }
            case HOURLY:
            {
                hourly_events(dawn);
                addEvent(HOURLY, 0, 3600, NORMAL);
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
                    addMsg(out, strlen(out));
                    dawn->players[e->data].travel_timer.active = 0;
                    //check_special_location(dawn, e->data); DEPRECATED
                }
                break;
            }
        }
        free(e);
    } while(nextIsDue());
}

char * eventToStr(enum Events x) {
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
        struct eventList * celist = elist;
        struct eventNode * tmp = celist->head;
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
        selectList(createEventList());
        PRINTWARN("Could not load events")
        errno = 0;
    } else {
        assert(elist == 0 && !eventQ_singleton);
        selectList(createEventList());
        struct eventList * celist = elist;

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
            struct eventNode * tmp = 0;
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
        updateAlarm();
    }
    printf(INFO "Events read (%zu bytes)\n", len);
}
