#pragma once
#include "ConfigFile.h"
#include "Engine/Mesh/MeshData.h"
#include <vector>
#include <map>
#include <unordered_map>
#include <string>
#include <memory>
#include <sstream>

struct aiNode;

class MeshProcessor
{
public:
    explicit MeshProcessor(const ConfigFile& config);
    ~MeshProcessor();

    bool generate();

    bool process(const ConfigFile::Mesh& mesh);

private:
    struct BoneAnim
    {
        std::string name;
        std::vector<MeshPositionKey> positionKeys;
        std::vector<MeshRotationKey> rotationKeys;
        std::vector<MeshScaleKey> scaleKeys;
    };

    struct Anim
    {
        MeshAnimation info;
        std::vector<BoneAnim> bones;
    };

    const ConfigFile& mConfig;
    std::stringstream mCxx;
    std::stringstream mHdr;
    std::stringstream mAnimCxx;
    std::stringstream mAnimHdr;
    std::unordered_map<std::string, size_t> mBoneMap;
    std::vector<MeshBone> mBoneList;
    std::vector<std::unique_ptr<std::string>> mBoneNames;
    std::map<std::string, Anim> mAnimations;

    void readBoneHierarchy(const aiNode* rootNode, size_t parentBoneIndex);

    bool loadAnimations(const ConfigFile::MeshAnimations& anim);
};
