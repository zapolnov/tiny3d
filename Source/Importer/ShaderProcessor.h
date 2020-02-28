#pragma once
#include "ConfigFile.h"
#include <sstream>

class ShaderProcessor
{
public:
    explicit ShaderProcessor(const ConfigFile& config);
    ~ShaderProcessor();

    bool generate();

    bool process(const ConfigFile::Shader& shader);

private:
    const ConfigFile& mConfig;
    std::stringstream mCxx;
    std::stringstream mHdr;
    std::stringstream mCxxMetal;
    std::stringstream mHdrMetal;

    bool compileMetalShader(const ConfigFile::Shader& shader);
};
