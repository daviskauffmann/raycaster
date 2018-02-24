#include <math.h>
#include <SDL.h>
#include <SDL_image.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define TITLE "Raycaster"
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

#define MAP_WIDTH 24
#define MAP_HEIGHT 24

#define MOVE_SPEED 5.0
#define SPRINT_MULT 2.0
#define ROTATE_SPEED 3.14

#define FOV 66

unsigned int get_pixel(SDL_Surface *surface, int x, int y)
{
    int bpp = surface->format->BytesPerPixel;
    // Here p is the address to the pixel we want to retrieve
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
        if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
        {
            return p[0] << 16 | p[1] << 8 | p[2];
        }
        else
        {
            return p[0] | p[1] << 8 | p[2] << 16;
        }
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

void set_pixel(SDL_Surface *surface, int x, int y, unsigned int pixel)
{
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to set */
    unsigned char *p = (unsigned char *)surface->pixels + y * surface->pitch + x * bpp;

    switch (bpp)
    {
    case 1:
    {
        *p = pixel;
    }
    break;
    case 2:
    {
        *(unsigned short *)p = pixel;
    }
    break;
    case 3:
    {
        if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
        {
            p[0] = (pixel >> 16) & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = pixel & 0xff;
        }
        else
        {
            p[0] = pixel & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = (pixel >> 16) & 0xff;
        }
    }
    break;
    case 4:
    {
        *(unsigned int *)p = pixel;
    }
    break;
    }
}

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

    // SDL Image
    IMG_Init(IMG_INIT_PNG);

    // textures
    SDL_Surface *textures[8];
    textures[0] = IMG_Load("bluestone.png");
    textures[1] = IMG_Load("colorstone.png");
    textures[2] = IMG_Load("eagle.png");
    textures[3] = IMG_Load("greystone.png");
    textures[4] = IMG_Load("mossy.png");
    textures[5] = IMG_Load("purplestone.png");
    textures[6] = IMG_Load("redbrick.png");
    textures[7] = IMG_Load("wood.png");

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
    double pos_x = 22.0;
    double pos_y = 11.5;
    double dir_x = -1;
    double dir_y = 0;
    double plane_x = 0;
    double plane_y = FOV / 100.0;

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
        for (int x = 0; x < SCREEN_WIDTH; x++)
        {
            for (int y = 0; y < SCREEN_HEIGHT; y++)
            {
                pixels[y * SCREEN_WIDTH + x] = 0;
            }
        }

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
                double rotate_speed = -x / 1000.0 * ROTATE_SPEED;

                //both camera direction and camera plane must be rotated
                double old_dir_x = dir_x;
                dir_x = dir_x * cos(rotate_speed) - dir_y * sin(rotate_speed);
                dir_y = old_dir_x * sin(rotate_speed) + dir_y * cos(rotate_speed);

                double old_plane_x = plane_x;
                plane_x = plane_x * cos(rotate_speed) - plane_y * sin(rotate_speed);
                plane_y = old_plane_x * sin(rotate_speed) + plane_y * cos(rotate_speed);
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
        double move_speed = MOVE_SPEED * delta_time;     //the constant value is in squares/second
        double rotate_speed = ROTATE_SPEED * delta_time; //the constant value is in radians/second

        if (lshift_down)
        {
            move_speed *= SPRINT_MULT;
        }

        if ((w_down && d_down) || (w_down && a_down) || (s_down && d_down) || (s_down && a_down))
        {
            move_speed /= sqrt(2);
        }

        if (w_down)
        {
            double dx = dir_x * move_speed;
            double dy = dir_y * move_speed;

            if (map[(int)(pos_x + dx)][(int)(pos_y)] == 0)
                pos_x += dx;
            if (map[(int)(pos_x)][(int)(pos_y + dy)] == 0)
                pos_y += dy;
        }

        if (a_down)
        {
            double dx = -dir_y * move_speed;
            double dy = dir_x * move_speed;

            if (map[(int)(pos_x + dx)][(int)(pos_y)] == 0)
                pos_x += dx;
            if (map[(int)(pos_x)][(int)(pos_y + dy)] == 0)
                pos_y += dy;
        }

        if (s_down)
        {
            double dx = -dir_x * move_speed;
            double dy = -dir_y * move_speed;

            if (map[(int)(pos_x + dx)][(int)(pos_y)] == 0)
                pos_x += dx;
            if (map[(int)(pos_x)][(int)(pos_y + dy)] == 0)
                pos_y += dy;
        }

        if (d_down)
        {
            double dx = dir_y * move_speed;
            double dy = -dir_x * move_speed;

            if (map[(int)(pos_x + dx)][(int)(pos_y)] == 0)
                pos_x += dx;
            if (map[(int)(pos_x)][(int)(pos_y + dy)] == 0)
                pos_y += dy;
        }

        // raycasting
        for (int x = 0; x < SCREEN_WIDTH; x++)
        {
            //calculate ray position and direction
            double camera_x = 2 * x / (double)SCREEN_WIDTH - 1; //x-coordinate in camera space
            double ray_dir_x = dir_x + plane_x * camera_x;
            double ray_dir_y = dir_y + plane_y * camera_x;

            //which box of the map we're in
            int map_x = (int)pos_x;
            int map_y = (int)pos_y;

            //length of ray from current position to next x or y-side
            double side_dist_x;
            double side_dist_y;

            //length of ray from one x or y-side to next x or y-side
            double delta_dist_x = fabs(1 / ray_dir_x);
            double delta_dist_y = fabs(1 / ray_dir_y);
            double perp_wall_dist;

            //what direction to step in x or y-direction (either +1 or -1)
            int step_x;
            int step_y;

            int hit = 0; //was there a wall hit?
            int side;    //was a NS or a EW wall hit?

            //calculate step and initial sideDist
            if (ray_dir_x < 0)
            {
                step_x = -1;
                side_dist_x = (pos_x - map_x) * delta_dist_x;
            }
            else
            {
                step_x = 1;
                side_dist_x = (map_x + 1.0 - pos_x) * delta_dist_x;
            }
            if (ray_dir_y < 0)
            {
                step_y = -1;
                side_dist_y = (pos_y - map_y) * delta_dist_y;
            }
            else
            {
                step_y = 1;
                side_dist_y = (map_y + 1.0 - pos_y) * delta_dist_y;
            }

            //perform DDA
            while (hit == 0)
            {
                //jump to next map square, OR in x-direction, OR in y-direction
                if (side_dist_x < side_dist_y)
                {
                    side_dist_x += delta_dist_x;
                    map_x += step_x;
                    side = 0;
                }
                else
                {
                    side_dist_y += delta_dist_y;
                    map_y += step_y;
                    side = 1;
                }

                //Check if ray has hit a wall
                if (map[map_x][map_y] > 0)
                {
                    hit = 1;
                }
            }

            //Calculate distance projected on camera direction (Euclidean distance will give fisheye effect!)
            if (side == 0)
            {
                perp_wall_dist = (map_x - pos_x + (1 - step_x) / 2) / ray_dir_x;
            }
            else
            {
                perp_wall_dist = (map_y - pos_y + (1 - step_y) / 2) / ray_dir_y;
            }

            //Calculate height of line to draw on screen
            int line_height = (int)((double)SCREEN_HEIGHT / perp_wall_dist);

            //calculate lowest and highest pixel to fill in current stripe
            int draw_start = -line_height / 2 + SCREEN_HEIGHT / 2;
            if (draw_start < 0)
            {
                draw_start = 0;
            }
            int draw_end = line_height / 2 + SCREEN_HEIGHT / 2;
            if (draw_end >= SCREEN_HEIGHT)
            {
                draw_end = SCREEN_HEIGHT - 1;
            }

#if 0
            //choose wall color
            unsigned int color;
            switch (map[map_x][map_y])
            {
            case 1:
                color = 0xff0000ff;
                break; //red
            case 2:
                color = 0xff00ff00;
                break; //green
            case 3:
                color = 0xffff0000;
                break; //blue
            case 4:
                color = 0xffffffff;
                break; //white
            default:
                color = 0xff00ffff;
                break; //yellow
            }

            //give x and y sides different brightness
            //make color darker for y-sides: R, G and B byte each divided through two with a "shift" and an "and"
            if (side == 1)
            {
                color = (color >> 1) & 0x7f7f7f;
            }

            // draw the pixels of the stripe as a vertical line
            for (int y = draw_start; y < draw_end; y++)
            {
                pixels[y * SCREEN_WIDTH + x] = color;
            }
#else
            // texturing calculations
            char tex_index = map[map_x][map_y] - 1; // 1 subtracted from it so that texture 0 can be used!
            SDL_Surface *tex = textures[tex_index];

            // calculate value of wall_x
            double wall_x; //where exactly the wall was hit
            if (side == 0)
            {
                wall_x = pos_y + perp_wall_dist * ray_dir_y;
            }
            else
            {
                wall_x = pos_x + perp_wall_dist * ray_dir_x;
            }
            wall_x -= floor(wall_x);

            // x coordinate on the texture
            int tex_x = (int)(wall_x * (double)tex->w);
            if (side == 0 && ray_dir_x > 0)
            {
                tex_x = tex->w - tex_x - 1;
            }
            if (side == 1 && ray_dir_y < 0)
            {
                tex_x = tex->w - tex_x - 1;
            }

            // draw the pixels of the stripe as a vertical line
            for (int y = draw_start; y < draw_end; y++)
            {
                int d = y * 256 - SCREEN_HEIGHT * 128 + line_height * 128; //256 and 128 factors to avoid floats
                int tex_y = ((d * tex->h) / line_height) / 256;

                SDL_LockSurface(tex);
                unsigned int tex_pixel = get_pixel(tex, tex_x, tex_y);
                SDL_UnlockSurface(tex);

                // make color darker for y-sides: R, G and B byte each divided through two with a "shift" and an "and"
                if (side == 1)
                {
                    tex_pixel = (tex_pixel >> 1) & 0x7f7f7f;
                }

                pixels[y * SCREEN_WIDTH + x] = tex_pixel;
            }
#endif

            // draw the floor
            unsigned int grey = 0xff646464;

            for (int y = draw_end; y < SCREEN_HEIGHT; y++)
            {
                pixels[y * SCREEN_WIDTH + x] = grey;
            }

            // draw the ceiling
            grey = (grey >> 1) & 0x7f7f7f;

            for (int y = 0; y < draw_start; y++)
            {
                pixels[y * SCREEN_WIDTH + x] = grey;
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

    for (int i = 0; i < 8; i++)
    {
        SDL_Surface *tex = textures[i];
        SDL_FreeSurface(tex);
    }
    IMG_Quit();

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
