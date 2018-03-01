#include <SDL/SDL_image.h>
#include <SDL/SDL.h>

#include "textures.h"
#include "utils.h"

internal unsigned int get_pixel(SDL_Surface *surface, int x, int y);
internal void set_pixel(SDL_Surface *surface, int x, int y, unsigned int pixel);

void textures_init()
{
    IMG_Init(IMG_INIT_PNG);
}

texture_t *textures_load(const char *file)
{
    SDL_Surface *surface = IMG_Load(file);

    texture_t *texture = malloc(sizeof(texture_t));
    texture->w = surface->w;
    texture->h = surface->h;
    texture->pixels = malloc(texture->w * surface->h * sizeof(unsigned int));
    for (int x = 0; x < texture->w; x++)
    {
        for (int y = 0; y < texture->h; y++)
        {
            texture->pixels[x + y * texture->w] = get_pixel(surface, x, y);
        }
    }

    SDL_FreeSurface(surface);

    return texture;
}

void textures_unload(texture_t *texture)
{
    free(texture->pixels);
    free(texture);
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
