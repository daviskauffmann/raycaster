#include <SDL/SDL.h>

#include "system.h"

#define FLAGS SDL_INIT_VIDEO | SDL_INIT_AUDIO

int system_init(void)
{
    if (SDL_Init(FLAGS) == -1)
    {
        SDL_Log("SDL_Init: %s", SDL_GetError());

        return 1;
    }

    SDL_Log("SDL initialized");

    return 0;
}

void system_quit(void)
{
    SDL_Quit();
}
