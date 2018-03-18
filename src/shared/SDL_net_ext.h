/* 
 * A custom extension to SDL_net.
 */

#ifndef SDL_NET_EXT_H
#define SDL_NET_EXT_H

#include <SDL/SDL.h>
#include <SDL/SDL_net.h>

#include "map.h"
#include "player.h"

#define PACKET_SIZE 1024

typedef enum {
    DATA_CONNECT_OK,
    DATA_CONNECT_FULL,
    DATA_CONNECT_BROADCAST,
    DATA_UDP_CONNECT_REQUEST,
    DATA_DISCONNECT_REQUEST,
    DATA_DISCONNECT_BROADCAST,
    DATA_MOVEMENT_REQUEST,
    DATA_MOVEMENT_BROADCAST
} SDLNet_DataType;

typedef struct
{
    SDLNet_DataType type;
} SDLNet_Data;

typedef struct
{
    SDLNet_Data data;
    int id;
} SDLNet_IdData;

typedef struct
{
    SDLNet_Data data;
    int id;
    Object objects[NUM_OBJECTS];
    Player players[MAX_PLAYERS];
} SDLNet_StateData;

typedef struct
{
    SDLNet_Data data;
    Player player;
} SDLNet_PlayerData;

typedef struct
{
    SDLNet_Data data;
    int id;
    double pos_x;
    double pos_y;
} SDLNet_PosData;

SDLNet_Data SDLNet_CreateData(SDLNet_DataType type);
SDLNet_IdData SDLNet_CreateIdData(SDLNet_DataType type, int id);
SDLNet_StateData SDLNet_CreateStateData(SDLNet_DataType type, int id);
SDLNet_PlayerData SDLNet_CreatePlayerData(SDLNet_DataType type, Player player);
SDLNet_PosData SDLNet_CreatePosData(SDLNet_DataType type, int id, double pos_x, double pos_y);

int SDLNet_TCP_SendData(TCPsocket sock, SDLNet_Data *data, int len);
int SDLNet_UDP_SendData(UDPsocket sock, UDPpacket *packet, IPaddress address, SDLNet_Data *data, int len);

#endif
