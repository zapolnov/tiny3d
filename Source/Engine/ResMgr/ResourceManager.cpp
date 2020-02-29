#include "ResourceManager.h"
#include "Engine/ResMgr/Shader.h"
#include "Engine/ResMgr/Texture.h"
#include "Engine/Mesh/Material.h"
#include "Engine/Core/Engine.h"
#include "Engine/Renderer/IRenderDevice.h"

ResourceManager::ResourceManager(Engine* engine)
    : mEngine(engine)
{
}

ResourceManager::~ResourceManager()
{
}

std::shared_ptr<Shader> ResourceManager::cachedShader(const ShaderCode* code)
{
    auto it = mShaders.find(code);
    if (it != mShaders.end()) {
        auto ptr = it->second.lock();
        if (ptr)
            return ptr;
    }

    auto shader = std::make_shared<Shader>(mEngine, mEngine->renderDevice()->createShaderProgram(code));
    mShaders[code] = shader;

    return shader;
}

std::shared_ptr<Material> ResourceManager::cachedMaterial(const MaterialData* data)
{
    auto it = mMaterials.find(data);
    if (it != mMaterials.end()) {
        auto ptr = it->second.lock();
        if (ptr)
            return ptr;
    }

    auto material = std::make_shared<Material>(mEngine, data);
    mMaterials[data] = material;

    return material;
}

std::shared_ptr<Texture> ResourceManager::cachedTexture(const TextureData* data)
{
    auto it = mTextures.find(data);
    if (it != mTextures.end()) {
        auto ptr = it->second.lock();
        if (ptr)
            return ptr;
    }

    auto texture = std::make_shared<Texture>(mEngine, mEngine->renderDevice()->createTexture(data));
    mTextures[data] = texture;

    return texture;
}
