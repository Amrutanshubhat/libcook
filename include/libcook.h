#ifndef _COOKER
#define _COOKER

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

// for non-modifiers take only lower 8 bits
typedef enum:uint32_t {
	KEY_A		= 'a',
	KEY_S		= 's',
	KEY_D		= 'd',
	KEY_W		= 'w',
	KEY_SPACE	= ' ',
	KEY_ENTER	= '\n',
	KEY_ESC		= 0x1b,

    KEY_SHIFT   = 1 << 17,
    KEY_CONTROL = 1 << 18,
    KEY_OPTION  = 1 << 19,
    KEY_COMMAND = 1 << 20,
} Keymap;

typedef void (*KeyPressCallback)(Keymap key, Keymap meta, bool is_pressed);
extern void SetKeypressCallback(KeyPressCallback);

extern uint32_t *const window_width, *const window_height;

extern void CreateWindow(const int width, const int height, const char *title);
extern uint32_t CreateShaderProgram(const char *vertex_glsl,
                                        const char *fragment_glsl);
extern bool StartCooking();
extern void EndCooking(); // batch renderer
extern void CloseWindow();
extern float TimeElapsed(); // time elapsed since window created

// Funtionalities can be added here
void DrawRectangle(const Vec2 position, const Vec2 dimension, const Color color, const bool filled);
void DrawLine(const Vec2 start, const Vec2 end, const Color color);
void DrawPoint(const Vec2 position, const Color color);
/*
 * index: how position coordinates are to be connected (using triangles ofcourse)
 * eg: 1 2 3 2 3 4 
 * 	=> triangle 1: vertex[0] vertex[1] vertex[2],
 *		triangle 2: vertex[2] vertex[3] vertex[4]
 * */
void DrawVertices(const Vec3* positions, const Color* colors, const int size, const uint32_t* indices, const int isize);
#endif
