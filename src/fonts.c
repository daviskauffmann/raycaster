#include <SDL/SDL_ttf.h>
#include <SDL/SDL.h>

#include "fonts.h"

void fonts_init(void)
{
    TTF_Init();
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
