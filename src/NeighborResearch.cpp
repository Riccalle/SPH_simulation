#include "SPHheader/NeighborResearch.h"

std::string getCellKey(Cell cell) {
    return std::to_string(cell.x) + "_" + std::to_string(cell.y);
}

Cell getCell(float posX, float posY, float cellSize) {
    Cell cell;

    cell.x = std::floor(posX / cellSize);
    cell.y = std::floor(posY / cellSize);

    return cell;
}

void buildGrid(std::vector<glm::vec3> &pos, float cellSize, 
    std::unordered_map<std::string, std::vector<int>> &grid) {
    
    grid.clear();

    for(int i = 0; i < pos.size(); i++) {
        Cell cell = getCell(pos[i].x, pos[i].y, cellSize);
        std::string key = getCellKey(cell);
        grid[key].push_back(i);
    }
}

void findNeighbor(std::vector<glm::vec3> &pos, float cellSize, float Hradius, 
    const std::unordered_map<std::string, std::vector<int>> &grid,
    std::vector<std::vector<int>> &neighbors
) {
    neighbors.clear();
    neighbors.resize(pos.size());

    for(int i = 0; i < pos.size(); i++) {
        Cell cell = getCell(pos[i].x, pos[i].y, cellSize);

        for(int dx = -1; dx <= 1; dx++) {
            for(int dy = -1; dy <= 1; dy++) {
                int cx = cell.x + dx;
                int cy = cell.y + dy;

                std::string key = getCellKey((Cell){cx, cy});

                auto neighborCell = grid.find(key);
                if(neighborCell == grid.end()) continue;

                for(int j : neighborCell->second) {
                    if(j == i) continue;

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
    NeighborResearch::cellSize = Hradius;
    NeighborResearch::Hradius = HRadius;
}

void NeighborResearch::updateResearch(std::vector<glm::vec3> &pos) {
    buildGrid(pos, NeighborResearch::cellSize, NeighborResearch::grid);
    findNeighbor(pos, NeighborResearch::cellSize, NeighborResearch::Hradius, NeighborResearch::grid, NeighborResearch::neighbors);
}

std::unordered_map<std::string, std::vector<int>> NeighborResearch::grid;
std::vector<std::vector<int>> NeighborResearch::neighbors;
