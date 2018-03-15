#ifndef MAP_H
#define MAP_H

#define MAP_WIDTH 24
#define MAP_HEIGHT 24
#define NUM_OBJECTS 19

typedef struct
{
    double x;
    double y;
    unsigned char sprite_index;
} Object;

extern unsigned char wall_map[MAP_WIDTH][MAP_HEIGHT];
extern unsigned char floor_map[MAP_WIDTH][MAP_HEIGHT];
extern unsigned char ceiling_map[MAP_WIDTH][MAP_HEIGHT];
extern Object objects[NUM_OBJECTS];

#endif
