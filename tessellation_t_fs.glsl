#version 430

layout(location = 11) uniform sampler2D treetex;


out vec4 fragcolor; 
in vec3 N;         

in vec2 tex_coord; 


void main(void)
{ 

	fragcolor  = texture(treetex, tex_coord);
	

}

