#include <SDL/SDL.h>
#include <SDL/SDL_net.h>

#include "map.h"
#include "player.h"
#include "SDL_net_ext.h"

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

int SDLNet_UDP_SendData(UDPsocket sock, UDPpacket *packet, IPaddress address, SDLNet_Data *data, int len)
{
    packet->address = address;
    packet->data = (Uint8 *)data;
    packet->len = len;

    const char *host = SDLNet_ResolveIP(&address);
    unsigned short port = SDLNet_Read16(&address.port);

    SDL_Log("UDP: Sending %d bytes of type %d to %s:%d", len, data->type, host, port);

    return SDLNet_UDP_Send(sock, -1, packet);
}
