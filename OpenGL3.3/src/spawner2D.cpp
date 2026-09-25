#include "SPHheader/spawner2D.h"

// Nparticles = area * spawnDensity
glm::ivec2 calculateSpawnCountPerAxis(glm::vec2 size, float spawnDensity) {
    float area = size.x * size.y;
    unsigned int particlesTargetNumber = area * spawnDensity;

    glm::vec2 dimensionVector = glm::vec2(size.x / (size.x + size.y), size.y / (size.x + size.y));
    float m = sqrt(particlesTargetNumber / (dimensionVector.x * dimensionVector.y));

    return glm::ivec2(
        std::ceil(dimensionVector.x * m), 
        std::ceil(dimensionVector.y * m)
    );
}

std::vector<glm::vec3> pointGrid(SpawnRegion spawnRegion, float spawnDensity, float jitterStrenght) {
    glm::ivec2 spawnCountPerAxis = calculateSpawnCountPerAxis(spawnRegion.size, spawnDensity);
    std::vector<glm::vec3> positions = {};
    srand(1);

    for (int i = 0; i < spawnCountPerAxis.x; i++) {
        for (int j = 0; j < spawnCountPerAxis.y; j++) {
            float tx = spawnRegion.size.x / (spawnCountPerAxis.x - 1);
            float ty = spawnRegion.size.y / (spawnCountPerAxis.y - 1);

            float x = i * tx + spawnRegion.position.x;
            float y = j * ty + spawnRegion.position.y;

            float angle = (float)rand() / (float)RAND_MAX * 2 * 3.141592f;
            glm::vec2 dir = glm::vec2(
                glm::cos(angle),
                glm::sin(angle)
            );
            x += dir.x * jitterStrenght;
            y += dir.y * jitterStrenght;

            glm::vec3 vec = glm::vec3(x, y, 0.0f);
            positions.push_back(vec);
        }
    }

    return positions;
}
