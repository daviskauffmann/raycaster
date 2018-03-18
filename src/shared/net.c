#include <SDL/SDL.h>
#include <SDL/SDL_net.h>

#include "net.h"

Net_Data Net_CreateData(Net_DataType type)
{
    Net_Data data;

    data.type = type;

    return data;
}

Net_IdData Net_CreateIdData(Net_DataType type, int id)
{
    Net_IdData id_data;

    id_data.data = Net_CreateData(type);
    id_data.id = id;

    return id_data;
}

Net_ConnectData Net_CreateConnectData(Net_DataType type, int id, Player players[MAX_CLIENTS])
{
    Net_ConnectData connect_data;

    connect_data.data = Net_CreateData(type);
    connect_data.id = id;
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        connect_data.players[i] = players[i];
    }

    return connect_data;
}

Net_PlayerData Net_CreatePlayerData(Net_DataType type, Player player)
{
    Net_PlayerData player_data;

    player_data.data = Net_CreateData(type);
    player_data.player = player;

    return player_data;
}

Net_PosData Net_CreatePosData(Net_DataType type, int id, double pos_x, double pos_y)
{
    Net_PosData pos_data;

    pos_data.data = Net_CreateData(type);
    pos_data.id = id;
    pos_data.pos_x = pos_x;
    pos_data.pos_y = pos_y;

    return pos_data;
}

int Net_TCP_Send(TCPsocket sock, Net_Data *data, int len)
{
    IPaddress *address = SDLNet_TCP_GetPeerAddress(sock);
    const char *host = SDLNet_ResolveIP(address);
    unsigned short port = SDLNet_Read16(&address->port);

    SDL_Log("TCP: Sending %d bytes of type %d to %s:%d", len, data->type, host, port);

    return SDLNet_TCP_Send(sock, data, len);
}

int Net_UDP_Send(UDPsocket sock, UDPpacket *packet, IPaddress address, Net_Data *data, int len)
{
    packet->address = address;
    packet->data = (Uint8 *)data;
    packet->len = len;

    const char *host = SDLNet_ResolveIP(&address);
    unsigned short port = SDLNet_Read16(&address.port);

    SDL_Log("UDP: Sending %d bytes of type %d to %s:%d", len, data->type, host, port);

    return SDLNet_UDP_Send(sock, -1, packet);
}
