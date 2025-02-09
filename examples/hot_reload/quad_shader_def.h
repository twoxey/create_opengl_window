#ifndef QUAD_TYPES

#define QUAD_rect    0
#define QUAD_ellipse 1
#define QUAD_image   2
#define QUAD_char    3

#define QUAD_TYPES
#endif // QUAD_TYPES

#ifndef Uniform
#define Uniform(type, name) uniform type name
#endif

Uniform(vec2, screen_size);
Uniform(vec2, size);
Uniform(vec2, offset);
Uniform(vec4, color);
Uniform(int, type);

