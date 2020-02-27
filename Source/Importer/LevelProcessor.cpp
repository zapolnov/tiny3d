#include "LevelProcessor.h"
#include "Game/Level.h"
#include "Util.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sstream>

LevelProcessor::LevelProcessor()
{
    mHdr << "#pragma once\n";
    mHdr << "#include \"Game/Level.h\"\n";
    mHdr << std::endl;

    mCxx << "#include \"Levels.h\"\n";
    mCxx << std::endl;
}

LevelProcessor::~LevelProcessor()
{
}

bool LevelProcessor::process(const ConfigFile& config, const ConfigFile::Level& level)
{
    int playerStartX = -1, playerStartY = -1;

    FILE* f = fopen(level.file.c_str(), "r");
    if (!f) {
        fprintf(stderr, "Can't open file \"%s\": %s\n", level.file.c_str(), strerror(errno));
        return false;
    }

    mHdr << "extern const Level " << level.id << ";\n";
    mCxx << "const Level " << level.id << " = {\n";
    mCxx << "    /* .walkable = */ {\n";

    char line[1024];
    int h = 0;
    while (fgets(line, sizeof(line), f)) {
        char* p = strrchr(line, '\n');
        if (p)
            *p = 0;

        size_t len = strlen(line);
        if (len != Level::Width) {
            fprintf(stderr, "Error in file \"%s\": invalid line width.\n", level.file.c_str());
            fclose(f);
            return false;
        }

        mCxx << "        ";
        for (int i = 0; i < Level::Width; i++) {
            switch (line[i]) {
                case '#':
                    mCxx << "false,";
                    break;

                case ' ':
                    mCxx << " true,";
                    break;

                case '*':
                    mCxx << " true,";
                    if (playerStartX >= 0) {
                        fprintf(stderr, "Error in file \"%s\": multiple player start positions.\n", level.file.c_str());
                        fclose(f);
                        return false;
                    }
                    playerStartX = i;
                    playerStartY = h;
                    break;

                default:
                    fprintf(stderr, "Error in file \"%s\": unknown character '%c'.\n", level.file.c_str(), line[i]);
                    fclose(f);
                    return false;
            }
        }

        mCxx << std::endl;
        ++h;
    }

    fclose(f);

    if (playerStartX < 0) {
        fprintf(stderr, "Error in file \"%s\": missing player start position.\n", level.file.c_str());
        return false;
    }
    if (h != Level::Height) {
        fprintf(stderr, "Error in file \"%s\": invalid height.\n", level.file.c_str());
        return false;
    }

    mCxx << "        },\n";
    mCxx << "    };\n";
    return true;
}

bool LevelProcessor::generate()
{
    if (!writeFile("Compiled/Levels.cpp", std::move(mCxx)))
        return false;
    if (!writeFile("Compiled/Levels.h", std::move(mHdr)))
        return false;
    return true;
}
