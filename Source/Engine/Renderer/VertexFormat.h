#pragma once
#include <vector>

enum class VertexType
{
    Float2,
    Float3,
    Float4,
    UByte4,
};

class VertexFormat
{
public:
    struct Attribute
    {
        VertexType type;
        unsigned offset;
        int bufferIndex;
    };

    VertexFormat()
    {
    }

    const std::vector<Attribute>& attributes() const { return mAttributes; }

    unsigned bufferCount() const { return mStride.size(); }
    unsigned stride(int bufferIndex) const { return mStride[bufferIndex]; }

    void addAttribute(VertexType type, int bufferIndex = 0)
    {
        if (bufferIndex >= int(mStride.size()))
            mStride.resize(bufferIndex + 1);

        Attribute attr;
        attr.type = type;
        attr.offset = mStride[bufferIndex];
        attr.bufferIndex = bufferIndex;
        mAttributes.emplace_back(std::move(attr));

        switch (type) {
            case VertexType::Float2: mStride[bufferIndex] += 2 * sizeof(float); break;
            case VertexType::Float3: mStride[bufferIndex] += 3 * sizeof(float); break;
            case VertexType::Float4: mStride[bufferIndex] += 4 * sizeof(float); break;
            case VertexType::UByte4: mStride[bufferIndex] += 4; break;
        }
    }

private:
    std::vector<Attribute> mAttributes;
    std::vector<unsigned> mStride;
};
