#ifndef NET_H
#define NET_H

#include <SDL/SDL_net.h>

#define PACKET_SIZE 256

typedef struct response_s
{
    char data[PACKET_SIZE];
    int len;
} response_t;

void tcp_send(TCPsocket socket, const char *fmt, ...);
response_t tcp_recv(TCPsocket socket);
void udp_send(UDPsocket socket, UDPpacket *packet, IPaddress address, const char *fmt, ...);
int udp_recv(UDPsocket socket, UDPpacket *packet);

#endif
