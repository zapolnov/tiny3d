#pragma once

struct Level
{
    static const int Width = 20;
    static const int Height = 20;

    bool walkable[Width * Height];
};
