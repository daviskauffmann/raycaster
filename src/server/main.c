#include <SDL/SDL_net.h>
#include <SDL/SDL.h>

#include "../shared/maths.h"
#include "../shared/utils.h"

#include "net.h"

int main(int argc, char *args[])
{
    // suppress warning C4100
    (void)argc;
    (void)args;

    net_init();
    net_quit();

    return 0;
}
