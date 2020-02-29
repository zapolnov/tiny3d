#include "ConfigFile.h"
#include <stdio.h>
#include <tinyxml.h>

namespace
{
    const TiXmlElement* mandatoryElement(const TiXmlElement* parent, const char* name)
    {
        const TiXmlElement* e = parent->FirstChildElement(name);
        if (!e) {
            fprintf(stderr, "Missing mandatory element \"%s\" in element \"%s\".\n", name, parent->Value());
            return nullptr;
        }
        return e;
    }

    bool mandatoryAttribute(const TiXmlElement* e, const char* name, std::string& outValue)
    {
        const char* value = e->Attribute(name);
        if (!value || *value == 0) {
            fprintf(stderr, "Missing mandatory attribute \"%s\" in element \"%s\".\n", name, e->Value());
            return false;
        }
        outValue = value;
        return true;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ConfigFile::Level::parse(const ConfigFile* config, const TiXmlElement* e)
{
    return mandatoryAttribute(e, "file", file);
}

bool ConfigFile::Texture::parse(const ConfigFile* config, const TiXmlElement* e)
{
    return mandatoryAttribute(e, "file", file);
}

bool ConfigFile::Material::parse(const ConfigFile* config, const TiXmlElement* e)
{
    const TiXmlElement* ee = mandatoryElement(e, "useShader");
    if (!ee || !config->mShaders.parseReference(ee, shaderId))
        return false;

    const char* tag = "useTexture";
    for (ee = e->FirstChildElement(tag); ee; ee = ee->NextSiblingElement(tag)) {
        std::string textureId;
        if (!config->mTextures.parseReference(ee, textureId))
            return false;
        textureIds.emplace_back(std::move(textureId));
    }

    return true;
}

bool ConfigFile::Mesh::parse(const ConfigFile* config, const TiXmlElement* e)
{
    if (!mandatoryAttribute(e, "file", file))
        return false;

    const char* tag = "useMaterial";
    for (const TiXmlElement* ee = e->FirstChildElement(tag); ee; ee = ee->NextSiblingElement(tag)) {
        std::string newMaterialId;
        if (!config->mMaterials.parseReference(ee, newMaterialId))
            return false;

        std::string oldMaterialId;
        if (!mandatoryAttribute(ee, "forId", oldMaterialId))
            return false;

        if (materialMapping.find(oldMaterialId) != materialMapping.end()) {
            fprintf(stderr, "Duplicate mapping for material id \"%s\".\n", oldMaterialId.c_str());
            return false;
        }

        materialMapping[oldMaterialId] = std::move(newMaterialId);
    }

    return true;
}

bool ConfigFile::Shader::parse(const ConfigFile* config, const TiXmlElement* e)
{
    return mandatoryAttribute(e, "file", file);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T> const T* ConfigFile::Collection<T>::find(const std::string& id) const
{
    auto it = map.find(id);
    if (it == map.end()) {
        fprintf(stderr, "No %s with id \"%s\".\n", T::Tag, id.c_str());
        return nullptr;
    }
    return &list[it->second];
}

template <typename T> bool ConfigFile::Collection<T>::parseReference(const TiXmlElement* e, std::string& outId) const
{
    if (!mandatoryAttribute(e, "id", outId))
        return false;

    return (find(outId) != nullptr);
}

template <typename T> bool ConfigFile::Collection<T>::parse(const ConfigFile* config, const TiXmlElement* parent)
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
        if (!value.parse(config, e))
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
    if (!mShaders.parse(this, root))
        return false;
    if (!mTextures.parse(this, root))
        return false;
    if (!mMaterials.parse(this, root))
        return false;
    if (!mMeshes.parse(this, root))
        return false;
    if (!mLevels.parse(this, root))
        return false;

    return true;
}
