#ifndef LIMITS_H_INCLUDED
#define LIMITS_H_INCLUDED

//networking
#define MAX_RECV_BUFFER     4096
#define MAX_MESSAGE_BUFFER  400
#define MAX_SENDQ_SIZE 950
#define SENDQ_INTERVAL 7

#define MAX_INVENTORY_SLOTS 25
#define MAX_MONSTERS        40
#define MAX_EVENT_TYPE      5
#define MAX_SHRINE_TYPE     3
#define MAX_REWARD_TYPE     3
#define MAX_PUNISHMENT_TYPE 3
#define MAX_ITEM_TYPE       3
#define MAX_WEAPON_TYPE     7
#define MAX_SHIELD_TYPE     5
#define MAX_MATERIAL_TYPE   8
#define MAX_WEATHER_TYPE    4
#define MAX_ARMOR_TYPE      5
#define MAX_SLAY_GOLD       10000000
#define MAX_BUILDINGS       5
//Seconds
#define HEALING_INTERVAL    1800
#define SAVING_INTERVAL     900

extern int MAX_NICK_LENGTH;
extern int MAX_CHANNEL_LENGTH;
#endif
