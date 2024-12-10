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

extern void CreateWindow(const int width, const int height, const char *title);
extern unsigned int CreateShaderProgram(const char *vertex_glsl,
                                        const char *fragment_glsl);
extern bool KeepCooking();
extern void EndCooking(); // batch renderer
void DoneCooking();       // close all

// Funtionalities can be added here
void DrawRectangle(const Vec2 position, const int width, const int height,
                   const Color color);
#endif
