#pragma once
#include <vector>
#include <memory>

struct MaterialData;
class Engine;
class IPipelineState;
class Shader;
class Texture;

class Material
{
public:
    Material(Engine* engine, const MaterialData* data);
    ~Material();

    void bind() const;

private:
    Engine* mEngine;
    std::unique_ptr<IPipelineState> mPipelineState;
    std::shared_ptr<Shader> mShader;
    std::vector<std::shared_ptr<Texture>> mTextures;
};
