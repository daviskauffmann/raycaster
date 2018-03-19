#include <SDL/SDL.h>
#include <SDL/SDL_net.h>

#include "SDL_net_ext.h"

TCPpacket *_SDLNet_TCP_AllocPacket(int size)
{
    TCPpacket *packet = malloc(sizeof(TCPpacket));

    packet->data = malloc(size);
    packet->maxlen = size;

    return packet;
}

void _SDLNet_TCP_FreePacket(TCPpacket *packet)
{
    free(packet->data);
    free(packet);
}

int SDLNet_TCP_SendExt(TCPsocket sock, void *data, int len)
{
    IPaddress *address = SDLNet_TCP_GetPeerAddress(sock);
    const char *host = SDLNet_ResolveIP(address);
    unsigned short port = SDLNet_Read16(&address->port);

    SDL_Log("TCP: Sending %d bytes to %s:%d", len, host, port);

    return SDLNet_TCP_Send(sock, data, len);
}

int SDLNet_TCP_RecvExt(TCPsocket sock, TCPpacket *packet)
{
    packet->len = SDLNet_TCP_Recv(sock, packet->data, packet->maxlen);

    if (packet->len > 0)
    {
        IPaddress *address = SDLNet_TCP_GetPeerAddress(sock);
        const char *host = SDLNet_ResolveIP(address);
        unsigned short port = SDLNet_Read16(&address->port);

        SDL_Log("TCP: Received %d bytes from %s:%d", packet->len, host, port);

        return 1;
    }

    return 0;
}

int SDLNet_UDP_SendExt(UDPsocket sock, UDPpacket *packet, IPaddress address, void *data, int len)
{
    packet->address = address;
    packet->data = (Uint8 *)data;
    packet->len = len;

    const char *host = SDLNet_ResolveIP(&packet->address);
    unsigned short port = SDLNet_Read16(&packet->address.port);

    SDL_Log("UDP: Sending %d bytes to %s:%d", packet->len, host, port);

    return SDLNet_UDP_Send(sock, -1, packet);
}

int SDLNet_UDP_RecvExt(UDPsocket sock, UDPpacket *packet)
{
    int recv = SDLNet_UDP_Recv(sock, packet);

    if (recv == 1)
    {
        const char *host = SDLNet_ResolveIP(&packet->address);
        unsigned short port = SDLNet_Read16(&packet->address.port);

        SDL_Log("UDP: Received %d bytes from %s:%d", packet->len, host, port);
    }

    return recv;
}
