#include "libcook.h"

int main(void) {
	const int width = 800, height = 600;
	Vec3 positions[] = {{.x=0.0f, .y=0.0f}, {.x=0, .y=height}, {.x=width, .y=0.0f}, {.x=width, .y=height}}; //vertex positions
	Color colors[]	= {{.r=255, .a=255}, {.g=255, .a=255}, {.b=255, .a=255}, {.r=100, .g=100, .b=100, .a=255}}; //each vertex color
	unsigned int indices[] = {0, 1, 2, 2, 3, 1};
	/*
	 	0 _______________ 2
		  |		|
		  |             |
		  |             |
		  |	        |	
         	1 |_____________| 3
	*/
	CreateWindow(width, height, "WindoW");
	while (StartCooking())
	{
		DrawVertices(positions, colors, 4, indices, 6); //size: positions and colors size, isize: indices size
		EndCooking();
	}
	CloseWindow();
}
