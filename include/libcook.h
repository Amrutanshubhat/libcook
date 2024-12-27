#ifndef _COOKER
#define _COOKER

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#undef GLFW_INCLUDE_NONE
#include <stdlib.h>

#define assert(value, ...)                                                     \
  if (!(value)) {                                                              \
    fprintf(stderr, "[ERROR]: " __VA_ARGS__);                                  \
    exit(EXIT_FAILURE);                                                        \
  }

typedef struct {
  float x;
  float y;
} Vec2;

typedef struct {
  float x;
  float y;
  float z;
} Vec3;

// 0 to 255
typedef struct {
  short r;
  short g;
  short b;
  short a;
} Color;
extern unsigned int *const window_width, *const window_height;
extern void CreateWindow(const int width, const int height, const char *title);
extern unsigned int CreateShaderProgram(const char *vertex_glsl,
                                        const char *fragment_glsl);
extern bool StartCooking();
extern void EndCooking(); // batch renderer
void CloseWindow();       // close all

// Funtionalities can be added here
void DrawRectangle(const Vec2 position, const Vec2 dimension, const Color color);
void DrawLine(const Vec2 start, const Vec2 end, const Color color);
/*
 * index: how position coordinates are to be connected (using triangles ofcourse)
 * eg: 1 2 3 2 3 4 
 * 	=> triangle 1: vertex[0] vertex[1] vertex[2],
 *		triangle 2: vertex[2] vertex[3] vertex[4]
 * */
void DrawVertices(const Vec3* positions, const Color* colors, const int size, const unsigned int* indices, const int isize);
#endif
