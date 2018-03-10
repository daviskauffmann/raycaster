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

int window_init(const char *title, int width, int height)
{
    w = width;
    h = height;

    window = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        w,
        h,
        SDL_WINDOW_FULLSCREEN_DESKTOP);

    if (!window)
    {
        SDL_Log("SDL_CreateWindow: %s", SDL_GetError());

        return 1;
    }

    renderer = SDL_CreateRenderer(
        window,
        -1,
        0);

    if (!renderer)
    {
        SDL_Log("SDL_CreateRenderer: %s", SDL_GetError());

        return 1;
    }

    screen = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ABGR8888,
        SDL_TEXTUREACCESS_STREAMING,
        w,
        h);

    if (!screen)
    {
        SDL_Log("SDL_CreateTexture: %s", SDL_GetError());

        return 1;
    }

    pixels = malloc(w * h * sizeof(unsigned int));

    // TODO: move this somewhere else
    SDL_SetRelativeMouseMode(SDL_TRUE);

    SDL_Log("Window initialized");

    return 0;
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
