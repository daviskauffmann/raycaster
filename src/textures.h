#ifndef IMAGE_H
#define IMAGE_H

extern int tw;
extern int th;

void textures_init(int texture_width, int texture_height);
unsigned int *textures_load(const char *file);
void textures_quit(void);

#endif
