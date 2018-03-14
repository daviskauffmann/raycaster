#ifndef NET_H
#define NET_H

#include <SDL/SDL_net.h>

#define PACKET_SIZE 1024

#define SDLNet_TCP_AllocPacket(size) _SDLNet_TCP_AllocPacket(size)
#define SDLNet_TCP_FreePacket(packet) _SDLNet_TCP_FreePacket(packet)
#define SDLNet_UDP_AllocPacket(size) SDLNet_AllocPacket(size)
#define SDLNet_UDP_FreePacket(packet) SDLNet_FreePacket(packet)

typedef struct
{
    int maxlen;
    unsigned char *data;
    int len;
} TCPpacket;

TCPpacket *_SDLNet_TCP_AllocPacket(int size);
void _SDLNet_TCP_FreePacket(TCPpacket *packet);
void tcp_send(TCPsocket socket, const char *fmt, ...);
int tcp_recv(TCPsocket socket, TCPpacket *packet);
void udp_send(UDPsocket socket, UDPpacket *packet, IPaddress address, const char *fmt, ...);
int udp_recv(UDPsocket socket, UDPpacket *packet);

#endif
