#include "Material.h"
#include "Engine/Core/Engine.h"
#include "Engine/ResMgr/ResourceManager.h"
#include "Engine/ResMgr/Shader.h"
#include "Engine/ResMgr/Texture.h"
#include "Engine/Renderer/IRenderDevice.h"
#include "Engine/Renderer/IPipelineState.h"
#include "Engine/Mesh/MeshData.h"

Material::Material(Engine* engine, const MeshMaterial* data)
    : mEngine(engine)
{
    mShader = mEngine->resourceManager()->cachedShader(data->shader);

    mTextures.reserve(data->textureCount);
    for (size_t i = 0; i < data->textureCount; i++)
        mTextures.emplace_back(mEngine->resourceManager()->cachedTexture(data->textures[i]));

    mPipelineState = mEngine->renderDevice()->createPipelineState(mShader->instance(), MeshVertex::format());
}

Material::~Material()
{
}

void Material::bind()
{
    mEngine->renderDevice()->setPipelineState(mPipelineState);

    size_t index = 0;
    for (const auto& texture : mTextures)
        mEngine->renderDevice()->setTexture(index, texture->instance());
}
