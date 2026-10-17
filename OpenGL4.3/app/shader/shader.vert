#version 430 core

struct Particle {
    vec4 pos;
    vec4 vel;
};

layout(std430, binding = 0) buffer Particles {
    Particle p[];
};

layout(location = 0) in vec3 meshVert;

uniform float radius;
out float speed;

void main() {
    uint i = gl_InstanceID;
    speed = length(p[i].vel.xy);
    vec3 centerPos = p[i].pos.xyz;
    vec2 pos = centerPos.xy + (meshVert.xy * radius);
    gl_Position = vec4(pos.xy, 0.0, 1.0);
}
