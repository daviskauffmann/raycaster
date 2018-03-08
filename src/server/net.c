#include <SDL/SDL_net.h>
#include <SDL/SDL.h>

#include "net.h"

#define PORT 1000

TCPsocket server = NULL;
TCPsocket client = NULL;

void net_init(void)
{
    SDLNet_Init();

    IPaddress address;
    SDLNet_ResolveHost(&address, NULL, PORT);

    server = SDLNet_TCP_Open(&address);

    SDL_Log("Listening on port %d", PORT);

    SDL_Delay(5000);

    client = SDLNet_TCP_Accept(server);

    char data[256];
    int bytesReceived = SDLNet_TCP_Recv(client, data, 256);
    data[bytesReceived] = 0;
    SDL_Log("Received %i bytes: \"%s\"", bytesReceived, data);
}

void net_quit(void)
{
    SDLNet_TCP_Close(server);
    SDLNet_Quit();
}
