/*
	proj_***_matrix: matrix transformation from local view to gl view
*/
#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec4 vcolor;

uniform mat4 proj_pos_matrix; // translates to -1 to 1
uniform mat4 proj_clr_matrix; // translates to 0 to 1

out vec4 cust_color;

void main() {
   gl_Position = proj_pos_matrix*vec4(pos, 1.0);
   cust_color = proj_clr_matrix*vcolor;
}
