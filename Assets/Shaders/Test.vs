#version 420

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uv;

layout(std140) uniform MatrixBuffer
{
    mat4 world;
	mat4 view;
	mat4 proj;
} matrixBuffer;

out vec2 fragUV;

void main()
{
	vec4 pos = matrixBuffer.proj * matrixBuffer.view * matrixBuffer.world * vec4(position, 1.0);
    fragUV = uv;
    gl_Position = pos;
}