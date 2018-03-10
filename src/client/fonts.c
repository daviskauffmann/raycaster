#include <SDL/SDL_ttf.h>
#include <SDL/SDL.h>

#include "fonts.h"

int fonts_init(void)
{
    if (TTF_Init() != 0)
    {
        SDL_Log("TTF_Init: %s", TTF_GetError());

        return 1;
    }

    SDL_Log("Fonts initialized");

    return 0;
}

TTF_Font *fonts_load(const char *file, int ptsize)
{
    return TTF_OpenFont(file, ptsize);
}

void fonts_unload(TTF_Font *font)
{
    TTF_CloseFont(font);
}

void fonts_quit(void)
{
    TTF_Quit();
}
