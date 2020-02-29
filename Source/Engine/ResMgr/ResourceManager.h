#pragma once
#include <unordered_map>

struct TextureData;
struct MeshMaterial;
struct ShaderCode;
class Engine;
class Texture;
class Shader;
class Material;

class ResourceManager
{
public:
    explicit ResourceManager(Engine* engine);
    ~ResourceManager();

    std::shared_ptr<Shader> cachedShader(const ShaderCode* code);
    std::shared_ptr<Material> cachedMaterial(const MeshMaterial* data);
    std::shared_ptr<Texture> cachedTexture(const TextureData* data);

private:
    Engine* mEngine;
    std::unordered_map<const ShaderCode*, std::weak_ptr<Shader>> mShaders;
    std::unordered_map<const MeshMaterial*, std::weak_ptr<Material>> mMaterials;
    std::unordered_map<const TextureData*, std::weak_ptr<Texture>> mTextures;
};
