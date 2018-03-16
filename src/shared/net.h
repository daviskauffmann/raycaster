#ifndef NET_H
#define NET_H

#include <SDL/SDL_net.h>

#define PACKET_SIZE 1024

#define SDLNet_TCP_AllocPacket(size) _SDLNet_TCP_AllocPacket(size)
#define SDLNet_TCP_FreePacket(packet) _SDLNet_TCP_FreePacket(packet)
#define SDLNet_UDP_AllocPacket(size) SDLNet_AllocPacket(size)
#define SDLNet_UDP_FreePacket(packet) SDLNet_FreePacket(packet)

typedef enum {
    PACKET_ENTER,
    PACKET_FULL,
    PACKET_CONNECT,
    PACKET_MOVEMENT,
    PACKET_ROTATION,
    PACKET_DISCONNECT
} PacketType;

typedef struct
{
    Uint8 *data;
    int len;
    int maxlen;
} TCPpacket;

TCPpacket *_SDLNet_TCP_AllocPacket(int size);
void _SDLNet_TCP_FreePacket(TCPpacket *packet);
int tcp_send(TCPsocket socket, const char *fmt, ...);
int tcp_recv(TCPsocket socket, TCPpacket *packet);
int udp_send(UDPsocket socket, UDPpacket *packet, IPaddress address, const char *fmt, ...);
int udp_recv(UDPsocket socket, UDPpacket *packet);

#endif
