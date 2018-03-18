#ifndef NET_H
#define NET_H

#include <stdbool.h>

#include "player.h"

#define PACKET_SIZE 1024
#define MAX_SOCKETS 2

typedef enum {
    DATA_CONNECT_OK,
    DATA_CONNECT_FULL,
    DATA_CONNECT_BROADCAST,
    DATA_UDP_CONNECT_REQUEST,
    DATA_DISCONNECT_REQUEST,
    DATA_DISCONNECT_BROADCAST,
    DATA_MOVEMENT_REQUEST,
    DATA_MOVEMENT_BROADCAST
} DataType;

typedef struct
{
    DataType type;
} Data;

typedef struct
{
    Data data;
    int id;
} IdData;

typedef struct
{
    Data data;
    int id;
    Player players[MAX_SOCKETS];
} ConnectData;

typedef struct
{
    Data data;
    Player player;
} PlayerData;

typedef struct
{
    Data data;
    int id;
    double pos_x;
    double pos_y;
} PosData;

#endif
