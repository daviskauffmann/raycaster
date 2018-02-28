#include <SDL/SDL_net.h>
#include <SDL/SDL.h>

#include "net.h"

void net_init(void)
{
    SDLNet_Init();
}

void net_quit(void)
{
    SDLNet_Quit();
}
