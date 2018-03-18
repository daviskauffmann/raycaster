/* 
 * A small extension to SDL_image. It adds a simple struct for loaded images
 * which allows for fast reading/writing of pixel data.
 */

#ifndef SDL_IMAGE_EXT_H
#define SDL_IMAGE_EXT_H

typedef struct
{
    int w;
    int h;
    unsigned int *pixels;
} IMG_Image;

IMG_Image *IMG_LoadAndConvert(const char *file);
void IMG_FreeImage(IMG_Image *image);

#endif
