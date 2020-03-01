#include "MeshProcessor.h"
#include "Util.h"
#include <assimp/Importer.hpp>
#include <assimp/IOStream.hpp>
#include <assimp/IOSystem.hpp>
#include <assimp/DefaultLogger.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <stdio.h>

namespace
{
    std::once_flag initOnce;

    class AssimpLogStream : public Assimp::LogStream
    {
    public:
        static void init()
        {
            std::call_once(initOnce, []() {
                    Assimp::DefaultLogger::create(nullptr, Assimp::Logger::VERBOSE, 0, nullptr);
                    const unsigned int levels = Assimp::Logger::Err | Assimp::Logger::Warn;
                    Assimp::DefaultLogger::get()->attachStream(new AssimpLogStream, levels);
                });
        }

        void write(const char* message) override
        {
            fprintf(stderr, "%s", message);
        }
    };
}

MeshProcessor::MeshProcessor(const ConfigFile& config)
    : mConfig(config)
{
    AssimpLogStream::init();

    mHdr << "#pragma once\n";
    mHdr << "#include \"Engine/Mesh/MeshData.h\"\n";
    mHdr << std::endl;
    mHdr << "namespace Meshes\n";
    mHdr << "{\n";

    mCxx << "#include \"Meshes.h\"\n";
    mCxx << "#include \"Materials.h\"\n";
    mCxx << std::endl;
    mCxx << "namespace Meshes\n";
    mCxx << "{\n";
    mCxx << std::endl;

    mAnimHdr << "#pragma once\n";
    mAnimHdr << "#include \"Engine/Mesh/MeshData.h\"\n";
    mAnimHdr << std::endl;
    mAnimHdr << "namespace Animations\n";
    mAnimHdr << "{\n";

    mAnimCxx << "#include \"Animations.h\"\n";
    mAnimCxx << std::endl;
    mAnimCxx << "namespace Animations\n";
    mAnimCxx << "{\n";
    mAnimCxx << std::endl;
}

MeshProcessor::~MeshProcessor()
{
}

bool MeshProcessor::generate()
{
    mHdr << "}\n";
    mCxx << "}\n";
    mAnimHdr << "}\n";
    mAnimCxx << "}\n";

    if (!writeTextFile("Compiled/Meshes.cpp", std::move(mCxx)))
        return false;
    if (!writeTextFile("Compiled/Meshes.h", std::move(mHdr)))
        return false;

    if (!writeTextFile("Compiled/Animations.cpp", std::move(mAnimCxx)))
        return false;
    if (!writeTextFile("Compiled/Animations.h", std::move(mAnimHdr)))
        return false;

    return true;
}

bool MeshProcessor::process(const ConfigFile::Mesh& mesh)
{
    const aiScene* scene = nullptr;
    const unsigned flags =
        aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices |
        aiProcess_ImproveCacheLocality |
        aiProcess_RemoveRedundantMaterials |
        aiProcess_SortByPType |
        aiProcess_FindInvalidData |
        aiProcess_SplitLargeMeshes |
        aiProcess_OptimizeMeshes |
        (!mesh.loadSkeleton ? aiProcess_PreTransformVertices : 0) |
        ( mesh.loadSkeleton ? aiProcess_LimitBoneWeights : 0) |
        aiProcess_GenSmoothNormals |
        aiProcess_CalcTangentSpace |
        aiProcess_GenUVCoords |
        aiProcess_TransformUVCoords |
        aiProcess_FlipUVs |
        0;

    Assimp::Importer importer;
    importer.SetPropertyInteger(AI_CONFIG_PP_SLM_VERTEX_LIMIT, 65535);
    importer.SetPropertyInteger(AI_CONFIG_PP_SLM_TRIANGLE_LIMIT, 1000000000);
    importer.SetPropertyInteger(AI_CONFIG_PP_LBW_MAX_WEIGHTS, 4);
    scene = importer.ReadFile(mesh.file, flags);
    if (!scene) {
        fprintf(stderr, "Unable to load file \"%s\": %s\n", mesh.file.c_str(), importer.GetErrorString());
        return false;
    }

    std::vector<MeshVertex> vertices;
    std::vector<MeshSkinningVertex> skinningVertices;
    std::vector<MeshMaterial> materials;
    std::vector<std::string> materialIds;
    std::vector<uint16_t> indices;
    glm::mat4 globalInverseTransform;

    mHdr << "    extern const MeshData " << mesh.id << ";\n";

    mBoneList.clear();
    mBoneMap.clear();
    mBoneNames.clear();
    if (mesh.loadSkeleton) {
        aiMatrix4x4 globalInvTransform = scene->mRootNode->mTransformation;
        globalInvTransform.Inverse();
        globalInverseTransform = glm::mat4(
                globalInvTransform.a1, globalInvTransform.b1, globalInvTransform.c1, globalInvTransform.d1,
                globalInvTransform.a2, globalInvTransform.b2, globalInvTransform.c2, globalInvTransform.d2,
                globalInvTransform.a3, globalInvTransform.b3, globalInvTransform.c3, globalInvTransform.d3,
                globalInvTransform.a4, globalInvTransform.b4, globalInvTransform.c4, globalInvTransform.d4
            );

        readBoneHierarchy(scene->mRootNode, MeshBone::InvalidIndex);
        if (mBoneList.size() >= MeshBone::InvalidIndex) {
            fprintf(stderr, "File \"%s\" has too many bones.\n", mesh.file.c_str());
            return false;
        }
    }

    for (size_t meshIndex = 0; meshIndex < scene->mNumMeshes; meshIndex++) {
        const aiMesh* sceneMesh = scene->mMeshes[meshIndex];

        if (sceneMesh->mPrimitiveTypes != aiPrimitiveType_TRIANGLE) {
            fprintf(stderr, "In \"%s\": mesh #%d is not made from triangles.\n", mesh.file.c_str(), int(meshIndex));
            return false;
        }

        uint16_t baseVertex = vertices.size();
        size_t vertexCount = sceneMesh->mNumVertices;
        if (baseVertex + vertexCount > 65535) {
            fprintf(stderr, "Mesh \"%s\" has more than 64K vertices.\n", mesh.file.c_str());
            return false;
        }

        const bool hasPositions = sceneMesh->HasPositions();
        const bool hasNormals = sceneMesh->HasNormals();
        const bool hasTangents = sceneMesh->HasTangentsAndBitangents();
        const bool hasTexCoords = sceneMesh->HasTextureCoords(0);
        const bool hasBones = mesh.loadSkeleton && sceneMesh->HasBones();

        for (size_t i = 0; i < vertexCount; i++) {
            MeshVertex v;

            if (!hasPositions)
                v.position = glm::vec3(0.0f);
            else {
                v.position.x = sceneMesh->mVertices[i].x;
                v.position.y = sceneMesh->mVertices[i].y;
                v.position.z = sceneMesh->mVertices[i].z;
            }

            if (!hasNormals)
                v.normal = glm::vec3(0.0f);
            else {
                v.normal.x = sceneMesh->mNormals[i].x;
                v.normal.y = sceneMesh->mNormals[i].y;
                v.normal.z = sceneMesh->mNormals[i].z;
            }

            if (!hasTangents) {
                v.tangent = glm::vec3(0.0f);
                v.bitangent = glm::vec3(0.0f);
            } else {
                v.tangent.x = sceneMesh->mTangents[i].x;
                v.tangent.y = sceneMesh->mTangents[i].y;
                v.tangent.z = sceneMesh->mTangents[i].z;
                v.bitangent.x = sceneMesh->mBitangents[i].x;
                v.bitangent.y = sceneMesh->mBitangents[i].y;
                v.bitangent.z = sceneMesh->mBitangents[i].z;
            }

            if (!hasTexCoords)
                v.texCoord = glm::vec2(0.0f);
            else {
                v.texCoord.x = sceneMesh->mTextureCoords[0][i].x;
                v.texCoord.y = sceneMesh->mTextureCoords[0][i].y;
            }

            vertices.emplace_back(std::move(v));
        }

        if (hasBones) {
            skinningVertices.resize(vertices.size());

            for (size_t boneIndex = 0; boneIndex < sceneMesh->mNumBones; boneIndex++) {
                const aiBone* sceneMeshBone = sceneMesh->mBones[boneIndex];
                std::string boneName(sceneMeshBone->mName.data, sceneMeshBone->mName.length);

                auto it = mBoneMap.find(boneName);
                if (it == mBoneMap.end()) {
                    fprintf(stderr, "File \"%s\" referenced unknown bone \"%s\".\n", mesh.file.c_str(), boneName.c_str());
                    return false;
                }

                const auto& m = sceneMeshBone->mOffsetMatrix;
                mBoneList[it->second].matrix = glm::mat4(
                        m.a1, m.b1, m.c1, m.d1,
                        m.a2, m.b2, m.c2, m.d2,
                        m.a3, m.b3, m.c3, m.d3,
                        m.a4, m.b4, m.c4, m.d4
                    );

                for (size_t i = 0; i < sceneMeshBone->mNumWeights; i++) {
                    size_t vertexId = sceneMeshBone->mWeights[i].mVertexId + baseVertex;
                    for (int index = 0; index < 4; index++) {
                        if (skinningVertices[vertexId].boneWeights[index] == 0.0f) {
                            skinningVertices[vertexId].boneWeights[index] = sceneMeshBone->mWeights[i].mWeight;
                            skinningVertices[vertexId].boneIndices[index] = uint8_t(it->second);
                            break;
                        }
                    }
                }
            }
        }

        size_t firstIndex = indices.size();
        for (size_t i = 0; i < sceneMesh->mNumFaces; i++) {
            if (sceneMesh->mFaces[i].mNumIndices != 3) {
                fprintf(stderr, "In \"%s\": mesh #%d has invalid face.\n", mesh.file.c_str(), int(meshIndex));
                return false;
            }

            indices.emplace_back(baseVertex + uint16_t(sceneMesh->mFaces[i].mIndices[0]));
            indices.emplace_back(baseVertex + uint16_t(sceneMesh->mFaces[i].mIndices[1]));
            indices.emplace_back(baseVertex + uint16_t(sceneMesh->mFaces[i].mIndices[2]));
        }

        aiString materialName;
        const aiMaterial* sceneMeshMaterial = scene->mMaterials[sceneMesh->mMaterialIndex];
        std::string materialId{sceneMesh->mName.data, sceneMesh->mName.length};
        if (sceneMeshMaterial->Get(AI_MATKEY_NAME, materialName) == AI_SUCCESS)
            materialId = std::string(materialName.data, materialName.length);

        auto it = mesh.materialMapping.find(materialId);
        if (it != mesh.materialMapping.end())
            materialId = it->second;

        const ConfigFile::Material* materialConfig = mConfig.materialWithId(materialId);
        if (!materialConfig)
            return false;

        MeshMaterial material;
        material.firstIndex = firstIndex;
        material.indexCount = indices.size() - firstIndex;
        materials.emplace_back(std::move(material));
        materialIds.emplace_back(std::move(materialId));
    }

    mAnimations.clear();
    for (const auto& anim : mesh.animations)
        loadAnimations(anim);

    mCxx << "    static const MeshVertex " << mesh.id << "Vertices[] = {\n";
    for (const auto& vertex : vertices) {
        mCxx << "        { { ";
        mCxx << vertex.position.x << ", ";
        mCxx << vertex.position.y << ", ";
        mCxx << vertex.position.z << ", ";
        mCxx << "}, { ";
        mCxx << vertex.normal.x << ", ";
        mCxx << vertex.normal.y << ", ";
        mCxx << vertex.normal.z << ", ";
        mCxx << "}, { ";
        mCxx << vertex.tangent.x << ", ";
        mCxx << vertex.tangent.y << ", ";
        mCxx << vertex.tangent.z << ", ";
        mCxx << "}, { ";
        mCxx << vertex.bitangent.x << ", ";
        mCxx << vertex.bitangent.y << ", ";
        mCxx << vertex.bitangent.z << ", ";
        mCxx << "}, { ";
        mCxx << vertex.texCoord.x << ", ";
        mCxx << vertex.texCoord.y << ", ";
        mCxx << "} },\n";
    }
    mCxx << "    };\n\n";

    if (mesh.loadSkeleton) {
        mCxx << "    static const MeshSkinningVertex " << mesh.id << "SkinningVertices[] = {\n";
        for (const auto& vertex : skinningVertices) {
            mCxx << "        { { ";
            for (int i = 0; i < 4; i++)
                mCxx << vertex.boneWeights[i] << ", ";
            mCxx << "}, { ";
            for (int i = 0; i < 4; i++)
                mCxx << unsigned(vertex.boneIndices[i]) << ", ";
            mCxx << "} },\n";
        }
        mCxx << "    };\n\n";
    }

    mCxx << "    static const uint16_t " << mesh.id << "Indices[] = {\n";
    for (auto index : indices)
        mCxx << "        " << index << ",\n";
    mCxx << "    };\n\n";

    if (mesh.loadSkeleton) {
        mCxx << "    static const MeshBone " << mesh.id << "Bones[] = {\n";
        size_t i = 0;
        for (const auto& bone : mBoneList) {
            mCxx << "        {\n";
            mCxx << "            /* .name = */ \"" << bone.name <<  "\",\n";
            if (bone.parentIndex == MeshBone::InvalidIndex)
                mCxx << "            /* .parentIndex = */ MeshBone::InvalidIndex,\n";
            else
                mCxx << "            /* .parentIndex = */ " << unsigned(bone.parentIndex) <<  ",\n";
            mCxx << "            /* .matrix = */ { ";
            for (int y = 0; y < 4; y++) {
                for (int x = 0; x < 4; x++)
                    mCxx << bone.matrix[y][x] << ", ";
            }
            mCxx << "},\n";
            mCxx << "        },\n";
            ++i;
        }
        mCxx << "    };\n\n";
    }

    for (const auto& it : mAnimations) {
        mAnimHdr << "    extern const MeshAnimation " << it.first << ";\n";

        for (const auto& jt : it.second.bones) {
            if (!jt.positionKeys.empty()) {
                mAnimCxx << "    static const MeshPositionKey " << it.first << "Positions_" << jt.name << "[] = {\n";
                for (const auto& key : jt.positionKeys)
                    mAnimCxx << "        { " << key.time << ", { " << key.position.x << ", " << key.position.y << ", " << key.position.z << " } },\n";
                mAnimCxx << "    };\n\n";
            }

            if (!jt.rotationKeys.empty()) {
                mAnimCxx << "    static const MeshRotationKey " << it.first << "Rotations_" << jt.name << "[] = {\n";
                for (const auto& key : jt.rotationKeys)
                    mAnimCxx << "        { " << key.time << ", { " << key.rotation.x << ", " << key.rotation.y << ", " << key.rotation.z << ", " << key.rotation.w << " } },\n";
                mAnimCxx << "    };\n\n";
            }

            if (!jt.scaleKeys.empty()) {
                mAnimCxx << "    static const MeshScaleKey " << it.first << "Scale_" << jt.name << "[] = {\n";
                for (const auto& key : jt.scaleKeys)
                    mAnimCxx << "        { " << key.time << ", { " << key.scale.x << ", " << key.scale.y << ", " << key.scale.z << " } },\n";
                mAnimCxx << "    };\n\n";
            }
        }

        mAnimCxx << "    static const MeshBoneAnimation " << it.first << "BoneAnimations[] = {\n";
        for (const auto& jt : it.second.bones) {
            mAnimCxx << "        {\n";

            if (!jt.positionKeys.empty())
                mAnimCxx << "            /* positionKeys = */ " << it.first << "Positions_" << jt.name << ",\n";
            else
                mAnimCxx << "            /* positionKeys = */ nullptr,\n";

            if (!jt.rotationKeys.empty())
                mAnimCxx << "            /* rotationKeys = */ " << it.first << "Rotations_" << jt.name << ",\n";
            else
                mAnimCxx << "            /* rotationKeys = */ nullptr,\n";

            if (!jt.scaleKeys.empty())
                mAnimCxx << "            /* scaleKeys = */ " << it.first << "Scale_" << jt.name << ",\n";
            else
                mAnimCxx << "            /* scaleKeys = */ nullptr,\n";

            mAnimCxx << "        },\n";
        }
        mAnimCxx << "    };\n\n";

        mAnimCxx << "    const MeshAnimation " << it.first << " = {\n";
        mAnimCxx << "        /* durationInTicks = */ " << it.second.info.durationInTicks << ",\n";
        mAnimCxx << "        /* ticksPerSecond = */ " << it.second.info.ticksPerSecond << ",\n";
        mAnimCxx << "        /* boneAnimations = */ " << it.first << "BoneAnimations,\n";
        mAnimCxx << "    };\n\n";
    }

    mCxx << "    static const MeshMaterial " << mesh.id << "Materials[] = {\n";
    size_t i = 0;
    for (const auto& material : materials) {
        mCxx << "        {\n";
        mCxx << "            /* .firstIndex = */ " << material.firstIndex <<  ",\n";
        mCxx << "            /* .indexCount = */ " << material.indexCount <<  ",\n";
        mCxx << "            /* .material = */ &Materials::" << materialIds[i] <<  ",\n";
        mCxx << "        },\n";
        ++i;
    }
    mCxx << "    };\n\n";

    mCxx << "    const MeshData " << mesh.id << " = {\n";
    mCxx << "        /* .vertices = */ " << mesh.id <<  "Vertices,\n";
    if (mesh.loadSkeleton) {
        mCxx << "        /* .bones = */ " << mesh.id <<  "Bones,\n";
        mCxx << "        /* .skinningVertices = */ " << mesh.id <<  "SkinningVertices,\n";
    } else {
        mCxx << "        /* .bones = */ nullptr,\n";
        mCxx << "        /* .skinningVertices = */ nullptr,\n";
    }
    mCxx << "        /* .indices = */ " << mesh.id <<  "Indices,\n";
    mCxx << "        /* .materials = */ " << mesh.id <<  "Materials,\n";
    mCxx << "        /* .vertexCount = */ " << vertices.size() << ",\n";
    mCxx << "        /* .indexCount = */ " << indices.size() << ",\n";
    mCxx << "        /* .materialCount = */ " << materials.size() << ",\n";
    mCxx << "        /* .boneCount = */ " << mBoneList.size() << ",\n";
    mCxx << "    };\n\n";

    return true;
}

void MeshProcessor::readBoneHierarchy(const aiNode* rootNode, size_t parentBoneIndex)
{
    mBoneNames.emplace_back(std::make_unique<std::string>(rootNode->mName.data, rootNode->mName.length));
    const std::string& boneName = *mBoneNames.back();

    const auto& transform = rootNode->mTransformation;

    MeshBone bone;
    bone.name = boneName.c_str();
    bone.parentIndex = uint8_t(parentBoneIndex);
    bone.matrix = glm::mat4(
            transform.a1, transform.b1, transform.c1, transform.d1,
            transform.a2, transform.b2, transform.c2, transform.d2,
            transform.a3, transform.b3, transform.c3, transform.d3,
            transform.a4, transform.b4, transform.c4, transform.d4
        );

    size_t boneIndex = mBoneList.size();
    mBoneList.emplace_back(std::move(bone));
    mBoneMap[boneName] = boneIndex;

    for (size_t i = 0; i < rootNode->mNumChildren; i++)
        readBoneHierarchy(rootNode->mChildren[i], boneIndex);
}

bool MeshProcessor::loadAnimations(const ConfigFile::MeshAnimations& anim)
{
    const aiScene* scene = nullptr;
    const unsigned flags =
        aiProcess_RemoveRedundantMaterials |
        0;

    Assimp::Importer importer;
    scene = importer.ReadFile(anim.file, flags);
    if (!scene) {
        fprintf(stderr, "Unable to load file \"%s\": %s\n", anim.file.c_str(), importer.GetErrorString());
        return false;
    }

    for (size_t i = 0; i < scene->mNumAnimations; i++) {
        const aiAnimation* sceneAnimation = scene->mAnimations[i];

        std::string animationName(sceneAnimation->mName.data, sceneAnimation->mName.length);
        if (anim.ignore.find(animationName) != anim.ignore.end())
            continue;
        auto it = anim.rename.find(animationName);
        if (it != anim.rename.end())
            animationName = it->second;

        Anim animItem;
        animItem.info.durationInTicks = float(sceneAnimation->mDuration);
        animItem.info.ticksPerSecond = float(sceneAnimation->mTicksPerSecond);
        animItem.bones.resize(mBoneList.size());

        for (size_t j = 0; j < sceneAnimation->mNumChannels; j++) {
            const aiNodeAnim* channel = sceneAnimation->mChannels[j];

            std::string boneName{channel->mNodeName.data, channel->mNodeName.length};
            auto it = mBoneMap.find(boneName);
            if (it == mBoneMap.end()) {
                fprintf(stderr, "Unknown bone \"%s\" in file \"%s\".\n", boneName.c_str(), anim.file.c_str());
                return false;
            }

            BoneAnim& boneAnim = animItem.bones[it->second];
            boneAnim.name = boneName;

            boneAnim.positionKeys.reserve(channel->mNumPositionKeys);
            for (size_t k = 0; k < channel->mNumPositionKeys; k++) {
                const aiVectorKey& key = channel->mPositionKeys[k];
                glm::vec3 position(key.mValue.x, key.mValue.y, key.mValue.z);
                boneAnim.positionKeys.emplace_back(MeshPositionKey{float(key.mTime), position});
            }

            boneAnim.rotationKeys.reserve(channel->mNumRotationKeys);
            for (size_t k = 0; k < channel->mNumRotationKeys; k++) {
                const aiQuatKey& key = channel->mRotationKeys[k];
                glm::quat rotation(key.mValue.w, key.mValue.x, key.mValue.y, key.mValue.z);
                boneAnim.rotationKeys.emplace_back(MeshRotationKey{float(key.mTime), rotation});
            }

            boneAnim.scaleKeys.reserve(channel->mNumScalingKeys);
            for (size_t k = 0; k < channel->mNumScalingKeys; k++) {
                const aiVectorKey& key = channel->mScalingKeys[k];
                glm::vec3 scale(key.mValue.x, key.mValue.y, key.mValue.z);
                boneAnim.scaleKeys.emplace_back(MeshScaleKey{float(key.mTime), scale});
            }
        }

        if (mAnimations.find(animationName) != mAnimations.end()) {
            fprintf(stderr, "Duplicate animation id \"%s\" in file \"%s\".\n", animationName.c_str(), anim.file.c_str());
            return false;
        }

        mAnimations[animationName] = std::move(animItem);
    }

    return true;
}
