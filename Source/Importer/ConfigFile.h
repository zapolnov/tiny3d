#pragma once
#include <functional>
#include <unordered_map>
#include <vector>
#include <string>

class TiXmlElement;

class ConfigFile
{
public:
    struct Level
    {
        std::string id;
        std::string file;

        static constexpr char Tag[] = "level";
        bool parse(const ConfigFile* config, const TiXmlElement* e);
    };

    struct Texture
    {
        std::string id;
        std::string file;

        static constexpr char Tag[] = "texture";
        bool parse(const ConfigFile* config, const TiXmlElement* e);
    };

    struct Material
    {
        std::string id;
        std::string shaderId;
        std::vector<std::string> textureIds;

        static constexpr char Tag[] = "material";
        bool parse(const ConfigFile* config, const TiXmlElement* e);
    };

    struct Mesh
    {
        std::string id;
        std::string file;
        std::unordered_map<std::string, std::string> materialMapping;

        static constexpr char Tag[] = "mesh";
        bool parse(const ConfigFile* config, const TiXmlElement* e);
    };

    struct Shader
    {
        std::string id;
        std::string file;

        static constexpr char Tag[] = "shader";
        bool parse(const ConfigFile* config, const TiXmlElement* e);
    };

    ConfigFile();
    ~ConfigFile();

    const std::vector<Level>& levels() const { return mLevels.list; }
    const std::vector<Texture>& textures() const { return mTextures.list; }
    const std::vector<Material>& materials() const { return mMaterials.list; }
    const std::vector<Mesh>& meshes() const { return mMeshes.list; }
    const std::vector<Shader>& shaders() const { return mShaders.list; }

    const Level* levelWithId(const std::string& id) const;
    const Texture* textureWithId(const std::string& id) const;
    const Material* materialWithId(const std::string& id) const;
    const Mesh* meshWithId(const std::string& id) const;
    const Shader* shaderWithId(const std::string& id) const;

    bool load(const std::string& file);

private:
    template <typename T> struct Collection
    {
        std::vector<T> list;
        std::unordered_map<std::string, size_t> map;

        const T* find(const std::string& id) const;
        bool parseReference(const TiXmlElement* e, std::string& outId) const;
        bool parse(const ConfigFile* config, const TiXmlElement* parent);
    };

    Collection<Level> mLevels;
    Collection<Texture> mTextures;
    Collection<Material> mMaterials;
    Collection<Mesh> mMeshes;
    Collection<Shader> mShaders;
};
