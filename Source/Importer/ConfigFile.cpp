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

    float optionalFloatAttribute(const TiXmlElement* e, const char* name, float defaultValue)
    {
        const char* value = e->Attribute(name);
        if (value)
            return strtod(value, nullptr);
        return defaultValue;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ConfigFile::Level::parse(ConfigFile* config, const TiXmlElement* e)
{
    return mandatoryAttribute(e, "file", file);
}

bool ConfigFile::Texture::parse(ConfigFile* config, const TiXmlElement* e)
{
    return mandatoryAttribute(e, "file", file);
}

bool ConfigFile::Material::parse(ConfigFile* config, const TiXmlElement* e)
{
    const char* fmt = e->Attribute("vertex");
    vertexFormat = (fmt ? fmt : "MeshVertex");

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

bool ConfigFile::Mesh::parse(ConfigFile* config, const TiXmlElement* e)
{
    if (!mandatoryAttribute(e, "file", file))
        return false;

    const char* skeleton = e->Attribute("loadSkeleton");
    loadSkeleton = (skeleton ? strcmp(skeleton, "true") == 0 : false);

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

    tag = "animations";
    for (const TiXmlElement* ee = e->FirstChildElement(tag); ee; ee = ee->NextSiblingElement(tag)) {
        MeshAnimations anim;
        anim.file = file;

        const char* customFile = ee->Attribute("file");
        if (customFile)
            anim.file = customFile;

        const char* innerTag = "ignore";
        for (const TiXmlElement* ignoreE = ee->FirstChildElement(innerTag); ignoreE; ignoreE = ignoreE->NextSiblingElement(innerTag)) {
            std::string id;
            if (!mandatoryAttribute(ignoreE, "id", id))
                return false;
            anim.ignore.emplace(std::move(id));
        }

        innerTag = "use";
        for (const TiXmlElement* useE = ee->FirstChildElement(innerTag); useE; useE = useE->NextSiblingElement(innerTag)) {
            std::string newId;
            if (!mandatoryAttribute(useE, "id", newId))
                return false;

            std::string oldId;
            if (!mandatoryAttribute(useE, "forId", oldId))
                return false;

            if (anim.rename.find(oldId) != anim.rename.end()) {
                fprintf(stderr, "Duplicate mapping for animation id \"%s\".\n", oldId.c_str());
                return false;
            }

            anim.rename[oldId] = std::move(newId);
        }

        animations.emplace_back(std::move(anim));
    }

    rotate = glm::vec3(0.0f);
    translate = glm::vec3(0.0f);
    scale = glm::vec3(1.0f);

    const TiXmlElement* ee = e->FirstChildElement("useInLevel");
    if (ee) {
        std::string asChar;
        if (!mandatoryAttribute(ee, "asChar", asChar))
            return false;
        if (asChar.length() != 1) {
            fprintf(stderr, "Invalid value of the \"asChar\" attribute.\n");
            return false;
        }

        if (config->mLevelMeshes.find(asChar[0]) != config->mLevelMeshes.end()) {
            fprintf(stderr, "Duplicate mesh binding for level character \"%c\".\n", asChar[0]);
            return false;
        }
        config->mLevelMeshes[asChar[0]] = id;

        rotate.x = optionalFloatAttribute(ee, "rX", 0.0f) * 3.1415f / 180.0f;
        rotate.y = optionalFloatAttribute(ee, "rY", 0.0f) * 3.1415f / 180.0f;
        rotate.z = optionalFloatAttribute(ee, "rZ", 0.0f) * 3.1415f / 180.0f;
        translate.x = optionalFloatAttribute(ee, "tX", 0.0f);
        translate.y = optionalFloatAttribute(ee, "tY", 0.0f);
        translate.z = optionalFloatAttribute(ee, "tZ", 0.0f);
        scale.x = optionalFloatAttribute(ee, "sX", 1.0f);
        scale.y = optionalFloatAttribute(ee, "sY", 1.0f);
        scale.z = optionalFloatAttribute(ee, "sZ", 1.0f);
    }

    return true;
}

bool ConfigFile::Shader::parse(ConfigFile* config, const TiXmlElement* e)
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

template <typename T> bool ConfigFile::Collection<T>::parse(ConfigFile* config, const TiXmlElement* parent)
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

const ConfigFile::Mesh* ConfigFile::meshForLevelChar(char ch) const
{
    auto it = mLevelMeshes.find(ch);
    if (it == mLevelMeshes.end())
        return nullptr;
    return meshWithId(it->second);
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
