#include "ShaderProcessor.h"
#include "Util.h"

ShaderProcessor::ShaderProcessor(const ConfigFile& config)
    : mConfig(config)
{
    mHdr << "#pragma once\n";
    mHdr << "#include \"Engine/Renderer/ShaderCode.h\"\n";
    mHdr << std::endl;
    mHdr << "namespace Shaders\n";
    mHdr << "{\n";

    mCxx << "#include \"Shaders.h\"\n";
    mCxx << "#include \"Shaders_Metal.h\"\n";
    mCxx << std::endl;
    mCxx << "namespace Shaders\n";
    mCxx << "{\n";
    mCxx << std::endl;

    mHdrMetal << "#pragma once\n";
    mHdrMetal << std::endl;
    mHdrMetal << "namespace Shaders::Metal\n";
    mHdrMetal << "{\n";

    mCxxMetal << "#include \"Shaders_Metal.h\"\n";
    mCxxMetal << std::endl;
    mCxxMetal << "namespace Shaders::Metal\n";
    mCxxMetal << "{\n";
    mCxxMetal << std::endl;
}

ShaderProcessor::~ShaderProcessor()
{
}

bool ShaderProcessor::generate()
{
    mHdr << "}\n";
    mCxx << "}\n";
    mHdrMetal << "}\n";
    mCxxMetal << "}\n";

    if (!writeTextFile("Compiled/Shaders.cpp", std::move(mCxx)))
        return false;
    if (!writeTextFile("Compiled/Shaders.h", std::move(mHdr)))
        return false;

    if (!writeTextFile("Compiled/Shaders_Metal.cpp", std::move(mCxxMetal)))
        return false;
    if (!writeTextFile("Compiled/Shaders_Metal.h", std::move(mHdrMetal)))
        return false;

    return true;
}

bool ShaderProcessor::process(const ConfigFile::Shader& shader)
{
    mHdr << "    extern const ShaderCode " << shader.id << ";\n";

    mCxx << "    const ShaderCode " << shader.id << " = {\n";
  #ifdef __APPLE__
    mCxx << "        /* .metal = */ &Metal::" << shader.id << ",\n";
    mCxx << "        /* .metalSize = */ sizeof(Metal::" << shader.id << "),\n";
  #else
    mCxx << "        /* .metal = */ nullptr,\n";
    mCxx << "        /* .metalSize = */ 0,\n";
  #endif
    mCxx << "    };\n\n";

    if (!compileMetalShader(shader))
        return false;

    return true;
}

bool ShaderProcessor::compileMetalShader(const ConfigFile::Shader& shader)
{
  #ifdef __APPLE__
    // FIXME: code below needs better escaping

    std::stringstream ss;
    ss << "xcrun -sdk macosx metal \"" << shader.file << "\" -c -o \".Temp/" << shader.id << ".air\"";
    if (system(ss.str().c_str()) != 0) {
        fprintf(stderr, "Error compiling shader \"%s\".\n", shader.file.c_str());
        return false;
    }

    ss.str({});
    ss << "xcrun -sdk macosx metallib \".Temp/" << shader.id << ".air\" -o \".Temp/" << shader.id << ".metallib\"";
    if (system(ss.str().c_str()) != 0) {
        fprintf(stderr, "Error creating library for shader \"%s\".\n", shader.file.c_str());
        return false;
    }

    std::stringstream data;
    if (!loadBinaryFile(".Temp/" + shader.id + ".metallib", data))
        return false;

    std::string bytes = data.str();

    mHdrMetal << "    extern const unsigned char " << shader.id << "[" << bytes.length() << "];\n";

    mCxxMetal << "    const unsigned char " << shader.id << "[" << bytes.length() << "] = {\n";
    for (auto ch : bytes)
        mCxxMetal << "        " << unsigned(uint8_t(ch)) << ",\n";
    mCxxMetal << "    };\n\n";
  #endif

    return true;
}
