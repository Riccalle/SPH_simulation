#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 3) in vec3 instanceVel;
layout (location = 2) in vec3 instancePos;

out vec3 vel;

void main()
{
    vec3 realPosition = aPos + instancePos;
    gl_Position = vec4(realPosition, 1.0);
    
    vel = instanceVel;
}
