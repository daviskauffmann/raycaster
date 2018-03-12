#include <SDL/SDL_net.h>
#include <SDL/SDL.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "net.h"

void tcp_send(TCPsocket socket, const char *fmt, ...)
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

    SDLNet_TCP_Send(socket, data, len);
}

response_t tcp_recv(TCPsocket socket)
{
    response_t response;

    response.len = SDLNet_TCP_Recv(socket, response.data, sizeof(response.data));

    if (response.len > 0)
    {
        IPaddress *address = SDLNet_TCP_GetPeerAddress(socket);
        const char *host = SDLNet_ResolveIP(address);
        unsigned short port = SDLNet_Read16(&address->port);

        SDL_Log("TCP: Received %d bytes from %s:%d: %s", response.len, host, port, response.data);
    }

    return response;
}

void udp_send(UDPsocket socket, UDPpacket *packet, IPaddress address, const char *fmt, ...)
{
    char buffer[PACKET_SIZE];

    va_list args;
    va_start(args, fmt);
    vsprintf_s(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    packet->address = address;
    packet->data = (unsigned char *)buffer;
    packet->len = strlen(buffer) + 1;

    const char *host = SDLNet_ResolveIP(&packet->address);
    unsigned short port = SDLNet_Read16(&packet->address.port);

    SDL_Log("UDP: Sending %d bytes to %s:%d: %s", packet->len, host, port, packet->data);

    SDLNet_UDP_Send(socket, -1, packet);
}

int udp_recv(UDPsocket socket, UDPpacket *packet)
{
    int res = SDLNet_UDP_Recv(socket, packet);

    if (res > 0)
    {
        const char *host = SDLNet_ResolveIP(&packet->address);
        unsigned short port = SDLNet_Read16(&packet->address.port);

        SDL_Log("UDP: Received %d bytes from %s:%i: %s", packet->len, host, port, packet->data);
    }

    return res;
}
