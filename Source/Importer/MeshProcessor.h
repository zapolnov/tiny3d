#pragma once
#include "ConfigFile.h"
#include <sstream>

class MeshProcessor
{
public:
    explicit MeshProcessor(const ConfigFile& config);
    ~MeshProcessor();

    bool process(const ConfigFile::Mesh& mesh);

    bool generate();

private:
    const ConfigFile& mConfig;
    std::stringstream mCxx;
    std::stringstream mHdr;
};
