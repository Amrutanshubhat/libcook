#include <stdlib.h>
#include <stdio.h>
#define GLAD_GL_IMPLEMENTATION
#include "glad/gl.h"
#undef GLAD_GL_IMPLEMENTATION
#include "GLFW/glfw3.h"
#include "libcook.h"

// Macro Magic
#define assert(value, ...)                                                     \
  if (!(value)) {                                                              \
    fprintf(stderr, "[ERROR]: " __VA_ARGS__);                                  \
    exit(EXIT_FAILURE);                                                        \
  }

// Structs and Static Variables
typedef struct {
  float position[3];
  float color[4];
} Vertex;

static struct {
  unsigned int width;
  unsigned int height;
  char title[32];
  GLFWwindow *handle;
} _window;

#define MAX_VERTICES 1000
#define MAX_INDICES 1500
static struct {
  int vert_cnt;
  int idx_cnt;
  unsigned int vbo;
  unsigned int vao;
  unsigned int ebo;
  unsigned int shader_program;
  Vertex vertices[MAX_VERTICES];
  unsigned int indices[MAX_INDICES];
} _gl;

// local helper functions
inline static void vertex_append(Vec3 position, Color color) {
  assert(_gl.vert_cnt < MAX_VERTICES, "Vertex overflow\n");
  _gl.vertices[_gl.vert_cnt].position[0] = position.x;
  _gl.vertices[_gl.vert_cnt].position[1] = position.y;
  _gl.vertices[_gl.vert_cnt].position[2] = position.z;
  _gl.vertices[_gl.vert_cnt].color[0] = (float)color.r / 255;
  _gl.vertices[_gl.vert_cnt].color[1] = (float)color.g / 255;
  _gl.vertices[_gl.vert_cnt].color[2] = (float)color.b / 255;
  _gl.vertices[_gl.vert_cnt].color[3] = (float)color.a / 255;
  _gl.vert_cnt++;
}

inline static void index_append(const unsigned int *idx, const int c) {
  assert(_gl.idx_cnt + c < MAX_INDICES, "Index Overflow\n");
  for (size_t i = 0; i < c; i++)
    _gl.indices[_gl.idx_cnt++] = idx[i];
}

static char *extract_shader_source_code(const char *filename) {
  FILE *f = fopen(filename, "r");
  char *src_code = nullptr;
  size_t len;
  getdelim(&src_code, &len, EOF, f);
  fclose(f);
  return src_code;
}

static unsigned int compile_shader(const char *filename, unsigned int type) {
  char *code = extract_shader_source_code(filename);
  unsigned int id = glCreateShader(type);
  const char *const _code_ptr = code;
  glShaderSource(id, 1, &_code_ptr, nullptr);
  glCompileShader(id);

  // error check
  int status;
  glGetShaderiv(id, GL_COMPILE_STATUS, &status);
  if (status == GL_FALSE) {
    int len;
    glGetShaderiv(id, GL_INFO_LOG_LENGTH, &len);
    char err[len];
    glGetShaderInfoLog(id, len, &len, err);
    assert(true, "Shader Compilation error: %s\n", err);
  }

  free(code);
  return id;
}

// Callbacks
static void error_callback(int error, const char *description) {
  fprintf(stderr, "GLFW Error: %s\n", description);
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action,
                         int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(_window.handle, GLFW_TRUE);
}

static void framebuffer_size_callback(GLFWwindow *window, int width,
                                      int height) {
  _window.width = width;
  _window.height = height;
  glViewport(0, 0, width, height);
}

// Engine Function definitions
/*
 * Loads glfw and sets up opengl context and config
 * */
void CreateWindow(const int width, const int height, const char *title) {
  assert(glfwInit(), "Failed to initialize the window\n");
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwSetErrorCallback(error_callback);

  _window.handle = glfwCreateWindow(width, height, title, nullptr, nullptr);

  assert(_window.handle, "window creation failed\n");
  glfwMakeContextCurrent(_window.handle);
  glfwSetKeyCallback(_window.handle, key_callback);
  glfwSetFramebufferSizeCallback(_window.handle, framebuffer_size_callback);
  glfwSwapInterval(1); // limit fps to 60

  gladLoadGL(glfwGetProcAddress);
  glGenVertexArrays(1, &_gl.vao);
  glGenBuffers(1, &_gl.vbo);
  glGenBuffers(1, &_gl.ebo);

  glBindVertexArray(_gl.vao);
  glBindBuffer(GL_ARRAY_BUFFER, _gl.vbo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _gl.ebo);

  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(*_gl.indices) * MAX_INDICES,
               nullptr, GL_DYNAMIC_DRAW);
  glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * MAX_VERTICES, nullptr,
               GL_DYNAMIC_DRAW);

  glVertexAttribPointer(
      0, sizeof(_gl.vertices[0].position) / sizeof(*(_gl.vertices[0].position)),
      GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(
      1, sizeof(_gl.vertices[0].color) / sizeof(*(_gl.vertices[0].color)),
      GL_FLOAT, GL_FALSE, sizeof(Vertex),
      (void *)sizeof(_gl.vertices[0].position));
  glEnableVertexAttribArray(1);

  const char *vertex_prog_file = "vertex.glsl";
  const char *fragment_prog_file = "fragment.glsl";
  _gl.shader_program =
      CreateShaderProgram(vertex_prog_file, fragment_prog_file);

  _window.width = width;
  _window.height = height;
  strncpy(_window.title, title, sizeof(_window.title) - 1);
}

unsigned int CreateShaderProgram(const char *const vertex_glsl,
                                 const char *const fragment_glsl) {
  // compiling
  unsigned int vs = compile_shader(vertex_glsl, GL_VERTEX_SHADER);
  unsigned int fs = compile_shader(fragment_glsl, GL_FRAGMENT_SHADER);

  // linking
  unsigned int sp = glCreateProgram();
  glAttachShader(sp, vs);
  glAttachShader(sp, fs);
  glLinkProgram(sp);
  glValidateProgram(sp);
  int status;
  glGetProgramiv(sp, GL_VALIDATE_STATUS, &status);
  assert(status, "Shader program linking failed\n")
      // cleanups
      glDeleteShader(vs);
  glDeleteShader(fs);

  return sp;
}

void DrawRectangle(const Vec2 pos, const int width, const int height,
                   const Color col) {
  Vec3 v[4] = {{.x = (2.0 * pos.x / _window.width) - 1,
                .y = (-2.0 * pos.y / _window.height) + 1,
                .z = 0.0f},
               {.x = (2.0 * pos.x / _window.width) - 1,
                .y = (-2.0 * (pos.y + height) / _window.height) + 1,
                .z = 0.0f},
               {.x = (2.0 * (pos.x + width) / _window.width) - 1,
                .y = (-2.0 * (pos.y + height) / _window.height) + 1,
                .z = 0.0f},
               {.x = (2.0 * (pos.x + width) / _window.width) - 1,
                .y = (-2.0 * pos.y / _window.height) + 1,
                .z = 0.0f}};
  unsigned int idx[] = {_gl.vert_cnt,     _gl.vert_cnt + 1, _gl.vert_cnt + 2,
                        _gl.vert_cnt + 2, _gl.vert_cnt + 3, _gl.vert_cnt};
  for (size_t i = 0; i < 4; i++) {
    vertex_append(v[i], col);
  }
  index_append(idx, sizeof(idx) / sizeof(*idx));
}

inline void EndCooking() {
  // clear screen
  glClearColor(0.06f, 0.06f, 0.06f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  glUseProgram(_gl.shader_program);
  glBindVertexArray(_gl.vao);
  glBindBuffer(GL_ARRAY_BUFFER, _gl.vbo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _gl.ebo);
  glBufferSubData(GL_ARRAY_BUFFER, 0, _gl.vert_cnt * sizeof(*_gl.vertices),
                  _gl.vertices);
  glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0,
                  _gl.idx_cnt * sizeof(*_gl.indices), _gl.indices);
  glDrawElements(GL_TRIANGLES, _gl.idx_cnt, GL_UNSIGNED_INT, nullptr);

  _gl.vert_cnt = 0;
  _gl.idx_cnt = 0;
  glfwSwapBuffers(_window.handle);
  glfwPollEvents();
}

inline bool KeepCooking() { return !glfwWindowShouldClose(_window.handle); }

void DoneCooking() {
  glfwDestroyWindow(_window.handle);
  glDeleteVertexArrays(1, &_gl.vao);
  glDeleteBuffers(1, &_gl.vbo);
  glDeleteProgram(_gl.shader_program);
  glfwTerminate();
}
