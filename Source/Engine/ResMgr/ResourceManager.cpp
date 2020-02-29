#include "ResourceManager.h"
#include "Engine/ResMgr/Shader.h"
#include "Engine/ResMgr/Texture.h"
#include "Engine/Mesh/Material.h"
#include "Engine/Mesh/StaticMesh.h"
#include "Engine/Core/Engine.h"
#include "Engine/Renderer/IRenderDevice.h"

namespace
{
    template <typename T, typename P, typename C>
    std::shared_ptr<T> cachedObject(std::unordered_map<const P*, std::weak_ptr<T>>& map, const P* key, C construct)
    {
        auto it = map.find(key);
        if (it != map.end()) {
            auto ptr = it->second.lock();
            if (ptr)
                return ptr;
        }

        auto obj = construct();
        map[key] = obj;

        return obj;
    }
}

ResourceManager::ResourceManager(Engine* engine)
    : mEngine(engine)
{
}

ResourceManager::~ResourceManager()
{
}

std::shared_ptr<Shader> ResourceManager::cachedShader(const ShaderCode* code)
{
    return cachedObject(mShaders, code, [this, code] {
            return std::make_shared<Shader>(mEngine, mEngine->renderDevice()->createShaderProgram(code));
        });
}

std::shared_ptr<Material> ResourceManager::cachedMaterial(const MaterialData* data)
{
    return cachedObject(mMaterials, data, [this, data] {
            return std::make_shared<Material>(mEngine, data);
        });
}

std::shared_ptr<Texture> ResourceManager::cachedTexture(const TextureData* data)
{
    return cachedObject(mTextures, data, [this, data] {
            return std::make_shared<Texture>(mEngine, mEngine->renderDevice()->createTexture(data));
        });
}

std::shared_ptr<StaticMesh> ResourceManager::cachedStaticMesh(const MeshData* data)
{
    return cachedObject(mStaticMeshes, data, [this, data] {
            return std::make_shared<StaticMesh>(mEngine, data);
        });
}
