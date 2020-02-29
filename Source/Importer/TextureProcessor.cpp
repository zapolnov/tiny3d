#include "TextureProcessor.h"
#include "Util.h"
#include <stb_image.h>

TextureProcessor::TextureProcessor(const ConfigFile& config)
    : mConfig(config)
{
    mHdr << "#pragma once\n";
    mHdr << "#include \"Engine/Renderer/TextureData.h\"\n";
    mHdr << std::endl;

    mCxx << "#include \"Textures.h\"\n";
}

TextureProcessor::~TextureProcessor()
{
}

bool TextureProcessor::process(const ConfigFile::Texture& texture)
{
    int w, h, n;
    unsigned char* data = stbi_load(texture.file.c_str(), &w, &h, &n, 3);
    if (!data) {
        fprintf(stderr, "Unable to load texture \"%s\".", texture.file.c_str());
        return false;
    }

    mHdr << "extern const TextureData " << texture.id << ";\n";

    mCxx << "\nstatic const unsigned char " << texture.id << "_pixels[] = {\n";
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++)
            mCxx << "    " << unsigned(data[y * w + x]) << ",\n";
    }
    mCxx << "};\n\n";
    mCxx << "const TextureData " << texture.id << " = {\n";
    mCxx << "    /* .pixels = */ " << texture.id <<  "_pixels,\n";
    mCxx << "    /* .width = */ " << w << ",\n";
    mCxx << "    /* .height = */ " << h << ",\n";
    mCxx << "};\n";

    stbi_image_free(data);

    return true;
}

bool TextureProcessor::generate()
{
    if (!writeTextFile("Compiled/Textures.cpp", std::move(mCxx)))
        return false;
    if (!writeTextFile("Compiled/Textures.h", std::move(mHdr)))
        return false;
    return true;
}