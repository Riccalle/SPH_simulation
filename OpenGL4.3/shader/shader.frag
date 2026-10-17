#version 430 core

out vec4 FragColor;
in float speed;

void main() {
    float maxSpeed = 2.5;
    float t = clamp(speed / maxSpeed, 0.0, 1.0);

    vec3 baseColor = vec3(0.0, 0.4, 0.9);
    vec3 fastColor = vec3(0.9, 0.9, 1.0);

    vec3 color = mix(baseColor, fastColor, t);
    FragColor = vec4(color, 1.0);
}
