#pragma once
#include "Engine/Core/Engine.h"
#include "Engine/Renderer/ITexture.h"
#include <memory>

class Texture
{
public:
    Texture(Engine* engine, std::unique_ptr<ITexture>&& texture)
        : mEngine(engine)
        , mInstance(std::move(texture))
    {
    }

    const std::unique_ptr<ITexture>& instance() { return mInstance; }

private:
    Engine* mEngine;
    std::unique_ptr<ITexture> mInstance;
};
