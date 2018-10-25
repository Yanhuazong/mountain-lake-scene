#version 430

layout(location = 0) uniform mat4 P;
layout(location = 1) uniform mat4 V;
layout(location = 2) uniform mat4 M;
layout(location = 10) uniform float slider[6];
in vec2 tex_coord_tc[];

out vec2 tex_coord_te[];

layout (vertices = 4) out;  //number of output verts of the tess. control shader

float dist_point_to_line_segment(vec3 p, vec3 l0, vec3 l1)
{
	vec3 v=l1-l0;
	vec3 w=p-l0;
	float b=clamp(dot(v,w)/dot(v,v),0.0,1.0);
	return distance(p,l0+b*v);
}

float outer_tess_level(float d)
{
	return clamp(40.0*exp(-0.001*10.0*d),0.0,32.0);
}
void main()
{

	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

	/*DEMO: Idea, let the tessellation levels depend on distance from the eye.

						3-------2
			eye.		|		|
						|		|
						0-------1
	
			Make gl_TessLevelOuter[0] depend on distance from eye to edge 3,0.
			Make gl_TessLevelOuter[1] depend on distance from eye to edge 0,1.
			Etc...

			TODO: write 2 functions: 
				float dist_point_to_line_segment(vec3 p, vec3 l0, vec3 l1);
				float outer_tess_level(float d);

				Let inner tesselation levels be averages of outer tessellation levels.
	*/
	vec3 eye=vec3(0.0);
	vec3 p0=vec3(V*M*gl_in[0].gl_Position);
	vec3 p1=vec3(V*M*gl_in[1].gl_Position);
	vec3 p2=vec3(V*M*gl_in[2].gl_Position);
	vec3 p3=vec3(V*M*gl_in[3].gl_Position);
	float d0=dist_point_to_line_segment(eye,p3,p0);
	float d1=dist_point_to_line_segment(eye,p0,p1);
	float d2=dist_point_to_line_segment(eye,p1,p2);
	float d3=dist_point_to_line_segment(eye,p2,p3);
	gl_TessLevelOuter[0] = outer_tess_level(d0);
	gl_TessLevelOuter[1] = outer_tess_level(d1);
	gl_TessLevelOuter[2] = outer_tess_level(d2);
	gl_TessLevelOuter[3] = outer_tess_level(d3);

	gl_TessLevelInner[0] = 0.5*gl_TessLevelOuter[1]+0.5*gl_TessLevelOuter[3];
	gl_TessLevelInner[1] = 0.5*gl_TessLevelOuter[0]+0.5*gl_TessLevelOuter[2];
	tex_coord_te[gl_InvocationID] = tex_coord_tc[gl_InvocationID];

}
