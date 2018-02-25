#include <math.h>
#include <SDL\SDL.h>
#include <SDL\SDL_image.h>
#include <SDL\SDL_net.h>
#include <SDL\SDL_ttf.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TITLE "Raycaster"
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 360

#define MAP_WIDTH 24
#define MAP_HEIGHT 24

#define NUM_OBJECTS 19

#define MOVE_SPEED 5.0
#define SPRINT_MULT 2.0
#define ROTATE_SPEED 3.0

#define FLOOR_TEXTURE_MULT 1
#define CEILING_TEXTURE_MULT 1

typedef struct object_s
{
    double x;
    double y;
    int sprite_index;
} object_t;

unsigned int get_pixel(SDL_Surface *surface, int x, int y);
void set_pixel(SDL_Surface *surface, int x, int y, unsigned int pixel);
void comb_sort(int *order, double *dist, int amount);

/* SDL */
SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
SDL_Texture *screen = NULL;
SDL_Surface *messages = NULL;

/* SDL_image */
SDL_Surface *textures[8];
SDL_Surface *sprites[3];

/* SDL_ttf */
TTF_Font *font = NULL;

/* Map */
char wall_map[MAP_WIDTH][MAP_HEIGHT] = {
    {8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 4, 4, 6, 4, 4, 6, 4, 6, 4, 4, 4, 6, 4},
    {8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4},
    {8, 0, 3, 3, 0, 0, 0, 0, 0, 8, 8, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6},
    {8, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6},
    {8, 0, 3, 3, 0, 0, 0, 0, 0, 8, 8, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4},
    {8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 4, 0, 0, 0, 0, 0, 6, 6, 6, 0, 6, 4, 6},
    {8, 8, 8, 8, 0, 8, 8, 8, 8, 8, 8, 4, 4, 4, 4, 4, 4, 6, 0, 0, 0, 0, 0, 6},
    {7, 7, 7, 7, 0, 7, 7, 7, 7, 0, 8, 0, 8, 0, 8, 0, 8, 4, 0, 4, 0, 6, 0, 6},
    {7, 7, 0, 0, 0, 0, 0, 0, 7, 8, 0, 8, 0, 8, 0, 8, 8, 6, 0, 0, 0, 0, 0, 6},
    {7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 6, 0, 0, 0, 0, 0, 4},
    {7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 6, 0, 6, 0, 6, 0, 6},
    {7, 7, 0, 0, 0, 0, 0, 0, 7, 8, 0, 8, 0, 8, 0, 8, 8, 6, 4, 6, 0, 6, 6, 6},
    {7, 7, 7, 7, 0, 7, 7, 7, 7, 8, 8, 4, 0, 6, 8, 4, 8, 3, 3, 3, 0, 3, 3, 3},
    {2, 2, 2, 2, 0, 2, 2, 2, 2, 4, 6, 4, 0, 0, 6, 0, 6, 3, 0, 0, 0, 0, 0, 3},
    {2, 2, 0, 0, 0, 0, 0, 2, 2, 4, 0, 0, 0, 0, 0, 0, 4, 3, 0, 0, 0, 0, 0, 3},
    {2, 0, 0, 0, 0, 0, 0, 0, 2, 4, 0, 0, 0, 0, 0, 0, 4, 3, 0, 0, 0, 0, 0, 3},
    {1, 0, 0, 0, 0, 0, 0, 0, 1, 4, 4, 4, 4, 4, 6, 0, 6, 3, 3, 0, 0, 0, 3, 3},
    {2, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 1, 2, 2, 2, 6, 6, 0, 0, 5, 0, 5, 0, 5},
    {2, 2, 0, 0, 0, 0, 0, 2, 2, 2, 0, 0, 0, 2, 2, 0, 5, 0, 5, 0, 0, 0, 5, 5},
    {2, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 2, 5, 0, 5, 0, 5, 0, 5, 0, 5},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5},
    {2, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 2, 5, 0, 5, 0, 5, 0, 5, 0, 5},
    {2, 2, 0, 0, 0, 0, 0, 2, 2, 2, 0, 0, 0, 2, 2, 0, 5, 0, 5, 0, 0, 0, 5, 5},
    {2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 5, 5, 5, 5, 5, 5, 5, 5, 5}};

char floor_map[MAP_WIDTH][MAP_HEIGHT] = {
    {3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
    {3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
    {3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
    {3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
    {3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
    {3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
    {3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
    {3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
    {3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
    {3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
    {3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
    {3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
    {3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
    {3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
    {3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
    {3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
    {3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
    {3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
    {3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
    {3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
    {3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
    {3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
    {3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
    {3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3}};

char ceiling_map[MAP_WIDTH][MAP_HEIGHT] = {
    {6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
    {6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
    {6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
    {6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
    {6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
    {6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
    {6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
    {6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
    {6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
    {6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
    {6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
    {6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
    {6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
    {6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
    {6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
    {6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
    {6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
    {6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
    {6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
    {6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
    {6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
    {6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
    {6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
    {6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6}};

object_t objects[NUM_OBJECTS] = {
    {20.5, 11.5, 2},
    {18.5, 4.50, 2},
    {10.0, 4.50, 2},
    {10.0, 12.5, 2},
    {3.50, 6.50, 2},
    {3.50, 20.5, 2},
    {3.50, 14.5, 2},
    {14.5, 20.5, 2},
    {18.5, 10.5, 1},
    {18.5, 11.5, 1},
    {18.5, 12.5, 1},
    {21.5, 1.50, 0},
    {15.5, 1.50, 0},
    {16.0, 1.80, 0},
    {16.2, 1.20, 0},
    {3.50, 2.50, 0},
    {9.50, 15.5, 0},
    {10.0, 15.1, 0},
    {10.5, 15.8, 0},
};

/* Player */
double pos_x = 22.0; // start position
double pos_y = 11.5;
double dir_x = -1.0; // direction vector
double dir_y = 0.0;
double plane_x = 0.0; // camera plane
double plane_y = 1.0;

/* Timing */
unsigned int previous_time = 0;
unsigned int current_time = 0;

/* Input */
bool w_down = false;
bool a_down = false;
bool s_down = false;
bool d_down = false;
bool lshift_down = false;

/* Rendering */
unsigned int buffer[SCREEN_HEIGHT][SCREEN_WIDTH];
double depth_buffer[SCREEN_WIDTH];
bool textured = true;
bool draw_walls = true;
bool draw_floor = true;
bool draw_objects = true;

/* Engine */
bool quit = false;

int main(int argc, char *args[])
{
    // setup SDL
    SDL_Init(SDL_INIT_VIDEO);
    SDL_SetRelativeMouseMode(SDL_TRUE);
    IMG_Init(IMG_INIT_PNG);
    SDLNet_Init();
    TTF_Init();

    // setup window
    window = SDL_CreateWindow(
        TITLE,
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(
        window,
        0,
        0);
    screen = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ABGR8888,
        SDL_TEXTUREACCESS_STATIC,
        SCREEN_WIDTH,
        SCREEN_HEIGHT);

    // load textures
    textures[0] = IMG_Load("eagle.png");
    textures[1] = IMG_Load("redbrick.png");
    textures[2] = IMG_Load("purplestone.png");
    textures[3] = IMG_Load("greystone.png");
    textures[4] = IMG_Load("bluestone.png");
    textures[5] = IMG_Load("mossy.png");
    textures[6] = IMG_Load("wood.png");
    textures[7] = IMG_Load("colorstone.png");

    // load sprites
    sprites[0] = IMG_Load("barrel.png");
    sprites[1] = IMG_Load("pillar.png");
    sprites[2] = IMG_Load("greenlight.png");

    // load font
    font = TTF_OpenFont("VeraMono.ttf", 24);

    // printf("FOV: %f\n", 2 * atan(plane_y) / M_PI * 180.0);

    while (!quit)
    {
        // timing for input and FPS counter
        previous_time = current_time;
        current_time = SDL_GetTicks();
        double delta_time = (current_time - previous_time) / 1000.0;

        printf("FPS: %d, pos: (%f, %f), dir: (%f, %f), cam: (%f, %f)\n", (int)(1 / delta_time), pos_x, pos_y, dir_x, dir_y, plane_x, plane_y);

        // handle input
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_MOUSEMOTION:
            {
                // the movement of the mouse in the x-direction
                int mouse_dx = event.motion.xrel;

                // calculate rotation vector
                // the constant value is in radians/second
                double angle = -mouse_dx / 1000.0 * ROTATE_SPEED;
                double rot_x = cos(angle);
                double rot_y = sin(angle);

                // both camera direction and camera plane must be rotated
                double old_dir_x = dir_x;
                dir_x = dir_x * rot_x - dir_y * rot_y;
                dir_y = old_dir_x * rot_y + dir_y * rot_x;

                double old_plane_x = plane_x;
                plane_x = plane_x * rot_x - plane_y * rot_y;
                plane_y = old_plane_x * rot_y + plane_y * rot_x;
            }
            break;
            case SDL_KEYDOWN:
            {
                SDL_Keycode key = event.key.keysym.sym;
                SDL_Keymod mod = event.key.keysym.mod;

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
                case SDLK_ESCAPE:
                {
                    quit = true;
                }
                break;
                }
            }
            break;
            case SDL_KEYUP:
            {
                SDL_Keycode key = event.key.keysym.sym;
                SDL_Keymod mod = event.key.keysym.mod;

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
                case SDLK_F1:
                {
                    SDL_SetWindowFullscreen(window, 0);
                }
                break;
                case SDLK_F2:
                {
                    SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
                }
                break;
                case SDLK_F3:
                {
                    textured = !textured;
                }
                break;
                case SDLK_F4:
                {
                    draw_walls = !draw_walls;
                }
                break;
                case SDLK_F5:
                {
                    draw_floor = !draw_floor;
                }
                break;
                case SDLK_F6:
                {
                    draw_objects = !draw_objects;
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

        // calculate base movement speed
        // the constant value is in squares/second
        double move_speed = MOVE_SPEED * delta_time;

        // sprinting
        if (lshift_down)
        {
            move_speed *= SPRINT_MULT;
        }

        // slow movement speed when moving diagonally
        if ((w_down && d_down) || (w_down && a_down) || (s_down && d_down) || (s_down && a_down))
        {
            move_speed /= sqrt(2);
        }

        // move forward
        if (w_down)
        {
            double dx = dir_x * move_speed;
            double dy = dir_y * move_speed;

            if (wall_map[(int)(pos_x + dx)][(int)(pos_y)] == 0)
            {
                pos_x += dx;
            }
            if (wall_map[(int)(pos_x)][(int)(pos_y + dy)] == 0)
            {
                pos_y += dy;
            }
        }

        // strafe left
        if (a_down)
        {
            double dx = -dir_y * move_speed;
            double dy = dir_x * move_speed;

            if (wall_map[(int)(pos_x + dx)][(int)(pos_y)] == 0)
            {
                pos_x += dx;
            }
            if (wall_map[(int)(pos_x)][(int)(pos_y + dy)] == 0)
            {
                pos_y += dy;
            }
        }

        // move backward
        if (s_down)
        {
            double dx = -dir_x * move_speed;
            double dy = -dir_y * move_speed;

            if (wall_map[(int)(pos_x + dx)][(int)(pos_y)] == 0)
            {
                pos_x += dx;
            }
            if (wall_map[(int)(pos_x)][(int)(pos_y + dy)] == 0)
            {
                pos_y += dy;
            }
        }

        // strafe right
        if (d_down)
        {
            double dx = dir_y * move_speed;
            double dy = -dir_x * move_speed;

            if (wall_map[(int)(pos_x + dx)][(int)(pos_y)] == 0)
            {
                pos_x += dx;
            }
            if (wall_map[(int)(pos_x)][(int)(pos_y + dy)] == 0)
            {
                pos_y += dy;
            }
        }

        // raycasting
        for (int x = 0; x < SCREEN_WIDTH; x++)
        {
            // calculate x-coordinate in camera space
            double camera_x = (2 * (double)x / (double)SCREEN_WIDTH) - 1.0;

            // calculate ray position and direction
            double ray_dir_x = (camera_x * plane_x) + dir_x;
            double ray_dir_y = (camera_x * plane_y) + dir_y;

            // length of ray from one x or y-side to next x or y-side
            double delta_dist_x = fabs(1.0 / ray_dir_x);
            double delta_dist_y = fabs(1.0 / ray_dir_y);

            // which box of the map we're in
            int map_x = (int)pos_x;
            int map_y = (int)pos_y;

            // length of ray from current position to next x or y-side
            double side_dist_x;
            double side_dist_y;

            // what direction to step in x or y-direction (either +1 or -1)
            int step_x;
            int step_y;

            // calculate step and initial sideDist
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

            // was a NS or a EW wall hit?
            int side;

            // perform DDA
            while (true)
            {
                // jump to next map square, OR in x-direction, OR in y-direction
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

                // check if ray has hit a wall
                if (wall_map[map_x][map_y] > 0)
                {
                    break;
                }
            }

            // calculate distance projected on camera direction (Euclidean distance will give fisheye effect!)
            // additionaly set the depth buffer at this slice to the distance
            double perp_wall_dist = depth_buffer[x] =
                side == 0
                    ? (map_x - pos_x + (1 - step_x) / 2) / ray_dir_x
                    : (map_y - pos_y + (1 - step_y) / 2) / ray_dir_y;

            // calculate height of line to draw on screen
            int line_height = (int)((double)SCREEN_HEIGHT / perp_wall_dist);

            // calculate lowest and highest pixel to fill in current stripe
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

            if (textured)
            {
                // calculate where exactly the wall was hit
                double wall_x;
                if (side == 0)
                {
                    wall_x = pos_y + perp_wall_dist * ray_dir_y;
                }
                else
                {
                    wall_x = pos_x + perp_wall_dist * ray_dir_x;
                }
                wall_x -= floor(wall_x);

                if (draw_walls)
                {
                    // choose a texture
                    int texture_index = wall_map[map_x][map_y] - 1;
                    SDL_Surface *texture = textures[texture_index];

                    // x coordinate on the texture
                    int texture_x = (int)(wall_x * (double)texture->w);
                    if (side == 0 && ray_dir_x > 0)
                    {
                        texture_x = texture->w - texture_x - 1;
                    }
                    if (side == 1 && ray_dir_y < 0)
                    {
                        texture_x = texture->w - texture_x - 1;
                    }

                    for (int y = draw_start; y < draw_end; y++)
                    {
                        // y coordinate on the texture
                        int texture_y = (((y * 256 - SCREEN_HEIGHT * 128 + line_height * 128) * texture->h) / line_height) / 256;

                        // get the color on the texture
                        unsigned int texture_pixel = get_pixel(texture, texture_x, texture_y);

                        // make color darker for y-sides: R, G and B byte each divided through two with a "shift" and an "and"
                        if (side == 1)
                        {
                            texture_pixel = (texture_pixel >> 1) & 0x7f7f7f;
                        }

                        buffer[y][x] = texture_pixel;
                    }
                }

                if (draw_floor)
                {
                    // x, y position of the floor texel at the bottom of the wall
                    double floor_x_wall;
                    double floor_y_wall;

                    // 4 different wall directions possible
                    if (side == 0 && ray_dir_x > 0)
                    {
                        floor_x_wall = map_x;
                        floor_y_wall = map_y + wall_x;
                    }
                    else if (side == 0 && ray_dir_x < 0)
                    {
                        floor_x_wall = map_x + 1.0;
                        floor_y_wall = map_y + wall_x;
                    }
                    else if (side == 1 && ray_dir_y > 0)
                    {
                        floor_x_wall = map_x + wall_x;
                        floor_y_wall = map_y;
                    }
                    else
                    {
                        floor_x_wall = map_x + wall_x;
                        floor_y_wall = map_y + 1.0;
                    }

                    // becomes < 0 when the integer overflows
                    if (draw_end < 0)
                    {
                        draw_end = SCREEN_HEIGHT;
                    }

                    // draw the floor from draw_end to the bottom of the screen
                    for (int y = draw_end + 1; y < SCREEN_HEIGHT; y++)
                    {
                        double current_dist = SCREEN_HEIGHT / (2.0 * y - SCREEN_HEIGHT);
                        double weight = current_dist / perp_wall_dist;

                        double current_x = weight * floor_x_wall + (1.0 - weight) * pos_x;
                        double current_y = weight * floor_y_wall + (1.0 - weight) * pos_y;

                        // floor
                        {
                            // choose a texture
                            int texture_index = floor_map[(int)current_x][(int)current_y];
                            SDL_Surface *texture = textures[texture_index];

                            // x, y coordinate of the texture
                            int texture_x;
                            int texture_y;
                            texture_x = (int)(current_x * texture->w / FLOOR_TEXTURE_MULT) % texture->w;
                            texture_y = (int)(current_y * texture->h / FLOOR_TEXTURE_MULT) % texture->h;

                            // get the color on the texture
                            unsigned int texture_pixel = get_pixel(texture, texture_x, texture_y);

                            // draw the pixel
                            buffer[y][x] = texture_pixel;
                        }

                        // ceiling
                        {
                            // choose a texture
                            int texture_index = ceiling_map[(int)current_x][(int)current_y];
                            SDL_Surface *texture = textures[texture_index];

                            // x, y coordinate of the texture
                            int texture_x;
                            int texture_y;
                            texture_x = (int)(current_x * texture->w / CEILING_TEXTURE_MULT) % texture->w;
                            texture_y = (int)(current_y * texture->h / CEILING_TEXTURE_MULT) % texture->h;

                            // get the color on the texture
                            unsigned int texture_pixel = get_pixel(texture, texture_x, texture_y);

                            // draw the pixel slightly darkened
                            buffer[SCREEN_HEIGHT - y][x] = (texture_pixel >> 1) & 0x7f7f7f;
                        }
                    }
                }
            }
            else
            {
                if (draw_walls)
                {
                    // choose wall color
                    unsigned int wall_color;
                    switch (wall_map[map_x][map_y])
                    {
                    case 1:
                        wall_color = 0xff0000ff; // red
                        break;
                    case 2:
                        wall_color = 0xff00ff00; // green
                        break;
                    case 3:
                        wall_color = 0xffff0000; // blue
                        break;
                    case 4:
                        wall_color = 0xffffffff; // white
                        break;
                    default:
                        wall_color = 0xff00ffff; // yellow
                        break;
                    }

                    // give x and y sides different brightness
                    if (side == 1)
                    {
                        wall_color = (wall_color >> 1) & 0x7f7f7f;
                    }

                    // draw the pixels of the stripe as a vertical line
                    for (int y = draw_start; y < draw_end; y++)
                    {
                        buffer[y][x] = wall_color;
                    }
                }

                if (draw_floor)
                {
                    // choose floor and ceiling colors
                    unsigned int floor_color = 0xff646464;
                    unsigned int ceiling_color = (floor_color >> 1) & 0x7f7f7f;

                    // draw the floor
                    for (int y = draw_end; y < SCREEN_HEIGHT; y++)
                    {
                        buffer[y][x] = floor_color;
                    }

                    // draw the ceiling
                    for (int y = 0; y < draw_start; y++)
                    {
                        buffer[y][x] = ceiling_color;
                    }
                }
            }
        }

        if (draw_objects)
        {
            // arrays used to sort the objects
            int object_order[NUM_OBJECTS];
            double object_dist[NUM_OBJECTS];

            // sort objects from far to close
            for (int i = 0; i < NUM_OBJECTS; i++)
            {
                object_order[i] = i;
                object_dist[i] = pow(pos_x - objects[i].x, 2) + pow(pos_x - objects[i].y, 2);
            }
            comb_sort(object_order, object_dist, NUM_OBJECTS);

            // after sorting the objects, do the projection and draw them
            for (int i = 0; i < NUM_OBJECTS; i++)
            {
                // translate object position to relative to camera
                double object_x = objects[object_order[i]].x - pos_x;
                double object_y = objects[object_order[i]].y - pos_y;

                // transform object with the inverse camera matrix
                // [ planeX   dirX ] -1                                       [ dirY      -dirX ]
                // [               ]       =  1/(planeX*dirY-dirX*planeY) *   [                 ]
                // [ planeY   dirY ]                                          [ -planeY  planeX ]
                // required for correct matrix multiplication
                double inv_det = 1.0 / (plane_x * dir_y - dir_x * plane_y);

                // transform_y is actually the depth inside the screen, that what Z is in 3D
                double transform_x = inv_det * (dir_y * object_x - dir_x * object_y);
                double transform_y = inv_det * (-plane_y * object_x + plane_x * object_y);

                // where the object is on the screen
                int object_screen_x = (int)((SCREEN_WIDTH / 2) * (1 + transform_x / transform_y));

                // calculate width and height of the object on screen
                // using "transform_y" instead of the real distance prevents fisheye
                int object_height = abs((int)(SCREEN_HEIGHT / (transform_y)));
                int object_width = abs((int)(SCREEN_HEIGHT / (transform_y)));

                // calculate the vertical stripes to draw the object
                int draw_start_x = -object_width / 2 + object_screen_x;
                if (draw_start_x < 0)
                {
                    draw_start_x = 0;
                }
                int draw_end_x = object_width / 2 + object_screen_x;
                if (draw_end_x >= SCREEN_WIDTH)
                {
                    draw_end_x = SCREEN_WIDTH - 1;
                }

                // calculate lowest and highest pixel to fill in current stripe
                int draw_start_y = -object_height / 2 + SCREEN_HEIGHT / 2;
                if (draw_start_y < 0)
                {
                    draw_start_y = 0;
                }
                int draw_end_y = object_height / 2 + SCREEN_HEIGHT / 2;
                if (draw_end_y >= SCREEN_HEIGHT)
                {
                    draw_end_y = SCREEN_HEIGHT - 1;
                }

                // choose the sprite
                int sprite_index = objects[object_order[i]].sprite_index;
                SDL_Surface *sprite = sprites[sprite_index];

                // loop through every vertical stripe of the sprite on screen
                for (int x = draw_start_x; x < draw_end_x; x++)
                {
                    // x coordinate on the sprite
                    int sprite_x = (256 * (x - (-object_width / 2 + object_screen_x)) * sprite->w / object_width) / 256;

                    //the conditions in the if are:
                    //1) it's in front of camera plane so you don't see things behind you
                    //2) it's on the screen (left)
                    //3) it's on the screen (right)
                    //4) depth_buffer, with perpendicular distance
                    if (transform_y > 0 && x > 0 && x < SCREEN_WIDTH && transform_y < depth_buffer[x])
                        for (int y = draw_start_y; y < draw_end_y; y++)
                        {
                            // y coordinate on the sprite
                            int sprite_y = (((y * 256 - SCREEN_HEIGHT * 128 + object_height * 128) * sprite->h) / object_height) / 256;

                            // get current color on the sprite
                            unsigned int color = get_pixel(sprite, sprite_x, sprite_y);

                            // draw the pixel if it isnt't black, black is the invisible color
                            if ((color & 0x00FFFFFF) != 0)
                            {
                                buffer[y][x] = color;
                            }
                        }
                }
            }
        }

        SDL_Surface *text_surface = TTF_RenderText_Solid(font, "Hello, World!", (SDL_Color){255, 255, 255, 255});
        SDL_Texture *text = SDL_CreateTextureFromSurface(renderer, text_surface);
        SDL_Rect text_rect;
        text_rect.x = 0;
        text_rect.y = 0;
        text_rect.w = 14 * 24;
        text_rect.h = 24;

        // update the screen
        SDL_UpdateTexture(
            screen,
            NULL,
            buffer,
            SCREEN_WIDTH * sizeof(unsigned int));
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, screen, NULL, NULL);
        SDL_RenderCopy(renderer, text, NULL, &text_rect);
        SDL_RenderPresent(renderer);
    }

    // cleanup
    TTF_CloseFont(font);
    TTF_Quit();

    SDLNet_Quit();

    for (int i = 0; i < 3; i++)
    {
        SDL_Surface *sprite = sprites[i];
        SDL_FreeSurface(sprite);
    }
    for (int i = 0; i < 8; i++)
    {
        SDL_Surface *screen = textures[i];
        SDL_FreeSurface(screen);
    }
    IMG_Quit();

    SDL_DestroyTexture(screen);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

unsigned int get_pixel(SDL_Surface *surface, int x, int y)
{
    if (x < 0 || x >= surface->w || y < 0 || y > surface->h)
    {
        return 0;
    }

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
        return 0;
    }
    break;
    }
}

void set_pixel(SDL_Surface *surface, int x, int y, unsigned int pixel)
{
    if (x < 0 || x >= surface->w || y < 0 || y > surface->h)
    {
        return;
    }

    int bpp = surface->format->BytesPerPixel;
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

void comb_sort(int *order, double *dist, int amount)
{
    int gap = amount;
    bool swapped = false;
    while (gap > 1 || swapped)
    {
        // shrink factor 1.3
        gap = (gap * 10) / 13;
        if (gap == 9 || gap == 10)
        {
            gap = 11;
        }
        if (gap < 1)
        {
            gap = 1;
        }

        swapped = false;
        for (int i = 0; i < amount - gap; i++)
        {
            int j = i + gap;
            if (dist[i] < dist[j])
            {
                double temp_dist = dist[i];
                dist[i] = dist[j];
                dist[j] = temp_dist;

                int temp_order = order[i];
                order[i] = order[j];
                order[j] = temp_order;

                swapped = true;
            }
        }
    }
}
