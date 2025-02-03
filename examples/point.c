#include "libcook.h"

int main(void) {
  const int width = 800, height = 600;
  Vec2 pos1 = {.x = width / 4.0, .y = height / 3.0};
  Vec2 pos2 = {.x = (float)width / 2, .y = (float)height / 2};
  Color col = {.r = 255, .g = 255, .b = 255, .a = 255};
  CreateWindow(width, height, "WindoW");
  while (StartCooking()) {
    DrawPoint(pos2, col);
    EndCooking();
  }
  CloseWindow();
}
