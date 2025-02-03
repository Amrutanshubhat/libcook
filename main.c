#include "libcook.h"
#include <math.h>
#include <stdio.h>

int main(void) {
	const int width = 800, height = 600;
	Vec2 rect_size = {
		.x = width/8.0,
		.y = height/7.0
	};
	CreateWindow(width, height, "WindoW");
	while (StartCooking())
	{
		Vec2 pos = {
			.x = ((sinf(TimeElapsed()*2)+1)/2)*(width-rect_size.x),
			.y = (float)(height-rect_size.y/2.0f)/2
		};
		Color col = {
			.r = 10,
			.g = 90,
			.b = 190,
			.a = 255
		};
		DrawRectangle(pos, rect_size, col, true); 	
		EndCooking();
	}
	CloseWindow();
}
