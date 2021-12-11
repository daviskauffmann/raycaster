#include <float.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WINDOW_TITLE "Raycaster"
#define WINDOW_WIDTH 320
#define WINDOW_HEIGHT 200

#define NUM_TEXTURES 8
#define NUM_SPRITES 3

#define MAP_WIDTH 24
#define MAP_HEIGHT 24
#define NUM_BILLBOARDS 19

#define SPEED 20.0f
#define SPRINT_MULT 2.0f
#define DRAG 5.0f
#define ROTATE_SPEED 3.0f
#define MOUSE_SENSITIVITY 2.0f

#define FLOOR_TEXTURE_MULT 1
#define CEILING_TEXTURE_MULT 1

#define FOG_STRENGTH 0.5f

#define FPS_CAP 60
#define FRAME_DELAY (1000 / FPS_CAP)

#define FONT_SIZE 16

#define PI 3.14159f

struct bitmap
{
    int width;
    int height;
    unsigned int *pixels;
};

struct camera
{
    float pos_x;
    float pos_y;
    float dir_x;
    float dir_y;
    float vel_x;
    float vel_y;
    float plane_x;
    float plane_y;
};

struct billboard
{
    float x;
    float y;
    unsigned char sprite_index;
    float sprite_scale_x;
    float sprite_scale_y;
    float sprite_translate_y;
};

unsigned char wall_map[MAP_WIDTH][MAP_HEIGHT] =
    {{8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 4, 4, 6, 4, 4, 6, 4, 6, 4, 4, 4, 6, 4},
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

unsigned char floor_map[MAP_WIDTH][MAP_HEIGHT] =
    {{3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
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

unsigned char ceiling_map[MAP_WIDTH][MAP_HEIGHT] =
    {{6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
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

struct billboard billboards[NUM_BILLBOARDS] =
    {{20.5f, 11.5f, 2, 1.0f, 1.0f, 0.0f},
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
     {10.5f, 15.8f, 0, 1.0f, 1.0f, 0.0f}};

unsigned int get_pixel(SDL_Surface *surface, int x, int y)
{
    int bpp = surface->format->BytesPerPixel;

    // here p is the address to the pixel we want to retrieve
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
        return 0; // shouldn't happen, but avoids warnings
    }
    break;
    }
}

void set_pixel(SDL_Surface *surface, int x, int y, unsigned int pixel)
{
    int bpp = surface->format->BytesPerPixel;

    // here p is the address to the pixel we want to retrieve
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

struct bitmap *bitmap_create(const char *file)
{
    struct bitmap *bitmap = malloc(sizeof(struct bitmap));

    if (!bitmap)
    {
        return NULL;
    }

    SDL_Surface *surface = IMG_Load(file);

    if (!surface)
    {
        return NULL;
    }

    bitmap->width = surface->w;
    bitmap->height = surface->h;
    bitmap->pixels = malloc(bitmap->width * bitmap->height * sizeof(unsigned int));
    for (int x = 0; x < bitmap->width; x++)
    {
        for (int y = 0; y < bitmap->height; y++)
        {
            bitmap->pixels[x + y * bitmap->width] = get_pixel(surface, x, y);
        }
    }

    SDL_FreeSurface(surface);

    return bitmap;
}

void bitmap_destroy(struct bitmap *bitmap)
{
    free(bitmap->pixels);
    free(bitmap);
}

void camera_accelerate(struct camera *camera, float acc_x, float acc_y, float delta_time)
{
    float new_pos_x = 0.5f * acc_x * powf(delta_time, 2) + camera->vel_x * delta_time + camera->pos_x;
    float new_pos_y = 0.5f * acc_y * powf(delta_time, 2) + camera->vel_y * delta_time + camera->pos_y;

    if (wall_map[(int)new_pos_x][(int)camera->pos_y] == 0)
    {
        camera->pos_x = new_pos_x;
        camera->vel_x = acc_x * delta_time + camera->vel_x;
    }
    else
    {
        camera->vel_x = 0.0f;
    }

    if (wall_map[(int)camera->pos_x][(int)new_pos_y] == 0)
    {
        camera->pos_y = new_pos_y;
        camera->vel_y = acc_y * delta_time + camera->vel_y;
    }
    else
    {
        camera->vel_y = 0.0f;
    }
}

void camera_rotate(struct camera *camera, float angle)
{
    float rot_x = cosf(angle);
    float rot_y = sinf(angle);

    // both camera direction and camera plane must be rotated
    float old_dir_x = camera->dir_x;
    camera->dir_x = camera->dir_x * rot_x - camera->dir_y * rot_y;
    camera->dir_y = old_dir_x * rot_y + camera->dir_y * rot_x;

    float old_plane_x = camera->plane_x;
    camera->plane_x = camera->plane_x * rot_x - camera->plane_y * rot_y;
    camera->plane_y = old_plane_x * rot_y + camera->plane_y * rot_x;
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

// algorithm at https://lodev.org/cgtutor/raycasting.html
void render(
    unsigned int *pixel_buffer,
    float *depth_buffer,
    struct camera *camera,
    struct bitmap **textures,
    struct bitmap **sprites,
    bool textured,
    bool draw_walls,
    bool draw_floor,
    bool draw_billboards,
    bool shading,
    bool foggy)
{
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
        float ray_dir_x = (camera_x * camera->plane_x) + camera->dir_x;
        float ray_dir_y = (camera_x * camera->plane_y) + camera->dir_y;

        // which box of the map we're in
        int map_x = (int)camera->pos_x;
        int map_y = (int)camera->pos_y;

        // length of ray from current position to next x or y-side
        float side_dist_x;
        float side_dist_y;

        // length of ray from one x or y-side to next x or y-side
        float delta_dist_x = fabsf(1 / ray_dir_x);
        float delta_dist_y = fabsf(1 / ray_dir_y);

        // what direction to step in x or y-direction (either +1 or -1)
        int step_x;
        int step_y;

        // calculate step and initial side_dist
        if (ray_dir_x < 0)
        {
            side_dist_x = (camera->pos_x - map_x) * delta_dist_x;
            step_x = -1;
        }
        else
        {
            side_dist_x = (map_x + 1 - camera->pos_x) * delta_dist_x;
            step_x = 1;
        }
        if (ray_dir_y < 0)
        {
            side_dist_y = (camera->pos_y - map_y) * delta_dist_y;
            step_y = -1;
        }
        else
        {
            side_dist_y = (map_y + 1 - camera->pos_y) * delta_dist_y;
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
                ? (map_x - camera->pos_x + (1 - step_x) / 2) / ray_dir_x
                : (map_y - camera->pos_y + (1 - step_y) / 2) / ray_dir_y;

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
                wall_x = camera->pos_y + perp_wall_dist * ray_dir_y;
            }
            else
            {
                wall_x = camera->pos_x + perp_wall_dist * ray_dir_x;
            }
            wall_x -= floorf(wall_x);

            if (draw_walls)
            {
                // choose a texture
                int texture_index = wall_map[map_x][map_y] - 1;
                struct bitmap *texture = textures[texture_index];

                // x coordinate on the texture
                int texture_x = (int)(wall_x * texture->width);
                if (side == 0 && ray_dir_x > 0)
                {
                    texture_x = texture->width - texture_x - 1;
                }
                if (side == 1 && ray_dir_y < 0)
                {
                    texture_x = texture->width - texture_x - 1;
                }

                for (int y = draw_start; y < draw_end; y++)
                {
                    // y coordinate on the texture
                    float d = (float)y * 2 - WINDOW_HEIGHT + line_height;
                    int texture_y = (int)(d * texture->height / line_height / 2);

                    // get the color on the texture
                    unsigned int color = texture->pixels[texture_x + texture_y * texture->width];

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

                    float current_x = weight * floor_x_wall + (1 - weight) * camera->pos_x;
                    float current_y = weight * floor_y_wall + (1 - weight) * camera->pos_y;

                    // floor
                    {
                        // choose a texture
                        int texture_index = floor_map[(int)current_x][(int)current_y];
                        struct bitmap *texture = textures[texture_index];

                        // x, y coordinate of the texture
                        int texture_x = (int)(current_x * texture->width / FLOOR_TEXTURE_MULT) % texture->width;
                        int texture_y = (int)(current_y * texture->height / FLOOR_TEXTURE_MULT) % texture->height;

                        // get the color on the texture
                        unsigned int color = texture->pixels[texture_x + texture_y * texture->width];

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
                        struct bitmap *texture = textures[texture_index];

                        // x, y coordinate of the texture
                        int texture_x = (int)(current_x * texture->width / CEILING_TEXTURE_MULT) % texture->width;
                        int texture_y = (int)(current_y * texture->height / CEILING_TEXTURE_MULT) % texture->height;

                        // get the color on the texture
                        unsigned int color = texture->pixels[texture_x + texture_y * texture->width];

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
                    color = 0xff0000ff; // red
                    break;
                case 1:
                    color = 0xff00ff00; // green
                    break;
                case 2:
                    color = 0xffff0000; // blue
                    break;
                case 3:
                    color = 0xffffff00; // cyan
                    break;
                case 4:
                    color = 0xffff00ff; // magenta
                    break;
                case 5:
                    color = 0xff00ffff; // yellow
                    break;
                case 6:
                    color = 0xff800080; // purple
                    break;
                case 7:
                    color = 0xff000080; // maroon
                    break;
                case 8:
                    color = 0xffffffff; // white
                    break;
                default:
                    color = 0x00000000; // black
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
                for (int y = draw_start; y < draw_end; y++)
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

    if (draw_billboards)
    {
        // arrays used to sort the billboards
        int billboard_order[NUM_BILLBOARDS];
        float billboard_dist[NUM_BILLBOARDS];

        // sort billboards from far to close
        for (int i = 0; i < NUM_BILLBOARDS; i++)
        {
            billboard_order[i] = i;
            billboard_dist[i] = powf(camera->pos_x - billboards[i].x, 2) + powf(camera->pos_y - billboards[i].y, 2);
        }
        comb_sort(billboard_order, billboard_dist, NUM_BILLBOARDS);

        // after sorting the billboards, do the projection and draw them
        for (int i = 0; i < NUM_BILLBOARDS; i++)
        {
            struct billboard billboard = billboards[billboard_order[i]];

            // translate billboard position to relative to camera
            float billboard_x = billboard.x - camera->pos_x;
            float billboard_y = billboard.y - camera->pos_y;

            // transform billboard with the inverse camera matrix
            // [ planeX   dirX ] -1                                         [ dirY      -dirX ]
            // [               ]    = 1 / (planeX * dirY - dirX * planeY) * [                 ]
            // [ planeY   dirY ]                                            [ -planeY  planeX ]
            // required for correct matrix multiplication
            float inv_det = 1 / (camera->plane_x * camera->dir_y - camera->dir_x * camera->plane_y);

            // transform_y is actually the depth inside the screen, that what Z is in 3D
            float transform_x = inv_det * (camera->dir_y * billboard_x - camera->dir_x * billboard_y);
            float transform_y = inv_det * (-camera->plane_y * billboard_x + camera->plane_x * billboard_y);

            // where the billboard is on the screen
            int billboard_screen_x = (int)((WINDOW_WIDTH / 2) * (1 + transform_x / transform_y));

            // calculate width and height of the billboard on screen
            // using transform_y instead of the real distance prevents fisheye
            int billboard_width = abs((int)(WINDOW_HEIGHT / transform_y * billboard.sprite_scale_x));
            int billboard_height = abs((int)(WINDOW_HEIGHT / transform_y * billboard.sprite_scale_y));

            // calculate the vertical stripes to draw the billboard
            int draw_start_x = -billboard_width / 2 + billboard_screen_x;
            if (draw_start_x < 0)
            {
                draw_start_x = 0;
            }
            int draw_end_x = billboard_width / 2 + billboard_screen_x;
            if (draw_end_x >= WINDOW_WIDTH)
            {
                draw_end_x = WINDOW_WIDTH - 1;
            }

            // move the billboard on the screen
            int translate_y = (int)(billboard.sprite_translate_y / transform_y);

            // calculate lowest and highest pixel to fill in current stripe
            int draw_start_y = -billboard_height / 2 + WINDOW_HEIGHT / 2 + translate_y;
            if (draw_start_y < 0)
            {
                draw_start_y = 0;
            }
            int draw_end_y = billboard_height / 2 + WINDOW_HEIGHT / 2 + translate_y;
            if (draw_end_y >= WINDOW_HEIGHT)
            {
                draw_end_y = WINDOW_HEIGHT - 1;
            }

            // calculate angle of billboard to camera
            // float angle = atan2f(billboard_y, billboard_x);

            // choose the sprite
            struct bitmap *sprite = sprites[billboard.sprite_index];

            // loop through every vertical stripe of the sprite on screen
            for (int x = draw_start_x; x < draw_end_x; x++)
            {
                // x coordinate on the sprite
                int sprite_x = (x - (-billboard_width / 2 + billboard_screen_x)) * sprite->width / billboard_width;

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
                            float d = ((float)y - translate_y) * 2 - WINDOW_HEIGHT + billboard_height;
                            int sprite_y = (int)(d * sprite->height / billboard_height / 2);

                            // get current color on the sprite
                            unsigned int color = sprite->pixels[sprite_x + sprite_y * sprite->width];

                            // apply fog
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
}

void draw_text(SDL_Renderer *renderer, TTF_Font *font, int px, int x, int y, SDL_Color fg, const char *const fmt, ...)
{
    char text[256];

    va_list ap;
    va_start(ap, fmt);

    vsprintf_s(text, sizeof(text), fmt, ap);

    va_end(ap);

    SDL_Surface *text_surface = TTF_RenderText_Solid(font, text, fg);

    SDL_Texture *text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);

    SDL_FreeSurface(text_surface);

    SDL_Rect text_rect;
    text_rect.x = x;
    text_rect.y = y;
    text_rect.w = px * (int)strlen(text);
    text_rect.h = px;

    SDL_RenderCopy(renderer, text_texture, NULL, &text_rect);

    SDL_DestroyTexture(text_texture);
}

int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);
    TTF_Init();
    SDL_SetRelativeMouseMode(SDL_TRUE);

    SDL_Window *window = SDL_CreateWindow(
        WINDOW_TITLE,
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        0);

    SDL_Renderer *renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_SOFTWARE);

    SDL_Texture *screen = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ABGR8888,
        SDL_TEXTUREACCESS_STREAMING,
        WINDOW_WIDTH,
        WINDOW_HEIGHT);

    struct bitmap *textures[NUM_TEXTURES];
    textures[0] = bitmap_create("assets/images/eagle.png");
    textures[1] = bitmap_create("assets/images/redbrick.png");
    textures[2] = bitmap_create("assets/images/purplestone.png");
    textures[3] = bitmap_create("assets/images/greystone.png");
    textures[4] = bitmap_create("assets/images/bluestone.png");
    textures[5] = bitmap_create("assets/images/mossy.png");
    textures[6] = bitmap_create("assets/images/wood.png");
    textures[7] = bitmap_create("assets/images/colorstone.png");

    struct bitmap *sprites[NUM_SPRITES];
    sprites[0] = bitmap_create("assets/images/barrel.png");
    sprites[1] = bitmap_create("assets/images/pillar.png");
    sprites[2] = bitmap_create("assets/images/greenlight.png");

    TTF_Font *font = TTF_OpenFont("assets/fonts/VeraMono.ttf", 24);

    struct camera *camera = malloc(sizeof(struct camera));

    camera->pos_x = 22.0f;
    camera->pos_y = 11.5f;
    camera->dir_x = -1.0f;
    camera->dir_y = 0.0f;
    camera->plane_x = 0.0f;
    camera->plane_y = 1.0f;
    camera->vel_x = 0.0f;
    camera->vel_y = 0.0f;

    printf("FOV: %d\n", (int)(2 * atanf(camera->plane_y) / PI * 180));

    unsigned int *pixel_buffer = malloc(WINDOW_WIDTH * WINDOW_HEIGHT * sizeof(unsigned int));
    float *depth_buffer = malloc(WINDOW_WIDTH * WINDOW_HEIGHT * sizeof(float));

    bool textured = true;
    bool draw_walls = true;
    bool draw_floor = true;
    bool draw_billboards = true;
    bool shading = true;
    bool foggy = true;

    bool quit = false;
    while (!quit)
    {
        static unsigned int current_time = 0;
        unsigned int frame_start = SDL_GetTicks();
        unsigned int previous_time = current_time;
        current_time = frame_start;
        float delta_time = (current_time - previous_time) / 1000.0f;
        unsigned int fps = (unsigned int)(1 / delta_time);

        int num_keys;
        const unsigned char *keys = SDL_GetKeyboardState(&num_keys);

        int mouse_x, mouse_y;
        unsigned int mouse = SDL_GetMouseState(&mouse_x, &mouse_y);

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_MOUSEMOTION:
            {
                int mouse_dx = event.motion.xrel;
                float angle = -mouse_dx / 1000.0f * MOUSE_SENSITIVITY;
                camera_rotate(camera, angle);
            }
            break;
            case SDL_KEYDOWN:
            {
                switch (event.key.keysym.sym)
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
                    draw_billboards = !draw_billboards;
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

        float acc_x = 0.0f;
        float acc_y = 0.0f;

        if (keys[SDL_SCANCODE_W])
        {
            acc_x += camera->dir_x;
            acc_y += camera->dir_y;
        }

        if (keys[SDL_SCANCODE_A])
        {
            acc_x += -camera->dir_y;
            acc_y += camera->dir_x;
        }

        if (keys[SDL_SCANCODE_S])
        {
            acc_x += -camera->dir_x;
            acc_y += -camera->dir_y;
        }

        if (keys[SDL_SCANCODE_D])
        {
            acc_x += camera->dir_y;
            acc_y += -camera->dir_x;
        }

        float acc_len = sqrtf(acc_x * acc_x + acc_y * acc_y);
        if (acc_len > 1.0f)
        {
            acc_x *= 1 / acc_len;
            acc_y *= 1 / acc_len;
        }

        acc_x *= SPEED;
        acc_y *= SPEED;

        if (keys[SDL_SCANCODE_LSHIFT])
        {
            acc_x *= SPRINT_MULT;
            acc_y *= SPRINT_MULT;
        }

        acc_x -= camera->vel_x * DRAG;
        acc_y -= camera->vel_y * DRAG;

        camera_accelerate(camera, acc_x, acc_y, delta_time);

        float angle = ROTATE_SPEED * delta_time;

        if (keys[SDL_SCANCODE_Q])
        {
            camera_rotate(camera, angle);
        }

        if (keys[SDL_SCANCODE_E])
        {
            camera_rotate(camera, -angle);
        }

        render(
            pixel_buffer,
            depth_buffer,
            camera,
            textures,
            sprites,
            textured,
            draw_walls,
            draw_floor,
            draw_billboards,
            shading,
            foggy);

        SDL_RenderClear(renderer);
        SDL_UpdateTexture(
            screen,
            NULL,
            pixel_buffer,
            WINDOW_WIDTH * sizeof(unsigned int));
        SDL_RenderCopy(renderer, screen, NULL, NULL);

        {
            int line = 0;
            SDL_Color white_color = {255, 255, 255, 255};

            draw_text(
                renderer,
                font,
                FONT_SIZE,
                0,
                FONT_SIZE * line++,
                white_color,
                "fps: %d",
                fps);

            draw_text(
                renderer,
                font,
                FONT_SIZE,
                0,
                FONT_SIZE * line++,
                white_color,
                "pos: (%f, %f)",
                camera->pos_x,
                camera->pos_y);

            draw_text(
                renderer,
                font,
                FONT_SIZE,
                0,
                FONT_SIZE * line++,
                white_color,
                "dir: (%f, %f), ang: %f",
                camera->dir_x,
                camera->dir_y,
                atanf(camera->dir_y / camera->dir_x) * 180.0f / PI);

            draw_text(
                renderer,
                font,
                FONT_SIZE,
                0,
                FONT_SIZE * line++,
                white_color,
                "vel: (%f, %f), mag: %f, ang: %f",
                camera->vel_x,
                camera->vel_y,
                sqrtf(powf(camera->vel_x, 2.0f) + powf(camera->vel_y, 2.0f)),
                atanf(camera->vel_y / camera->vel_x) * 180.0f / PI);

            draw_text(
                renderer,
                font,
                FONT_SIZE,
                0,
                FONT_SIZE * line++,
                white_color,
                "acc: (%f, %f), mag: %f, ang: %f",
                acc_x,
                acc_y,
                sqrtf(powf(acc_x, 2.0f) + powf(acc_y, 2.0f)),
                atanf(acc_y / acc_x) * 180.0f / PI);

            draw_text(
                renderer,
                font,
                FONT_SIZE,
                0,
                FONT_SIZE * line++,
                white_color,
                "plane: (%f, %f)",
                camera->plane_x,
                camera->plane_y);
        }

        SDL_RenderPresent(renderer);

        unsigned int frame_end = SDL_GetTicks();
        unsigned int frame_time = frame_end - frame_start;
        if (FRAME_DELAY > frame_time)
        {
            SDL_Delay(FRAME_DELAY - frame_time);
        }
    }

    free(depth_buffer);
    free(pixel_buffer);

    free(camera);

    TTF_CloseFont(font);

    for (int i = 0; i < NUM_TEXTURES; i++)
    {
        bitmap_destroy(textures[i]);
    }
    for (int i = 0; i < NUM_SPRITES; i++)
    {
        bitmap_destroy(sprites[i]);
    }

    SDL_DestroyTexture(screen);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    return 0;
}
