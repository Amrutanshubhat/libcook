#include "glad/gl.h"
#include "GLFW/glfw3.h"
#include "libcook.h"
#include <math.h>

int main(void) {
  const int width = 800, height = 600;
  const int rect_width = width / 8, rect_height = height / 7;
  CreateWindow(width, height, "WindoW");
  while (StartCooking()) {
    Vec2 pos = {.x = ((sinf(glfwGetTime() * 2) + 1) / 2) * (width - rect_width),
                .y = (float)(height - rect_height / 2.0f) / 2};
    Color col = {.r = 10, .g = 90, .b = 190, .a = 255};
    DrawRectangle(pos, rect_width, rect_height, col);
    EndCooking();
  }
  CloseWindow();
}
