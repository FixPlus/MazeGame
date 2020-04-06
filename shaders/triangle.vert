#version 430

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inUV;
layout (location = 2) in vec3 inColor;
layout (location = 3) in vec3 inNormal;

layout (binding = 0) uniform UBO 
{
	mat4 projectionMatrix;
	vec4 lightDirection;
	vec4 viewPos;
	float lodBias;
} ubo;

layout (location = 0) out vec3 outUV;
layout (location = 1) out vec3 outNormal;
layout (location = 2) out vec3 outTrace;
layout (location = 3) out vec3 outViewTrace;
layout (location = 4) out float outLodBias;
layout (location = 5) out vec3 outColor;

out gl_PerVertex 
{
    vec4 gl_Position;   
};

//The light source is attached to camera

void main() 
{

	outUV = inUV;

	gl_Position = ubo.projectionMatrix * vec4(inPos.xyz, 1.0);

	vec3 trace = vec3(ubo.lightDirection);
//	vec3 norm_trace = normalize(trace);
	vec3 normal = normalize(inNormal);


	outNormal = normalize(inNormal);
	outTrace = trace;
	outViewTrace = ubo.viewPos.xyz - inPos.xyz;
	outLodBias = length(outViewTrace) / 200.0;
	if(dot(outNormal, outViewTrace) < 0.0f){
		outUV = vec3(0.0f, 0.0f, 0.0f);
		outColor = vec3(-1.0f, -1.0f, -1.0f);
	}
	else{
		outColor = inColor;
	}
}
