#version 420

in vec2 fragUV;

out vec4 finalColor;

void main()
{
    finalColor = vec4(fragUV, 0.0, 1.0);
}