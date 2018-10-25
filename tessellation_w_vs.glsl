#version 430
layout(location = 0) uniform mat4 P;
layout(location = 1) uniform mat4 V;
layout(location = 2) uniform mat4 M;
layout(location = 10) uniform float slider[6];
in vec3 pos_attrib;
out vec2 tex_coord_tc;

void main(void)
{
	 vec3 p = pos_attrib;
	
	 tex_coord_tc = vec2(pos_attrib.x/2+0.5,pos_attrib.y/2+0.5)/50.0*6.0;
	
	 gl_Position = vec4(p, 1.0);
	
}