#version 430

layout(location = 0) uniform mat4 P;
layout(location = 1) uniform mat4 V;
layout(location = 2) uniform mat4 M;

in vec2 tex_coord_attrib;
in vec3 pos_attrib;
//out vec2 tex_coord_tc;
out vec2 tex_coord;

void main(void)
{
	 vec3 p = pos_attrib+vec3(0.0,0.0,-6.2);
	 
	// tex_coord_tc = pos_attrib.xy/50.0;
	 gl_Position = P*V*M*vec4(p, 1.0);
	 tex_coord = pos_attrib.xy;
}