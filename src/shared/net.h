#ifndef NET_H
#define NET_H

#include <SDL/SDL_net.h>

#include "player.h"

#define PACKET_SIZE 1024
#define MAX_SOCKETS 2

typedef struct
{
    enum
    {
        PACKET_ENTER,
        PACKET_FULL,
        PACKET_CONNECT,
        PACKET_SYNC,
        PACKET_MOVEMENT,
        PACKET_ROTATION,
        PACKET_DISCONNECT
    } type;
    union {
        struct
        {
            int id;
        } enter;
        struct
        {
            int id;
        } connect;
        struct
        {
            Player players[MAX_SOCKETS];
        } sync;
        struct
        {
            int id;
            double pos_x;
            double pos_y;
        } movement;
        struct
        {
            int id;
        } disconnect;
    } data;
} Packet;

#endif
