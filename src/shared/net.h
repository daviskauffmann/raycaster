#ifndef NET_H
#define NET_H

#include <SDL/SDL.h>
#include <SDL/SDL_net.h>

#include "player.h"

#define PACKET_SIZE 1024
#define MAX_CLIENTS 2

typedef enum {
    DATA_CONNECT_OK,
    DATA_CONNECT_FULL,
    DATA_CONNECT_BROADCAST,
    DATA_UDP_CONNECT_REQUEST,
    DATA_DISCONNECT_REQUEST,
    DATA_DISCONNECT_BROADCAST,
    DATA_MOVEMENT_REQUEST,
    DATA_MOVEMENT_BROADCAST
} Net_DataType;

typedef struct
{
    Net_DataType type;
} Net_Data;

typedef struct
{
    Net_Data data;
    int id;
} Net_IdData;

typedef struct
{
    Net_Data data;
    int id;
    Player players[MAX_CLIENTS];
} Net_ConnectData;

typedef struct
{
    Net_Data data;
    Player player;
} Net_PlayerData;

typedef struct
{
    Net_Data data;
    int id;
    double pos_x;
    double pos_y;
} Net_PosData;

Net_Data Net_CreateData(Net_DataType type);
Net_IdData Net_CreateIdData(Net_DataType type, int id);
Net_ConnectData Net_CreateConnectData(Net_DataType type, int id, Player players[MAX_CLIENTS]);
Net_PlayerData Net_CreatePlayerData(Net_DataType type, Player player);
Net_PosData Net_CreatePosData(Net_DataType type, int id, double pos_x, double pos_y);

int Net_TCP_Send(TCPsocket sock, Net_Data *data, int len);
int Net_UDP_Send(UDPsocket sock, UDPpacket *packet, IPaddress address, Net_Data *data, int len);

#endif
