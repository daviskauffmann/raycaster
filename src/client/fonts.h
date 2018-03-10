#ifndef FONTS_H
#define FONTS_H

#include <SDL/SDL_ttf.h>
#include <SDL/SDL.h>

int fonts_init(void);
TTF_Font *fonts_load(const char *file, int ptsize);
void fonts_unload(TTF_Font *font);
void fonts_quit(void);

#endif
