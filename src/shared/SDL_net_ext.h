/* 
 * A custom extension to SDL_net.
 */

#ifndef SDL_NET_EXT_H
#define SDL_NET_EXT_H

#include <SDL/SDL.h>
#include <SDL/SDL_net.h>

#define SDLNet_TCP_AllocPacket(size) _SDLNet_TCP_AllocPacket(size)
#define SDLNet_TCP_FreePacket(packet) _SDLNet_TCP_FreePacket(packet)
#define SDLNet_UDP_AllocPacket(size) SDLNet_AllocPacket(size)
#define SDLNet_UDP_FreePacket(packet) SDLNet_FreePacket(packet)

typedef struct
{
    Uint8 *data;
    int len;
    int maxlen;
} TCPpacket;

TCPpacket *_SDLNet_TCP_AllocPacket(int size);
void _SDLNet_TCP_FreePacket(TCPpacket *packet);

int SDLNet_TCP_SendExt(TCPsocket sock, void *data, int len);
int SDLNet_TCP_RecvExt(TCPsocket sock, TCPpacket *packet);

int SDLNet_UDP_SendExt(UDPsocket sock, UDPpacket *packet, IPaddress address, void *data, int len);
int SDLNet_UDP_RecvExt(UDPsocket sock, UDPpacket *packet);

#endif
