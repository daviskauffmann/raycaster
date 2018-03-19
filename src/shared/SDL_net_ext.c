#include <SDL/SDL.h>
#include <SDL/SDL_net.h>

#include "map.h"
#include "player.h"
#include "SDL_net_ext.h"

static char buffer[PACKET_SIZE];

SDLNet_Data SDLNet_CreateData(SDLNet_DataType type)
{
    SDLNet_Data data;

    data.type = type;

    return data;
}

SDLNet_IdData SDLNet_CreateIdData(SDLNet_DataType type, int id)
{
    SDLNet_IdData id_data;

    id_data.data = SDLNet_CreateData(type);
    id_data.id = id;

    return id_data;
}

SDLNet_StateData SDLNet_CreateStateData(SDLNet_DataType type, int id)
{
    SDLNet_StateData state_data;

    state_data.data = SDLNet_CreateData(type);
    state_data.id = id;
    for (int i = 0; i < NUM_OBJECTS; i++)
    {
        state_data.objects[i] = objects[i];
    }
    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        state_data.players[i] = players[i];
    }

    return state_data;
}

SDLNet_PlayerData SDLNet_CreatePlayerData(SDLNet_DataType type, Player player)
{
    SDLNet_PlayerData player_data;

    player_data.data = SDLNet_CreateData(type);
    player_data.player = player;

    return player_data;
}

SDLNet_MoveData SDLNet_CreateMoveData(SDLNet_DataType type, int id, double dx, double dy)
{
    SDLNet_MoveData move_data;

    move_data.data = SDLNet_CreateData(type);
    move_data.id = id;
    move_data.dx = dx;
    move_data.dy = dy;

    return move_data;
}

SDLNet_PosData SDLNet_CreatePosData(SDLNet_DataType type, int id, double pos_x, double pos_y)
{
    SDLNet_PosData pos_data;

    pos_data.data = SDLNet_CreateData(type);
    pos_data.id = id;
    pos_data.pos_x = pos_x;
    pos_data.pos_y = pos_y;

    return pos_data;
}

int SDLNet_TCP_SendData(TCPsocket sock, SDLNet_Data *data, int len)
{
    IPaddress *address = SDLNet_TCP_GetPeerAddress(sock);
    const char *host = SDLNet_ResolveIP(address);
    unsigned short port = SDLNet_Read16(&address->port);

    SDL_Log("TCP: Sending %d bytes of type %d to %s:%d", len, data->type, host, port);

    return SDLNet_TCP_Send(sock, data, len);
}

SDLNet_Data *SDLNet_TCP_RecvData(TCPsocket sock, int *len)
{
    *len = SDLNet_TCP_Recv(sock, buffer, sizeof(buffer));

    SDLNet_Data *data = (SDLNet_Data *)buffer;

    IPaddress *address = SDLNet_TCP_GetPeerAddress(sock);
    const char *host = SDLNet_ResolveIP(address);
    unsigned short port = SDLNet_Read16(&address->port);

    SDL_Log("TCP: Received %d bytes of type %d from %s:%d", *len, data->type, host, port);

    return data;
}

int SDLNet_UDP_SendData(UDPsocket sock, UDPpacket *packet, IPaddress address, SDLNet_Data *data, int len)
{
    packet->address = address;
    packet->data = (Uint8 *)data;
    packet->len = len;

    const char *host = SDLNet_ResolveIP(&packet->address);
    unsigned short port = SDLNet_Read16(&packet->address.port);

    SDL_Log("UDP: Sending %d bytes of type %d to %s:%d", packet->len, data->type, host, port);

    return SDLNet_UDP_Send(sock, -1, packet);
}

SDLNet_Data *SDLNet_UDP_RecvData(UDPsocket sock, UDPpacket *packet, int *recv)
{
    *recv = SDLNet_UDP_Recv(sock, packet);

    SDLNet_Data *data = (SDLNet_Data *)packet->data;

    const char *host = SDLNet_ResolveIP(&packet->address);
    unsigned short port = SDLNet_Read16(&packet->address.port);

    SDL_Log("UDP: Received %d bytes of type %d from %s:%d", packet->len, data->type, host, port);

    return data;
}
