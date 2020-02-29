#pragma once
#include <vector>
#include <memory>

struct MeshData;
class Engine;
class Material;
class IRenderBuffer;

class StaticMesh
{
public:
    StaticMesh(Engine* engine, const MeshData* data);
    ~StaticMesh();

    void render() const;

private:
    struct Element
    {
        unsigned firstIndex;
        unsigned indexCount;
        std::shared_ptr<Material> material;
    };

    Engine* mEngine;
    std::vector<Element> mElements;
    std::unique_ptr<IRenderBuffer> mVertexBuffer;
    std::unique_ptr<IRenderBuffer> mIndexBuffer;
};
