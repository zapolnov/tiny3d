#pragma once
#include "Engine/Core/IGame.h"
#include <memory>

class Engine;
class Level;

class Game : public IGame
{
public:
    explicit Game(Engine* engine);
    ~Game();

private:
    Engine* mEngine;
    std::unique_ptr<Level> mLevel;
};
