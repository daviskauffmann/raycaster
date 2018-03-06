#include <float.h>
#include <math.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_net.h>
#include <SDL/SDL_mixer.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "audio.h"
#include "fonts.h"
#include "images.h"
#include "math.h"
#include "net.h"
#include "utils.h"
#include "window.h"

#define NUM_TEXTURES 8
#define NUM_SPRITES 3
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

// TODO: these should be specified on a per object basis
#define SPRITE_SCALE_X 1.0
#define SPRITE_SCALE_Y 1.0
#define SPRITE_TRANSLATE_Y 0.0

#define FOG_STRENGTH 0.5

typedef struct object_s
{
    double x;
    double y;
    unsigned char sprite_index;
} object_t;

void comb_sort(int *order, double *dist, int amount);
unsigned int color_darken(unsigned int color);
unsigned int color_fog(unsigned int color, double distance);

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

double pos_x = 22.0; // start position
double pos_y = 11.5;
double dir_x = -1.0; // direction vector
double dir_y = 0.0;
double plane_x = 0.0; // camera plane
double plane_y = 1.0;

unsigned int previous_time = 0;
unsigned int current_time = 0;

bool textured = true;
bool draw_walls = true;
bool draw_floor = true;
bool draw_objects = true;
bool shading = true;
bool foggy = true;

bool quit = false;

int main(int argc, char *args[])
{
    // suppress warning C4100
    (void)argc;
    (void)args;

    window_init("Raycaster", 320, 200);
    images_init();
    fonts_init();
    audio_init();
    net_init();

    image_t **textures = malloc(NUM_TEXTURES * sizeof(image_t *));
#if 1
    textures[0] = images_load("assets/images/eagle.png");
    textures[1] = images_load("assets/images/redbrick.png");
    textures[2] = images_load("assets/images/purplestone.png");
    textures[3] = images_load("assets/images/greystone.png");
    textures[4] = images_load("assets/images/bluestone.png");
    textures[5] = images_load("assets/images/mossy.png");
    textures[6] = images_load("assets/images/wood.png");
    textures[7] = images_load("assets/images/colorstone.png");
#else
#define TEXTURE_WIDTH 64
#define TEXTURE_HEIGHT 64

    for (int i = 0; i < NUM_TEXTURES; i++)
    {
        textures[i] = malloc(sizeof(image_t));
        textures[i]->w = TEXTURE_WIDTH;
        textures[i]->h = TEXTURE_HEIGHT;
        textures[i]->pixels = malloc(textures[i]->w * textures[i]->h * sizeof(unsigned int));
    }

    //generate some textures
    for (int x = 0; x < TEXTURE_WIDTH; x++)
    {
        for (int y = 0; y < TEXTURE_HEIGHT; y++)
        {
            int xorcolor = (x * 256 / TEXTURE_WIDTH) ^ (y * 256 / TEXTURE_HEIGHT);
            //int xcolor = x * 256 / texWidth;
            int ycolor = y * 256 / TEXTURE_HEIGHT;
            int xycolor = y * 128 / TEXTURE_HEIGHT + x * 128 / TEXTURE_WIDTH;
            textures[0]->pixels[TEXTURE_WIDTH * y + x] = 65536 * 254 * (x != y && x != TEXTURE_WIDTH - y); //flat red texture with black cross
            textures[1]->pixels[TEXTURE_WIDTH * y + x] = xycolor + 256 * xycolor + 65536 * xycolor;        //sloped greyscale
            textures[2]->pixels[TEXTURE_WIDTH * y + x] = 256 * xycolor + 65536 * xycolor;                  //sloped yellow gradient
            textures[3]->pixels[TEXTURE_WIDTH * y + x] = xorcolor + 256 * xorcolor + 65536 * xorcolor;     //xor greyscale
            textures[4]->pixels[TEXTURE_WIDTH * y + x] = 256 * xorcolor;                                   //xor green
            textures[5]->pixels[TEXTURE_WIDTH * y + x] = 65536 * 192 * (x % 16 && y % 16);                 //red bricks
            textures[6]->pixels[TEXTURE_WIDTH * y + x] = 65536 * ycolor;                                   //red gradient
            textures[7]->pixels[TEXTURE_WIDTH * y + x] = 128 + 256 * 128 + 65536 * 128;                    //flat grey texture
        }
    }
#endif

    image_t **sprites = malloc(NUM_SPRITES * sizeof(image_t));
    sprites[0] = images_load("assets/images/barrel.png");
    sprites[1] = images_load("assets/images/pillar.png");
    sprites[2] = images_load("assets/images/greenlight.png");

    Mix_Music **tracks = malloc(NUM_TRACKS * sizeof(Mix_Music *));
    tracks[0] = audio_load_music("assets/audio/background.mp3");

    Mix_Chunk **sounds = malloc(NUM_SOUNDS * sizeof(Mix_Chunk *));
    sounds[0] = audio_load_chunk("assets/audio/shoot.wav");

    TTF_Font *font = fonts_load("assets/fonts/VeraMono.ttf", 24);

    // printf("FOV: %f\n", 2 * atan(plane_y) / M_PI * 180.0);

    double *depth_buffer = malloc(w * h * sizeof(double));

    while (!quit)
    {
        // timing for input and FPS counter
        previous_time = current_time;
        current_time = SDL_GetTicks();
        double delta_time = (current_time - previous_time) / 1000.0;

        // handle input
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
                case SDLK_F1:
                {
                    textured = !textured;
                }
                break;
                case SDLK_F2:
                {
                    draw_walls = !draw_walls;
                }
                break;
                case SDLK_F3:
                {
                    draw_floor = !draw_floor;
                }
                break;
                case SDLK_F4:
                {
                    draw_objects = !draw_objects;
                }
                break;
                case SDLK_F5:
                {
                    shading = !shading;
                }
                break;
                case SDLK_F6:
                {
                    foggy = !foggy;
                }
                break;
                case SDLK_1:
                {
                    if (audio_playing_music())
                    {
                        audio_stop_music();
                    }
                    else
                    {
                        audio_play_music(tracks[0], -1);
                    }
                }
                break;
                case SDLK_2:
                {
                    if (audio_playing_music())
                    {
                        if (audio_paused_music())
                        {
                            audio_resume_music();
                        }
                        else
                        {
                            audio_pause_music();
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

        const unsigned char *keys = SDL_GetKeyboardState(NULL);
        unsigned int mouse = SDL_GetMouseState(NULL, NULL);

        // calculate base movement speed
        // the constant value is in squares/second
        double move_speed = MOVE_SPEED * delta_time;

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
            // precompute 1 / sqrt(2)
            move_speed *= 0.71;
        }

        // move forward
        if (keys[SDL_SCANCODE_W])
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
        if (keys[SDL_SCANCODE_A])
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
        if (keys[SDL_SCANCODE_S])
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
        if (keys[SDL_SCANCODE_D])
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
        local double shoot_timer = 0.0;
        shoot_timer += delta_time;
        if (mouse & SDL_BUTTON(SDL_BUTTON_LEFT))
        {
            if (shoot_timer >= 0.25)
            {
                shoot_timer = 0.0;

                audio_play_chunk(sounds[0], 0);
            }
        }

        // raycasting
        for (int x = 0; x < w; x++)
        {
            // clear the pixel and depth buffers
            for (int y = 0; y < h; y++)
            {
                window_pset(x, y, 0);
                depth_buffer[x + y * w] = DBL_MAX;
            }

            // calculate x-coordinate in camera space
            double camera_x = (2.0 * x / w) - 1;

            // calculate ray position and direction
            double ray_dir_x = (camera_x * plane_x) + dir_x;
            double ray_dir_y = (camera_x * plane_y) + dir_y;

            // which box of the map we're in
            int map_x = (int)pos_x;
            int map_y = (int)pos_y;

            // length of ray from current position to next x or y-side
            double side_dist_x;
            double side_dist_y;

            // length of ray from one x or y-side to next x or y-side
            double delta_dist_x = fabs(1 / ray_dir_x);
            double delta_dist_y = fabs(1 / ray_dir_y);

            // what direction to step in x or y-direction (either +1 or -1)
            int step_x;
            int step_y;

            // calculate step and initial sideDist
            if (ray_dir_x < 0)
            {
                side_dist_x = (pos_x - map_x) * delta_dist_x;
                step_x = -1;
            }
            else
            {
                side_dist_x = (map_x + 1 - pos_x) * delta_dist_x;
                step_x = 1;
            }
            if (ray_dir_y < 0)
            {
                side_dist_y = (pos_y - map_y) * delta_dist_y;
                step_y = -1;
            }
            else
            {
                side_dist_y = (map_y + 1 - pos_y) * delta_dist_y;
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
            double perp_wall_dist =
                side == 0
                    ? (map_x - pos_x + (1 - step_x) / 2) / ray_dir_x
                    : (map_y - pos_y + (1 - step_y) / 2) / ray_dir_y;

            // calculate height of line to draw on screen
            int line_height = (int)(h / perp_wall_dist);

            // calculate lowest and highest pixel to fill in current stripe
            int draw_start = -line_height / 2 + h / 2;
            if (draw_start < 0)
            {
                draw_start = 0;
            }
            int draw_end = line_height / 2 + h / 2;
            if (draw_end >= h)
            {
                draw_end = h - 1;
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
                    image_t *texture = textures[texture_index];

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
                        int texture_y = (((y * 256 - h * 128 + line_height * 128) * texture->h) / line_height) / 256;

                        // get the color on the texture
                        unsigned int color = texture->pixels[texture_x + texture_y * texture->w];

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
                        window_pset(x, y, color);
                        depth_buffer[x + y * w] = perp_wall_dist;
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
                        floor_x_wall = (double)map_x;
                        floor_y_wall = map_y + wall_x;
                    }
                    else if (side == 0 && ray_dir_x < 0)
                    {
                        floor_x_wall = (double)(map_x + 1);
                        floor_y_wall = map_y + wall_x;
                    }
                    else if (side == 1 && ray_dir_y > 0)
                    {
                        floor_x_wall = map_x + wall_x;
                        floor_y_wall = (double)map_y;
                    }
                    else // if (side == 1 && ray_dir_y < 0)
                    {
                        floor_x_wall = map_x + wall_x;
                        floor_y_wall = (double)(map_y + 1);
                    }

                    // becomes < 0 when the integer overflows
                    if (draw_end < 0)
                    {
                        draw_end = h;
                    }

                    // draw the floor from draw_end to the bottom of the screen
                    for (int y = draw_end + 1; y < h; y++)
                    {
                        double current_dist = h / (2.0 * y - h);
                        double weight = current_dist / perp_wall_dist;

                        double current_x = weight * floor_x_wall + (1 - weight) * pos_x;
                        double current_y = weight * floor_y_wall + (1 - weight) * pos_y;

                        // floor
                        {
                            // choose a texture
                            int texture_index = floor_map[(int)current_x][(int)current_y];
                            image_t *texture = textures[texture_index];

                            // x, y coordinate of the texture
                            int texture_x = (int)(current_x * texture->w / FLOOR_TEXTURE_MULT) % texture->w;
                            int texture_y = (int)(current_y * texture->h / FLOOR_TEXTURE_MULT) % texture->h;

                            // get the color on the texture
                            unsigned int color = texture->pixels[texture_x + texture_y * texture->w];

                            if (foggy)
                            {
                                color = color_fog(color, current_dist);
                            }

                            // draw the pixel
                            window_pset(x, y, color);
                            depth_buffer[x + y * w] = current_dist;
                        }

                        // ceiling
                        {
                            // choose a texture
                            int texture_index = ceiling_map[(int)current_x][(int)current_y];
                            image_t *texture = textures[texture_index];

                            // x, y coordinate of the texture
                            int texture_x = (int)(current_x * texture->w / CEILING_TEXTURE_MULT) % texture->w;
                            int texture_y = (int)(current_y * texture->h / CEILING_TEXTURE_MULT) % texture->h;

                            // get the color on the texture
                            unsigned int color = texture->pixels[texture_x + texture_y * texture->w];

                            if (foggy)
                            {
                                color = color_fog(color, current_dist);
                            }

                            if (shading)
                            {
                                color = color_darken(color);
                            }

                            // draw the pixel
                            window_pset(x, h - y, color);
                            depth_buffer[x + (h - y) * w] = current_dist;
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
                        window_pset(x, y, color);
                        depth_buffer[x + y * w] = perp_wall_dist;
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
                    for (int y = draw_end + 1; y < h; y++)
                    {
                        window_pset(x, y, floor_color);
                        depth_buffer[x + y * w] = perp_wall_dist;
                    }

                    // draw the ceiling
                    for (int y = 0; y < draw_start; y++)
                    {
                        window_pset(x, y, ceiling_color);
                        depth_buffer[x + y * w] = perp_wall_dist;
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
                double inv_det = 1 / (plane_x * dir_y - dir_x * plane_y);

                // transform_y is actually the depth inside the screen, that what Z is in 3D
                double transform_x = inv_det * (dir_y * object_x - dir_x * object_y);
                double transform_y = inv_det * (-plane_y * object_x + plane_x * object_y);

                // where the object is on the screen
                int object_screen_x = (int)((w / 2) * (1 + transform_x / transform_y));

                // calculate width and height of the object on screen
                // using transform_y instead of the real distance prevents fisheye
                int object_width = abs((int)(h / transform_y * SPRITE_SCALE_X));
                int object_height = abs((int)(h / transform_y * SPRITE_SCALE_Y));

                // calculate the vertical stripes to draw the object
                int draw_start_x = -object_width / 2 + object_screen_x;
                if (draw_start_x < 0)
                {
                    draw_start_x = 0;
                }
                int draw_end_x = object_width / 2 + object_screen_x;
                if (draw_end_x >= w)
                {
                    draw_end_x = w - 1;
                }

                // move the object on the screen
                int translate_y = (int)(SPRITE_TRANSLATE_Y / transform_y);

                // calculate lowest and highest pixel to fill in current stripe
                int draw_start_y = -object_height / 2 + h / 2 + translate_y;
                if (draw_start_y < 0)
                {
                    draw_start_y = 0;
                }
                int draw_end_y = object_height / 2 + h / 2 + translate_y;
                if (draw_end_y >= h)
                {
                    draw_end_y = h - 1;
                }

                // calculate angle of object to player
                // double angle = atan2(object_y, object_y);

                // choose the sprite
                int sprite_index = object.sprite_index;
                image_t *sprite = sprites[sprite_index];

                // loop through every vertical stripe of the sprite on screen
                for (int x = draw_start_x; x < draw_end_x; x++)
                {
                    // x coordinate on the sprite
                    int sprite_x = (256 * (x - (-object_width / 2 + object_screen_x)) * sprite->w / object_width) / 256;

                    // the conditions in the if are:
                    // 1) it's in front of camera plane so you don't see things behind you
                    // 2) it's on the screen (left)
                    // 3) it's on the screen (right)
                    if (transform_y > 0 && x > 0 && x < w)
                    {
                        for (int y = draw_start_y; y < draw_end_y; y++)
                        {
                            // depth_buffer, with perpendicular distance
                            if (transform_y < depth_buffer[x + y * w])
                            {
                                // y coordinate on the sprite
                                int sprite_y = ((((y - translate_y) * 256 - h * 128 + object_height * 128) * sprite->h) / object_height) / 256;

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
                                    // unsigned int previous_color = window_pget(x, y);

                                    window_pset(x, y, color);
                                    depth_buffer[x + y * w] = transform_y;
                                }
                            }
                        }
                    }
                }
            }
        }

        // calculate fps
        static double fps_update_timer = 0.0;
        static int fps = 0;
        fps_update_timer += delta_time;
        if (fps_update_timer >= 0.25)
        {
            fps_update_timer = 0.0;
            fps = (int)(1 / delta_time);
        }

        window_update();

        // // display FPS
        // {
        //     int fps_text_len = snprintf(NULL, 0, "FPS: %d", fps);
        //     char *fps_text_buffer = malloc(fps_text_len + 1);
        //     sprintf_s(fps_text_buffer, fps_text_len + 1, "FPS: %d", fps);
        //     SDL_Surface *fps_text_surface = TTF_RenderText_Solid(font, fps_text_buffer, (SDL_Color){255, 255, 255, 255});
        //     free(fps_text_buffer);
        //     SDL_Texture *fps_text_texture = SDL_CreateTextureFromSurface(renderer, fps_text_surface);
        //     SDL_FreeSurface(fps_text_surface);
        //     SDL_Rect fps_text_rect;
        //     fps_text_rect.x = 0;
        //     fps_text_rect.y = 0;
        //     fps_text_rect.w = 24 * fps_text_len;
        //     fps_text_rect.h = 25;
        //     SDL_RenderCopy(renderer, fps_text_texture, NULL, &fps_text_rect);
        //     SDL_DestroyTexture(fps_text_texture);
        // }

        // // display position
        // {
        //     int pos_text_len = snprintf(NULL, 0, "Pos: (%f, %f)", pos_x, pos_y);
        //     char *pos_text_buffer = malloc(pos_text_len + 1);
        //     sprintf_s(pos_text_buffer, pos_text_len + 1, "Pos: (%f, %f)", pos_x, pos_y);
        //     SDL_Surface *pos_text_surface = TTF_RenderText_Solid(font, pos_text_buffer, (SDL_Color){255, 255, 255, 255});
        //     SDL_Texture *pos_text_texture = SDL_CreateTextureFromSurface(renderer, pos_text_surface);
        //     SDL_FreeSurface(pos_text_surface);
        //     SDL_Rect pos_text_rect;
        //     pos_text_rect.x = 0;
        //     pos_text_rect.y = 25;
        //     pos_text_rect.w = 24 * pos_text_len;
        //     pos_text_rect.h = 25;
        //     SDL_RenderCopy(renderer, pos_text_texture, NULL, &pos_text_rect);
        //     SDL_DestroyTexture(pos_text_texture);
        // }

        // // display direction
        // {
        //     int dir_text_len = snprintf(NULL, 0, "Dir: (%f, %f)", dir_x, dir_y);
        //     char *dir_text_buffer = malloc(dir_text_len + 1);
        //     sprintf_s(dir_text_buffer, dir_text_len + 1, "Dir: (%f, %f)", dir_x, dir_y);
        //     SDL_Surface *dir_text_surface = TTF_RenderText_Solid(font, dir_text_buffer, (SDL_Color){255, 255, 255, 255});
        //     SDL_Texture *dir_text_texture = SDL_CreateTextureFromSurface(renderer, dir_text_surface);
        //     SDL_FreeSurface(dir_text_surface);
        //     SDL_Rect dir_text_rect;
        //     dir_text_rect.x = 0;
        //     dir_text_rect.y = 50;
        //     dir_text_rect.w = 24 * dir_text_len;
        //     dir_text_rect.h = 25;
        //     SDL_RenderCopy(renderer, dir_text_texture, NULL, &dir_text_rect);
        //     SDL_DestroyTexture(dir_text_texture);
        // }

        // // display camera plane
        // {
        //     int plane_text_len = snprintf(NULL, 0, "Plane: (%f, %f)", plane_x, plane_y);
        //     char *plane_text_buffer = malloc(plane_text_len + 1);
        //     sprintf_s(plane_text_buffer, plane_text_len + 1, "Plane: (%f, %f)", plane_x, plane_y);
        //     SDL_Surface *plane_text_surface = TTF_RenderText_Solid(font, plane_text_buffer, (SDL_Color){255, 255, 255, 255});
        //     SDL_Texture *plane_text_texture = SDL_CreateTextureFromSurface(renderer, plane_text_surface);
        //     SDL_FreeSurface(plane_text_surface);
        //     SDL_Rect plane_text_rect;
        //     plane_text_rect.x = 0;
        //     plane_text_rect.y = 75;
        //     plane_text_rect.w = 24 * plane_text_len;
        //     plane_text_rect.h = 25;
        //     SDL_RenderCopy(renderer, plane_text_texture, NULL, &plane_text_rect);
        //     SDL_DestroyTexture(plane_text_texture);
        // }
    }

    fonts_unload(font);

    for (int i = 0; i < NUM_TRACKS; i++)
    {
        audio_unload_music(tracks[i]);
    }
    for (int i = 0; i < NUM_SOUNDS; i++)
    {
        audio_unload_chunk(sounds[i]);
    }

    for (int i = 0; i < NUM_TEXTURES; i++)
    {
        images_unload(textures[i]);
    }

    free(depth_buffer);

    net_quit();
    audio_quit();
    fonts_quit();
    images_quit();
    window_close();

    return 0;
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
        double double_red = (double)red;
        double double_green = (double)green;
        double double_blue = (double)blue;

        double_red /= fog_intensity;
        double_green /= fog_intensity;
        double_blue /= fog_intensity;

        red = (int)double_red;
        green = (int)double_green;
        blue = (int)double_blue;
    }

    // recombine the colors
    return ((red & 0x0ff) << 16) | ((green & 0x0ff) << 8) | (blue & 0x0ff);
}
