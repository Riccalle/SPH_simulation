#ifndef SPAWNER_2D
#define SPAWNER_2D

#include <vector>
#include <iostream>
#include <time.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct SpawnRegion {
    glm::vec2 position;
    glm::vec2 size;
};

glm::ivec2 calculateSpawnCountPerAxis(glm::vec2 size, float spawnDensity);
std::vector<glm::vec3> pointGrid(SpawnRegion spawnRegion, float spawnDensity, float jitterStrenght);

#endif
