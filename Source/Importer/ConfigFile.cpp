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

    std::vector<Texture> textures;
    for (const TiXmlElement* e = root->FirstChildElement("texture"); e; e = e->NextSiblingElement("texture")) {
        const char* id = e->Attribute("id");
        const char* file = e->Attribute("file");
        if (!id || !file) {
            fprintf(stderr, "Unable to parse config file: missing mandatory attribute in element \"texture\".\n");
            return false;
        }

        Texture texture;
        texture.id = id;
        texture.file = file;
        textures.emplace_back(std::move(texture));
    }

    std::vector<Mesh> meshes;
    for (const TiXmlElement* e = root->FirstChildElement("mesh"); e; e = e->NextSiblingElement("mesh")) {
        const char* id = e->Attribute("id");
        const char* file = e->Attribute("file");
        if (!id || !file) {
            fprintf(stderr, "Unable to parse config file: missing mandatory attribute in element \"mesh\".\n");
            return false;
        }

        Mesh mesh;
        mesh.id = id;
        mesh.file = file;
        meshes.emplace_back(std::move(mesh));
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
    mTextures = std::move(textures);
    mMeshes = std::move(meshes);
    mShaders = std::move(shaders);

    return true;
}
