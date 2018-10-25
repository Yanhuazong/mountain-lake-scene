#version 430

uniform sampler2D height;

in vec3 pos_attrib;
out vec2 tex_coord_tc;

void main(void)
{
	 vec3 p = pos_attrib+vec3(0.0,0.0,-6.2);
	 
	 tex_coord_tc = pos_attrib.xy/50.0;
	 gl_Position = vec4(p, 1.0);
}