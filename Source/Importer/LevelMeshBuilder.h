#pragma once
#include "Game/Level.h"
#include <vector>
#include <sstream>

class LevelMeshBuilder
{
public:
    LevelMeshBuilder();
    ~LevelMeshBuilder();

    size_t vertexCount() const { return mVertices.size(); }
    size_t indexCount() const { return mIndices.size(); }

    void generateCxxCode(const std::string& levelId, std::stringstream& ss) const;

    void createFloor(float x, float y);
    void createWall(float x, float y);

private:
    std::vector<LevelVertex> mVertices;
    std::vector<uint16_t> mIndices;

    void createSquareIndices();
    void createHorizontalSquare(float x, float y, float z);
};
