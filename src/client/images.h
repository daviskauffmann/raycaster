#ifndef IMAGES_H
#define IMAGES_H

typedef struct image_s
{
    int w;
    int h;
    unsigned int *pixels;
} image_t;

int images_init();
image_t *images_load(const char *file);
void images_unload(image_t *image);
void images_quit(void);

#endif
