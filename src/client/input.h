#ifndef INPUT_H
#define INPUT_H

#include <SDL/SDL.h>

void input_handle(
    void (*fn_keydown)(SDL_KeyboardEvent key),
    void (*fn_keyup)(SDL_KeyboardEvent key),
    void (*fn_mousedown)(SDL_MouseButtonEvent button),
    void (*fn_mouseup)(SDL_MouseButtonEvent button),
    void (*fn_mousemotion)(SDL_MouseMotionEvent motion),
    void (*fn_quit)(void));

#endif
