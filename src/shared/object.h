#ifndef OBJECT_H
#define OBJECT_H

#define NUM_OBJECTS 19

typedef struct
{
    double x;
    double y;
    unsigned char sprite_index;
} Object;

extern Object objects[NUM_OBJECTS];

#endif
