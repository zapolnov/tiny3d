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

    std::vector<Level> levels;

    TiXmlElement* root = xml.RootElement();
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

    mLevels = std::move(levels);

    return true;
}
