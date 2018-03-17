#ifndef IMAGE_H
#define IMAGE_H

typedef struct
{
    int w;
    int h;
    unsigned int *pixels;
} IMG_Image;

IMG_Image *IMG_LoadAndConvert(const char *file);
void IMG_FreeImage(IMG_Image *image);

#endif
