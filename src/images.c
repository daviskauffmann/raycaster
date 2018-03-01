#include <SDL/SDL_image.h>
#include <SDL/SDL.h>

#include "images.h"
#include "utils.h"

internal unsigned int get_pixel(SDL_Surface *surface, int x, int y);
internal void set_pixel(SDL_Surface *surface, int x, int y, unsigned int pixel);

void images_init()
{
    IMG_Init(IMG_INIT_PNG);
}

image_t *images_load(const char *file)
{
    SDL_Surface *surface = IMG_Load(file);

    image_t *image = malloc(sizeof(image_t));
    image->w = surface->w;
    image->h = surface->h;
    image->pixels = malloc(image->w * surface->h * sizeof(unsigned int));
    for (int x = 0; x < image->w; x++)
    {
        for (int y = 0; y < image->h; y++)
        {
            image->pixels[x + y * image->w] = get_pixel(surface, x, y);
        }
    }

    SDL_FreeSurface(surface);

    return image;
}

void images_unload(image_t *image)
{
    free(image->pixels);
    free(image);
}

void images_quit(void)
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
