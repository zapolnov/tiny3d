#include "ConfigFile.h"
#include <stdio.h>
#include <tinyxml.h>

ConfigFile::ConfigFile()
{
}

ConfigFile::~ConfigFile()
{
}

bool ConfigFile::load(const std::string& file)
{
    TiXmlDocument xml;
    if (!xml.LoadFile(file.c_str())) {
        fprintf(stderr, "Unable to parse config file: at line %d, column %d: %s\n",
            xml.ErrorRow(), xml.ErrorCol(), xml.ErrorDesc());
        return false;
    }

    TiXmlElement* root = xml.RootElement();

    std::vector<Level> levels;
    for (const TiXmlElement* e = root->FirstChildElement("level"); e; e = e->NextSiblingElement("level")) {
        const char* id = e->Attribute("id");
        const char* file = e->Attribute("file");
        if (!id || !file) {
            fprintf(stderr, "Unable to parse config file: missing mandatory attribute in element \"level\".\n");
            return false;
        }

        Level level;
        level.id = id;
        level.file = file;
        levels.emplace_back(std::move(level));
    }

    std::vector<Shader> shaders;
    for (const TiXmlElement* e = root->FirstChildElement("shader"); e; e = e->NextSiblingElement("shader")) {
        const char* id = e->Attribute("id");
        const char* file = e->Attribute("file");
        if (!id || !file) {
            fprintf(stderr, "Unable to parse config file: missing mandatory attribute in element \"shader\".\n");
            return false;
        }

        Shader shader;
        shader.id = id;
        shader.file = file;
        shaders.emplace_back(std::move(shader));
    }

    mLevels = std::move(levels);
    mShaders = std::move(shaders);

    return true;
}
