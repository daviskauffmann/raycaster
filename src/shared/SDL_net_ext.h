/* 
 * A custom extension to SDL_net.
 */

#ifndef SDL_NET_EXT_H
#define SDL_NET_EXT_H

#include <SDL/SDL.h>
#include <SDL/SDL_net.h>

#define PACKET_SIZE 1024

typedef struct
{
    Uint8 *data;
    int len;
    int maxlen;
} TCPpacket;

int SDLNet_TCP_SendExt(TCPsocket sock, void *data, int len);
void *SDLNet_TCP_RecvExt(TCPsocket sock, int *len);

int SDLNet_UDP_SendExt(UDPsocket sock, UDPpacket *packet, IPaddress address, void *data, int len);
void *SDLNet_UDP_RecvExt(UDPsocket sock, UDPpacket *packet, int *recv);

#endif
