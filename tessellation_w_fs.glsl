#version 430
layout(location = 2) uniform mat4 M;
layout(location = 1) uniform mat4 V;
layout(location = 0) uniform mat4 P;
layout(location = 4) uniform sampler2D tex;
layout(location = 5) uniform sampler2D watertex;
layout(location = 6) uniform sampler2D waternormal;
layout(location = 7) uniform sampler2D refraction_tex;
layout(location = 8) uniform sampler2D shadow_tex;
layout(location = 3) uniform float time;
out vec4 fragcolor; 
          
in float dist;
in vec2 tex_coord; 
in vec4 clipspace;
in vec4 pos_eye;
in vec3 normal_eye;

const float nearPlane = 0.1f;
const float farPlane = 1000.0f;


//Define distortion strength
const float distortionStrength = 0.04f;
//Define texture coordinate scaling
const float tex_coord_Scaling = 1.0f;

const float fresnel_Strength = 1.0f;

const float spetre_coe = 3.0f;

void main(void)
{   
	vec3 v_eye = normalize(vec3(vec4(0,0,0,1.0) - pos_eye));
	//vec3 v_world = normalize(vec3(inverse(V)*vec4(cam_v,0)));

	//vec3 norm_eye = vec3(transpose(inverse(P*V))*vec4(0,1,0,0));
	vec3 norm_eye = vec3(0,1,0);
    float movefactor=0;
	movefactor+=0.02*time;
//	movefactor=0.2*movefactor;
	

	//Define fresnel coefficient
	float blend_factor = max(dot(norm_eye,v_eye),0);
	float blend_fScale = 1.0f*clamp(pow(blend_factor,fresnel_Strength),0.0,1.0f);

	vec2 ndc=(clipspace.xy/clipspace.w)/2.0+vec2(0.5f);
	vec2 reflectiontex_coord=vec2(ndc.x,ndc.y);
	vec2 refractiontex_coord=vec2(ndc.x,ndc.y);

	float depth_color = texture(shadow_tex,reflectiontex_coord).r;
	float floorDistance = 2.0*nearPlane *farPlane /(farPlane + nearPlane - (2*depth_color - 1.0)*(farPlane - nearPlane));

	depth_color = gl_FragCoord.z;//*gl_FragCoord.w;
	float waterDistance = 2.0*nearPlane *farPlane /(farPlane + nearPlane - (2*depth_color - 1.0)*(farPlane - nearPlane));

	vec2 tex_coordS = tex_coord*tex_coord_Scaling;

	vec2 distortedTexCoords = texture(watertex, vec2(tex_coordS.x + movefactor, tex_coordS.y)).rg*0.1;
	distortedTexCoords = tex_coordS + vec2(-distortedTexCoords.x+ movefactor, distortedTexCoords.y+movefactor);
	vec2 distortion = (texture(watertex, distortedTexCoords).rg * 2.0 - 1.0)* distortionStrength*blend_fScale;
	//vec2 distortion_01 = (texture(watertex, vec2(tex_coordS.x + movefactor, tex_coordS.y)).rg*2.0 - 1.0)* distortionStrength;
	//vec2 distortion_02 = (texture(watertex, vec2(-tex_coordS.x + movefactor, tex_coordS.y+ movefactor)).rg*2.0 - 1.0)* distortionStrength;
	//vec2 distortion = (distortion_01 + distortion_02)*blend_fScale;

	reflectiontex_coord+=distortion;
	reflectiontex_coord.x=clamp(reflectiontex_coord.x,0.001,0.99);
	reflectiontex_coord.y=clamp(reflectiontex_coord.y,0.001,0.99);

	refractiontex_coord += distortion;
	refractiontex_coord.x=clamp(refractiontex_coord.x,0.001,0.99);
	refractiontex_coord.y=clamp(refractiontex_coord.y,0.001,0.99);

	vec4 reflection=texture(tex,reflectiontex_coord);
	vec4 refraction=texture(refraction_tex,refractiontex_coord)*blend_fScale;
	refraction.a = 1.0f;

	vec4 normalcolor=texture(waternormal,distortedTexCoords*2.0f);
	vec3 N=vec3(normalcolor.r*2.0-1.0, normalcolor.b, normalcolor.g*2.0-1.0);
	N=normalize(N);


	//const vec3 light = vec3(0.0, 1.0, 1.0) - vec3(inverse(P*V)*clipspace); //light direction
	const vec3 light = vec3(V * vec4(0.0, 1.0, 1.0,0.0)) ;
	const vec3 spec_color = vec3(1.0, 1.0, 1.0);

	vec3 v = v_eye;
	vec3 r = reflect(-light, N);
	float spec=pow(max(0.0, dot(r, v)),spetre_coe);
	
	vec4 color2=mix(reflection,refraction,blend_fScale);

    //color2.a = 1.0f*clamp(pow(1-blend_factor,0.1),0.92,1.0);
	vec4 color0 = vec4(0.8, 0.9, 1.0, 1.0);
    vec4 color1 = vec4(1.0f);
	vec4 colorF = mix(color0,color1,blend_factor);
	float depth=waterDistance-floorDistance;
	fragcolor =  vec4(vec3(color2*colorF),1.0)+vec4(spec*spec_color*0.3,0.0)* pow((1-blend_factor),6.0);
//	fragcolor.a = clamp(depth/5,0.0,1.0);

}

