#include "MaterialProcessor.h"
#include "Util.h"
#include <stb_image.h>

MaterialProcessor::MaterialProcessor(const ConfigFile& config)
    : mConfig(config)
{
    mHdr << "#pragma once\n";
    mHdr << "#include \"Engine/Mesh/MaterialData.h\"\n";
    mHdr << std::endl;
    mHdr << "namespace Materials\n";
    mHdr << "{\n";

    mCxx << "#include \"Materials.h\"\n";
    mCxx << "#include \"Shaders.h\"\n";
    mCxx << "#include \"Textures.h\"\n";
    mCxx << std::endl;
    mCxx << "namespace Materials\n";
    mCxx << "{\n";
    mCxx << std::endl;
}

MaterialProcessor::~MaterialProcessor()
{
}

bool MaterialProcessor::process(const ConfigFile::Material& material)
{
    mHdr << "    extern const MaterialData " << material.id << ";\n";

    mCxx << "    const TextureData* const " << material.id << "Textures[] = {\n";
    for (const auto& textureId : material.textureIds)
        mCxx << "        &Textures::" << textureId << ",\n";
    mCxx << "    };\n\n";

    mCxx << "    const MaterialData " << material.id << " = {\n";
    mCxx << "        /* .textureCount = */ " << material.textureIds.size() <<  ",\n";
    mCxx << "        /* .textures = */ " << material.id << "Textures,\n";
    mCxx << "        /* .shader = */ &Shaders::" << material.shaderId << ",\n";
    mCxx << "    };\n\n";

    return true;
}

bool MaterialProcessor::generate()
{
    mHdr << "}\n";
    mCxx << "}\n";

    if (!writeTextFile("Compiled/Materials.cpp", std::move(mCxx)))
        return false;
    if (!writeTextFile("Compiled/Materials.h", std::move(mHdr)))
        return false;

    return true;
}
