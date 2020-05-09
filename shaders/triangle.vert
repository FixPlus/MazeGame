#version 430

// Vertex attributes
layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;
layout (location = 3) in vec3 inColor;

// Instanced attributes
layout (location = 4) in vec3 instancePos;
layout (location = 5) in vec3 instanceRot;
layout (location = 6) in float instanceScale;

layout (binding = 0) uniform UBO
{
	mat4 projection;
	vec4 viewPos;
	vec4 lightPos;

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

	outUV = vec3(inUV, 0.0f);


	mat3 mx, my, mz;
	
	// rotate around x
	float s = sin(instanceRot.x);
	float c = cos(instanceRot.x);

	mx[0] = vec3(c, s, 0.0);
	mx[1] = vec3(-s, c, 0.0);
	mx[2] = vec3(0.0, 0.0, 1.0);
	
	// rotate around y
	s = sin(instanceRot.y);
	c = cos(instanceRot.y);

	my[0] = vec3(c, 0.0, s);
	my[1] = vec3(0.0, 1.0, 0.0);
	my[2] = vec3(-s, 0.0, c);
	
	// rot around z
	s = sin(instanceRot.z);
	c = cos(instanceRot.z);	
	
	mz[0] = vec3(1.0, 0.0, 0.0);
	mz[1] = vec3(0.0, c, s);
	mz[2] = vec3(0.0, -s, c);
	
	mat3 rotMat = mz * my * mx;

	vec4 locPos = vec4(inPos.xyz * rotMat, 1.0);
	vec4 pos = vec4((locPos.xyz * instanceScale * 5.0f) + instancePos, 1.0);



	gl_Position = ubo.projection * pos;

	vec3 trace = vec3(ubo.lightPos);
//	vec3 norm_trace = normalize(trace);
//	vec3 normal = normalize(inNormal);


	outNormal = normalize(inNormal.xyz * rotMat);
	outTrace = trace;
	outViewTrace = ubo.viewPos.xyz - pos.xyz;
	outLodBias = length(outViewTrace) / 200.0;
	if(dot(outNormal, outViewTrace) < 0.0f){
		outUV = vec3(0.0f, 0.0f, 0.0f);
		outColor = vec3(-1.0f, -1.0f, -1.0f);
	}
	else{
		outColor = inColor;
	}
}
