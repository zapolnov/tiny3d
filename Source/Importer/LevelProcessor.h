#pragma once
#include "ConfigFile.h"
#include <sstream>

class LevelProcessor
{
public:
    LevelProcessor();
    ~LevelProcessor();

    bool process(const ConfigFile& config, const ConfigFile::Level& level);

    bool generate();

private:
    std::stringstream mCxx;
    std::stringstream mHdr;
};
