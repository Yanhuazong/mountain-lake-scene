#version 430            
uniform mat4 PVM;
uniform mat4 M;
uniform mat4 V;
out float depth;
in vec3 pos_attrib; //in object space
out vec3 pos;
out vec2 tex_coord;

layout(location = 3) uniform float time;
void main(void)
{
	float theta = time*0.02f;
	vec3 pos_temp = vec3(0.0f);
	pos_temp.x = cos(theta)*pos_attrib.x + sin(theta)*pos_attrib.z;
	pos_temp.y = pos_attrib.y;
	pos_temp.z = -sin(theta)*pos_attrib.x + cos(theta)*pos_attrib.z;
   //Compute clip-space vertex position
   gl_Position = PVM*vec4(pos_temp, 1.0);     //w = 1 becase this is a point
  
   pos = pos_attrib;
   vec3 p_world=vec3(M*(pos,1.0));
   tex_coord=p_world.xy;
}