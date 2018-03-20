#ifndef MAP_H
#define MAP_H

#define MAP_WIDTH 24
#define MAP_HEIGHT 24
#define NUM_OBJECTS 19

struct object
{
    double x;
    double y;
    unsigned char sprite_index;
};

extern unsigned char wall_map[MAP_WIDTH][MAP_HEIGHT];
extern unsigned char floor_map[MAP_WIDTH][MAP_HEIGHT];
extern unsigned char ceiling_map[MAP_WIDTH][MAP_HEIGHT];
extern struct object objects[NUM_OBJECTS];

#endif
