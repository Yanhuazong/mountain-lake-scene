#version 430
layout(location = 0) uniform mat4 P;
layout(location = 1) uniform mat4 V;
layout(location = 2) uniform mat4 M;
layout(location = 3) uniform float time;

in vec2 tex_coord_te[];


//layout (quads, equal_spacing, ccw) in;	//discrete LOD
//Try some of these other options
layout (quads, fractional_odd_spacing, ccw) in;	//continuous LOD
//layout (quads, fractional_even_spacing, ccw) in;	//continuous LOD

out float dist;
out vec4 pos_eye;
out vec4 clipspace;
out vec2 tex_coord; 
out vec3 normal_eye;

void main()
{
	const float u = gl_TessCoord.x;
	const float v = gl_TessCoord.y;

	const vec4 p0 = gl_in[0].gl_Position;
	const vec4 p1 = gl_in[1].gl_Position;
	const vec4 p2 = gl_in[2].gl_Position;
	const vec4 p3 = gl_in[3].gl_Position;

	vec4 p = (1.0-u)*(1.0-v)*p0 + u*(1.0-v)*p1 + u*v*p2 + (1.0-u)*v*p3;

	//DEMO: apply terrain height offset in z-direction
	// p.z+=*sin(10.0*p.x)*sin(5.0*p.y+time);

	tex_coord=(1.0-u)*(1.0-v)*tex_coord_te[0] + u*(1.0-v)*tex_coord_te[1] + u*v*tex_coord_te[2] + (1.0-u)*v*tex_coord_te[3];
	tex_coord=vec2(tex_coord.y,tex_coord.x);
	vec4 p_eye = V*M*p;
	pos_eye = V*M*p;

	normal_eye = vec3(transpose(inverse(V))*vec4(0.0,1.0,0.0,0.0));

	dist = p_eye.z;
	clipspace=P*p_eye;

	gl_Position = clipspace;

}
