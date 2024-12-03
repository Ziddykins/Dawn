#ifndef LIMITS_H_INCLUDED
#define LIMITS_H_INCLUDED

#include <limits.h>

/* networking + RFC/Server-defined limits */
#define MAX_MESSAGE_BUFFER   (400)        /* Max length of a single outgoing message */
#define MAX_RECV_BUFFER      (4096)       /* Max received packet size by the server */
#define MAX_SENDQ_SIZE       (500)        /* Max length of all messages sent in last SENDQ_INTERVAL seconds */
#define MAX_MSGS_IN_INTERVAL (10)         /* Max number of messages sent in last SENDQ_INTERVAL seconds */
#define SENDQ_INTERVAL       (5)          /* Time a message lives inside the Message History List */
#define MAX_MONSTER_LENGTH   (64)         /* Max length of monster nickname */
#define MAX_HOSTNAME_LENGTH  (63)         /* Max length of a hostname by itself (i.e. ident!nickname@->HOSTNAME<-) */
#define MAX_IDENT_LENGTH     (10)         /* Max length of username/ident */
#define AUTH_KEY_LEN         (24)
extern unsigned int MAX_NICK_LENGTH; /* IRCd-specific, so parse for it on connect */
extern unsigned int MAX_CHANNEL_LENGTH; /* IRCd-specific, so parse for it on connect */


#define MAX_INVENTORY_SLOTS  (25)
#define MAX_MONSTERS         (40)
#define MAX_SPECIAL_MONSTERS (5)
#define MAX_EVENT_TYPE       (5)
#define MAX_SHRINE_TYPE      (3)
#define MAX_REWARD_TYPE      (3)
#define MAX_PUNISHMENT_TYPE  (3)
#define MAX_ITEM_TYPE        (3)
#define MAX_WEAPON_TYPE      (12)
#define MAX_SHIELD_TYPE      (5)
#define MAX_MATERIAL_TYPE    (8)
#define MAX_WEATHER_TYPE     (3)
#define MAX_ARMOR_TYPE       (5)
#define MAX_SLAY_GOLD        (INT_MAX)
#define MAX_BUILDINGS        (64) /* per town/map */
#define MAT_COUNT            (8)

#define PERLIN_SCALE         (1.0f/(1<<8))
#define PERLIN_V_SCALE       (3.25f)
#define GENERATION_TRIALS    (10000)

/* Upper bounds for map dimensions */
#define MAXIMUM_XSIZE_MAP    (1024)
#define MAXIMUM_YSIZE_MAP    (1024)

// Intervals in seconds
#define HEALING_INTERVAL     (1800)
#define SAVING_INTERVAL      (900)
#define BOUNTY_RESET_INTERVAL (86400)
#define LOTTERY_COLLECT_INTERVAL (1800)
#define LOTTERY_REWARD_INTERVAL  (43200)

#endif
