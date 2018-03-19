#ifndef DATA_H
#define DATA_H

#include "map.h"
#include "player.h"

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
    Object objects[NUM_OBJECTS];
    Player players[MAX_PLAYERS];
} StateData;

typedef struct
{
    Data data;
    Player player;
} PlayerData;

typedef struct
{
    Data data;
    int id;
    double dx;
    double dy;
} MoveData;

typedef struct
{
    Data data;
    int id;
    double pos_x;
    double pos_y;
} PosData;

Data data_create(DataType type);
IdData id_data_create(DataType type, int id);
StateData state_data_create(DataType type, int id);
PlayerData player_data_create(DataType type, Player player);
MoveData move_data_create(DataType type, int id, double dx, double dy);
PosData pos_data_create(DataType type, int id, double pos_x, double pos_y);

#endif
