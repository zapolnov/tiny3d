#pragma once
#include <unordered_map>

struct TextureData;
struct MaterialData;
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
    std::shared_ptr<Material> cachedMaterial(const MaterialData* data);
    std::shared_ptr<Texture> cachedTexture(const TextureData* data);

private:
    Engine* mEngine;
    std::unordered_map<const ShaderCode*, std::weak_ptr<Shader>> mShaders;
    std::unordered_map<const MaterialData*, std::weak_ptr<Material>> mMaterials;
    std::unordered_map<const TextureData*, std::weak_ptr<Texture>> mTextures;
};
