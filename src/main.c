#include <math.h>
#include <SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define TITLE "Raycaster"
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

#define TEX_WIDTH 64
#define TEX_HEIGHT 64

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

    // textures
    unsigned int textures[8][TEX_WIDTH][TEX_HEIGHT];
    for (int x = 0; x < TEX_WIDTH; x++)
    {
        for (int y = 0; y < TEX_HEIGHT; y++)
        {
            int xorcolor = (x * 256 / TEX_WIDTH) ^ (y * 256 / TEX_HEIGHT);
            //int xcolor = x * 256 / texWidth;
            int ycolor = y * 256 / TEX_HEIGHT;
            int xycolor = y * 128 / TEX_HEIGHT + x * 128 / TEX_WIDTH;
            textures[0][x][y] = 65536 * 254 * (x != y && x != TEX_WIDTH - y); //flat red texture with black cross
            textures[1][x][y] = xycolor + 256 * xycolor + 65536 * xycolor;    //sloped greyscale
            textures[2][x][y] = 256 * xycolor + 65536 * xycolor;              //sloped yellow gradient
            textures[3][x][y] = xorcolor + 256 * xorcolor + 65536 * xorcolor; //xor greyscale
            textures[4][x][y] = 256 * xorcolor;                               //xor green
            textures[5][x][y] = 65536 * 192 * (x % 16 && y % 16);             //red bricks
            textures[6][x][y] = 65536 * ycolor;                               //red gradient
            textures[7][x][y] = 128 + 256 * 128 + 65536 * 128;                //flat grey texture
        }
    }

    // map
    char map[MAP_WIDTH][MAP_HEIGHT] =
        {
            {4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 7, 7, 7, 7, 7, 7, 7, 7},
            {4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 7},
            {4, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7},
            {4, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7},
            {4, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 7},
            {4, 0, 4, 0, 0, 0, 0, 5, 5, 5, 5, 5, 5, 5, 5, 5, 7, 7, 0, 7, 7, 7, 7, 7},
            {4, 0, 5, 0, 0, 0, 0, 5, 0, 5, 0, 5, 0, 5, 0, 5, 7, 0, 0, 0, 7, 7, 7, 1},
            {4, 0, 6, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 5, 7, 0, 0, 0, 0, 0, 0, 8},
            {4, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 7, 7, 1},
            {4, 0, 8, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 5, 7, 0, 0, 0, 0, 0, 0, 8},
            {4, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 5, 7, 0, 0, 0, 7, 7, 7, 1},
            {4, 0, 0, 0, 0, 0, 0, 5, 5, 5, 5, 0, 5, 5, 5, 5, 7, 7, 7, 7, 7, 7, 7, 1},
            {6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 0, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
            {8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4},
            {6, 6, 6, 6, 6, 6, 0, 6, 6, 6, 6, 0, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
            {4, 4, 4, 4, 4, 4, 0, 4, 4, 4, 6, 0, 6, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3},
            {4, 0, 0, 0, 0, 0, 0, 0, 0, 4, 6, 0, 6, 2, 0, 0, 0, 0, 0, 2, 0, 0, 0, 2},
            {4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 2, 0, 0, 5, 0, 0, 2, 0, 0, 0, 2},
            {4, 0, 0, 0, 0, 0, 0, 0, 0, 4, 6, 0, 6, 2, 0, 0, 0, 0, 0, 2, 2, 0, 2, 2},
            {4, 0, 6, 0, 6, 0, 0, 0, 0, 4, 6, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 2},
            {4, 0, 0, 5, 0, 0, 0, 0, 0, 4, 6, 0, 6, 2, 0, 0, 0, 0, 0, 2, 2, 0, 2, 2},
            {4, 0, 6, 0, 6, 0, 0, 0, 0, 4, 6, 0, 6, 2, 0, 0, 5, 0, 0, 2, 0, 0, 0, 2},
            {4, 0, 0, 0, 0, 0, 0, 0, 0, 4, 6, 0, 6, 2, 0, 0, 0, 0, 0, 2, 0, 0, 0, 2},
            {4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 1, 1, 1, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3}};

    // player
    double player_x = 22.0;
    double player_y = 11.5;
    double player_a = -(M_PI / 2);
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
        // clear screen
        // for (int x = 0; x < SCREEN_WIDTH; x++)
        // {
        //     for (int y = 0; y < SCREEN_HEIGHT; y++)
        //     {
        //         pixels[y * SCREEN_WIDTH + x] = 0;
        //     }
        // }

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
        if (lshift_down)
        {
            player_move_speed = player_sprint_speed;
        }
        else
        {
            player_move_speed = player_walk_speed;
        }

        if ((w_down && d_down) || (w_down && a_down) || (s_down && d_down) || (s_down && a_down))
        {
            player_move_speed /= sqrt(2);
        }

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

            // texture sample coordinates
            double sample_x = 0.0;

            // store the length of the ray, as well as the x and y coordinates of the map where the ray is
            double ray_length = 0.0;
            int map_x;
            int map_y;

            // cast the ray
            bool hit = false;
            bool boundary = false;
            while (!hit && ray_length < DEPTH)
            {
                // move the way forward
                ray_length += RESOLUTION;

                // get map coordinate from current ray position
                map_x = (int)(player_x + ray_x * ray_length);
                map_y = (int)(player_y + ray_y * ray_length);

                // test if ray is out of bounds
                if (map_x < 0 || map_x >= MAP_WIDTH || map_y < 0 || map_y >= MAP_HEIGHT)
                {
                    hit = true;
                    ray_length = DEPTH;
                }
                else
                {
                    // test if there is a wall
                    if (map[map_x][map_y] > 0)
                    {
                        hit = true;

                        // determine tile midpoint
                        double mid_x = (double)map_x + 0.5;
                        double mid_y = (double)map_y + 0.5;

                        // determine the collision point
                        double point_x = player_x + ray_x * ray_length;
                        double point_y = player_y + ray_y * ray_length;

                        // calculate angle between the two points
                        double angle = atan2(point_y - mid_y, point_x - mid_x);

                        // calculate sample point
                        if (angle >= -M_PI * 0.25 && angle < M_PI * 0.25)
                        {
                            sample_x = point_y - (double)map_y;
                        }
                        if (angle >= M_PI * 0.25 && angle < M_PI * 0.75)
                        {
                            sample_x = point_x - (double)map_x;
                        }
                        if (angle < -M_PI * 0.25 && angle >= -M_PI * 0.75)
                        {
                            sample_x = point_x - (double)map_x;
                        }
                        if (angle >= M_PI * 0.75 || angle < -M_PI * 0.75)
                        {
                            sample_x = point_y - (double)map_y;
                        }
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

                    // find the sample y
                    double sample_y = ((double)y - (double)ceiling_y) / ((double)floor_y - (double)ceiling_y);

                    // determine which texture based on map coordinates
                    int tex_num = map[map_x][map_y] - 1;

                    // get the pixel from the texture based on the sample coordinates
                    int tex_x = (int)(sample_y * TEX_HEIGHT);
                    int tex_y = (int)(sample_x * TEX_WIDTH);

                    // set the color to that
                    if (tex_x < TEX_WIDTH && tex_y < TEX_HEIGHT)
                    {
                        wall_color = textures[tex_num][tex_x][tex_y];
                    }

                    // draw the color
                    pixels[y * SCREEN_WIDTH + x] = wall_color;
                }
                // this is a floor
                else
                {
                    // determine floor color
                    unsigned int floor_color = 0xff00ff00;

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
