#include "ShaderProcessor.h"
#include "Util.h"
#include <StandAlone/ResourceLimits.h>
#include <glslang_c_interface.h>

ShaderProcessor::ShaderProcessor(const ConfigFile& config)
    : mConfig(config)
{
    mHdr << "#pragma once\n";
    mHdr << "#include \"Engine/Renderer/ShaderCode.h\"\n";
    mHdr << std::endl;
    mHdr << "namespace Shaders\n";
    mHdr << "{\n";

    mCxx << "#include \"Shaders.h\"\n";
    mCxx << "#include \"Shaders_Vulkan.h\"\n";
    mCxx << "#include \"Shaders_Metal.h\"\n";
    mCxx << std::endl;
    mCxx << "namespace Shaders\n";
    mCxx << "{\n";
    mCxx << std::endl;

    mHdrVulkan << "#pragma once\n";
    mHdrVulkan << std::endl;
    mHdrVulkan << "namespace Shaders::Vulkan\n";
    mHdrVulkan << "{\n";

    mCxxVulkan << "#include \"Shaders_Vulkan.h\"\n";
    mCxxVulkan << std::endl;
    mCxxVulkan << "namespace Shaders::Vulkan\n";
    mCxxVulkan << "{\n";
    mCxxVulkan << std::endl;

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
    mHdrVulkan << "}\n";
    mCxxVulkan << "}\n";
    mHdrMetal << "}\n";
    mCxxMetal << "}\n";

    if (!writeTextFile("Compiled/Shaders.cpp", std::move(mCxx)))
        return false;
    if (!writeTextFile("Compiled/Shaders.h", std::move(mHdr)))
        return false;

    if (!writeTextFile("Compiled/Shaders_Vulkan.cpp", std::move(mCxxVulkan)))
        return false;
    if (!writeTextFile("Compiled/Shaders_Vulkan.h", std::move(mHdrVulkan)))
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
    mCxx << "        /* .vulkanVertex = */ nullptr,\n";
    mCxx << "        /* .vulkanVertexSize = */ 0,\n";
    mCxx << "        /* .vulkanFragment = */ nullptr,\n";
    mCxx << "        /* .vulkanFragmentSize = */ 0,\n";
  #else
    mCxx << "        /* .metal = */ nullptr,\n";
    mCxx << "        /* .metalSize = */ 0,\n";
    mCxx << "        /* .vulkanVertex = */ &Vulkan::" << shader.id << "Vertex,\n";
    mCxx << "        /* .vulkanVertexSize = */ sizeof(Vulkan::" << shader.id << "Vertex),\n";
    mCxx << "        /* .vulkanFragment = */ &Vulkan::" << shader.id << "Fragment,\n";
    mCxx << "        /* .vulkanFragmentSize = */ sizeof(Vulkan::" << shader.id << "Fragment),\n";
  #endif
    mCxx << "    };\n\n";

    if (!compileMetalShader(shader))
        return false;
    if (!compileVulkanShader(shader))
        return false;

    return true;
}

bool ShaderProcessor::compileMetalShader(const ConfigFile::Shader& shader)
{
  #ifdef __APPLE__
    // FIXME: code below needs better escaping

    std::stringstream ss;
    ss << "xcrun -sdk macosx metal \"" << shader.file << ".metal\" -c -o \".Temp/" << shader.id << ".air\"";
    if (system(ss.str().c_str()) != 0) {
        fprintf(stderr, "Error compiling shader \"%s.metal\".\n", shader.file.c_str());
        return false;
    }

    ss.str({});
    ss << "xcrun -sdk macosx metallib \".Temp/" << shader.id << ".air\" -o \".Temp/" << shader.id << ".metallib\"";
    if (system(ss.str().c_str()) != 0) {
        fprintf(stderr, "Error creating library for shader \"%s.metal\".\n", shader.file.c_str());
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

#ifndef __APPLE__
static std::string extractShader(const std::string& bytes, const std::string& type)
{
    std::string header = "{{" + type + "}}";
    size_t indexStart = bytes.find(header);
    if (indexStart == std::string::npos) {
        fprintf(stderr, "{{%s}} not found\n", type.c_str());
        return std::string();
    }

    indexStart += header.length();
    size_t indexEnd = bytes.find("{{", indexStart);
    if (indexEnd != std::string::npos)
        return bytes.substr(indexStart, indexEnd - indexStart);

    return bytes.substr(indexStart);
}

static bool compileVulkan(glslang_stage_t stage, const std::string& bytes, std::string& outCompiled)
{
    glslang_input_t input = {};
    input.language = GLSLANG_SOURCE_GLSL;
    input.stage = stage;
    input.client = GLSLANG_CLIENT_VULKAN;
    input.client_version = GLSLANG_TARGET_VULKAN_1_1;
    input.target_language = GLSLANG_TARGET_SPV;
    input.target_language_version = GLSLANG_TARGET_SPV_1_3;
    input.code = bytes.c_str();
    input.default_version = 100;
    input.default_profile = GLSLANG_NO_PROFILE;
    input.force_default_version_and_profile = false;
    input.forward_compatible = false;
    input.messages = GLSLANG_MSG_DEFAULT_BIT;
    input.resource = &glslang::DefaultTBuiltInResource;

    glslang_shader_t* shader = glslang_shader_create(&input);

    if (!glslang_shader_preprocess(shader, &input)) {
        fprintf(stderr, "%s\n", glslang_shader_get_info_log(shader));
        glslang_shader_delete(shader);
        return false;
    }

    if (!glslang_shader_parse(shader, &input)) {
        fprintf(stderr, "%s\n", glslang_shader_get_info_log(shader));
        glslang_shader_delete(shader);
        return false;
    }

    glslang_program_t* program = glslang_program_create();
    glslang_program_add_shader(program, shader);

    if (!glslang_program_link(program, GLSLANG_MSG_SPV_RULES_BIT | GLSLANG_MSG_VULKAN_RULES_BIT)) {
        fprintf(stderr, "%s\n", glslang_program_get_info_log(program));
        glslang_program_delete(program);
        glslang_shader_delete(shader);
        return false;
    }

    glslang_program_SPIRV_generate(program, stage);

    if (glslang_program_SPIRV_get_messages(program))
    {
        fprintf(stderr, "%s", glslang_program_SPIRV_get_messages(program));
        glslang_program_delete(program);
        glslang_shader_delete(shader);
        return false;
    }

    size_t size = glslang_program_SPIRV_get_size(program) * sizeof(unsigned int);
    outCompiled.resize(size);
    memcpy(&outCompiled[0], glslang_program_SPIRV_get_ptr(program), size);

    glslang_program_delete(program);
    glslang_shader_delete(shader);

    return true;
}
#endif

bool ShaderProcessor::compileVulkanShader(const ConfigFile::Shader& shader)
{
  #ifndef __APPLE__
    std::stringstream data;
    if (!loadBinaryFile(shader.file + ".vulkan", data))
        return false;

    std::string bytes = data.str();

    glslang_initialize_process();

    std::string vertex;
    if (!compileVulkan(GLSLANG_STAGE_VERTEX, extractShader(bytes, "vertex"), vertex))
        return false;

    mHdrVulkan << "    extern const unsigned char " << shader.id << "Vertex[" << vertex.length() << "];\n";
    mCxxVulkan << "    const unsigned char " << shader.id << "Vertex[" << vertex.length() << "] = {\n";
    for (auto ch : vertex)
        mCxxVulkan << "        " << unsigned(uint8_t(ch)) << ",\n";
    mCxxVulkan << "    };\n\n";

    std::string fragment;
    if (!compileVulkan(GLSLANG_STAGE_FRAGMENT, extractShader(bytes, "fragment"), fragment))
        return false;

    mHdrVulkan << "    extern const unsigned char " << shader.id << "Fragment[" << fragment.length() << "];\n";
    mCxxVulkan << "    const unsigned char " << shader.id << "Fragment[" << fragment.length() << "] = {\n";
    for (auto ch : fragment)
        mCxxVulkan << "        " << unsigned(uint8_t(ch)) << ",\n";
    mCxxVulkan << "    };\n\n";
  #endif

    return true;
}
