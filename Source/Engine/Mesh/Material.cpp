#include "Material.h"
#include "Engine/Core/Engine.h"
#include "Engine/ResMgr/ResourceManager.h"
#include "Engine/ResMgr/Shader.h"
#include "Engine/ResMgr/Texture.h"
#include "Engine/Renderer/IRenderDevice.h"
#include "Engine/Renderer/IPipelineState.h"
#include "Engine/Mesh/MeshData.h"
#include "Engine/Mesh/MaterialData.h"

Material::Material(Engine* engine, const MaterialData* data)
    : mEngine(engine)
{
    mShader = mEngine->resourceManager()->cachedShader(data->shader);

    mTextures.reserve(data->textureCount);
    for (size_t i = 0; i < data->textureCount; i++)
        mTextures.emplace_back(mEngine->resourceManager()->cachedTexture(data->textures[i]));

    mPipelineState = mEngine->renderDevice()->createPipelineState(Triangles, mShader->instance(), data->vertexFormat());
}

Material::~Material()
{
}

void Material::bind() const
{
    mEngine->renderDevice()->setPipelineState(mPipelineState);

    size_t index = 0;
    for (const auto& texture : mTextures) {
        mEngine->renderDevice()->setTexture(index, texture->instance());
        ++index;
    }
}
