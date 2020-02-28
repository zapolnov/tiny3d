#pragma once
#include "ConfigFile.h"
#include <sstream>

class LevelProcessor
{
public:
    explicit LevelProcessor(const ConfigFile& config);
    ~LevelProcessor();

    bool process(const ConfigFile::Level& level);

    bool generate();

private:
    const ConfigFile& mConfig;
    std::stringstream mCxx;
    std::stringstream mHdr;
};
