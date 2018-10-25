#version 430

uniform samplerCube cubemap;

layout(location = 3) uniform float time;

out vec4 fragcolor;           
in vec3 pos;
in float depth;
in vec2 tex_coord;
void main(void)
{   
   //fragcolor = vec4(pos, 1.0);
   float lowerlimit=0.0;
   float upperlimit=0.8;
   vec4 fogcolor=vec4(0.8,0.88,0.91,1.0);
   float factor=(pos.y-lowerlimit)/(upperlimit-lowerlimit);
   factor=clamp(factor,0.0,1.0);
   fragcolor = texture(cubemap, pos);
//   fragcolor=mix(fogcolor,fragcolor,factor);
 //fragcolor=vec4(factor,1.0);
}




















