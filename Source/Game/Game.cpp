#include "Game.h"
#include "Resources/Compiled/Levels.h"

Game::Game(Engine* engine)
    : mEngine(engine)
{
    mLevel = std::make_unique<Level>(mEngine, &level1);
}

Game::~Game()
{
}

void Game::render()
{
    mLevel->render();
}
