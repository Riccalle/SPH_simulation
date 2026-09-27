#version 430 core

struct Particle {
    vec4 pos;
    vec4 vel;
};

layout(std430, binding = 0) buffer Particles {
    Particle p[];
}

void main() {
    uint i = gl_VertexID;
    vec3 pos = p[i].pos.xyz;
    gl_Position = vec4(pos.xyz, 1.0);
}
