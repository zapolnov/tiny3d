#include "ConfigFile.h"
#include "LevelProcessor.h"

int main(int argc, char** argv)
{
    ConfigFile config;
    if (!config.load("assets.xml"))
        return 1;

    LevelProcessor levels;
    for (const auto& level : config.levels()) {
        printf("Compiling level \"%s\"...\n", level.file.c_str());
        if (!levels.process(config, level))
            return 1;
    }

    levels.generate();

    return 0;
}
