#include <math.h>
#include <SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define TITLE "Raycaster"
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

#define MAP_WIDTH 24
#define MAP_HEIGHT 24

#define FOV 90
#define DEPTH 64.0
#define RESOLUTION 0.01

int main(int argc, char *args[])
{
    // SDL
    SDL_Init(SDL_INIT_VIDEO);
    SDL_SetRelativeMouseMode(SDL_TRUE);
    SDL_Window *window = SDL_CreateWindow(
        TITLE,
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer = SDL_CreateRenderer(
        window,
        -1,
        0);
    SDL_Texture *texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ABGR8888,
        SDL_TEXTUREACCESS_STATIC,
        SCREEN_WIDTH,
        SCREEN_HEIGHT);

    // map
    char map[MAP_WIDTH][MAP_HEIGHT] =
        {
            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 2, 2, 2, 2, 2, 0, 0, 0, 0, 3, 0, 3, 0, 3, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 2, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 2, 0, 0, 0, 2, 0, 0, 0, 0, 3, 0, 0, 0, 3, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 2, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 2, 2, 0, 2, 2, 0, 0, 0, 0, 3, 0, 3, 0, 3, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 4, 4, 4, 4, 4, 4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 4, 0, 4, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 4, 0, 0, 0, 0, 5, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 4, 0, 4, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 4, 0, 4, 4, 4, 4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 4, 4, 4, 4, 4, 4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}};

    // player
    double player_x = 22.0;
    double player_y = 12.0;
    double player_a = 0.0;
    double player_walk_speed = 5.0;
    double player_sprint_speed = 10.0;
    double player_move_speed = player_walk_speed;
    double player_turn_speed = 3;

    // timing
    unsigned int old_time = SDL_GetTicks();
    unsigned int current_time = 0;
    double delta_time = 0.0;

    // input
    bool w_down = false;
    bool a_down = false;
    bool s_down = false;
    bool d_down = false;
    bool lshift_down = false;

    // rendering
    unsigned int *pixels = malloc(SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(unsigned int));

    bool quit = false;
    while (!quit)
    {
        // calculate delta time
        old_time = current_time;
        current_time = SDL_GetTicks();
        delta_time = (current_time - old_time) / 1000.0;

        char buffer[256];
        snprintf(buffer, sizeof(buffer), "%s - FPS: %d", TITLE, (int)(1 / delta_time));
        SDL_SetWindowTitle(window, buffer);

        // read input
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_MOUSEMOTION:
            {
                int x = event.motion.xrel;

                player_a += (double)x / 1000.0 * player_turn_speed;
            }
            break;
            case SDL_KEYDOWN:
            {
                SDL_Keycode key = event.key.keysym.sym;

                switch (key)
                {
                case SDLK_w:
                {
                    w_down = true;
                }
                break;
                case SDLK_a:
                {
                    a_down = true;
                }
                break;
                case SDLK_s:
                {
                    s_down = true;
                }
                break;
                case SDLK_d:
                {
                    d_down = true;
                }
                break;
                case SDLK_LSHIFT:
                {
                    lshift_down = true;
                }
                break;
                }
            }
            break;
            case SDL_KEYUP:
            {
                SDL_Keycode key = event.key.keysym.sym;

                switch (key)
                {
                case SDLK_w:
                {
                    w_down = false;
                }
                break;
                case SDLK_a:
                {
                    a_down = false;
                }
                break;
                case SDLK_s:
                {
                    s_down = false;
                }
                break;
                case SDLK_d:
                {
                    d_down = false;
                }
                break;
                case SDLK_LSHIFT:
                {
                    lshift_down = false;
                }
                break;
                }
            }
            break;
            case SDL_QUIT:
            {
                quit = true;
            }
            break;
            }
        }

        // apply input to player
        if (w_down)
        {
            double dx = sin(player_a) * player_move_speed * delta_time;
            double dy = cos(player_a) * player_move_speed * delta_time;

            if (map[(int)(player_x + dx)][(int)player_y] == 0)
            {
                player_x += dx;
            }

            if (map[(int)player_x][(int)(player_y + dy)] == 0)
            {
                player_y += dy;
            }
        }

        if (a_down)
        {
            double dx = cos(-player_a) * player_move_speed * delta_time;
            double dy = sin(-player_a) * player_move_speed * delta_time;

            if (map[(int)(player_x - dx)][(int)player_y] == 0)
            {
                player_x -= dx;
            }

            if (map[(int)player_x][(int)(player_y - dy)] == 0)
            {
                player_y -= dy;
            }
        }

        if (s_down)
        {
            double dx = sin(player_a) * player_move_speed * delta_time;
            double dy = cos(player_a) * player_move_speed * delta_time;

            if (map[(int)(player_x - dx)][(int)player_y] == 0)
            {
                player_x -= dx;
            }

            if (map[(int)player_x][(int)(player_y - dy)] == 0)
            {
                player_y -= dy;
            }
        }

        if (d_down)
        {
            double dx = cos(-player_a) * player_move_speed * delta_time;
            double dy = sin(-player_a) * player_move_speed * delta_time;

            if (map[(int)(player_x + dx)][(int)player_y] == 0)
            {
                player_x += dx;
            }

            if (map[(int)player_x][(int)(player_y + dy)] == 0)
            {
                player_y += dy;
            }
        }

        if (lshift_down)
        {
            player_move_speed = player_sprint_speed;
        }
        else
        {
            player_move_speed = player_walk_speed;
        }

        // calculate pixels
        double fov = FOV * M_PI / 180.0;
        for (int x = 0; x < SCREEN_WIDTH; x++)
        {
            // calculate the ray angle for each column
            double ray_angle = player_a + atan((2.0 * (double)x / (double)SCREEN_WIDTH - 1.0) * tan(fov / 2.0));
            // double ray_angle = (player_a - fov / 2.0) + fov * ((double)x / (double)SCREEN_WIDTH);

            // unit vector for ray
            double ray_x = sin(ray_angle);
            double ray_y = cos(ray_angle);

            // cast the ray
            double ray_length = 0.0;
            bool hit = false;
            bool boundary = false;
            while (!hit && ray_length < DEPTH)
            {
                // move the way forward
                ray_length += RESOLUTION;

                // get map coordinate from current ray position
                int x = (int)(player_x + ray_x * ray_length);
                int y = (int)(player_y + ray_y * ray_length);

                // test if ray is out of bounds
                if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT)
                {
                    hit = true;
                    ray_length = DEPTH;
                }
                else
                {
                    // test if there is a wall
                    if (map[x][y] > 0)
                    {
                        hit = true;
                    }
                }
            }

            double distance = ((ray_x * sin(player_a)) + (ray_y * cos(player_a))) * ray_length;

            // calculate the y coordinate where the ceiling and floor starts
            int ceiling_y = ((double)SCREEN_HEIGHT / 2.0) * (1.0 - 1.0 / distance);
            // int ceiling_y = ((double)SCREEN_HEIGHT / 2.0) - ((double)SCREEN_HEIGHT / ray_length);
            int floor_y = SCREEN_HEIGHT - ceiling_y;

            for (int y = 0; y < SCREEN_HEIGHT; y++)
            {
                // check if this is a ceiling
                if (y < ceiling_y)
                {
                    // determine ceiling color
                    unsigned int ceiling_color = 0xffff0000;

                    // draw the color
                    pixels[y * SCREEN_WIDTH + x] = ceiling_color;
                }
                // check if this is a wall
                else if (y >= ceiling_y && y <= floor_y)
                {
                    // determine wall color
                    unsigned int wall_color = 0x00000000;
                    if (ray_length <= DEPTH / 4.0)
                    {
                        wall_color = 0xffffffff;
                    }
                    else if (ray_length < DEPTH / 3.0)
                    {
                        wall_color = 0xffeeeeee;
                    }
                    else if (ray_length < DEPTH / 2.0)
                    {
                        wall_color = 0xffdddddd;
                    }
                    else
                    {
                        wall_color = 0xffcccccc;
                    }

                    // draw the color
                    pixels[y * SCREEN_WIDTH + x] = wall_color;
                }
                // this is a floor
                else
                {
                    // determine floor color
                    unsigned int floor_color = 0x00000000;
                    double b = 1.0 - ((double)y - (double)SCREEN_HEIGHT / 2.0) / ((double)SCREEN_HEIGHT / 2.0);
                    if (b < 0.25)
                    {
                        floor_color = 0xff00ff00;
                    }
                    else if (b < 0.5)
                    {
                        floor_color = 0xff00ee00;
                    }
                    else if (b < 0.75)
                    {
                        floor_color = 0xff00dd00;
                    }
                    else
                    {
                        floor_color = 0xff00cc00;
                    }

                    // draw the color
                    pixels[y * SCREEN_WIDTH + x] = floor_color;
                }
            }
        }

        // draw to screen
        SDL_UpdateTexture(
            texture,
            NULL,
            pixels,
            SCREEN_WIDTH * sizeof(unsigned int));
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }

    free(pixels);

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
