#include "AnimatedMesh.h"
#include "Engine/Mesh/MeshData.h"
#include "Engine/Core/Engine.h"
#include "Engine/Renderer/IRenderDevice.h"
#include "Engine/Renderer/IRenderBuffer.h"
#include <cassert>
#include <glm/gtc/matrix_transform.hpp>

AnimatedMesh::AnimatedMesh(Engine* engine, const MeshData* data)
    : StaticMesh(engine, data)
    , mBones(data->bones)
    , mBoneCount(data->boneCount)
    , mGlobalInverseTransform(*data->globalInverseTransform)
    , mMatrices(new glm::mat4[data->boneCount])
    , mAnimation(nullptr)
    , mTime(0.0f)
{
    mSkinningVertexBuffer = mEngine->renderDevice()->createBufferWithData(
        data->skinningVertices, data->skinningVertexCount * sizeof(MeshSkinningVertex));
}

AnimatedMesh::~AnimatedMesh()
{
}

void AnimatedMesh::addTime(float time)
{
    mTime += time;
}

float AnimatedMesh::animationDuration() const
{
    if (!mAnimation)
        return 0.0f;

    float ticksPerSecond = (mAnimation->ticksPerSecond > 0.0f ? mAnimation->ticksPerSecond : 25.0f);
    return mAnimation->durationInTicks * ticksPerSecond;
}

void AnimatedMesh::setAnimation(const MeshAnimation* anim)
{
    if (mAnimation != anim) {
        mAnimation = anim;
        mTime = 0.0f;
    }
}

void AnimatedMesh::render() const
{
    mEngine->renderDevice()->setVertexBuffer(1, mSkinningVertexBuffer);

    StaticMesh::render();
}

template <class T> T interpolatedValue(float time, float duration, const T* keys, size_t keyCount,
    const T& defaultValue, const std::function<T(const T&, const T&, float)>& interpolate)
{
    if (keyCount < 2) {
        if (keyCount == 0)
            return defaultValue;
        return keys[0];
    }

    const T* begin = keys;
    const T* end = keys + keyCount;
    const T* key1;
    const T* key2 = std::lower_bound(begin, end, time, [](const T& value, float t) -> bool { return value.time < t; });

    if (key2 != end && key2->time == time)
        return *key2;

    float key1Time;
    float key2Time;
    if (key2 != begin && key2 != end) {
        key1 = key2 - 1;
        key1Time = key1->time;
        key2Time = key2->time;
    } else {
        key1 = end - 1;
        if (key2 == begin) {
            key1Time = -(duration - key1->time);
            assert(key1Time <= 0.0f);
            key2Time = key2->time;
        } else {
            key2 = begin;
            key1Time = key1->time;
            key2Time = duration;
        }
    }

    float timeDelta = key2Time - key1Time;
    float factor = (timeDelta != 0.0f ? (time - key1Time) / timeDelta : 0.0f);
    assert(factor >= 0.0f && factor <= 1.0f);

    T value = interpolate(*key1, *key2, factor);
    value.time = time;

    return value;
}

void AnimatedMesh::calculatePose(float time, glm::mat4* matrices) const
{
    if (!mAnimation) {
        for (size_t i = 0; i < mBoneCount; i++)
            matrices[i] = mBones[i].matrix;
        return;
    }

    float ticksPerSecond = (mAnimation->ticksPerSecond > 0.0f ? mAnimation->ticksPerSecond : 25.0f);
    float timeInTicks = fmodf(mTime / ticksPerSecond, mAnimation->durationInTicks);

    for (size_t boneIndex = 0; boneIndex < mBoneCount; boneIndex++) {
        const MeshBoneAnimation* anim = &mAnimation->boneAnimations[boneIndex];

        glm::mat4 transform;
        if (!anim->positionKeys && !anim->scaleKeys && !anim->rotationKeys)
            transform = glm::mat4(1.0f);
        else {
            MeshPositionKey pos = interpolatedValue<MeshPositionKey>(timeInTicks, mAnimation->durationInTicks,
                anim->positionKeys, anim->positionKeyCount, MeshPositionKey{timeInTicks, glm::vec3(0.0f)},
                [](const MeshPositionKey& key1, const MeshPositionKey& key2, float factor) -> MeshPositionKey {
                    return MeshPositionKey{0.0f, key1.position + (key2.position - key1.position) * factor};
                });

            MeshRotationKey rot = interpolatedValue<MeshRotationKey>(timeInTicks, mAnimation->durationInTicks,
                anim->rotationKeys, anim->rotationKeyCount, MeshRotationKey{timeInTicks, glm::quat()},
                [](const MeshRotationKey& key1, const MeshRotationKey& key2, float factor) -> MeshRotationKey {
                    return MeshRotationKey{0.0f, glm::slerp(key1.rotation, key2.rotation, factor)};
                });

            MeshScaleKey scale = interpolatedValue<MeshScaleKey>(timeInTicks, mAnimation->durationInTicks,
                anim->scaleKeys, anim->scaleKeyCount, MeshScaleKey{timeInTicks, glm::vec3(1.0f)},
                [](const MeshScaleKey& key1, const MeshScaleKey& key2, float factor) -> MeshScaleKey {
                    return MeshScaleKey{0.0f, key1.scale + (key2.scale - key1.scale) * factor};
                });

            transform  = glm::translate(glm::mat4(1.0f), pos.position);
            transform *= glm::mat4_cast(rot.rotation);
            transform  = glm::scale(transform, scale.scale);
        }

        uint8_t parentBone = mBones[boneIndex].parentIndex;
        if (parentBone == MeshBone::InvalidIndex)
            transform = mGlobalInverseTransform * transform;
        else {
            assert(parentBone < boneIndex);
            transform = matrices[parentBone] * transform;
        }

        matrices[boneIndex] = transform;
    }

    for (size_t boneIndex = 0; boneIndex < mBoneCount; boneIndex++)
        matrices[boneIndex] *= mBones[boneIndex].matrix;
}
