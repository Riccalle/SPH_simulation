#include "SPHheader/NeighborResearch.h"

uint64_t getCellKey(int x, int y) {
    const uint64_t p1 = 73856093ull;
    const uint64_t p2 = 19349663ull;

    uint64_t uint64_tx = static_cast<uint64_t>(static_cast<uint32_t>(x));
    uint64_t uint64_ty = static_cast<uint64_t>(static_cast<uint32_t>(y));

    return (uint64_tx * p1) ^ (uint64_ty * p2);
}

Cell getCell(float posX, float posY, float cellSize) {
    Cell cell;

    cell.x = std::floor(posX / cellSize);
    cell.y = std::floor(posY / cellSize);

    return cell;
}

void buildGrid(std::vector<glm::vec3> &pos, float cellSize, 
    std::unordered_map<uint64_t, std::vector<int>> &grid) {
    
    grid.clear();

    for(int i = 0; i < pos.size(); i++) {
        Cell cell = getCell(pos[i].x, pos[i].y, cellSize);
        uint64_t key = getCellKey(cell.x, cell.y);
        grid[key].push_back(i);
    }
}

void findNeighbor(std::vector<glm::vec3> &pos, float cellSize, float Hradius, 
    const std::unordered_map<uint64_t, std::vector<int>> &grid,
    std::vector<std::vector<int>> &neighbors
) {
    if (neighbors.size() != pos.size())
        neighbors.resize(pos.size());
    for (auto &n : neighbors) 
        n.clear(); // Cleaning the neighbors
    

    for(int i = 0; i < pos.size(); i++) {
        Cell cell = getCell(pos[i].x, pos[i].y, cellSize);

        for(int dx = -1; dx <= 1; dx++) {
            for(int dy = -1; dy <= 1; dy++) {
                int cx = cell.x + dx;
                int cy = cell.y + dy;
                uint64_t key = getCellKey(cx, cy);

                auto neighborCell = grid.find(key);
                if(neighborCell == grid.end()) continue;

                for (int j : neighborCell->second) {
                    float Dx = pos[i].x - pos[j].x;
                    float Dy = pos[i].y - pos[j].y;

                    if(Dx * Dx + Dy * Dy < Hradius * Hradius) {
                        neighbors[i].push_back(j);
                    }
                }
            }
        }
    }
}

NeighborResearch::NeighborResearch(float radius, float HRadius) {
    NeighborResearch::cellSize = HRadius;
    NeighborResearch::Hradius = HRadius;
}

void NeighborResearch::updateResearch(std::vector<glm::vec3> &pos) {
    buildGrid(pos, NeighborResearch::cellSize, NeighborResearch::grid);
    findNeighbor(pos, NeighborResearch::cellSize, NeighborResearch::Hradius, NeighborResearch::grid, NeighborResearch::neighbors);
}

std::unordered_map<uint64_t, std::vector<int>> NeighborResearch::grid;
std::vector<std::vector<int>> NeighborResearch::neighbors;
