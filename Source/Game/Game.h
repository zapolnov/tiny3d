#pragma once
#include "Engine/Core/IGame.h"
#include "Engine/Math/PerspectiveCamera.h"
#include <memory>

class Engine;
class Level;
class AnimatedMesh;
struct LevelData;

class Game : public IGame
{
public:
    explicit Game(Engine* engine);
    ~Game();

    void update(float frameTime);
    void render();

private:
    Engine* mEngine;
    PerspectiveCamera mCamera;
    std::unique_ptr<Level> mLevel;
    std::shared_ptr<AnimatedMesh> mPlayerMesh;
    glm::vec3 mPlayerPos;

    void loadLevel(const LevelData* level);
};
