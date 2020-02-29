#include "LevelMeshBuilder.h"

static glm::vec2 makeTexCoord(int tileX, int tileY)
{
    float x = tileX * (32.0f / 256.0f);
    float y = tileY * (32.0f / 256.0f);
    return glm::vec2(x, y);
}

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
        ss << "}, { ";
        ss << vertex.texCoord.x << ", ";
        ss << vertex.texCoord.y << ", ";
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
    createHorizontalSquare(x, y, 0.0f, 3, 2);
}

void LevelMeshBuilder::createWall(float x, float y)
{
    y = LevelHeight - y - 1;
    float x1 = x, y1 = y, x2 = x + 1.0f, y2 = y + 1.0f, z1 = 0.0f, z2 = 1.0f;

    createHorizontalSquare(x, y, 1.0f, 1, 1);

    createSquareIndices();
    mVertices.emplace_back(LevelVertex{ { x1, y1, z1 }, makeTexCoord(3, 0) });
    mVertices.emplace_back(LevelVertex{ { x2, y1, z1 }, makeTexCoord(3, 0) });
    mVertices.emplace_back(LevelVertex{ { x2, y1, z2 }, makeTexCoord(3, 0) });
    mVertices.emplace_back(LevelVertex{ { x1, y1, z2 }, makeTexCoord(3, 0) });
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

void LevelMeshBuilder::createHorizontalSquare(float x, float y, float z, int tileX, int tileY)
{
    float x1 = x, y1 = y, x2 = x + 1.0f, y2 = y + 1.0f;
    createSquareIndices();
    mVertices.emplace_back(LevelVertex{ { x1, y1, z }, makeTexCoord(tileX + 0, tileY + 0) });
    mVertices.emplace_back(LevelVertex{ { x2, y1, z }, makeTexCoord(tileX + 1, tileY + 0) });
    mVertices.emplace_back(LevelVertex{ { x2, y2, z }, makeTexCoord(tileX + 1, tileY + 1) });
    mVertices.emplace_back(LevelVertex{ { x1, y2, z }, makeTexCoord(tileX + 0, tileY + 1) });
}
