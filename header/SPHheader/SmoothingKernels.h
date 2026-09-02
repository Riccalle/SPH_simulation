#ifndef SMOOTHING_KERNELS
#define SMOOTHING_KERNELS

#include <iostream>
#include <string>
#include <cmath>
#include <unordered_map>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

float densityKernel(float radius, float Hradius);

glm::vec3 pressureGradientKernel(glm::vec3 radius, float Hradius);

glm::vec3 pressureForce(int i, int j, std::vector<glm::vec3> &pos, std::vector<float> &pressure, std::vector<float> &density, float mass, float Hradius);

void computePressure(float targetDensity, float stiffness, std::vector<float> &density, std::vector<float> &pressure);

#endif
