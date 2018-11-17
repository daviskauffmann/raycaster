#ifndef BITMAP_H
#define BITMAP_H

struct bitmap
{
    int width;
    int height;
    unsigned int *pixels;
};

struct bitmap *bitmap_create(const char *file);
void bitmap_destroy(struct bitmap *bitmap);

#endif
