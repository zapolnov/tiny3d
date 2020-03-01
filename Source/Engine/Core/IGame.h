#pragma once

class IGame
{
public:
    virtual ~IGame() = default;

    virtual void update(float frameTime) = 0;
    virtual void render() = 0;
};
