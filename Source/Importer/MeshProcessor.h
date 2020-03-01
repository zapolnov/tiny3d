#pragma once
#include "ConfigFile.h"
#include <vector>
#include <unordered_map>
#include <string>
#include <memory>
#include <sstream>

struct aiNode;
struct MeshBone;

class MeshProcessor
{
public:
    explicit MeshProcessor(const ConfigFile& config);
    ~MeshProcessor();

    bool generate();

    bool process(const ConfigFile::Mesh& mesh);

private:
    const ConfigFile& mConfig;
    std::stringstream mCxx;
    std::stringstream mHdr;
    std::unordered_map<std::string, size_t> mBoneMap;
    std::vector<MeshBone> mBoneList;
    std::vector<std::unique_ptr<std::string>> mBoneNames;

    void readBoneHierarchy(const aiNode* rootNode, size_t parentBoneIndex);
};
