#include <SDL/SDL_net.h>
#include <SDL/SDL.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "net.h"

TCPpacket *_SDLNet_TCP_AllocPacket(int size)
{
    TCPpacket *packet = malloc(sizeof(TCPpacket));

    packet->maxlen = size;
    packet->data = malloc(size * sizeof(unsigned char));
    packet->len = -1;

    return packet;
}

void _SDLNet_TCP_FreePacket(TCPpacket *packet)
{
    free(packet->data);
    free(packet);
}

int tcp_send(TCPsocket socket, const char *fmt, ...)
{
    char data[PACKET_SIZE];

    va_list args;
    va_start(args, fmt);
    vsprintf_s(data, sizeof(data), fmt, args);
    va_end(args);

    int len = strlen(data) + 1;

    IPaddress *address = SDLNet_TCP_GetPeerAddress(socket);
    const char *host = SDLNet_ResolveIP(address);
    unsigned short port = SDLNet_Read16(&address->port);

    SDL_Log("TCP: Sending %d bytes to %s:%d: %s", len, host, port, data);

    return SDLNet_TCP_Send(socket, data, len);
}

int tcp_recv(TCPsocket socket, TCPpacket *packet)
{
    packet->len = SDLNet_TCP_Recv(socket, packet->data, packet->maxlen);

    if (packet->len > 0)
    {
        IPaddress *address = SDLNet_TCP_GetPeerAddress(socket);
        const char *host = SDLNet_ResolveIP(address);
        unsigned short port = SDLNet_Read16(&address->port);

        SDL_Log("TCP: Received %d bytes from %s:%d: %s", packet->len, host, port, packet->data);
    }

    return packet->len;
}

int udp_send(UDPsocket socket, UDPpacket *packet, IPaddress address, const char *fmt, ...)
{
    char data[PACKET_SIZE];

    va_list args;
    va_start(args, fmt);
    vsprintf_s(data, sizeof(data), fmt, args);
    va_end(args);

    packet->address = address;
    packet->data = (unsigned char *)data;
    packet->len = strlen(data) + 1;

    const char *host = SDLNet_ResolveIP(&packet->address);
    unsigned short port = SDLNet_Read16(&packet->address.port);

    SDL_Log("UDP: Sending %d bytes to %s:%d: %s", packet->len, host, port, packet->data);

    return SDLNet_UDP_Send(socket, -1, packet);
}

int udp_recv(UDPsocket socket, UDPpacket *packet)
{
    int recv = SDLNet_UDP_Recv(socket, packet);

    if (recv > 0)
    {
        const char *host = SDLNet_ResolveIP(&packet->address);
        unsigned short port = SDLNet_Read16(&packet->address.port);

        SDL_Log("UDP: Received %d bytes from %s:%i: %s", packet->len, host, port, packet->data);

        return packet->len;
    }

    return 0;
}
