/*
	- vec3 : components for position / direction / color
	- vec4 : vec3 + perspective (for camera)
*/
#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec4 vcolor;

// uniform mat4 pos_matrix;
// uniform mat3 color_matrix;

out vec4 cust_color;

void main() {
   //gl_Position = pos_matrix*vec4(pos, 1.0);
   gl_Position = vec4(pos, 1.0);
   //cust_color = color_matrix*vcolor;
   cust_color = vcolor;
}
