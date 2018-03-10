#include <SDL/SDL_net.h>
#include <SDL/SDL.h>
#include <stdbool.h>

#include "../shared/utils.h"

#include "system.h"

#define SERVER_PORT 1000
#define PACKET_SIZE 256

IPaddress ip;
TCPsocket tcp_server = NULL;

UDPsocket udp_server = NULL;
UDPpacket *packet = NULL;

int main(int argc, char *args[])
{
    // suppress warning C4100
    (void)argc;
    (void)args;

    if (system_init() != 0)
    {
        SDL_Log("Initialization failed");

        return 1;
    }

    /* Net */
    if (SDLNet_Init() != 0)
    {
        SDL_Log("SDLNet_Init: %s", SDLNet_GetError());

        return 1;
    }

    /* TCP */
    if (SDLNet_ResolveHost(&ip, NULL, SERVER_PORT) != 0)
    {
        SDL_Log("SDLNet_ResolveHost: %s", SDLNet_GetError());

        return 1;
    }

    tcp_server = SDLNet_TCP_Open(&ip);

    if (!tcp_server)
    {
        SDL_Log("SDLNet_TCP_Open: %s", SDLNet_GetError());

        return 1;
    }

    SDL_Log("Listening on port %d", SERVER_PORT);

    /* UDP */
    udp_server = SDLNet_UDP_Open(SERVER_PORT);

    if (!udp_server)
    {
        SDL_Log("SDLNet_UDP_Open: %s", SDLNet_GetError());

        return 1;
    }

    packet = SDLNet_AllocPacket(PACKET_SIZE);

    if (!packet)
    {
        SDL_Log("SDLNet_AllocPacket: %s", SDLNet_GetError());

        return 1;
    }

    bool quit = false;
    while (!quit)
    {
        TCPsocket client = SDLNet_TCP_Accept(tcp_server);

        if (client)
        {
            SDL_Log("Connected to client %s:%i", SDLNet_ResolveIP(&ip), SDLNet_Read16(&ip.port));

            SDLNet_TCP_Close(client);
        }
    }

    SDLNet_FreePacket(packet);
    SDLNet_UDP_Close(udp_server);
    SDLNet_TCP_Close(tcp_server);
    SDLNet_Quit();

    system_quit();

    return 0;
}
