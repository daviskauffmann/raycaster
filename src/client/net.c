#include <SDL/SDL_net.h>
#include <SDL/SDL.h>

#include "net.h"

#define HOST "127.0.0.1"
#define PORT 1000

TCPsocket socket = NULL;

void net_init(void)
{
    SDLNet_Init();

    IPaddress address;
    SDLNet_ResolveHost(&address, HOST, PORT);

    socket = SDLNet_TCP_Open(&address);
    if (socket)
    {
        SDL_Log("Connected to server %s:%i", SDLNet_ResolveIP(&address), SDLNet_Read16(&address.port));
    }
    else
    {
        SDL_Log("Failed to connect to server: %s", SDLNet_GetError());

        return;
    }

    const char message[] = "Hello, World!";
    SDLNet_TCP_Send(socket, message, sizeof(message));
}

void net_quit(void)
{
    if (socket)
    {
        SDLNet_TCP_Close(socket);
    }

    SDLNet_Quit();
}
