#pragma once
#include <memory>

struct MeshData;
class Engine;
class IRenderBuffer;

class StaticMesh
{
public:
    StaticMesh(Engine* engine, const MeshData* data);
    ~StaticMesh();

private:
    Engine* mEngine;
    std::unique_ptr<IRenderBuffer> mVertexBuffer;
    std::unique_ptr<IRenderBuffer> mIndexBuffer;
};
