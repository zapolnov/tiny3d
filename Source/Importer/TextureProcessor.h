#pragma once
#include "ConfigFile.h"
#include <sstream>

class TextureProcessor
{
public:
    explicit TextureProcessor(const ConfigFile& config);
    ~TextureProcessor();

    bool process(const ConfigFile::Texture& texture);

    bool generate();

private:
    const ConfigFile& mConfig;
    std::stringstream mCxx;
    std::stringstream mHdr;
};
