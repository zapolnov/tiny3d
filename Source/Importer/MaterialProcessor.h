#pragma once
#include "ConfigFile.h"
#include <sstream>

class MaterialProcessor
{
public:
    explicit MaterialProcessor(const ConfigFile& config);
    ~MaterialProcessor();

    bool process(const ConfigFile::Material& material);

    bool generate();

private:
    const ConfigFile& mConfig;
    std::stringstream mCxx;
    std::stringstream mHdr;
};
