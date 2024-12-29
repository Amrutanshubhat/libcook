/*
- Only responsible for setting color for fragment (pixel eventually)
- 4th component of vec4 is opacity
*/

#version 400 core

in vec4 cust_color;
out vec4 colorout;

void main() {
	colorout = cust_color;
}
