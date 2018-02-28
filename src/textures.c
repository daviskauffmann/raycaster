#include <SDL/SDL_image.h>
#include <SDL/SDL.h>

#include "textures.h"
#include "utils.h"

int tw = 0;
int th = 0;

internal unsigned int get_pixel(SDL_Surface *surface, int x, int y);
internal void set_pixel(SDL_Surface *surface, int x, int y, unsigned int pixel);

void textures_init(int texture_width, int texture_height)
{
    tw = texture_width;
    th = texture_height;

    IMG_Init(IMG_INIT_PNG);
}

unsigned int *textures_load(const char *file)
{
    unsigned int *pixels = malloc(tw * th * sizeof(unsigned int));

    SDL_Surface *surface = IMG_Load(file);
    for (int tx = 0; tx < tw; tx++)
    {
        for (int ty = 0; ty < th; ty++)
        {
            pixels[tx + ty * tw] = get_pixel(surface, tx, ty);
        }
    }
    SDL_FreeSurface(surface);

    return pixels;
}

void textures_quit(void)
{
    IMG_Quit();
}

internal unsigned int get_pixel(SDL_Surface *surface, int x, int y)
{
    int bpp = surface->format->BytesPerPixel;
    unsigned char *p = (unsigned char *)surface->pixels + y * surface->pitch + x * bpp;

    switch (bpp)
    {
    case 1:
    {
        return *p;
    }
    break;
    case 2:
    {
        return *(unsigned short *)p;
    }
    break;
    case 3:
    {
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
        return p[0] << 16 | p[1] << 8 | p[2];
#else
        return p[0] | p[1] << 8 | p[2] << 16;
#endif
    }
    break;
    case 4:
    {
        return *(unsigned int *)p;
    }
    break;
    default:
    {
        return 0;
    }
    break;
    }
}

internal void set_pixel(SDL_Surface *surface, int x, int y, unsigned int pixel)
{
    int bpp = surface->format->BytesPerPixel;
    unsigned char *p = (unsigned char *)surface->pixels + y * surface->pitch + x * bpp;

    switch (bpp)
    {
    case 1:
    {
        *p = (unsigned char)pixel;
    }
    break;
    case 2:
    {
        *(unsigned short *)p = (unsigned short)pixel;
    }
    break;
    case 3:
    {
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
        p[0] = (pixel >> 16) & 0xff;
        p[1] = (pixel >> 8) & 0xff;
        p[2] = pixel & 0xff;
#else
        p[0] = pixel & 0xff;
        p[1] = (pixel >> 8) & 0xff;
        p[2] = (pixel >> 16) & 0xff;
#endif
    }
    break;
    case 4:
    {
        *(unsigned int *)p = pixel;
    }
    break;
    }
}
