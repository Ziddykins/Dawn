#include "include/status.h"
#include "include/events.h"

static EventList elist = 0; //currently selected list (there may only be one at a time)
static struct Bot * bot;

void init_timers (struct Bot *dawn) {
    //Times are defined in limits.h and are in seconds
    struct sigaction act;
    memset(&act, '\0', sizeof act);

    act.sa_handler = eventHandler; //use simple signal handler
    act.sa_flags = SA_RESTART; //use SA_SIGINFO for more advanced handler

    if(sigaction(SIGALRM, &act, NULL) < 0) { //hook SIGALRM to call our messageHandler function
        perror("sigaction");
        exit(1);
    }

    selectList(createEventList());
    bot = dawn;
    addEvent(HEALING, 0, HEALING_INTERVAL, 0);
    addEvent(SAVING, 0, SAVING_INTERVAL, 0);
    addEvent(HOURLY, 0, 3600, 0);
    printf("Timers started\n");
}

static int eventQ_singleton = 0; //only one instance may be created at any time

struct eventList {
    struct eventNode * root;
    size_t len;
};

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

    struct eventNode * tmp = celist->root;
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
    if(cmlist->root == 0)
        return;

    time_t curtime = time(0);
    time_t event = cmlist->root->event_time;
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
        struct eventNode * next = celist->root->next;
        free(celist->root->elem);
        free(celist->root);
        celist->root = next;
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
    struct eventNode * tmp = cmlist->root, * prev = 0;
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
        struct eventNode * prev_root = cmlist->root;
        if(!(cmlist->root = calloc(1, sizeof *cmlist->root))) {
            perror("calloc eventNode");
            exit(1);
        }
        cmlist->root->next = prev_root;
        prev = cmlist->root;
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
    updateAlarm(); //root may have been replaced so we reset the alarm to the next event in the queue
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
    printFromNode(cmlist->root);
    printf("\n");
}

struct event * retrEvent() { //callee must free the data himself
    if(elist == 0)
        return 0;
    struct eventList * cmlist = (struct eventList *)elist;
    if(cmlist->root == 0)
        return 0;

    struct event * ret = cmlist->root->elem;
    struct eventNode * toFree = cmlist->root;
    cmlist->root = cmlist->root->next;
    free(toFree);
    cmlist->len--;
    updateAlarm();
    return ret;
}

time_t timeToNextMsg() {
    if(elist == 0)
        return time(0);
    struct eventList * cmlist = (struct eventList *)elist;
    if(cmlist->root == 0)
        return time(0);
    return cmlist->root->event_time;
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
    if(cmlist->root == 0)
        return 0;
    return cmlist->root->event_time <= time(0);
}

void eventHandler(int sig) {
    assert(sig == SIGALRM);
    struct event * e;
    do {
        e = retrEvent(); //handle event

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
                for (j=0; j< bot->player_count; j++) {
                    if ((bot->players[e->event].health + 5) <= bot->players[e->event].max_health) {
                        bot->players[e->event].health += 5;
                    } else {
                        bot->players[e->event].health = bot->players[e->event].max_health;
                    }
                }
                addEvent(HEALING, 0, HEALING_INTERVAL, 0);
                break;
            }
            case SAVING:
            {
                struct Bot temp;
                size_t size = sizeof(temp);
                save_players(bot, size);
                addEvent(SAVING, 0, SAVING_INTERVAL, 0);
                break;
            }
            case HOURLY:
            {
                hourly_events(bot);
                addEvent(HOURLY, 0, 3600, 0);
                break;
            }
            case TRAVEL:
            {
                if(bot->players[e->data].travel_timer.active) {
                    char out[MAX_MESSAGE_BUFFER];
                    bot->players[e->data].current_map.cur_x = bot->players[e->data].travel_timer.x;
                    bot->players[e->data].current_map.cur_y = bot->players[e->data].travel_timer.y;
                    sprintf(out, "PRIVMSG %s :%s has arrived at %d,%d\r\n", bot->active_room, bot->players[e->data].username,
                            bot->players[e->data].current_map.cur_x, bot->players[e->data].current_map.cur_y);
                    addMsg(out, strlen(out));
                    bot->players[e->data].travel_timer.active = 0;
                    check_special_location(bot, e->data);
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
