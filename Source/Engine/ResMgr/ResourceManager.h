#pragma once
#include <unordered_map>
#include <memory>

struct TextureData;
struct MaterialData;
struct MeshData;
struct ShaderCode;
class Engine;
class Texture;
class Shader;
class Material;
class AnimatedMesh;
class StaticMesh;

class ResourceManager
{
public:
    explicit ResourceManager(Engine* engine);
    ~ResourceManager();

    std::shared_ptr<Shader> cachedShader(const ShaderCode* code);
    std::shared_ptr<Material> cachedMaterial(const MaterialData* data);
    std::shared_ptr<Texture> cachedTexture(const TextureData* data);
    std::shared_ptr<AnimatedMesh> cachedAnimatedMesh(const MeshData* data);
    std::shared_ptr<StaticMesh> cachedStaticMesh(const MeshData* data);

private:
    Engine* mEngine;
    std::unordered_map<const ShaderCode*, std::weak_ptr<Shader>> mShaders;
    std::unordered_map<const MaterialData*, std::weak_ptr<Material>> mMaterials;
    std::unordered_map<const TextureData*, std::weak_ptr<Texture>> mTextures;
    std::unordered_map<const MeshData*, std::weak_ptr<AnimatedMesh>> mAnimatedMeshes;
    std::unordered_map<const MeshData*, std::weak_ptr<StaticMesh>> mStaticMeshes;
};
