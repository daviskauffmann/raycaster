#ifndef DATA_H
#define DATA_H

#include "map.h"
#include "player.h"

#define PACKET_SIZE 1024

enum data_type
{
    DATA_CONNECT_OK,
    DATA_CONNECT_FULL,
    DATA_CONNECT_BROADCAST,
    DATA_UDP_CONNECT_REQUEST,
    DATA_DISCONNECT_REQUEST,
    DATA_DISCONNECT_BROADCAST,
    DATA_MOVEMENT_REQUEST,
    DATA_MOVEMENT_BROADCAST
};

struct data
{
    enum data_type type;
};

struct id_data
{
    struct data data;
    int id;
};

struct state_data
{
    struct data data;
    int id;
    struct object objects[NUM_OBJECTS];
    struct player players[MAX_PLAYERS];
};

struct player_data
{
    struct data data;
    struct player player;
};

struct move_data
{
    struct data data;
    int id;
    double dx;
    double dy;
};

struct pos_data
{
    struct data data;
    int id;
    double pos_x;
    double pos_y;
};

struct data data_create(enum data_type type);
struct id_data id_data_create(enum data_type type, int id);
struct state_data state_data_create(enum data_type type, int id);
struct player_data player_data_create(enum data_type type, struct player player);
struct move_data move_data_create(enum data_type type, int id, double dx, double dy);
struct pos_data pos_data_create(enum data_type type, int id, double pos_x, double pos_y);

#endif
