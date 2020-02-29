#include "StaticMesh.h"
#include "Engine/Mesh/MeshData.h"
#include "Engine/Core/Engine.h"
#include "Engine/Renderer/IRenderDevice.h"
#include "Engine/Renderer/IRenderBuffer.h"
#pragma once

StaticMesh::StaticMesh(Engine* engine, const MeshData* data)
    : mEngine(engine)
{
    mVertexBuffer = mEngine->renderDevice()->createBufferWithData(data->vertices, data->vertexCount * sizeof(MeshVertex));
    mIndexBuffer = mEngine->renderDevice()->createBufferWithData(data->indices, data->indexCount * sizeof(uint16_t));
}

StaticMesh::~StaticMesh()
{
}
