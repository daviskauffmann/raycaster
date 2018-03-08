#include <SDL/SDL.h>

#include "input.h"

void input_handle(
    void (*fn_keydown)(SDL_KeyboardEvent key),
    void (*fn_keyup)(SDL_KeyboardEvent key),
    void (*fn_mousedown)(SDL_MouseButtonEvent button),
    void (*fn_mouseup)(SDL_MouseButtonEvent button),
    void (*fn_mousemotion)(SDL_MouseMotionEvent motion),
    void (*fn_quit)(void))
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_KEYDOWN:
            fn_keydown(event.key);
            break;
        case SDL_KEYUP:
            fn_keyup(event.key);
            break;
        case SDL_MOUSEBUTTONDOWN:
            fn_mousedown(event.button);
            break;
        case SDL_MOUSEBUTTONUP:
            fn_mouseup(event.button);
            break;
        case SDL_MOUSEMOTION:
            fn_mousemotion(event.motion);
            break;
        case SDL_QUIT:
            fn_quit();
            break;
        }
    }
}
