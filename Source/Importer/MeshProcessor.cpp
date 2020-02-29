#include "MeshProcessor.h"
#include "Engine/Mesh/MeshData.h"
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

    mCxx << "#include \"Meshes.h\"\n";
    mCxx << std::endl;
}

MeshProcessor::~MeshProcessor()
{
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
        aiProcess_PreTransformVertices |
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
    std::vector<MeshMaterial> materials;
    std::vector<uint16_t> indices;

    mHdr << "extern const MeshData " << mesh.id << ";\n";

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

        const ConfigFile::Material* materialConfig = mConfig.materialWithId(materialId);
        //if (!materialConfig)
        //    return false;

        MeshMaterial material;
        material.firstIndex = firstIndex;
        material.indexCount = indices.size() - firstIndex;
        materials.emplace_back(std::move(material));
    }

    mCxx << "static const MeshVertex " << mesh.id << "Vertices[] = {\n";
    for (const auto& vertex : vertices) {
        mCxx << "    { { ";
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
    mCxx << "};\n\n";

    mCxx << "static const uint16_t " << mesh.id << "Indices[] = {\n";
    for (auto index : indices)
        mCxx << "    " << index << ",\n";
    mCxx << "};\n\n";

    mCxx << "static const MeshMaterial " << mesh.id << "Materials[] = {\n";
    for (const auto& material : materials) {
        mCxx << "    {\n";
        mCxx << "        /* .firstIndex = */ " << material.firstIndex <<  ",\n";
        mCxx << "        /* .indexCount = */ " << material.indexCount <<  ",\n";
        mCxx << "    },\n";
    }
    mCxx << "};\n\n";

    mCxx << "const MeshData " << mesh.id << " = {\n";
    mCxx << "    /* .vertices = */ " << mesh.id <<  "Vertices,\n";
    mCxx << "    /* .indices = */ " << mesh.id <<  "Indices,\n";
    mCxx << "    /* .materials = */ " << mesh.id <<  "Materials,\n";
    mCxx << "    /* .vertexCount = */ " << vertices.size() << ",\n";
    mCxx << "    /* .indexCount = */ " << indices.size() << ",\n";
    mCxx << "    /* .materialCount = */ " << materials.size() << ",\n";
    mCxx << "};\n";

    return true;
}

bool MeshProcessor::generate()
{
    if (!writeTextFile("Compiled/Meshes.cpp", std::move(mCxx)))
        return false;
    if (!writeTextFile("Compiled/Meshes.h", std::move(mHdr)))
        return false;
    return true;
}
