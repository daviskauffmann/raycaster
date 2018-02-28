#include <float.h>
#include <math.h>
#include <SDL\SDL_image.h>
#include <SDL\SDL_net.h>
#include <SDL\SDL_mixer.h>
#include <SDL\SDL_ttf.h>
#include <SDL\SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TITLE "Raycaster"
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 400

#define NUM_IMAGES 11
#define IMAGE_WIDTH 64
#define IMAGE_HEIGHT 64

#define NUM_TRACKS 1
#define NUM_SOUNDS 1

#define MAP_WIDTH 24
#define MAP_HEIGHT 24

#define NUM_OBJECTS 19

#define MOVE_SPEED 5.0
#define SPRINT_MULT 2.0
#define ROTATE_SPEED 3.0

#define FLOOR_TEXTURE_MULT 1
#define CEILING_TEXTURE_MULT 1

#define SPRITE_SCALE_U 1
#define SPRITE_SCALE_V 1
#define SPRITE_MOVE_V 0.0

#define FOG_STRENGTH 0.5

typedef struct object_s
{
    double x;
    double y;
    int sprite_index;
} object_t;

unsigned int get_pixel(SDL_Surface *surface, int x, int y);
void set_pixel(SDL_Surface *surface, int x, int y, unsigned int pixel);
void comb_sort(int *order, double *dist, int amount);
unsigned int color_darken(unsigned int color);
unsigned int color_fog(unsigned int color, double distance);

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
SDL_Texture *screen = NULL;

unsigned int images[NUM_IMAGES][IMAGE_WIDTH][IMAGE_HEIGHT];

Mix_Music *tracks[NUM_TRACKS];
Mix_Chunk *sounds[NUM_SOUNDS];

TTF_Font *font = NULL;

int wall_map[MAP_WIDTH][MAP_HEIGHT] = {
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

int floor_map[MAP_WIDTH][MAP_HEIGHT] = {
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

int ceiling_map[MAP_WIDTH][MAP_HEIGHT] = {
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
    {20.5, 11.5, 10},
    {18.5, 4.50, 10},
    {10.0, 4.50, 10},
    {10.0, 12.5, 10},
    {3.50, 6.50, 10},
    {3.50, 20.5, 10},
    {3.50, 14.5, 10},
    {14.5, 20.5, 10},
    {18.5, 10.5, 9},
    {18.5, 11.5, 9},
    {18.5, 12.5, 9},
    {21.5, 1.50, 8},
    {15.5, 1.50, 8},
    {16.0, 1.80, 8},
    {16.2, 1.20, 8},
    {3.50, 2.50, 8},
    {9.50, 15.5, 8},
    {10.0, 15.1, 8},
    {10.5, 15.8, 8},
};

double pos_x = 22.0; // start position
double pos_y = 11.5;
double dir_x = -1.0; // direction vector
double dir_y = 0.0;
double plane_x = 0.0; // camera plane
double plane_y = 1.0;

unsigned int previous_time = 0;
unsigned int current_time = 0;

bool w_down = false;
bool a_down = false;
bool s_down = false;
bool d_down = false;
bool lshift_down = false;
bool lbutton_down;

unsigned int pixel_buffer[SCREEN_HEIGHT][SCREEN_WIDTH];
double depth_buffer[SCREEN_HEIGHT][SCREEN_WIDTH];

bool textured = true;
bool draw_walls = true;
bool draw_floor = true;
bool draw_objects = true;
bool shading = true;
bool foggy = true;

int main(int argc, char *args[])
{
    // suppress warning C4100
    (void)argc;
    (void)args;

    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_SetRelativeMouseMode(SDL_TRUE);

    window = SDL_CreateWindow(
        TITLE,
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        0);

    renderer = SDL_CreateRenderer(
        window,
        -1,
        0);

    screen = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ABGR8888,
        SDL_TEXTUREACCESS_STREAMING,
        SCREEN_WIDTH,
        SCREEN_HEIGHT);

    IMG_Init(IMG_INIT_PNG);
    for (int i = 0; i < NUM_IMAGES; i++)
    {
        SDL_Surface *surface = NULL;

        switch (i)
        {
        case 0:
            surface = IMG_Load("assets/images/eagle.png");
            break;
        case 1:
            surface = IMG_Load("assets/images/redbrick.png");
            break;
        case 2:
            surface = IMG_Load("assets/images/purplestone.png");
            break;
        case 3:
            surface = IMG_Load("assets/images/greystone.png");
            break;
        case 4:
            surface = IMG_Load("assets/images/bluestone.png");
            break;
        case 5:
            surface = IMG_Load("assets/images/mossy.png");
            break;
        case 6:
            surface = IMG_Load("assets/images/wood.png");
            break;
        case 7:
            surface = IMG_Load("assets/images/colorstone.png");
            break;
        case 8:
            surface = IMG_Load("assets/images/barrel.png");
            break;
        case 9:
            surface = IMG_Load("assets/images/pillar.png");
            break;
        case 10:
            surface = IMG_Load("assets/images/greenlight.png");
            break;
        }

        for (int x = 0; x < IMAGE_WIDTH; x++)
        {
            for (int y = 0; y < IMAGE_HEIGHT; y++)
            {
                images[i][x][y] = get_pixel(surface, x, y);
            }
        }

        SDL_FreeSurface(surface);
    }

    Mix_Init(MIX_INIT_MP3);
    Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096);
    tracks[0] = Mix_LoadMUS("assets/audio/background.mp3");
    sounds[0] = Mix_LoadWAV("assets/audio/shoot.wav");

    SDLNet_Init();

    // load font
    TTF_Init();
    font = TTF_OpenFont("assets/fonts/VeraMono.ttf", 24);

    // printf("FOV: %f\n", 2 * atan(plane_y) / M_PI * 180.0);

    bool quit = false;
    while (!quit)
    {
        // timing for input and FPS counter
        previous_time = current_time;
        current_time = SDL_GetTicks();
        double delta_time = (current_time - previous_time) / 1000.0;

        // printf("FPS: %d, pos: (%f, %f), dir: (%f, %f), cam: (%f, %f)\n", (int)(1 / delta_time), pos_x, pos_y, dir_x, dir_y, plane_x, plane_y);

        // handle input
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_MOUSEBUTTONDOWN:
            {
                SDL_MouseButtonEvent button = event.button;

                switch (button.button)
                {
                case SDL_BUTTON_LEFT:
                {
                    lbutton_down = true;
                }
                break;
                }
            }
            break;
            case SDL_MOUSEBUTTONUP:
            {
                SDL_MouseButtonEvent button = event.button;

                switch (button.button)
                {
                case SDL_BUTTON_LEFT:
                {
                    lbutton_down = false;
                }
                break;
                }
            }
            break;
            case SDL_MOUSEMOTION:
            {
                // the movement of the mouse in the x, y direction
                int mouse_dx = event.motion.xrel;
                // int mouse_dy = event.motion.yrel;

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
                // SDL_Keymod mod = event.key.keysym.mod;

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
                case SDLK_F7:
                {
                    shading = !shading;
                }
                break;
                case SDLK_F8:
                {
                    foggy = !foggy;
                }
                break;
                case SDLK_1:
                {
                    if (Mix_PlayingMusic())
                    {
                        Mix_HaltMusic();
                    }
                    else
                    {
                        Mix_PlayMusic(tracks[0], -1);
                    }
                }
                break;
                case SDLK_2:
                {
                    if (Mix_PlayingMusic())
                    {
                        if (Mix_PausedMusic())
                        {
                            Mix_ResumeMusic();
                        }
                        else
                        {
                            Mix_PauseMusic();
                        }
                    }
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
                // SDL_Keymod mod = event.key.keysym.mod;

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

        // shooting
        static double shoot_timer = 0.0;
        shoot_timer += delta_time;
        if (lbutton_down)
        {
            if (shoot_timer >= 0.25)
            {
                shoot_timer = 0.0;

                Mix_PlayChannel(-1, sounds[0], 0);
            }
        }

        // raycasting
        for (int x = 0; x < SCREEN_WIDTH; x++)
        {
            // clear the pixel and depth buffers
            for (int y = 0; y < SCREEN_HEIGHT; y++)
            {
                pixel_buffer[y][x] = 0;
                depth_buffer[y][x] = DBL_MAX;
            }

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
            double perp_wall_dist =
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

                    // x coordinate on the texture
                    int texture_x = (int)(wall_x * (double)IMAGE_WIDTH);
                    if (side == 0 && ray_dir_x > 0)
                    {
                        texture_x = IMAGE_WIDTH - texture_x - 1;
                    }
                    if (side == 1 && ray_dir_y < 0)
                    {
                        texture_x = IMAGE_WIDTH - texture_x - 1;
                    }

                    for (int y = draw_start; y <= draw_end; y++)
                    {
                        // y coordinate on the texture
                        int texture_y = (((y * 256 - SCREEN_HEIGHT * 128 + line_height * 128) * IMAGE_HEIGHT) / line_height) / 256;

                        // get the color on the texture
                        unsigned int color = images[texture_index][texture_x][texture_y];

                        // make color darker for y-sides: R, G and B byte each divided through two with a "shift" and an "and"
                        if (shading && side == 1)
                        {
                            color = color_darken(color);
                        }

                        if (foggy)
                        {
                            color = color_fog(color, perp_wall_dist);
                        }

                        // draw the pixel
                        pixel_buffer[y][x] = color;
                        depth_buffer[y][x] = perp_wall_dist;
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

                            // x, y coordinate of the texture
                            int texture_x = (int)(current_x * IMAGE_WIDTH / FLOOR_TEXTURE_MULT) % IMAGE_WIDTH;
                            int texture_y = (int)(current_y * IMAGE_HEIGHT / FLOOR_TEXTURE_MULT) % IMAGE_HEIGHT;

                            // get the color on the texture
                            unsigned int color = images[texture_index][texture_x][texture_y];

                            if (foggy)
                            {
                                color = color_fog(color, current_dist);
                            }

                            // draw the pixel
                            pixel_buffer[y][x] = color;
                            depth_buffer[y][x] = current_dist;
                        }

                        // ceiling
                        {
                            // choose a texture
                            int texture_index = ceiling_map[(int)current_x][(int)current_y];

                            // x, y coordinate of the texture
                            int texture_x = (int)(current_x * IMAGE_WIDTH / CEILING_TEXTURE_MULT) % IMAGE_WIDTH;
                            int texture_y = (int)(current_y * IMAGE_HEIGHT / CEILING_TEXTURE_MULT) % IMAGE_HEIGHT;

                            // get the color on the texture
                            unsigned int color = images[texture_index][texture_x][texture_y];

                            if (foggy)
                            {
                                color = color_fog(color, current_dist);
                            }

                            if (shading)
                            {
                                color = color_darken(color);
                            }

                            // draw the pixel
                            pixel_buffer[SCREEN_HEIGHT - y][x] = color;
                            depth_buffer[SCREEN_HEIGHT - y][x] = current_dist;
                        }
                    }
                }
            }
            else
            {
                if (draw_walls)
                {
                    // choose wall color
                    unsigned int color;
                    switch (wall_map[map_x][map_y])
                    {
                    case 0:
                        color = 0xff00ffff; // yellow
                        break;
                    case 1:
                        color = 0xff0000ff; // red
                        break;
                    case 2:
                        color = 0xff00ff00; // green
                        break;
                    case 3:
                        color = 0xffff0000; // blue
                        break;
                    case 4:
                        color = 0xffff00ff; // purple
                        break;
                    case 5:
                        color = 0xffffff00; // yellow
                        break;
                    default:
                        color = 0xffffffff; // white
                        break;
                    }

                    // give x and y sides different brightness
                    if (shading && side == 1)
                    {
                        color = color_darken(color);
                    }

                    if (foggy)
                    {
                        color = color_fog(color, perp_wall_dist);
                    }

                    // draw the pixels of the stripe as a vertical line
                    for (int y = draw_start; y <= draw_end; y++)
                    {
                        pixel_buffer[y][x] = color;
                        depth_buffer[y][x] = perp_wall_dist;
                    }
                }

                if (draw_floor)
                {
                    // choose floor and ceiling colors
                    unsigned int floor_color = 0xff646464;
                    unsigned int ceiling_color = floor_color;

                    if (shading)
                    {
                        ceiling_color = color_darken(ceiling_color);
                    }

                    // draw the floor
                    for (int y = draw_end + 1; y < SCREEN_HEIGHT; y++)
                    {
                        pixel_buffer[y][x] = floor_color;
                        depth_buffer[y][x] = perp_wall_dist;
                    }

                    // draw the ceiling
                    for (int y = 0; y < draw_start; y++)
                    {
                        pixel_buffer[y][x] = ceiling_color;
                        depth_buffer[y][x] = perp_wall_dist;
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
                object_dist[i] = pow(pos_x - objects[i].x, 2) + pow(pos_y - objects[i].y, 2);
            }
            comb_sort(object_order, object_dist, NUM_OBJECTS);

            // after sorting the objects, do the projection and draw them
            for (int i = 0; i < NUM_OBJECTS; i++)
            {
                object_t object = objects[object_order[i]];

                // translate object position to relative to camera
                double object_x = object.x - pos_x;
                double object_y = object.y - pos_y;

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
                // using transform_y instead of the real distance prevents fisheye
                int object_width = abs((int)(SCREEN_HEIGHT / (transform_y))) * SPRITE_SCALE_U;
                int object_height = abs((int)(SCREEN_HEIGHT / (transform_y))) * SPRITE_SCALE_V;

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

                // move the object on the screen
                int move_v = (int)(SPRITE_MOVE_V / transform_y);

                // calculate lowest and highest pixel to fill in current stripe
                int draw_start_y = -object_height / 2 + SCREEN_HEIGHT / 2 + move_v;
                if (draw_start_y < 0)
                {
                    draw_start_y = 0;
                }
                int draw_end_y = object_height / 2 + SCREEN_HEIGHT / 2 + move_v;
                if (draw_end_y >= SCREEN_HEIGHT)
                {
                    draw_end_y = SCREEN_HEIGHT - 1;
                }

                // calculate angle of object to player
                // double angle = atan2(object_y, object_y);

                // choose the sprite
                int sprite_index = object.sprite_index;

                // loop through every vertical stripe of the sprite on screen
                for (int x = draw_start_x; x < draw_end_x; x++)
                {
                    // x coordinate on the sprite
                    int sprite_x = (256 * (x - (-object_width / 2 + object_screen_x)) * IMAGE_WIDTH / object_width) / 256;

                    // the conditions in the if are:
                    // 1) it's in front of camera plane so you don't see things behind you
                    // 2) it's on the screen (left)
                    // 3) it's on the screen (right)
                    if (transform_y > 0 && x > 0 && x < SCREEN_WIDTH)
                    {
                        for (int y = draw_start_y; y < draw_end_y; y++)
                        {
                            // depth_buffer, with perpendicular distance
                            if (transform_y < depth_buffer[y][x])
                            {
                                // y coordinate on the sprite
                                int sprite_y = ((((y - move_v) * 256 - SCREEN_HEIGHT * 128 + object_height * 128) * IMAGE_HEIGHT) / object_height) / 256;

                                // get current color on the sprite
                                unsigned int color = images[sprite_index][sprite_x][sprite_y];

                                if (foggy)
                                {
                                    color = color_fog(color, transform_y);
                                }

                                // draw the pixel if it isnt't black, black is the invisible color
                                if ((color & 0x00ffffff) != 0)
                                {
                                    // used for translucency
                                    // unsigned int previous_color = pixel_buffer[y][x];

                                    pixel_buffer[y][x] = color;
                                    depth_buffer[y][x] = transform_y;
                                }
                            }
                        }
                    }
                }
            }
        }

        char text_buffer[256];
        sprintf_s(text_buffer, sizeof(text_buffer), "FPS: %d", (int)(1 / delta_time));
        SDL_Surface *text_surface = TTF_RenderText_Solid(font, text_buffer, (SDL_Color){255, 255, 255, 255});
        SDL_Texture *text = SDL_CreateTextureFromSurface(renderer, text_surface);
        SDL_Rect text_rect = {0, 0, 100, 50};

        // update the screen
        SDL_UpdateTexture(
            screen,
            NULL,
            pixel_buffer,
            SCREEN_WIDTH * sizeof(unsigned int));

        // render
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, screen, NULL, NULL);
        SDL_RenderCopy(renderer, text, NULL, &text_rect);
        SDL_RenderPresent(renderer);
    }

    TTF_CloseFont(font);
    TTF_Quit();

    SDLNet_Quit();

    for (int i = 0; i < NUM_TRACKS; i++)
    {
        Mix_FreeMusic(tracks[i]);
    }
    for (int i = 0; i < NUM_SOUNDS; i++)
    {
        Mix_FreeChunk(sounds[i]);
    }
    Mix_CloseAudio();
    Mix_Quit();

    IMG_Quit();

    SDL_DestroyTexture(screen);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}

unsigned int get_pixel(SDL_Surface *surface, int x, int y)
{
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
        return 0;
    }
    break;
    }
}

void set_pixel(SDL_Surface *surface, int x, int y, unsigned int pixel)
{
    int bpp = surface->format->BytesPerPixel;
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

unsigned int color_darken(unsigned int color)
{
    // darken the color
    return (color >> 1) & 0x7f7f7f;
}

// TODO: optimize
// this drops the framerate by about 20
unsigned int color_fog(unsigned int color, double distance)
{
    // separate the colors
    int red = (color >> 16) & 0x0FF;
    int green = (color >> 8) & 0x0FF;
    int blue = color & 0x0FF;

    // modify the colors
    double fog_intensity = distance * FOG_STRENGTH;
    if (fog_intensity > 1)
    {
        double float_red = (double)red;
        double float_green = (double)green;
        double float_blue = (double)blue;

        float_red /= fog_intensity;
        float_green /= fog_intensity;
        float_blue /= fog_intensity;

        red = (int)float_red;
        green = (int)float_green;
        blue = (int)float_blue;
    }

    // recombine the colors
    return ((red & 0x0ff) << 16) | ((green & 0x0ff) << 8) | (blue & 0x0ff);
}
