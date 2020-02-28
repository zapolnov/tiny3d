#include "ConfigFile.h"
#include "ShaderProcessor.h"
#include "LevelProcessor.h"

int main(int argc, char** argv)
{
    ConfigFile config;
    if (!config.load("assets.xml"))
        return 1;

    LevelProcessor levels(config);
    for (const auto& level : config.levels()) {
        fprintf(stderr, "Compiling level \"%s\"...\n", level.file.c_str());
        if (!levels.process(level))
            return 1;
    }

    ShaderProcessor shaders(config);
    for (const auto& shader : config.shaders()) {
        fprintf(stderr, "Compiling shader \"%s\"...\n", shader.file.c_str());
        if (!shaders.process(shader))
            return 1;
    }

    if (!levels.generate())
        return 1;
    if (!shaders.generate())
        return 1;

    return 0;
}
