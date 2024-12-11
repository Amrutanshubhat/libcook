/*
 * vertex and index arrays are static with constant storage allocated which are copied to gpu's buffer only once per frame to render all vertices. 
 * */
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

enum Gl_modes{
	POLYGON_MODE, LINE_MODE, POINT_MODE
};

/*
 * eventhough memory is allocated in heap, the buffer size is limited to the defines below.
 * Exceeding the allocation during the runtime causes the program to abort.
 * */
#define MAX_VERTICES 3000
#define MAX_INDICES 4500
static struct _gl_mode{
	size_t vert_capacity;
	size_t idx_capacity;
	size_t vert_cnt;
	size_t idx_cnt;
	Vertex* vertices;
	unsigned int* indices;
} _gl_mode[POINT_MODE+1];

static struct {
  unsigned int vbo;
  unsigned int vao;
  unsigned int ebo;
  unsigned int shader_program;
} _gl;

// local helper functions
inline static void vertex_append(const Vec3 position, const Color color, enum Gl_modes mode) {
	struct _gl_mode* handle = &_gl_mode[mode];
	if (handle->vert_capacity == handle->vert_cnt) {
		handle->vert_capacity = handle->vert_capacity == 0 ? 120 : 2*handle->vert_capacity;
		handle->vertices = realloc(handle->vertices, handle->vert_capacity*sizeof(*handle->vertices));
		assert(handle->vertices, "Vertex realloc failed\n");
	}
  handle->vertices[handle->vert_cnt].position[0] = position.x;
  handle->vertices[handle->vert_cnt].position[1] = position.y;
  handle->vertices[handle->vert_cnt].position[2] = position.z;
  handle->vertices[handle->vert_cnt].color[0] = (float)color.r / 255;
  handle->vertices[handle->vert_cnt].color[1] = (float)color.g / 255;
  handle->vertices[handle->vert_cnt].color[2] = (float)color.b / 255;
  handle->vertices[handle->vert_cnt].color[3] = (float)color.a / 255;
  handle->vert_cnt++;
}

inline static void index_append(const unsigned int *idx, const int c, enum Gl_modes mode) {
	struct _gl_mode* handle = &_gl_mode[mode];
	if (handle->idx_capacity == handle->idx_cnt) {
		handle->idx_capacity = handle->idx_capacity == 0 ? 120 : 2*handle->idx_capacity;
		handle->indices = realloc(handle->indices, handle->idx_capacity*sizeof(*handle->indices));
		assert(handle->indices, "Index realloc failed\n");
	}
  for (size_t i = 0; i < c; i++)
    handle->indices[handle->idx_cnt++] = handle->vert_cnt+idx[i];
}

static char *extract_shader_source_code(const char *filename) {
  FILE *f = fopen(filename, "r");
  assert(f, "Failed to open the file: %s\n", filename);
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

// Function definitions
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

  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*MAX_INDICES,
               nullptr, GL_DYNAMIC_DRAW);
  glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex)*MAX_VERTICES, nullptr,
               GL_DYNAMIC_DRAW);

  // Note: Beware of these hardcoded numbers
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)(3*sizeof(float)));
  glEnableVertexAttribArray(1);

  const char *vertex_prog_file = "src/vertex.glsl";
  const char *fragment_prog_file = "src/fragment.glsl";
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

  unsigned int idx[] = { 0, 1, 2, 2, 3, 0 };
  enum Gl_modes mode = POLYGON_MODE;
  // append index first to avoid vert_cnt mismatch
  index_append(idx, sizeof(idx) / sizeof(*idx), mode);
  for (size_t i = 0; i < 4; i++)
    vertex_append(v[i], col, mode);
}

void DrawLine(const Vec2 start, const Vec2 end, const Color color, const int thickness) {
	//TODO
}

inline void EndCooking() {
  // clear screen
  glClearColor(0.06f, 0.06f, 0.06f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  glUseProgram(_gl.shader_program);
  glBindVertexArray(_gl.vao);
  glBindBuffer(GL_ARRAY_BUFFER, _gl.vbo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _gl.ebo);
  
  // copy all gl modes data
  struct _gl_mode* ptr = nullptr;
  size_t tot_vcap=0, tot_icap=0;

  for (size_t i=0, offset=0; i<=POINT_MODE; i++) {
	  int draw_mode;
	  switch(i) {
		  case 0: draw_mode=GL_TRIANGLES; break;
		  case 1: draw_mode=GL_LINE; break; 
		  case 2: draw_mode=GL_POINTS; break; 
		  default: assert(false, "Unknown draw call\n");
	  }

	  ptr = &_gl_mode[i];
	  if (!ptr->vert_cnt) continue;

	  glBufferSubData(GL_ARRAY_BUFFER, 
					  tot_vcap*sizeof(*ptr->vertices), 
					  ptr->vert_cnt*sizeof(*ptr->vertices), 
					  ptr->vertices);

	  offset = tot_icap*sizeof(*ptr->indices);

	  glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 
					  offset, 
					  ptr->idx_cnt*sizeof(*ptr->indices), 
					  ptr->indices);

	  glDrawElements(draw_mode, 
					  ptr->idx_cnt, 
					  GL_UNSIGNED_INT, 
					  (const void *)offset);

	  tot_vcap += ptr->vert_cnt;
	  tot_icap += ptr->idx_cnt;
	  ptr->idx_cnt = 0;
	  ptr->vert_cnt = 0;
  }
	assert(tot_vcap <= MAX_VERTICES, "Vertex Limit Exceede\n");
	assert(tot_icap <= MAX_INDICES, "Index Limit Exceede\n");
  glfwSwapBuffers(_window.handle);
  glfwPollEvents();
}

inline bool StartCooking() { return !glfwWindowShouldClose(_window.handle); }

void CloseWindow() {
  glfwDestroyWindow(_window.handle);
  for (size_t i=0; i<=POINT_MODE; i++) {
	  if (_gl_mode[i].vert_capacity) free(_gl_mode[i].vertices);
	  if (_gl_mode[i].idx_capacity) free(_gl_mode[i].indices);
  }
  glDeleteVertexArrays(1, &_gl.vao);
  glDeleteBuffers(1, &_gl.vbo);
  glDeleteProgram(_gl.shader_program);
  glfwTerminate();
}