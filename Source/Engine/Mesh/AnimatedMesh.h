#pragma once
#include "Engine/Mesh/StaticMesh.h"
#include <glm/mat4x4.hpp>
#include <memory>

struct MeshAnimation;
struct MeshBone;

class AnimatedMesh : public StaticMesh
{
public:
    AnimatedMesh(Engine* engine, const MeshData* data);
    ~AnimatedMesh();

    void addTime(float time);
    float animationDuration() const;
    void setAnimation(const MeshAnimation* anim);

    void render() const override;

private:
    const MeshBone* mBones;
    size_t mBoneCount;
    glm::mat4 mGlobalInverseTransform;
    std::unique_ptr<IRenderBuffer> mSkinningVertexBuffer;
    std::unique_ptr<IRenderBuffer> mMatrixBuffer;
    std::unique_ptr<glm::mat4[]> mMatrices;
    const MeshAnimation* mAnimation;
    float mTime;

    void calculatePose(float time) const;
};
