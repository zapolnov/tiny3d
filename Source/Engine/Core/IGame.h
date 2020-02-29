#pragma once

class IGame
{
public:
    virtual ~IGame() = default;
    virtual void render() = 0;
};
