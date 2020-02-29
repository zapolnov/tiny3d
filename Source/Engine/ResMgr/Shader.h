#pragma once
#include "Engine/Core/Engine.h"
#include "Engine/Renderer/IShaderProgram.h"
#include <memory>

class Shader
{
public:
    Shader(Engine* engine, std::unique_ptr<IShaderProgram>&& shader)
        : mEngine(engine)
        , mInstance(std::move(shader))
    {
    }

    const std::unique_ptr<IShaderProgram>& instance() { return mInstance; }

private:
    Engine* mEngine;
    std::unique_ptr<IShaderProgram> mInstance;
};
