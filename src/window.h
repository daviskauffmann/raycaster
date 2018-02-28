#ifndef WINDOW_H
#define WINDOW_H

extern int w;
extern int h;

void window_init(const char *title, int width, int height);
void window_pset(int x, int y, unsigned int color);
unsigned int window_pget(int x, int y);
void window_update(void);
void window_close(void);

#endif
