#version 430

uniform sampler2D tex;
layout(location=6) uniform int pass;

out vec4 fragcolor; 
in vec3 N;         
in float dist;
in vec2 tex_coord; 
in vec4 p_eye; 
in float depth;
in float visibility;

//layout(location = 0) out vec4 RefractionColor;

layout(location = 1) out vec4 depthColor;

void main(void)
{ 
	const vec3 light = vec3(0.0, 0.0, 1.0); //light direction
	const vec4 ambient_color = vec4(0.7, 0.7, 0.65, 1.0);
	const vec4 diffuse_color = vec4(0.722, 0.627, 0.545, 1.0);
	const vec4 spec_color = vec4(0.120, 0.120, 0.060, 1.0);

	vec3 v = p_eye.xyz;
	vec3 r = reflect(-light, N);

	if(pass==1)
	{if(depth < 0.5)
	{
			discard;
	}
	fragcolor  = texture(tex, tex_coord)*(ambient_color + diffuse_color*max(0.0, dot(N, light)) + spec_color*pow(max(0.0, dot(r, v)), 15.0));
	}
	
	if(pass==2)
	{
		if(depth > 0.0)
		{
			discard;
		}
		
		fragcolor  = texture(tex, tex_coord)*(ambient_color + diffuse_color*max(0.0, dot(N, light)) + spec_color*pow(max(0.0, dot(r, v)), 15.0));
		

		depthColor = vec4(pow(1.0-gl_FragCoord.z*gl_FragCoord.w,20.0));
	}


	else{
		if(depth < 0.5)
	{
			discard;
	}
	fragcolor  = texture(tex, tex_coord)*(ambient_color + diffuse_color*max(0.0, dot(N, light)) + spec_color*pow(max(0.0, dot(r, v)), 15.0));
	}
	fragcolor=mix(vec4(0.24,0.34,0.38,1.0)*1.3,fragcolor,exp(0.008*dist));

}

