#ifndef _METH
#define _METH

#include <stddef.h>

typedef float mat3x3[3*3];
typedef float mat4x4[4*4];

extern float* mat_multiply(float* x, size_t rx, size_t cx, float* y, size_t ry, size_t cy);
extern void mat_transform(float* mat, size_t r, size_t c);
extern void mat_print(float* mat, size_t r, size_t c);
#endif
