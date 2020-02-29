#include "StaticMesh.h"
#include "Engine/Mesh/MeshData.h"
#include "Engine/Core/Engine.h"
#include "Engine/ResMgr/ResourceManager.h"
#include "Engine/Mesh/Material.h"
#include "Engine/Renderer/IRenderDevice.h"
#include "Engine/Renderer/IRenderBuffer.h"

StaticMesh::StaticMesh(Engine* engine, const MeshData* data)
    : mEngine(engine)
{
    mVertexBuffer = mEngine->renderDevice()->createBufferWithData(data->vertices, data->vertexCount * sizeof(MeshVertex));
    mIndexBuffer = mEngine->renderDevice()->createBufferWithData(data->indices, data->indexCount * sizeof(uint16_t));

    mElements.reserve(data->materialCount);
    for (size_t i = 0; i < data->materialCount; i++) {
        Element e;
        e.firstIndex = data->materials[i].firstIndex;
        e.indexCount = data->materials[i].indexCount;
        e.material = mEngine->resourceManager()->cachedMaterial(data->materials[i].material);
        mElements.emplace_back(std::move(e));
    }
}

StaticMesh::~StaticMesh()
{
}

void StaticMesh::render() const
{
    mEngine->renderDevice()->setVertexBuffer(mVertexBuffer);
    for (const auto& e : mElements) {
        e.material->bind();
        mEngine->renderDevice()->drawIndexedPrimitive(Triangles, mIndexBuffer, e.firstIndex, e.indexCount);
    }
}
