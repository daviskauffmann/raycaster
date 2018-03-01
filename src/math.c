#include "math.h"

vec2_t vec2_add(vec2_t a, vec2_t b)
{
    vec2_t r;
    r.x = a.x + b.x;
    r.y = a.y + b.y;
    return r;
}
