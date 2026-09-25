#ifndef NEIGHBOR_RESEARCH_H
#define NEIGHBOR_RESEARCH_H

#include <iostream>
#include <string>
#include <cmath>
#include <unordered_map>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct Cell {
    int x;
    int y;
};

uint64_t getCellKey(int x, int y);

Cell getCell(float posX, float posY, float cellSize);

void buildGrid(std::vector<glm::vec3> &pos, float cellSize, 
    std::unordered_map<uint64_t, std::vector<int>> &grid);

void findNeighbor(std::vector<glm::vec3> &pos, float cellSize, float Hradius, 
    const std::unordered_map<uint64_t, std::vector<int>> &grid,
    std::vector<std::vector<int>> &neighbors
);

class NeighborResearch {
    public:
        float cellSize;
        float Hradius;

        NeighborResearch(float radius, float HRadius);

        static std::unordered_map<uint64_t, std::vector<int>> grid;
        static std::vector<std::vector<int>> neighbors;

        void updateResearch(std::vector<glm::vec3> &pos);
};

#endif
