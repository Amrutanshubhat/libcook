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
  uint8_t color[4];
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

typedef enum { 
	TRIANGLE_MODE, LINE_MODE, POINT_MODE, MODE_SENTINEL 
} Gl_modes;

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
static void append_vert_ind(const Vertex* vertices, const int size, 
							const unsigned int *indices, const int isize, 
							Gl_modes mode) {

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

  glBufferSubData(GL_ARRAY_BUFFER, 
				  ptr->vert_cnt*sizeof(*vertices), 
				  sizeof(*vertices)*size, vertices);
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

typedef struct {
	GLint size;
	GLenum type;
	GLboolean norm;
	GLsizei stride;
	int off_sz;
} Va_ptr_attrib;
	
static void
intitialize_gl_mode(struct _gl_mode* ptr, Va_ptr_attrib* va_ptrs, int size, 
						const char* vs, const char* fs) {

	  //vertex array and attributes
	  glGenVertexArrays(1, &ptr->vao);
	  glGenBuffers(1, &ptr->vbo);
	  glGenBuffers(1, &ptr->ebo);

	  glBindVertexArray(ptr->vao);
	  glBindBuffer(GL_ARRAY_BUFFER, ptr->vbo);
	  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ptr->ebo);

	  for (size_t i=0, offset=0; i<size; i++) {
		  glVertexAttribPointer(i, va_ptrs[i].size, 
				  				va_ptrs[i].type, va_ptrs[i].norm, 
								va_ptrs[i].stride, (void *)offset);
		  glEnableVertexAttribArray(i);
		  offset += va_ptrs[i].size*va_ptrs[i].off_sz;
	  }
	  glBindBuffer(GL_ARRAY_BUFFER, 0);
	  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	  glBindVertexArray(0);

	  // shader program compilation
	  ptr->sp = CreateShaderProgram(vs, fs);
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
  glfwSwapInterval(1); // 1 to limit fps to 60

  gladLoadGL(glfwGetProcAddress);

  _window.width = width;
  _window.height = height;

  //initialize uniform variables
  float pos_mat[] = {
	  2.0f/_window.width, 0, 0, -1,
	  0, -2.0f/_window.height, 0, 1,
	  0, 0, 1, 0,
	  0, 0, 0, 1
  };

  float clr_mat[] = {
	1.0f/_color_bit, 0, 0, 0,
	0, 1.0f/_color_bit, 0, 0,
	0, 0, 1.0f/_color_bit, 0,
	0, 0, 0, 1.0f/_color_bit
  };

  // initialize memory for all gl_modes seperately
  for (size_t i=0; i<MODE_SENTINEL; i++) {
	  struct _gl_mode* ptr = &_gl_mode[i];
	  switch (i) {
		  case TRIANGLE_MODE:
		  case LINE_MODE:
		  case POINT_MODE: {
			  Va_ptr_attrib attribs[] = {
				  {3, GL_FLOAT, GL_FALSE, sizeof(Vertex), sizeof(float)},
				  {4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(Vertex), sizeof(uint8_t)},
				  {2, GL_FLOAT, GL_FALSE, sizeof(Vertex), sizeof(float)}
			  };

			  intitialize_gl_mode(ptr, attribs, sizeof(attribs)/sizeof(*attribs), 
					  "src/vertex.glsl", "src/fragment.glsl");

			  int pm_pos = glGetUniformLocation(ptr->sp, "proj_pos_matrix");
			  int pm_clr = glGetUniformLocation(ptr->sp, "proj_clr_matrix");
			  glUseProgram(ptr->sp);
			  glUniformMatrix4fv(pm_pos, 1, GL_TRUE, pos_mat);
			  glUniformMatrix4fv(pm_clr, 1, GL_TRUE, clr_mat);
			  glUseProgram(0);
			  break;
			}
	  }
  }
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
  Vertex v[] = {
	{.position={pos.x, pos.y}, .color={col.r, col.g, col.b, col.a}},	
	{.position={pos.x, pos.y+dim.y}, .color={col.r, col.g, col.b, col.a}},	
	{.position={pos.x+dim.x, pos.y+dim.y}, .color={col.r, col.g, col.b, col.a}},	
	{.position={pos.x+dim.x, pos.y}, .color={col.r, col.g, col.b, col.a}}
  };
  unsigned int idx[] = {0, 1, 2, 2, 3, 0};
  append_vert_ind(v, sizeof(v)/sizeof(*v), idx, sizeof(idx)/sizeof(*idx), TRIANGLE_MODE);
}

void DrawLine(const Vec2 start, const Vec2 end, const Color col) {
	Vertex v[] = {
		{.position={start.x, start.y}, .color={col.r, col.g, col.b, col.a}},
		{.position={end.x, end.y}, .color={col.r, col.g, col.b, col.a}}
	};
  unsigned int idx[] = {0, 1};
  append_vert_ind(v, sizeof(v)/sizeof(*v), idx, sizeof(idx)/sizeof(*idx), LINE_MODE);
}

void DrawPoint(const Vec2 pos, const Color col) {
	Vertex v[] = {
		{.position={pos.x, pos.y}, .color={col.r, col.g, col.b, col.a}}
	};
	unsigned int idx[] = {0};
  append_vert_ind(v, sizeof(v)/sizeof(*v), idx, sizeof(idx)/sizeof(*idx), POINT_MODE);
}

void DrawVertices(const Vec3 *positions, const Color *colors, const int size,
                  const unsigned int *indices, const int isize) {
  Vertex v[size];
  for (size_t i=0; i<size; i++) {
    v[i].position[0] = positions[i].x;
    v[i].position[1] = positions[i].y;
    v[i].position[2] = positions[i].z;
    v[i].color[0] = colors[i].r;
    v[i].color[1] = colors[i].g;
    v[i].color[2] = colors[i].b;
    v[i].color[3] = colors[i].a;
  }
  append_vert_ind(v, size, indices, isize, TRIANGLE_MODE);
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
