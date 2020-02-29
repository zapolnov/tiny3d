#pragma once
#include <vector>
#include <memory>

struct MeshMaterial;
class Engine;
class IPipelineState;
class Shader;
class Texture;

class Material
{
public:
    Material(Engine* engine, const MeshMaterial* data);
    ~Material();

    void bind();

private:
    Engine* mEngine;
    std::unique_ptr<IPipelineState> mPipelineState;
    std::shared_ptr<Shader> mShader;
    std::vector<std::shared_ptr<Texture>> mTextures;
};
