#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec3 instancePos;

out vec3 ourColor;

void main()
{
    vec3 realPosition = aPos + instancePos;
    gl_Position = vec4(realPosition, 1.0);
    
    ourColor = aColor;
}
