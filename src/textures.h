#ifndef IMAGE_H
#define IMAGE_H

typedef struct texture_s
{
    int w;
    int h;
    unsigned int *pixels;
} texture_t;

void textures_init();
texture_t *textures_load(const char *file);
void textures_unload(texture_t *texture);
void textures_quit(void);

#endif
