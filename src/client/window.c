#include <SDL/SDL_image.h>
#include <SDL/SDL.h>

#include "../shared/utils.h"

#include "window.h"

int w = 0;
int h = 0;

internal SDL_Window *window = NULL;
internal SDL_Renderer *renderer = NULL;
internal SDL_Texture *screen = NULL;
internal unsigned int *pixels;

void window_init(const char *title, int width, int height)
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

    w = width;
    h = height;

    pixels = malloc(w * h * sizeof(unsigned int));

    window = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        w,
        h,
        SDL_WINDOW_FULLSCREEN_DESKTOP);

    renderer = SDL_CreateRenderer(
        window,
        -1,
        0);

    screen = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ABGR8888,
        SDL_TEXTUREACCESS_STREAMING,
        w,
        h);

    // TODO: move this somewhere else
    SDL_SetRelativeMouseMode(SDL_TRUE);
}

void window_pset(int x, int y, unsigned int color)
{
    pixels[x + y * w] = color;
}

unsigned int window_pget(int x, int y)
{
    return pixels[x + y * w];
}

void window_update(void)
{
    SDL_RenderClear(renderer);
    SDL_UpdateTexture(
        screen,
        NULL,
        pixels,
        w * sizeof(unsigned int));
    SDL_RenderCopy(renderer, screen, NULL, NULL);
    SDL_RenderPresent(renderer);
}

void window_close(void)
{
    free(pixels);
    SDL_DestroyTexture(screen);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
