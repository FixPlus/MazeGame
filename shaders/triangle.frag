#version 430
#define TEXTURE_SAMPLER(a) samplerColor ## a

#define TEXTURE(N) layout (binding = N) uniform sampler2D TEXTURE_SAMPLER(N)

TEXTURE(1);


layout (location = 0) in vec3 inUV;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec3 inTrace;
layout (location = 3) in vec3 inViewTrace;
layout (location = 4) in float inLodBias;
layout (location = 5) in vec3 inColor;

layout (location = 0) out vec4 outFragColor;

sampler2D getTextureSampler(int id){
	switch(id){
		case 1: return TEXTURE_SAMPLER(1);
		default: return TEXTURE_SAMPLER(1);
	}
}
void main() 
{
	
	if(inColor.x < 0.0f)
		outFragColor = vec4(0.0f, 0.0f, 0.0f, 0.0f);
	else{
	  float len_v_trace = length(inViewTrace);
	  vec4 color = texture(samplerColor1, vec2(inUV.x, inUV.y), 1.0f);
	  
	  vec3 norm_trace = normalize(inTrace);
	  vec3 norm_view_trace = normalize(inViewTrace);
//	  vec3 reflect = normalize(reflect(norm_trace, inNormal));
	  float enlighted = max((dot(inNormal, -norm_trace) + 1.0) * 0.5 *(750.0 - len_v_trace) / 750.00, 0.0);
	  float fading = min(max((len_v_trace - 200.0f) / 750.00, 0.0f), 1.0f);
	//  float enlighted = max((dot(inNormal, -norm_trace) + 1.0) * 0.5, 0.0);
	//  enlighted = enlighted *((abs(dot(reflect, norm_view_trace) + 4.0) / 5.0));
	//  vec3 specular = pow(max(dot(reflect, norm_view_trace), 0.0), 16) * vec3(1.0);
	//	float specular = max(pow(max(dot(reflect, norm_view_trace), 0.0), 16) * (200.0 - len_v_trace) / 200.00, 0.0);
	  
	//  if(dot(inNormal, -norm_trace) < 0)
	//  	specular *= 0;
	  outFragColor = vec4(vec3(color.r * inColor.r, color.g * inColor.g, color.b * inColor.b) * (enlighted * 0.9 + 0.1), color.a);
	  vec4 revFragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f) - outFragColor;
	  outFragColor = fading * revFragColor + outFragColor;
	  //outFragColor = vec4(specular, 1.0);
	}
}
