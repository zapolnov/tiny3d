#include "LevelMeshBuilder.h"

LevelMeshBuilder::LevelMeshBuilder()
{
}

LevelMeshBuilder::~LevelMeshBuilder()
{
}

void LevelMeshBuilder::generateCxxCode(const std::string& levelId, std::stringstream& ss) const
{
    ss << "const LevelVertex " << levelId << "Vertices[] = {\n";
    for (const auto& vertex : mVertices) {
        ss << "    { { ";
        ss << vertex.position.x << ", ";
        ss << vertex.position.y << ", ";
        ss << vertex.position.z << ", ";
        ss << "} },\n";
    }
    ss << "};\n\n";

    ss << "const uint16_t " << levelId << "Indices[] = {\n";
    for (auto index : mIndices)
        ss << "    " << index << ",\n";
    ss << "};\n";
}

void LevelMeshBuilder::createFloor(float x, float y)
{
    y = LevelHeight - y - 1;
    createHorizontalSquare(x, y, 0.0f);
}

void LevelMeshBuilder::createWall(float x, float y)
{
    y = LevelHeight - y - 1;
    float x1 = x, y1 = y, x2 = x + 1.0f, y2 = y + 1.0f, z1 = 0.0f, z2 = 1.0f;

    createHorizontalSquare(x, y, 1.0f);

    createSquareIndices();
    mVertices.emplace_back(LevelVertex{ { x1, y1, z1 } });
    mVertices.emplace_back(LevelVertex{ { x2, y1, z1 } });
    mVertices.emplace_back(LevelVertex{ { x2, y1, z2 } });
    mVertices.emplace_back(LevelVertex{ { x1, y1, z2 } });
}

void LevelMeshBuilder::createSquareIndices()
{
    uint16_t index = mVertices.size();
    mIndices.emplace_back(index + 0);
    mIndices.emplace_back(index + 1);
    mIndices.emplace_back(index + 2);
    mIndices.emplace_back(index + 2);
    mIndices.emplace_back(index + 3);
    mIndices.emplace_back(index + 0);
}

void LevelMeshBuilder::createHorizontalSquare(float x, float y, float z)
{
    float x1 = x, y1 = y, x2 = x + 1.0f, y2 = y + 1.0f;
    createSquareIndices();
    mVertices.emplace_back(LevelVertex{ { x1, y1, z } });
    mVertices.emplace_back(LevelVertex{ { x2, y1, z } });
    mVertices.emplace_back(LevelVertex{ { x2, y2, z } });
    mVertices.emplace_back(LevelVertex{ { x1, y2, z } });
}
