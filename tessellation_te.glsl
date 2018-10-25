#version 430
layout(location = 0) uniform mat4 P;
layout(location = 1) uniform mat4 V;
layout(location = 2) uniform mat4 M;

layout(location = 3) uniform float time;
uniform sampler2D height;
//layout(location=6) uniform int doClip=0;
in vec2 tex_coord_te[];
out vec2 tex_coord; 

//layout (quads, equal_spacing, ccw) in;	//discrete LOD
//Try some of these other options
layout (quads, fractional_odd_spacing, ccw) in;	//continuous LOD
//layout (quads, fractional_even_spacing, ccw) in;	//continuous LOD

out float dist;
out vec3 N;
out vec4 p_eye;
out float depth;
out float visibility;
void main()
{
	const float u = gl_TessCoord.x;
	const float v = gl_TessCoord.y;

	const vec4 p0 = gl_in[0].gl_Position;
	const vec4 p1 = gl_in[1].gl_Position;
	const vec4 p2 = gl_in[2].gl_Position;
	const vec4 p3 = gl_in[3].gl_Position;

	vec4 p = (1.0-u)*(1.0-v)*p0 + u*(1.0-v)*p1 + u*v*p2 + (1.0-u)*v*p3;
	tex_coord=(1.0-u)*(1.0-v)*tex_coord_te[0] + u*(1.0-v)*tex_coord_te[1] + u*v*tex_coord_te[2] + (1.0-u)*v*tex_coord_te[3];
	vec4 heightmap = texture(height, tex_coord);
	//apply terrain height offset in z-direction
	p.z += heightmap.r*15;

	//compute normal
	// read neightbor heights using an arbitrary small offset
	 const ivec3 off = ivec3(-1,0,1);
	  float hL =  textureOffset(height, tex_coord, off.xy).r*15;
	  float hR =  textureOffset(height, tex_coord, off.zy).r*15;    
	  float hD =  textureOffset(height, tex_coord, off.yx).r*15;    
	  float hU =  textureOffset(height, tex_coord, off.yz).r*15;    

    // deduce terrain normal
	
	  N.x = hL - hR;
	  N.y = hD - hU;
	  N.z = 2.0;
	  N =normalize(N);
	  vec4 p_eye = vec4(0.0);
	  p_eye = V*M*p;

	vec4 p_world = M*p;
	depth = p_world.z;
	dist = p_eye.z;
	gl_Position = P*p_eye;

}
