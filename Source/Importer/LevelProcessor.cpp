#include "LevelProcessor.h"
#include "LevelMeshBuilder.h"
#include "Util.h"
#include "Game/Level.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sstream>

LevelProcessor::LevelProcessor(const ConfigFile& config)
    : mConfig(config)
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

bool LevelProcessor::process(const ConfigFile::Level& level)
{
    int playerStartX = -1, playerStartY = -1;

    FILE* f = fopen(level.file.c_str(), "r");
    if (!f) {
        fprintf(stderr, "Can't open file \"%s\": %s\n", level.file.c_str(), strerror(errno));
        return false;
    }

    LevelMeshBuilder meshBuilder;

    mHdr << "extern const LevelData " << level.id << ";\n";
    mHdr << "extern const LevelVertex " << level.id << "Vertices[];\n";
    mHdr << "extern const uint16_t " << level.id << "Indices[];\n";

    mCxx << "const LevelData " << level.id << " = {\n";
    mCxx << "    /* .walkable = */ {\n";

    char line[1024];
    int y = 0;
    while (fgets(line, sizeof(line), f)) {
        char* p = strrchr(line, '\n');
        if (p)
            *p = 0;

        size_t len = strlen(line);
        if (len != LevelWidth) {
            fprintf(stderr, "Error in file \"%s\": invalid line width.\n", level.file.c_str());
            fclose(f);
            return false;
        }

        mCxx << "        ";
        for (int x = 0; x < LevelWidth; x++) {
            switch (line[x]) {
                case '#':
                    mCxx << "false,";
                    meshBuilder.createWall(x, y);
                    break;

                case ' ':
                    mCxx << " true,";
                    meshBuilder.createFloor(x, y);
                    break;

                case '*':
                    mCxx << " true,";
                    meshBuilder.createFloor(x, y);
                    if (playerStartX >= 0) {
                        fprintf(stderr, "Error in file \"%s\": multiple player start positions.\n", level.file.c_str());
                        fclose(f);
                        return false;
                    }
                    playerStartX = x;
                    playerStartY = y;
                    break;

                default:
                    fprintf(stderr, "Error in file \"%s\": unknown character '%c'.\n", level.file.c_str(), line[x]);
                    fclose(f);
                    return false;
            }
        }

        mCxx << std::endl;
        ++y;
    }

    fclose(f);

    if (playerStartX < 0) {
        fprintf(stderr, "Error in file \"%s\": missing player start position.\n", level.file.c_str());
        return false;
    }
    if (y != LevelHeight) {
        fprintf(stderr, "Error in file \"%s\": invalid height.\n", level.file.c_str());
        return false;
    }

    mCxx << "        },\n";
    mCxx << "        /* .playerX = */ " << playerStartX << ",\n";
    mCxx << "        /* .playerY = */ " << playerStartY << ",\n";
    mCxx << "        /* .vertices = */ " << level.id << "Vertices,\n";
    mCxx << "        /* .indices = */ " << level.id << "Indices,\n";
    mCxx << "        /* .vertexCount = */ " << meshBuilder.vertexCount() << ",\n";
    mCxx << "        /* .indexCount = */ " << meshBuilder.indexCount() << ",\n";
    mCxx << "    };\n\n";

    meshBuilder.generateCxxCode(level.id, mCxx);

    return true;
}

bool LevelProcessor::generate()
{
    if (!writeTextFile("Compiled/Levels.cpp", std::move(mCxx)))
        return false;
    if (!writeTextFile("Compiled/Levels.h", std::move(mHdr)))
        return false;
    return true;
}