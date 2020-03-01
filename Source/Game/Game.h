#pragma once
#include "Engine/Core/IGame.h"
#include "Engine/Math/PerspectiveCamera.h"
#include <memory>

class Engine;
class Level;
class AnimatedMesh;

class Game : public IGame
{
public:
    explicit Game(Engine* engine);
    ~Game();

    void render();

private:
    Engine* mEngine;
    PerspectiveCamera mCamera;
    std::unique_ptr<Level> mLevel;
    std::shared_ptr<AnimatedMesh> mPlayerMesh;
};
