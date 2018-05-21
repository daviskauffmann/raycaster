#include <float.h>
#include <math.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_mixer.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SDL_image_ext.h"

#define SDL_FLAGS SDL_INIT_VIDEO | SDL_INIT_AUDIO

#define WINDOW_TITLE "Raycaster"
#define WINDOW_X SDL_WINDOWPOS_UNDEFINED
#define WINDOW_Y SDL_WINDOWPOS_UNDEFINED
#define WINDOW_WIDTH 320
#define WINDOW_HEIGHT 200
#define WINDOW_FLAGS 0

#define RENDERER_INDEX -1
#define RENDERER_FLAGS 0

#define SCREEN_FORMAT SDL_PIXELFORMAT_ABGR8888
#define SCREEN_ACCESS SDL_TEXTUREACCESS_STREAMING

#define IMG_FLAGS IMG_INIT_PNG

#define MIX_FLAGS 0
#define AUDIO_FREQUENCY 44100
#define AUDIO_FORMAT MIX_DEFAULT_FORMAT
#define AUDIO_CHANNELS 2
#define AUDIO_CHUNK_SIZE 1024

#define NUM_TEXTURES 8
#define NUM_SPRITES 3
#define NUM_TRACKS 1
#define NUM_SOUNDS 1

#define MAP_WIDTH 24
#define MAP_HEIGHT 24
#define NUM_OBJECTS 19

#define MOVE_SPEED 5.0f
#define SPRINT_MULT 2.0f
#define ROTATE_SENSITIVITY 3.0f
#define MOUSE_SENSITIVITY 2.0f

#define FLOOR_TEXTURE_MULT 1
#define CEILING_TEXTURE_MULT 1

#define FOG_STRENGTH 0.5f

#define FPS_CAP 60
#define FRAME_DELAY (1000 / FPS_CAP)

struct object
{
    float x;
    float y;
    unsigned char sprite_index;
    float sprite_scale_x;
    float sprite_scale_y;
    float sprite_translate_y;
};

struct player
{
    float pos_x;
    float pos_y;
    float dir_x;
    float dir_y;
    float plane_x;
    float plane_y;
};

void player_move(struct player *player, float dx, float dy);
void player_rotate(struct player *player, float angle);
void comb_sort(int *order, float *dist, int amount);
unsigned int color_darken(unsigned int color);
unsigned int color_fog(unsigned int color, float distance);

unsigned char wall_map[MAP_WIDTH][MAP_HEIGHT] = {
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

unsigned char floor_map[MAP_WIDTH][MAP_HEIGHT] = {
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

unsigned char ceiling_map[MAP_WIDTH][MAP_HEIGHT] = {
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

struct object objects[NUM_OBJECTS] = {
    {20.5f, 11.5f, 2, 1.0f, 1.0f, 0.0f},
    {18.5f, 4.50f, 2, 1.0f, 1.0f, 0.0f},
    {10.0f, 4.50f, 2, 1.0f, 1.0f, 0.0f},
    {10.0f, 12.5f, 2, 1.0f, 1.0f, 0.0f},
    {3.50f, 6.50f, 2, 1.0f, 1.0f, 0.0f},
    {3.50f, 20.5f, 2, 1.0f, 1.0f, 0.0f},
    {3.50f, 14.5f, 2, 1.0f, 1.0f, 0.0f},
    {14.5f, 20.5f, 2, 1.0f, 1.0f, 0.0f},
    {18.5f, 10.5f, 1, 1.0f, 1.0f, 0.0f},
    {18.5f, 11.5f, 1, 1.0f, 1.0f, 0.0f},
    {18.5f, 12.5f, 1, 1.0f, 1.0f, 0.0f},
    {21.5f, 1.50f, 0, 1.0f, 1.0f, 0.0f},
    {15.5f, 1.50f, 0, 1.0f, 1.0f, 0.0f},
    {16.0f, 1.80f, 0, 1.0f, 1.0f, 0.0f},
    {16.2f, 1.20f, 0, 1.0f, 1.0f, 0.0f},
    {3.50f, 2.50f, 0, 1.0f, 1.0f, 0.0f},
    {9.50f, 15.5f, 0, 1.0f, 1.0f, 0.0f},
    {10.0f, 15.1f, 0, 1.0f, 1.0f, 0.0f},
    {10.5f, 15.8f, 0, 1.0f, 1.0f, 0.0f},
};

int main(int argc, char *args[])
{
    // suppress warning C4100
    (void)argc;
    (void)args;

    // init SDL
    if (SDL_Init(SDL_FLAGS) != 0)
    {
        SDL_Log("SDL_Init: %s", SDL_GetError());

        return 1;
    }

    SDL_SetRelativeMouseMode(SDL_TRUE);

    // create window
    SDL_Window *window = SDL_CreateWindow(
        WINDOW_TITLE,
        WINDOW_X,
        WINDOW_Y,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        WINDOW_FLAGS);

    if (!window)
    {
        SDL_Log("SDL_CreateWindow: %s", SDL_GetError());

        return 1;
    }

    // create renderer
    SDL_Renderer *renderer = SDL_CreateRenderer(
        window,
        RENDERER_INDEX,
        RENDERER_FLAGS);

    if (!renderer)
    {
        SDL_Log("SDL_CreateRenderer: %s", SDL_GetError());

        return 1;
    }

    // create screen texture
    SDL_Texture *screen = SDL_CreateTexture(
        renderer,
        SCREEN_FORMAT,
        SCREEN_ACCESS,
        WINDOW_WIDTH,
        WINDOW_HEIGHT);

    if (!screen)
    {
        SDL_Log("SDL_CreateTexture: %s", SDL_GetError());

        return 1;
    }

    // init SDL_image
    if ((IMG_Init(IMG_FLAGS) & IMG_FLAGS) != IMG_FLAGS)
    {
        SDL_Log("IMG_Init: %s", IMG_GetError());

        return 1;
    }

    // init SDL_mixer
    if ((Mix_Init(MIX_FLAGS) & MIX_FLAGS) != MIX_FLAGS)
    {
        SDL_Log("Mix_Init: %s", Mix_GetError());

        return 1;
    }

    // setup audio
    if (Mix_OpenAudio(AUDIO_FREQUENCY, AUDIO_FORMAT, AUDIO_CHANNELS, AUDIO_CHUNK_SIZE) != 0)
    {
        SDL_Log("Mix_OpenAudio: %s", Mix_GetError());

        return 1;
    }

    // init SDL_ttf
    if (TTF_Init() != 0)
    {
        SDL_Log("TTF_Init: %s", TTF_GetError());

        return 1;
    }

    // load textures
    IMG_Image *textures[NUM_TEXTURES];
    textures[0] = IMG_LoadAndConvert("assets/images/eagle.png");
    textures[1] = IMG_LoadAndConvert("assets/images/redbrick.png");
    textures[2] = IMG_LoadAndConvert("assets/images/purplestone.png");
    textures[3] = IMG_LoadAndConvert("assets/images/greystone.png");
    textures[4] = IMG_LoadAndConvert("assets/images/bluestone.png");
    textures[5] = IMG_LoadAndConvert("assets/images/mossy.png");
    textures[6] = IMG_LoadAndConvert("assets/images/wood.png");
    textures[7] = IMG_LoadAndConvert("assets/images/colorstone.png");

    // load sprites
    IMG_Image *sprites[NUM_SPRITES];
    sprites[0] = IMG_LoadAndConvert("assets/images/barrel.png");
    sprites[1] = IMG_LoadAndConvert("assets/images/pillar.png");
    sprites[2] = IMG_LoadAndConvert("assets/images/greenlight.png");

    // load music
    Mix_Music *tracks[NUM_TRACKS];
    tracks[0] = Mix_LoadMUS("assets/audio/background.mp3");

    // load sounds
    Mix_Chunk *sounds[NUM_SOUNDS];
    sounds[0] = Mix_LoadWAV("assets/audio/shoot.wav");

    // load font
    TTF_Font *font = TTF_OpenFont("assets/fonts/VeraMono.ttf", 24);

    // game settings
    bool fps_cap = true;
    bool textured = true;
    bool draw_walls = true;
    bool draw_floor = true;
    bool draw_objects = true;
    bool shading = true;
    bool foggy = true;

    // init player
    struct player *player = malloc(sizeof(struct player));

    player->pos_x = 22.0f;
    player->pos_y = 11.5f;
    player->dir_x = -1.0f;
    player->dir_y = 0.0f;
    player->plane_x = 0.0f;
    player->plane_y = 1.0f;

    SDL_Log("FOV: %d", (int)(2 * atanf(player->plane_y) / M_PI * 180));

    // render buffers
    unsigned int *pixel_buffer = malloc(WINDOW_WIDTH * WINDOW_HEIGHT * sizeof(unsigned int));
    float *depth_buffer = malloc(WINDOW_WIDTH * WINDOW_HEIGHT * sizeof(float));

    // main loop
    bool quit = false;
    unsigned int current_time = 0;
    while (!quit)
    {
        // timer for fps cap
        unsigned int frame_start = SDL_GetTicks();

        // calculate time passed since last frame
        unsigned int previous_time = current_time;
        current_time = frame_start;
        float delta_time = (current_time - previous_time) / 1000.0f;

        // calculate fps
        static float fps_update_timer = 0.0f;
        static unsigned int fps = 0;
        fps_update_timer += delta_time;
        if (fps_update_timer >= 0.25f)
        {
            fps_update_timer = 0.0f;
            fps = (unsigned int)(1 / delta_time);
        }

        // handle input
        const unsigned char *keys = SDL_GetKeyboardState(NULL);
        unsigned int mouse = SDL_GetMouseState(NULL, NULL);

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_MOUSEMOTION:
            {
                // the movement of the mouse in the x, y direction
                int mouse_dx = event.motion.xrel;
                // int mouse_dy = event.motion.yrel;

                // calculate rotation angle
                float angle = -mouse_dx / 1000.0f * MOUSE_SENSITIVITY;

                player_rotate(player, angle);
            }
            break;
            case SDL_KEYDOWN:
            {
                SDL_Keycode key = event.key.keysym.sym;
                // SDL_Keymod mod = event.key.keysym.mod;

                switch (key)
                {
                case SDLK_F1:
                {
                    fps_cap = !fps_cap;
                }
                break;
                case SDLK_F2:
                {
                    textured = !textured;
                }
                break;
                case SDLK_F3:
                {
                    draw_walls = !draw_walls;
                }
                break;
                case SDLK_F4:
                {
                    draw_floor = !draw_floor;
                }
                break;
                case SDLK_F5:
                {
                    draw_objects = !draw_objects;
                }
                break;
                case SDLK_F6:
                {
                    shading = !shading;
                }
                break;
                case SDLK_F7:
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
                case SDLK_TAB:
                {
                    SDL_SetRelativeMouseMode(!SDL_GetRelativeMouseMode());
                }
                break;
                case SDLK_RETURN:
                {
                    if (keys[SDL_SCANCODE_LALT])
                    {
                        unsigned int flags = SDL_GetWindowFlags(window);

                        if (flags & SDL_WINDOW_FULLSCREEN_DESKTOP)
                        {
                            SDL_SetWindowFullscreen(window, 0);
                        }
                        else
                        {
                            SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
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
            case SDL_QUIT:
            {
                quit = true;
            }
            break;
            }
        }

        // calculate base movement speed
        // the constant value is in squares/second
        float move_speed = MOVE_SPEED * delta_time;

        // sprinting
        if (keys[SDL_SCANCODE_LSHIFT])
        {
            move_speed *= SPRINT_MULT;
        }

        // slow movement speed when moving diagonally
        if ((keys[SDL_SCANCODE_W] && keys[SDL_SCANCODE_D]) ||
            (keys[SDL_SCANCODE_W] && keys[SDL_SCANCODE_A]) ||
            (keys[SDL_SCANCODE_S] && keys[SDL_SCANCODE_D]) ||
            (keys[SDL_SCANCODE_S] && keys[SDL_SCANCODE_A]))
        {
            // precomputed 1 / sqrt(2)
            move_speed *= 0.71f;
        }

        // move forward
        if (keys[SDL_SCANCODE_W])
        {
            float dx = player->dir_x * move_speed;
            float dy = player->dir_y * move_speed;

            player_move(player, dx, dy);
        }

        // strafe left
        if (keys[SDL_SCANCODE_A])
        {
            float dx = -player->dir_y * move_speed;
            float dy = player->dir_x * move_speed;

            player_move(player, dx, dy);
        }

        // move backward
        if (keys[SDL_SCANCODE_S])
        {
            float dx = -player->dir_x * move_speed;
            float dy = -player->dir_y * move_speed;

            player_move(player, dx, dy);
        }

        // strafe right
        if (keys[SDL_SCANCODE_D])
        {
            float dx = player->dir_y * move_speed;
            float dy = -player->dir_x * move_speed;

            player_move(player, dx, dy);
        }

        // calculate rotation angle
        // the constant value is in radians/second
        float angle = ROTATE_SENSITIVITY * delta_time;

        if (keys[SDL_SCANCODE_Q])
        {
            player_rotate(player, angle);
        }

        if (keys[SDL_SCANCODE_E])
        {
            player_rotate(player, -angle);
        }

        // shooting
        static float shoot_timer = 0.0f;
        shoot_timer += delta_time;
        if (mouse & SDL_BUTTON(SDL_BUTTON_LEFT))
        {
            if (shoot_timer >= 0.25f)
            {
                shoot_timer = 0.0f;

                Mix_PlayChannel(-1, sounds[0], 0);
            }
        }

        // raycasting
        for (int x = 0; x < WINDOW_WIDTH; x++)
        {
            // clear the pixel and depth buffers
            for (int y = 0; y < WINDOW_HEIGHT; y++)
            {
                pixel_buffer[x + y * WINDOW_WIDTH] = 0;
                depth_buffer[x + y * WINDOW_WIDTH] = FLT_MAX;
            }

            // calculate x-coordinate in camera space
            float camera_x = (2.0f * x / WINDOW_WIDTH) - 1;

            // calculate ray position and direction
            float ray_dir_x = (camera_x * player->plane_x) + player->dir_x;
            float ray_dir_y = (camera_x * player->plane_y) + player->dir_y;

            // which box of the map we're in
            int map_x = (int)player->pos_x;
            int map_y = (int)player->pos_y;

            // length of ray from current position to next x or y-side
            float side_dist_x;
            float side_dist_y;

            // length of ray from one x or y-side to next x or y-side
            float delta_dist_x = fabsf(1 / ray_dir_x);
            float delta_dist_y = fabsf(1 / ray_dir_y);

            // what direction to step in x or y-direction (either +1 or -1)
            int step_x;
            int step_y;

            // calculate step and initial sideDist
            if (ray_dir_x < 0)
            {
                side_dist_x = (player->pos_x - map_x) * delta_dist_x;
                step_x = -1;
            }
            else
            {
                side_dist_x = (map_x + 1 - player->pos_x) * delta_dist_x;
                step_x = 1;
            }
            if (ray_dir_y < 0)
            {
                side_dist_y = (player->pos_y - map_y) * delta_dist_y;
                step_y = -1;
            }
            else
            {
                side_dist_y = (map_y + 1 - player->pos_y) * delta_dist_y;
                step_y = 1;
            }

            // was a NS or a EW wall hit?
            int side;

            // perform DDA
            while (true)
            {
                // jump to next map square, OR in x-direction, OR in y-direction
                if (side_dist_x < side_dist_y)
                {
                    map_x += step_x;
                    side_dist_x += delta_dist_x;
                    side = 0;
                }
                else
                {
                    map_y += step_y;
                    side_dist_y += delta_dist_y;
                    side = 1;
                }

                // check if ray has hit a wall
                if (wall_map[map_x][map_y] > 0)
                {
                    break;
                }
            }

            // calculate distance projected on camera direction (Euclidean distance will give fisheye effect!)
            float perp_wall_dist =
                side == 0
                    ? (map_x - player->pos_x + (1 - step_x) / 2) / ray_dir_x
                    : (map_y - player->pos_y + (1 - step_y) / 2) / ray_dir_y;

            // calculate height of line to draw on screen
            int line_height = (int)(WINDOW_HEIGHT / perp_wall_dist);

            // calculate lowest and highest pixel to fill in current stripe
            int draw_start = -line_height / 2 + WINDOW_HEIGHT / 2;
            if (draw_start < 0)
            {
                draw_start = 0;
            }
            int draw_end = line_height / 2 + WINDOW_HEIGHT / 2;
            if (draw_end >= WINDOW_HEIGHT)
            {
                draw_end = WINDOW_HEIGHT - 1;
            }

            if (textured)
            {
                // calculate where exactly the wall was hit
                float wall_x;
                if (side == 0)
                {
                    wall_x = player->pos_y + perp_wall_dist * ray_dir_y;
                }
                else
                {
                    wall_x = player->pos_x + perp_wall_dist * ray_dir_x;
                }
                wall_x -= floorf(wall_x);

                if (draw_walls)
                {
                    // choose a texture
                    int texture_index = wall_map[map_x][map_y] - 1;
                    IMG_Image *texture = textures[texture_index];

                    // x coordinate on the texture
                    int texture_x = (int)(wall_x * texture->w);
                    if (side == 0 && ray_dir_x > 0)
                    {
                        texture_x = texture->w - texture_x - 1;
                    }
                    if (side == 1 && ray_dir_y < 0)
                    {
                        texture_x = texture->w - texture_x - 1;
                    }

                    for (int y = draw_start; y <= draw_end; y++)
                    {
                        // y coordinate on the texture
                        int texture_y = (((y * 256 - WINDOW_HEIGHT * 128 + line_height * 128) * texture->h) / line_height) / 256;

                        // get the color on the texture
                        unsigned int color = texture->pixels[texture_x + texture_y * texture->w];

                        // make color darker for y-sides
                        if (shading && side == 1)
                        {
                            color = color_darken(color);
                        }

                        // apply fog
                        if (foggy)
                        {
                            color = color_fog(color, perp_wall_dist);
                        }

                        // draw the pixel
                        pixel_buffer[x + y * WINDOW_WIDTH] = color;
                        depth_buffer[x + y * WINDOW_WIDTH] = perp_wall_dist;
                    }
                }

                if (draw_floor)
                {
                    // x, y position of the floor texel at the bottom of the wall
                    float floor_x_wall;
                    float floor_y_wall;

                    // 4 different wall directions possible
                    if (side == 0 && ray_dir_x > 0)
                    {
                        floor_x_wall = (float)map_x;
                        floor_y_wall = map_y + wall_x;
                    }
                    else if (side == 0 && ray_dir_x < 0)
                    {
                        floor_x_wall = (float)(map_x + 1);
                        floor_y_wall = map_y + wall_x;
                    }
                    else if (side == 1 && ray_dir_y > 0)
                    {
                        floor_x_wall = map_x + wall_x;
                        floor_y_wall = (float)map_y;
                    }
                    else // if (side == 1 && ray_dir_y < 0)
                    {
                        floor_x_wall = map_x + wall_x;
                        floor_y_wall = (float)(map_y + 1);
                    }

                    // becomes < 0 when the integer overflows
                    if (draw_end < 0)
                    {
                        draw_end = WINDOW_HEIGHT;
                    }

                    // draw the floor from draw_end to the bottom of the screen
                    for (int y = draw_end + 1; y < WINDOW_HEIGHT; y++)
                    {
                        float current_dist = WINDOW_HEIGHT / (2.0f * y - WINDOW_HEIGHT);
                        float weight = current_dist / perp_wall_dist;

                        float current_x = weight * floor_x_wall + (1 - weight) * player->pos_x;
                        float current_y = weight * floor_y_wall + (1 - weight) * player->pos_y;

                        // floor
                        {
                            // choose a texture
                            int texture_index = floor_map[(int)current_x][(int)current_y];
                            IMG_Image *texture = textures[texture_index];

                            // x, y coordinate of the texture
                            int texture_x = (int)(current_x * texture->w / FLOOR_TEXTURE_MULT) % texture->w;
                            int texture_y = (int)(current_y * texture->h / FLOOR_TEXTURE_MULT) % texture->h;

                            // get the color on the texture
                            unsigned int color = texture->pixels[texture_x + texture_y * texture->w];

                            // apply fog
                            if (foggy)
                            {
                                color = color_fog(color, current_dist);
                            }

                            // draw the pixel
                            pixel_buffer[x + y * WINDOW_WIDTH] = color;
                            depth_buffer[x + y * WINDOW_WIDTH] = current_dist;
                        }

                        // ceiling
                        {
                            // choose a texture
                            int texture_index = ceiling_map[(int)current_x][(int)current_y];
                            IMG_Image *texture = textures[texture_index];

                            // x, y coordinate of the texture
                            int texture_x = (int)(current_x * texture->w / CEILING_TEXTURE_MULT) % texture->w;
                            int texture_y = (int)(current_y * texture->h / CEILING_TEXTURE_MULT) % texture->h;

                            // get the color on the texture
                            unsigned int color = texture->pixels[texture_x + texture_y * texture->w];

                            // apply fog
                            if (foggy)
                            {
                                color = color_fog(color, current_dist);
                            }

                            // darken the ceiling
                            if (shading)
                            {
                                color = color_darken(color);
                            }

                            // draw the pixel
                            pixel_buffer[x + (WINDOW_HEIGHT - y) * WINDOW_WIDTH] = color;
                            depth_buffer[x + (WINDOW_HEIGHT - y) * WINDOW_WIDTH] = current_dist;
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

                    // make color darker for y-sides
                    if (shading && side == 1)
                    {
                        color = color_darken(color);
                    }

                    // apply fog
                    if (foggy)
                    {
                        color = color_fog(color, perp_wall_dist);
                    }

                    // draw the pixels of the stripe as a vertical line
                    for (int y = draw_start; y <= draw_end; y++)
                    {
                        pixel_buffer[x + y * WINDOW_WIDTH] = color;
                        depth_buffer[x + y * WINDOW_WIDTH] = perp_wall_dist;
                    }
                }

                if (draw_floor)
                {
                    // choose floor and ceiling colors
                    unsigned int floor_color = 0xff646464;
                    unsigned int ceiling_color = floor_color;

                    // darken the ceiling
                    if (shading)
                    {
                        ceiling_color = color_darken(ceiling_color);
                    }

                    // draw the floor
                    for (int y = draw_end + 1; y < WINDOW_HEIGHT; y++)
                    {
                        pixel_buffer[x + y * WINDOW_WIDTH] = floor_color;
                        depth_buffer[x + y * WINDOW_WIDTH] = perp_wall_dist;
                    }

                    // draw the ceiling
                    for (int y = 0; y < draw_start; y++)
                    {
                        pixel_buffer[x + y * WINDOW_WIDTH] = ceiling_color;
                        depth_buffer[x + y * WINDOW_WIDTH] = perp_wall_dist;
                    }
                }
            }
        }

        if (draw_objects)
        {
            // arrays used to sort the objects
            int object_order[NUM_OBJECTS];
            float object_dist[NUM_OBJECTS];

            // sort objects from far to close
            for (int i = 0; i < NUM_OBJECTS; i++)
            {
                object_order[i] = i;
                object_dist[i] = powf(player->pos_x - objects[i].x, 2) + powf(player->pos_y - objects[i].y, 2);
            }
            comb_sort(object_order, object_dist, NUM_OBJECTS);

            // after sorting the objects, do the projection and draw them
            for (int i = 0; i < NUM_OBJECTS; i++)
            {
                struct object object = objects[object_order[i]];

                // translate object position to relative to camera
                float object_x = object.x - player->pos_x;
                float object_y = object.y - player->pos_y;

                // transform object with the inverse camera matrix
                // [ planeX   dirX ] -1                                       [ dirY      -dirX ]
                // [               ]       =  1/(planeX*dirY-dirX*planeY) *   [                 ]
                // [ planeY   dirY ]                                          [ -planeY  planeX ]
                // required for correct matrix multiplication
                float inv_det = 1 / (player->plane_x * player->dir_y - player->dir_x * player->plane_y);

                // transform_y is actually the depth inside the screen, that what Z is in 3D
                float transform_x = inv_det * (player->dir_y * object_x - player->dir_x * object_y);
                float transform_y = inv_det * (-player->plane_y * object_x + player->plane_x * object_y);

                // where the object is on the screen
                int object_screen_x = (int)((WINDOW_WIDTH / 2) * (1 + transform_x / transform_y));

                // calculate width and height of the object on screen
                // using transform_y instead of the real distance prevents fisheye
                int object_width = abs((int)(WINDOW_HEIGHT / transform_y * object.sprite_scale_x));
                int object_height = abs((int)(WINDOW_HEIGHT / transform_y * object.sprite_scale_y));

                // calculate the vertical stripes to draw the object
                int draw_start_x = -object_width / 2 + object_screen_x;
                if (draw_start_x < 0)
                {
                    draw_start_x = 0;
                }
                int draw_end_x = object_width / 2 + object_screen_x;
                if (draw_end_x >= WINDOW_WIDTH)
                {
                    draw_end_x = WINDOW_WIDTH - 1;
                }

                // move the object on the screen
                int translate_y = (int)(object.sprite_translate_y / transform_y);

                // calculate lowest and highest pixel to fill in current stripe
                int draw_start_y = -object_height / 2 + WINDOW_HEIGHT / 2 + translate_y;
                if (draw_start_y < 0)
                {
                    draw_start_y = 0;
                }
                int draw_end_y = object_height / 2 + WINDOW_HEIGHT / 2 + translate_y;
                if (draw_end_y >= WINDOW_HEIGHT)
                {
                    draw_end_y = WINDOW_HEIGHT - 1;
                }

                // calculate angle of object to player
                // float angle = atan2f(object_y, object_x);

                // choose the sprite
                IMG_Image *sprite = sprites[object.sprite_index];

                // loop through every vertical stripe of the sprite on screen
                for (int x = draw_start_x; x < draw_end_x; x++)
                {
                    // x coordinate on the sprite
                    int sprite_x = (256 * (x - (-object_width / 2 + object_screen_x)) * sprite->w / object_width) / 256;

                    // the conditions in the if are:
                    // 1) it's in front of camera plane so you don't see things behind you
                    // 2) it's on the screen (left)
                    // 3) it's on the screen (right)
                    if (transform_y > 0 && x > 0 && x < WINDOW_WIDTH)
                    {
                        for (int y = draw_start_y; y < draw_end_y; y++)
                        {
                            // depth_buffer, with perpendicular distance
                            if (transform_y < depth_buffer[x + y * WINDOW_WIDTH])
                            {
                                // y coordinate on the sprite
                                int sprite_y = ((((y - translate_y) * 256 - WINDOW_HEIGHT * 128 + object_height * 128) * sprite->h) / object_height) / 256;

                                // get current color on the sprite
                                unsigned int color = sprite->pixels[sprite_x + sprite_y * sprite->w];

                                if (foggy)
                                {
                                    color = color_fog(color, transform_y);
                                }

                                // draw the pixel if it isnt't black, black is the invisible color
                                if ((color & 0x00ffffff) != 0)
                                {
                                    // used for translucency
                                    // unsigned int previous_color = pixel_buffer[x + y * w];

                                    pixel_buffer[x + y * WINDOW_WIDTH] = color;
                                    depth_buffer[x + y * WINDOW_WIDTH] = transform_y;
                                }
                            }
                        }
                    }
                }
            }
        }

        // clear the renderer
        SDL_RenderClear(renderer);

        // display pixel buffer
        SDL_UpdateTexture(
            screen,
            NULL,
            pixel_buffer,
            WINDOW_WIDTH * sizeof(unsigned int));
        SDL_RenderCopy(renderer, screen, NULL, NULL);

        // display FPS
        {
            char buffer[256];
            sprintf_s(buffer, sizeof(buffer), "FPS: %d", fps);
            SDL_Surface *text_surface = TTF_RenderText_Solid(font, buffer, (SDL_Color){255, 255, 255, 255});
            SDL_Texture *text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
            SDL_FreeSurface(text_surface);
            SDL_Rect text_rect;
            text_rect.x = 0;
            text_rect.y = 0;
            text_rect.w = 24 * strlen(buffer);
            text_rect.h = 25;
            SDL_RenderCopy(renderer, text_texture, NULL, &text_rect);
            SDL_DestroyTexture(text_texture);
        }

        // display position
        {
            char buffer[256];
            sprintf_s(buffer, sizeof(buffer), "Pos: (%f, %f)", player->pos_x, player->pos_y);
            SDL_Surface *text_surface = TTF_RenderText_Solid(font, buffer, (SDL_Color){255, 255, 255, 255});
            SDL_Texture *text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
            SDL_FreeSurface(text_surface);
            SDL_Rect text_rect;
            text_rect.x = 0;
            text_rect.y = 25;
            text_rect.w = 24 * strlen(buffer);
            text_rect.h = 25;
            SDL_RenderCopy(renderer, text_texture, NULL, &text_rect);
            SDL_DestroyTexture(text_texture);
        }

        // display direction
        {
            char buffer[256];
            sprintf_s(buffer, sizeof(buffer), "Dir: (%f, %f)", player->dir_x, player->dir_y);
            SDL_Surface *text_surface = TTF_RenderText_Solid(font, buffer, (SDL_Color){255, 255, 255, 255});
            SDL_Texture *text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
            SDL_FreeSurface(text_surface);
            SDL_Rect text_rect;
            text_rect.x = 0;
            text_rect.y = 50;
            text_rect.w = 24 * strlen(buffer);
            text_rect.h = 25;
            SDL_RenderCopy(renderer, text_texture, NULL, &text_rect);
            SDL_DestroyTexture(text_texture);
        }

        // display camera plane
        {
            char buffer[256];
            sprintf_s(buffer, sizeof(buffer), "Plane: (%f, %f)", player->plane_x, player->plane_y);
            SDL_Surface *text_surface = TTF_RenderText_Solid(font, buffer, (SDL_Color){255, 255, 255, 255});
            SDL_Texture *text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
            SDL_FreeSurface(text_surface);
            SDL_Rect text_rect;
            text_rect.x = 0;
            text_rect.y = 75;
            text_rect.w = 24 * strlen(buffer);
            text_rect.h = 25;
            SDL_RenderCopy(renderer, text_texture, NULL, &text_rect);
            SDL_DestroyTexture(text_texture);
        }

        // display the renderer
        SDL_RenderPresent(renderer);

        // apply fps cap
        unsigned int frame_end = SDL_GetTicks();
        unsigned int frame_time = frame_end - frame_start;

        if (fps_cap && FRAME_DELAY > frame_time)
        {
            SDL_Delay(FRAME_DELAY - frame_time);
        }
    }

    // free resources
    free(depth_buffer);
    free(pixel_buffer);

    free(player);

    TTF_CloseFont(font);
    TTF_Quit();

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

    for (int i = 0; i < NUM_TEXTURES; i++)
    {
        IMG_FreeImage(textures[i]);
    }
    for (int i = 0; i < NUM_SPRITES; i++)
    {
        IMG_FreeImage(sprites[i]);
    }
    IMG_Quit();

    SDL_DestroyTexture(screen);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}

void player_move(struct player *player, float dx, float dy)
{
    if (wall_map[(int)(player->pos_x + dx)][(int)(player->pos_y)] == 0)
    {
        player->pos_x += dx;
    }
    if (wall_map[(int)(player->pos_x)][(int)(player->pos_y + dy)] == 0)
    {
        player->pos_y += dy;
    }
}

void player_rotate(struct player *player, float angle)
{
    float rot_x = cosf(angle);
    float rot_y = sinf(angle);

    // both camera direction and camera plane must be rotated
    float old_dir_x = player->dir_x;
    player->dir_x = player->dir_x * rot_x - player->dir_y * rot_y;
    player->dir_y = old_dir_x * rot_y + player->dir_y * rot_x;

    float old_plane_x = player->plane_x;
    player->plane_x = player->plane_x * rot_x - player->plane_y * rot_y;
    player->plane_y = old_plane_x * rot_y + player->plane_y * rot_x;
}

void comb_sort(int *order, float *dist, int amount)
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
                float temp_dist = dist[i];
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
    // R, G and B byte each divided through two with a "shift" and an "and"
    return (color >> 1) & 0x7f7f7f;
}

// TODO: optimize
// this drops the framerate by about 20
unsigned int color_fog(unsigned int color, float distance)
{
    // separate the colors
    int red = (color >> 16) & 0x0ff;
    int green = (color >> 8) & 0x0ff;
    int blue = color & 0x0ff;

    // modify the colors
    float fog_intensity = distance * FOG_STRENGTH;

    if (fog_intensity > 1)
    {
        float redf = (float)red;
        float greenf = (float)green;
        float bluef = (float)blue;

        redf /= fog_intensity;
        greenf /= fog_intensity;
        bluef /= fog_intensity;

        red = (int)redf;
        green = (int)greenf;
        blue = (int)bluef;
    }

    // recombine the colors
    return ((red & 0x0ff) << 16) | ((green & 0x0ff) << 8) | (blue & 0x0ff);
}
