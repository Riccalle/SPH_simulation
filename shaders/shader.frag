#version 330 core

out vec4 FragColor;
in vec3 vel;

void main()
{
    vec3 baseColor = vec3(0.0, 0.6, 1.0);
    vec3 fastColor = vec3(1.0, 1.0, 1.0);

    float minSpeed = 0.1;
    float maxSpeed = 2.2;

    float speed = length(vel.xy);
    float t = clamp((speed - minSpeed) / (maxSpeed - minSpeed), 0.0, 1.0);
    
    vec3 finalColor = mix(baseColor, fastColor, t);
    FragColor = vec4(finalColor, 1.0);
}
