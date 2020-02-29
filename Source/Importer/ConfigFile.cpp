#include "ConfigFile.h"
#include <stdio.h>
#include <tinyxml.h>

namespace
{
    bool mandatoryAttribute(const TiXmlElement* e, const char* name, std::string& outValue)
    {
        const char* value = e->Attribute(name);
        if (!value || *value == 0) {
            fprintf(stderr, "Unable to parse config file: missing mandatory attribute \"%s\" in element \"%s\".\n",
                name, e->Value());
            return false;
        }
        outValue = value;
        return true;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ConfigFile::Level::parse(const TiXmlElement* e)
{
    return mandatoryAttribute(e, "file", file);
}

bool ConfigFile::Texture::parse(const TiXmlElement* e)
{
    return mandatoryAttribute(e, "file", file);
}

bool ConfigFile::Material::parse(const TiXmlElement* e)
{
    return true;
}

bool ConfigFile::Mesh::parse(const TiXmlElement* e)
{
    return mandatoryAttribute(e, "file", file);
}

bool ConfigFile::Shader::parse(const TiXmlElement* e)
{
    return mandatoryAttribute(e, "file", file);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T> const T* ConfigFile::Collection<T>::find(const std::string& id) const
{
    auto it = map.find(id);
    if (it == map.end()) {
        fprintf(stderr, "No %s with id \"%s\".", T::Tag, id.c_str());
        return nullptr;
    }
    return &list[it->second];
}

template <typename T> bool ConfigFile::Collection<T>::parse(const TiXmlElement* parent)
{
    for (const TiXmlElement* e = parent->FirstChildElement(T::Tag); e; e = e->NextSiblingElement(T::Tag)) {
        std::string id;
        if (!mandatoryAttribute(e, "id", id))
            return false;

        if (map.find(id) != map.end()) {
            fprintf(stderr, "Unable to parse config file: duplicate %s \"%s\".\n", T::Tag, id.c_str());
            return false;
        }

        T value;
        value.id = id;
        if (!value.parse(e))
            return false;

        map[id] = list.size();
        list.emplace_back(std::move(value));
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ConfigFile::ConfigFile()
{
}

ConfigFile::~ConfigFile()
{
}

const ConfigFile::Level* ConfigFile::levelWithId(const std::string& id) const { return mLevels.find(id); }
const ConfigFile::Texture* ConfigFile::textureWithId(const std::string& id) const { return mTextures.find(id); }
const ConfigFile::Material* ConfigFile::materialWithId(const std::string& id) const { return mMaterials.find(id); }
const ConfigFile::Mesh* ConfigFile::meshWithId(const std::string& id) const { return mMeshes.find(id); }
const ConfigFile::Shader* ConfigFile::shaderWithId(const std::string& id) const { return mShaders.find(id); }

bool ConfigFile::load(const std::string& file)
{
    TiXmlDocument xml;
    if (!xml.LoadFile(file.c_str())) {
        fprintf(stderr, "Unable to parse config file: at line %d, column %d: %s\n",
            xml.ErrorRow(), xml.ErrorCol(), xml.ErrorDesc());
        return false;
    }

    TiXmlElement* root = xml.RootElement();
    if (!mLevels.parse(root))
        return false;
    if (!mTextures.parse(root))
        return false;
    if (!mMaterials.parse(root))
        return false;
    if (!mMeshes.parse(root))
        return false;
    if (!mShaders.parse(root))
        return false;

    return true;
}
