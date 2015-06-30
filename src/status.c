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

    if(sigaction(SIGALRM, &act, NULL) < 0) { //hook SIGALRM to call our messageHandler function
        perror("sigaction");
        exit(1);
    }

    load_events(fn);
    dawn = b;
    addEvent(HEALING, 0, HEALING_INTERVAL, 0);
    addEvent(SAVING, 0, SAVING_INTERVAL, 0);
    addEvent(HOURLY, 0, 3600, 0);
    printf("Timers started\n");
}

static int eventQ_singleton = 0; //only one instance may be created at any time

EventList createEventList() {
    if(eventQ_singleton) //if an instance already exists do not create a new one
        return 0;
    eventQ_singleton++;
    struct eventList * newlist = calloc(1, sizeof *newlist);
    if(!newlist) {
        perror("calloc createEventList");
        exit(1);
    }
    return newlist;
}

void deleteEventList() {
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
    struct eventList* cmlist = (struct eventList *)elist;
    if(cmlist->head == 0)
        return;

    time_t curtime = time(0);
    time_t event = cmlist->head->event_time;
    if(event < curtime)
        eventHandler(SIGALRM);
    else
        alarm((unsigned int)(event - curtime));
}

void selectList(EventList x) {
    elist = x;
}

void removeEvent(struct eventNode * prev) {
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

void addEvent(enum Events event, int eData, unsigned int offset, int unique) {
    if(elist == 0)
        return;
    struct eventList * cmlist = (struct eventList *)elist;

    time_t newtime = time(0) + offset;
    struct eventNode * tmp = cmlist->head, * prev = 0;
    if(unique) {
        while(tmp != 0 && tmp->event_time < newtime) { //go to the place where we need to insert the new event
            if((unsigned)tmp->elem->event == event && tmp->elem->data == eData) {
                tmp = prev;
                removeEvent(prev);
            }
            prev = tmp;
            if(tmp != 0)
                tmp = tmp->next;
        }
        struct eventNode * scanner = tmp, * prevScanner = prev;
        while(scanner != 0) {
            if((unsigned)tmp->elem->event == event && tmp->elem->data == eData) {
                removeEvent(prevScanner);
                scanner = prevScanner;
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
        struct eventNode * prev_head = cmlist->head;
        if(!(cmlist->head = calloc(1, sizeof *cmlist->head))) {
            perror("calloc eventNode");
            exit(1);
        }
        cmlist->head->next = prev_head;
        prev = cmlist->head;
    } else { //or insert it where it belongs
        struct eventNode * prevnext = prev->next;
        if(!(prev->next = calloc(1, sizeof *prev))) {
            perror("calloc eventNode");
            exit(1);
        }
        prev = prev->next;
        prev->next = prevnext;
    }
    if(!(prev->elem = malloc(sizeof *prev->elem))) {
        perror("malloc event");
        exit(1);
    }
    prev->elem->event = event;
    prev->elem->data = eData;
    prev->event_time = newtime;

    cmlist->len++;
    printf("STATUS: Added Event %s with data %d, %zu(+%u) sec.\n", eventToStr(event), eData, time(0), offset);
    updateAlarm(); //head may have been replaced so we reset the alarm to the next event in the queue
    return;
}

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
    struct eventList * cmlist = (struct eventList *)elist;
    printFromNode(cmlist->head);
    printf("\n");
}

struct event * retrEvent() { //callee must free the data himself
    if(elist == 0)
        return 0;
    struct eventList * cmlist = (struct eventList *)elist;
    if(cmlist->head == 0)
        return 0;

    struct event * ret = cmlist->head->elem;
    struct eventNode * toFree = cmlist->head;
    cmlist->head = cmlist->head->next;
    free(toFree);
    cmlist->len--;
    updateAlarm();
    return ret;
}

time_t timeToNextMsg() {
    if(elist == 0)
        return time(0);
    struct eventList * cmlist = (struct eventList *)elist;
    if(cmlist->head == 0)
        return time(0);
    return cmlist->head->event_time;
}

size_t listLen() {
    if(elist == 0)
        return 0;
    struct eventList * cmlist = (struct eventList *)elist;
    return cmlist->len;
}

int nextIsDue() {
    if(elist == 0)
        return 0;
    struct eventList * cmlist = (struct eventList *)elist;
    if(cmlist->head == 0)
        return 0;
    return cmlist->head->event_time <= time(0);
}

void eventHandler(int sig) {
    assert(sig == SIGALRM);
    struct event * e;
    do {
        e = retrEvent(); //handle event
        printf("RECEIVED %s\n", eventToStr(e->event));
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
                for (j=0; j< dawn->player_count; j++) {
                    if ((dawn->players[e->event].health + 5) <= dawn->players[e->event].max_health) {
                        dawn->players[e->event].health += 5;
                    } else {
                        dawn->players[e->event].health = dawn->players[e->event].max_health;
                    }
                }
                addEvent(HEALING, 0, HEALING_INTERVAL, 0);
                break;
            }
            case SAVING:
            {
                persistent_save(dawn);
                addEvent(SAVING, 0, SAVING_INTERVAL, 0);
                break;
            }
            case HOURLY:
            {
                hourly_events(dawn);
                addEvent(HOURLY, 0, 3600, 0);
                break;
            }
            case TRAVEL:
            {
                if(dawn->players[e->data].travel_timer.active) {
                    char out[MAX_MESSAGE_BUFFER];
                    dawn->players[e->data].current_map.cur_x = dawn->players[e->data].travel_timer.x;
                    dawn->players[e->data].current_map.cur_y = dawn->players[e->data].travel_timer.y;
                    sprintf(out, "PRIVMSG %s :%s has arrived at %d,%d\r\n", dawn->active_room, dawn->players[e->data].username,
                            dawn->players[e->data].current_map.cur_x, dawn->players[e->data].current_map.cur_y);
                    addMsg(out, strlen(out));
                    dawn->players[e->data].travel_timer.active = 0;
                    check_special_location(dawn, e->data);
                }
                break;
            }
            default:
                continue;
        }
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
        perror("ERR: STATUS: fopen");
        errno = 0;
    } else {
        if(elist == 0) {
            printf("WARN: STATUS: tried to save but there was no event list\n");
            return;
        }
        struct eventList * celist = elist;
        struct eventNode * tmp = celist->head;
        while(tmp != 0) {
            if(!(ret = fwrite(&tmp->event_time, sizeof tmp->event_time, 1, file))) {
                perror("status fwrite");
                errno = 0;
                printf("ERR: STATUS: event file corrupted while saving.");
                break;
            }
            len += ret * sizeof tmp->event_time;
            if(!(ret = fwrite(&tmp->elem->event, sizeof tmp->elem->event, 1, file))) {
                perror("status fwrite");
                errno = 0;
                printf("ERR: STATUS: event file corrupted while saving.");
                break;
            }
            len += ret * sizeof tmp->elem->event;
            if(!(ret = fwrite(&tmp->elem->data, sizeof tmp->elem->data, 1, file))) {
                perror("status fwrite");
                errno = 0;
                printf("ERR: STATUS: event file corrupted while saving.");
                break;
            }
            len += ret * sizeof tmp->elem->data;
            tmp = tmp->next;
        }
        fclose(file);
    }
    printf("Saved %zu Eventbytes\n", len);
}
void load_events(char const * fn) {
    FILE * file;
    if(!(file = fopen(fn, "rb"))) {
        selectList(createEventList());
        perror("WARN: STATUS: fopen");
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

        if(!fread(&tmpT, sizeof tmpT, 1, file)) {
            perror("ERR: STATUS: fread");
            fclose(file);
            return;
        }

        if(!fread(&tmpE, sizeof tmpE, 1, file)) {
            perror("ERR: STATUS: fread");
            fclose(file);
            return;
        }

        if(!fread(&tmpD, sizeof tmpD, 1, file)) {
            perror("ERR: STATUS: fread");
            fclose(file);
            return;
        }

        struct eventNode * tmp = celist->head = malloc(sizeof *celist->head);
        while(!feof(file)) {
            tmp->elem = malloc(sizeof *tmp->elem);
            tmp->event_time = tmpT;
            tmp->elem->event = tmpE;
            tmp->elem->data = tmpD;

            if(!fread(&tmpT, sizeof tmpT, 1, file)) {
                perror("ERR: STATUS: fread");
                break;
            }

            if(!fread(&tmpE, sizeof tmpE, 1, file)) {
                perror("ERR: STATUS: fread");
                break;
            }

            if(!fread(&tmpD, sizeof tmpD, 1, file)) {
                perror("ERR: STATUS: fread");
                break;
            }

            tmp->next = malloc(sizeof *tmp);
            tmp = tmp->next;
        }
        fclose(file);
        updateAlarm();
    }
}
