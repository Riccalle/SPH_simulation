#include "SPHheader/SmoothingKernels.h"

// Il densityKernel deve essere normalizzato a 1
float densityKernel(float radius, float Hradius) {
    if(radius > Hradius) return 0.0f;

    float k = 4.0f / (M_PI * pow(Hradius, 8)); // Normalization constant
    float delta = Hradius * Hradius - radius * radius;
    return k * delta * delta * delta;
}

glm::vec3 pressureGradientKernel(glm::vec3 radius, float Hradius) {
    float radiusValue = glm::length(glm::vec2(radius.x, radius.y));
    if(radiusValue < 1e-6f || radiusValue > Hradius) return glm::vec3(0.0f);

    float k = -30.0f / (M_PI * pow(Hradius, 5));
    float tempCoeff = k * (Hradius - radiusValue) * (Hradius - radiusValue) / radiusValue;

    glm::vec3 grad = tempCoeff * radius;
    grad.z = 0.0f;

    return grad;
}

glm::vec3 pressureForce(
    int i, int j,
    std::vector<glm::vec3> &pos,
    std::vector<float> &pressure,
    std::vector<float> &density,
    float mass,
    float Hradius
) {
    glm::vec3 radius = pos[i] - pos[j];
    glm::vec3 pressureGrad = pressureGradientKernel(radius, Hradius);

    float rhoj = std::max(density[j], 1e-6f);
    glm::vec3 force = -mass * (pressure[i] + pressure[j]) / (2.0f * rhoj) * pressureGrad;
    return force;
}

void computePressure(float targetDensity, float stiffness, std::vector<float> &density, std::vector<float> &pressure) {
    pressure.resize(density.size());

    for(int i = 0; i < density.size(); i++) {
        pressure[i] = std::max(stiffness * (density[i] - targetDensity), 0.0f);
    }
}
