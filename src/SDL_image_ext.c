#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

#include "SDL_image_ext.h"

static unsigned int get_pixel(SDL_Surface *surface, int x, int y);
static void set_pixel(SDL_Surface *surface, int x, int y, unsigned int pixel);

IMG_Image *IMG_LoadAndConvert(const char *file)
{
    SDL_Surface *surface = IMG_Load(file);

    if (!surface)
    {
        SDL_Log("IMG_Load: %s", IMG_GetError());

        return NULL;
    }

    IMG_Image *image = malloc(sizeof(IMG_Image));
    image->w = surface->w;
    image->h = surface->h;
    image->pixels = malloc(image->w * image->h * sizeof(unsigned int));
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

void IMG_FreeImage(IMG_Image *image)
{
    free(image->pixels);
    free(image);
}

static unsigned int get_pixel(SDL_Surface *surface, int x, int y)
{
    int bpp = surface->format->BytesPerPixel;

    // here p is the address to the pixel we want to retrieve
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
        return 0; // shouldn't happen, but avoids warnings
    }
    break;
    }
}

static void set_pixel(SDL_Surface *surface, int x, int y, unsigned int pixel)
{
    int bpp = surface->format->BytesPerPixel;

    // here p is the address to the pixel we want to retrieve
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