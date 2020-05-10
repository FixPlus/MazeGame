#version 450

layout (location = 0) out vec3 outUVW;

layout (binding = 0) uniform UBO
{
	mat4 projection;
	vec4 viewPos;
	vec4 lightPos;

} ubo;


void main() 
{
	outUVW = vec3((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2, gl_VertexIndex & 2);
	gl_Position = vec4(outUVW.st * 2.0f - 1.0f, 0.0f, 1.0f);
}