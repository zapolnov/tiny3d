#pragma once
#include "Engine/Input/Key.h"
#include <unordered_set>

class Engine;

class InputManager
{
public:
    explicit InputManager(Engine* engine);
    ~InputManager();

    bool isKeyPressed(Key key) const;

    void injectKeyPress(Key key);
    void injectKeyRelease(Key key);

private:
    Engine* mEngine;
    std::unordered_set<Key> mPressedKeys;
};
