#include "libcook.h"
#include <math.h>
#include <stdio.h>

void key_press(Keymap k, Keymap m, bool p) {
	if (k == 0x1B)
		CloseWindow();
}

int main(void) {
	const int width = 800, height = 600;
	Vec2 rect_size = {
		.x = width/8.0,
		.y = height/7.0
	};
	CreateWindow(width, height, "WindoW");
	SetKeypressCallback(key_press);
	while (StartCooking())
	{
		Vec2 pos = {
			.x = ((sinf(TimeElapsed()*2)+1)/2)*(width-rect_size.x),
			.y = (float)(height-rect_size.y/2.0f)/2
		};
		Color col = {
			.r = 0xAA,
			.g = 0x0A,
			.b = 0xDD,
			.a = 0xFF
		};
		DrawRectangle(pos, rect_size, col, true); 	
		EndCooking();
	}
	CloseWindow();
}
