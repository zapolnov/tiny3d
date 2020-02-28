#pragma once
#include <vector>
#include <string>

class ConfigFile
{
public:
    struct Level
    {
        std::string id;
        std::string file;
    };

    struct Shader
    {
        std::string id;
        std::string file;
    };

    ConfigFile();
    ~ConfigFile();

    const std::vector<Level>& levels() const { return mLevels; }
    const std::vector<Shader>& shaders() const { return mShaders; }

    bool load(const std::string& file);

private:
    std::vector<Level> mLevels;
    std::vector<Shader> mShaders;
};
