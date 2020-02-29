#include "TextureProcessor.h"
#include "Util.h"
#include <stb_image.h>

TextureProcessor::TextureProcessor(const ConfigFile& config)
    : mConfig(config)
{
    mHdr << "#pragma once\n";
    mHdr << "#include \"Engine/Renderer/TextureData.h\"\n";
    mHdr << std::endl;
    mHdr << "namespace Textures\n";
    mHdr << "{\n";

    mCxx << "#include \"Textures.h\"\n";
    mCxx << std::endl;
    mCxx << "namespace Textures\n";
    mCxx << "{\n";
    mCxx << std::endl;
}

TextureProcessor::~TextureProcessor()
{
}

bool TextureProcessor::process(const ConfigFile::Texture& texture)
{
    int w, h, n;
    unsigned char* data = stbi_load(texture.file.c_str(), &w, &h, &n, 4);
    if (!data) {
        fprintf(stderr, "Unable to load texture \"%s\".", texture.file.c_str());
        return false;
    }

    mHdr << "    extern const TextureData " << texture.id << ";\n";

    mCxx << "    static const unsigned char " << texture.id << "Pixels[] = {\n";
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            mCxx << "        ";
            for (int i = 0; i < 4; i++)
                mCxx << unsigned(data[(y * w + x) * 4 + i]) << ',';
            mCxx << std::endl;
        }
    }
    mCxx << "    };\n\n";

    mCxx << "    const TextureData " << texture.id << " = {\n";
    mCxx << "        /* .pixels = */ " << texture.id <<  "Pixels,\n";
    mCxx << "        /* .width = */ " << w << ",\n";
    mCxx << "        /* .height = */ " << h << ",\n";
    mCxx << "    };\n\n";

    stbi_image_free(data);

    return true;
}

bool TextureProcessor::generate()
{
    mHdr << "}\n";
    mCxx << "}\n";

    if (!writeTextFile("Compiled/Textures.cpp", std::move(mCxx)))
        return false;
    if (!writeTextFile("Compiled/Textures.h", std::move(mHdr)))
        return false;

    return true;
}
