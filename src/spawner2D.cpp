#include "SPHheader/spawner2D.h"

// Nparticles = area * spawnDensity
glm::ivec2 calculateSpawnCountPerAxis(glm::vec2 size, float spawnDensity) {
    float area = size.x * size.y;
    unsigned int particlesTargetNumber = area * spawnDensity;

    glm::vec2 dimensionVector = glm::vec2(size.x / (size.x + size.y), size.y / (size.x + size.y));
    float m = sqrt(particlesTargetNumber / (dimensionVector.x * dimensionVector.y));

    return glm::vec2(
        std::ceil(dimensionVector.x * m), 
        std::ceil(dimensionVector.y * m)
    );
}

std::vector<glm::vec2> pointGrid(SpawnRegion spawnRegion, glm::vec2 size, float spawnDensity) {
    glm::ivec2 spawnCountPerAxis = calculateSpawnCountPerAxis(size, spawnDensity);
    std::vector<glm::vec2> positions = {};

    for (int i = 0; i < spawnCountPerAxis.x; i++) {
        for (int j = 0; j < spawnCountPerAxis.y; j++) {
            float x = i * spawnRegion.size.x / spawnCountPerAxis.x + spawnRegion.position.x;
            float y = j * spawnRegion.size.y / spawnCountPerAxis.y + spawnRegion.position.y;
        }
    }
}
