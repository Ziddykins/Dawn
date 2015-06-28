#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include "include/status.h"
#include "include/limits.h"
#include "include/network.h"
#include "include/player.h"
#include "include/colors.h"
#include "include/events.h"
#include "include/map.h"

static EventList elist = 0; //currently selected list (there may only be one at a time)
static struct Bot * bot;

void set_timer (int timer, time_t amount) {
    addEvent(timer, 0, (unsigned int)amount);
    printf("Timer %d set for %ld in the future\n", timer, amount);
}

void init_timers (struct Bot *dawn) {
    //Times are defined in limits.h and are in seconds
    struct sigaction act;
    memset(&act, '\0', sizeof act);

    act.sa_handler = eventHandler; //use simple signal handler
    //act.sa_flags = SA_SIGINFO;

    if(sigaction(SIGALRM, &act, NULL) < 0) { //hook SIGALRM to call our messageHandler function
        perror("sigaction");
        exit(1);
    }

    selectList(createEventList());
    bot = dawn;
    set_timer(HEALING, HEALING_INTERVAL);
    set_timer(SAVING, SAVING_INTERVAL);
    set_timer(HOURLY, 3600);
    printf("Timers started\n");
}

static int singleton = 0; //only one instance may be created at any time

struct eventList {
    struct eventNode * root;
    size_t len;
};

EventList createEventList() {
    if(singleton) //if an instance already exists do not create a new one
        return 0;
    singleton++;
    struct eventList * newlist = calloc(1, sizeof *newlist);
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
    singleton--;
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

void addEvent(int event, int playerID, unsigned int offset) {
    if(elist == 0)
        return;
    struct eventList * cmlist = (struct eventList *)elist;

    time_t newtime = time(0) + offset;
    struct eventNode * tmp = cmlist->root, * prev = 0;
    if(event == TRAVEL) {
        while(tmp != 0 && tmp->event_time < newtime) { //go to the place where we need to insert the new event
            if(tmp->elem->event == event && tmp->elem->data == playerID)
                removeEvent(prev);
            prev = tmp;
            tmp = tmp->next;
        }
        struct eventNode * scanner = tmp, * prevScanner = prev;
        while(scanner != 0) {
            if(tmp->elem->event == event && tmp->elem->data == playerID)
                removeEvent(prev);
            prev = scanner;
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
        cmlist->root = calloc(1, sizeof *cmlist->root);
        cmlist->root->next = prev_root;
        prev = cmlist->root;
    } else { //or insert it where it belongs
        struct eventNode * prevnext = prev->next;
        prev->next = calloc(1, sizeof *prev);
        prev = prev->next;
        prev->next = prevnext;
    }
    prev->elem = malloc(sizeof *prev->elem);
    prev->elem->event = event;
    prev->elem->data = playerID;
    prev->event_time = newtime;

    cmlist->len++;
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

struct event * retrMsg() { //callee must free the data himself
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
        e = retrMsg(); //handle event

        switch (e->event) {
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
                set_timer(HEALING, HEALING_INTERVAL);
                break;
            }
            case SAVING:
            {
                struct Bot temp;
                size_t size = sizeof(temp);
                save_players(bot, size);
                set_timer(SAVING, SAVING_INTERVAL);
                break;
            }
            case HOURLY:
            {
                hourly_events(bot);
                set_timer(HOURLY, 3600);
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
                    send_socket(out);
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
