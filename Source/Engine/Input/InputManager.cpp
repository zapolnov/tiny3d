#include "InputManager.h"

InputManager::InputManager(Engine* engine)
    : mEngine(engine)
{
}

InputManager::~InputManager()
{
}

bool InputManager::isKeyPressed(Key key) const
{
    return mPressedKeys.find(key) != mPressedKeys.end();
}

void InputManager::injectKeyPress(Key key)
{
    if (key != KeyNone)
        mPressedKeys.emplace(key);
}

void InputManager::injectKeyRelease(Key key)
{
    if (key != KeyNone)
        mPressedKeys.erase(key);
}
