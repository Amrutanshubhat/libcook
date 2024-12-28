
/*
 * vertex and elemnt objects data are uploaded to gpu's buffer and single draw-
 		call is made at the end for each draw mode(3 currently).
 * */
#include <stdio.h>
#include <stdlib.h>
#define GLAD_GL_IMPLEMENTATION
#include "glad/gl.h"
#undef GLAD_GL_IMPLEMENTATION
#include "GLFW/glfw3.h"
#include "libcook.h"

// Structs and Static Variables
typedef struct {
  float position[3];
  float color[4];
  float texture[2];
} Vertex;

static struct {
  unsigned int width;
  unsigned int height;
  GLFWwindow *handle;
} _window;

const int _color_bit = (1<<8)-1;

unsigned int *const window_width = &_window.width;
unsigned int *const window_height = &_window.height;

enum Gl_modes { TRIANGLE_MODE, LINE_MODE, POINT_MODE, MODE_SENTINEL };

static struct _gl_mode {
  unsigned int vbo;
  unsigned int vao;
  unsigned int ebo;
  unsigned int sp;
  size_t vert_capacity;
  size_t idx_capacity;
  size_t vert_cnt;
  size_t idx_cnt;
} _gl_mode[MODE_SENTINEL];

// local helper functions
static void append_vert_ind(const Vec3 *positions, const Color *colors,
                            const int size, const unsigned int *indices,
                            const int isize, enum Gl_modes mode) {

  struct _gl_mode *ptr = &_gl_mode[mode];

  glBindBuffer(GL_ARRAY_BUFFER, ptr->vbo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ptr->ebo);
  //check for vertex capacity in buffer object

  if (ptr->vert_capacity < ptr->vert_cnt+size) {
	  ptr->vert_capacity = ptr->vert_cnt+size;
	  Vertex temp[ptr->vert_cnt];
	  glGetBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(temp), temp);

	  glBufferData(GL_ARRAY_BUFFER, 
			  sizeof(Vertex)*ptr->vert_capacity, nullptr,
			  GL_DYNAMIC_DRAW);
	  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(temp), temp);
  }

  // check for elemnt buffer object
  if (ptr->idx_capacity < ptr->idx_cnt+isize) {
	  ptr->idx_capacity = ptr->idx_cnt+isize;
	  unsigned int temp[ptr->idx_cnt];
	  glGetBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(temp), temp);

	  glBufferData(GL_ELEMENT_ARRAY_BUFFER, 
			  sizeof(unsigned int)*ptr->idx_capacity,
			  nullptr, GL_DYNAMIC_DRAW);
	  glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(temp), temp);
  }

  Vertex v[size];
  for (size_t i = 0; i < size; i++) {
    v[i].position[0] = positions[i].x;
    v[i].position[1] = positions[i].y;
    v[i].position[2] = positions[i].z;
    v[i].color[0] = colors[i].r;
    v[i].color[1] = colors[i].g;
    v[i].color[2] = colors[i].b;
    v[i].color[3] = colors[i].a;
  }
  glBufferSubData(GL_ARRAY_BUFFER, ptr->vert_cnt*sizeof(Vertex), sizeof(v), v);
  // deal with index data
  unsigned int ind[isize];
  for (size_t i = 0; i < isize; i++) {
    ind[i] = ptr->vert_cnt + indices[i];
  }

  glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 
				  ptr->idx_cnt*sizeof(unsigned int), sizeof(ind), ind);
  ptr->vert_cnt += size;
  ptr->idx_cnt += isize;

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
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
    assert(status, "Shader Compilation error: %s\n", err);
  }

  free(code);
  return id;
}

void update_fps() {
  static double ps;
  static int frame_cnt;
  double cur_sec = glfwGetTime();
  double elapsed = cur_sec - ps;
  if (elapsed > 1) {
    ps = cur_sec;
    int fps = frame_cnt / elapsed;
    char title[10] = {0};
    sprintf(title, "%d", fps);
    glfwSetWindowTitle(_window.handle, title);
    frame_cnt = 0;
  }
  frame_cnt++;
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
  glfwSwapInterval(0); // 1 to limit fps to 60

  gladLoadGL(glfwGetProcAddress);

  // Currently common shader program for all gl modes
  const char *vertex_prog_file = "src/vertex.glsl";
  const char *fragment_prog_file = "src/fragment.glsl";
  unsigned int shader_program = CreateShaderProgram(vertex_prog_file, fragment_prog_file);

  //initialize uniform variables
  float pos_mat[] = {
	  2.0f/width, 0, 0, -1,
	  0, -2.0f/height, 0, 1,
	  0, 0, 1, 0,
	  0, 0, 0, 1
  };

  float clr_mat[] = {
	1.0f/_color_bit, 0, 0, 0,
	0, 1.0f/_color_bit, 0, 0,
	0, 0, 1.0f/_color_bit, 0,
	0, 0, 0, 1.0f/_color_bit
  };

  int proj_mat_pos = glGetUniformLocation(shader_program, "proj_pos_matrix");
  int proj_mat_clr = glGetUniformLocation(shader_program, "proj_clr_matrix");
  glUseProgram(shader_program);
  glUniformMatrix4fv(proj_mat_pos, 1, GL_TRUE, pos_mat);
  glUniformMatrix4fv(proj_mat_clr, 1, GL_TRUE, clr_mat);
  glUseProgram(0);

  // initialize memory for all gl_modes seperately
  for (size_t i=0; i<MODE_SENTINEL; i++) {

	  struct _gl_mode* ptr = &_gl_mode[i];
	  glGenVertexArrays(1, &ptr->vao);
	  glGenBuffers(1, &ptr->vbo);
	  glGenBuffers(1, &ptr->ebo);

	  glBindVertexArray(ptr->vao);
	  glBindBuffer(GL_ARRAY_BUFFER, ptr->vbo);
	  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ptr->ebo);

	  // Note: Beware of these hardcoded numbers
	  //
	  // position
	  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);
	  glEnableVertexAttribArray(0);

	  // color
	  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
							(void *)(3 * sizeof(float)));
	  glEnableVertexAttribArray(1);

	  // texture
	  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
							(void *)(7 * sizeof(float)));
	  glEnableVertexAttribArray(2);

	  ptr->sp = shader_program;
	  glBindBuffer(GL_ARRAY_BUFFER, 0);
	  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	  glBindVertexArray(0);
  }

  _window.width = width;
  _window.height = height;
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
  if (status == GL_FALSE) {
    int max_len = 256;
    char log[max_len];
    memset(log, 0, sizeof(log));
    glGetProgramInfoLog(sp, max_len, nullptr, log);
    assert(status, "Shader program linking failed: %u\n%s\n", sp, log);
  }
  // cleanups
  glDeleteShader(vs);
  glDeleteShader(fs);
  return sp;
}

void DrawRectangle(const Vec2 pos, const Vec2 dim, const Color col) {
  // NOTE (Beware): few constants hence used directly
  Vec3 v[4] = {{.x = pos.x, .y = pos.y},
               {.x = pos.x, .y = pos.y + dim.y},
               {.x = pos.x + dim.x, .y = pos.y + dim.y},
               {.x = pos.x + dim.x, .y = pos.y}};
  Color c[] = {col, col, col, col};
  unsigned int idx[] = {0, 1, 2, 2, 3, 0};
  append_vert_ind(v, c, 4, idx, 6, TRIANGLE_MODE);
}

void DrawLine(const Vec2 start, const Vec2 end, const Color col) {
  Vec3 v[2] = {{.x = start.x, .y = start.y}, {.x = end.x, .y = end.y}};
  Color c[] = {col, col};
  unsigned int idx[] = {0, 1};
  append_vert_ind(v, c, 2, idx, 2, LINE_MODE);
}

void DrawVertices(const Vec3 *positions, const Color *colors, const int size,
                  const unsigned int *indices, const int isize) {
	append_vert_ind(positions, colors, size, indices, isize, TRIANGLE_MODE);
}

inline void EndCooking() {
  // clear screen
  // glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // copy all gl modes data
  struct _gl_mode *ptr = nullptr;

  for (size_t i = 0; i < MODE_SENTINEL; i++) {
    ptr = &_gl_mode[i];
    if (!ptr->vert_cnt)
      continue;

    int draw_mode;
    switch (i) {
    case 0:
      draw_mode = GL_TRIANGLES;
      break;
    case 1:
      draw_mode = GL_LINES;
      break;
    case 2:
      draw_mode = GL_POINTS;
      break;
    default:
      assert(false, "Unknown draw call\n");
    }

	glUseProgram(ptr->sp);
	glBindVertexArray(ptr->vao);
	glBindBuffer(GL_ARRAY_BUFFER, ptr->vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ptr->ebo);

    glDrawElements(draw_mode, ptr->idx_cnt, GL_UNSIGNED_INT, nullptr);

	ptr->vert_cnt = ptr->idx_cnt=0;
  }
  glfwSwapBuffers(_window.handle);
  glfwPollEvents();
  update_fps();
}

inline bool StartCooking() { return !glfwWindowShouldClose(_window.handle); }

void CloseWindow() {
  glfwDestroyWindow(_window.handle);
  for (size_t i = 0; i < MODE_SENTINEL; i++) {
	  glDeleteVertexArrays(1, &_gl_mode[i].vao);
	  glDeleteBuffers(1, &_gl_mode[i].vbo);
	  glDeleteProgram(_gl_mode[i].sp);
  }
  glfwTerminate();
}
