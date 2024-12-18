#ifndef _COOKER
#define _COOKER

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#undef GLFW_INCLUDE_NONE

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
#endif
