#pragma once
#include <vector>

enum class VertexType
{
    Float2,
    Float3,
};

class VertexFormat
{
public:
    struct Attribute
    {
        VertexType type;
        unsigned offset;
    };

    VertexFormat()
        : mStride(0)
    {
    }

    const std::vector<Attribute>& attributes() const { return mAttributes; }
    unsigned stride() const { return mStride; }

    void addAttribute(VertexType type)
    {
        Attribute attr;
        attr.type = type;
        attr.offset = mStride;
        mAttributes.emplace_back(std::move(attr));

        switch (type) {
            case VertexType::Float2: mStride += 2 * sizeof(float); break;
            case VertexType::Float3: mStride += 3 * sizeof(float); break;
        }
    }

private:
    std::vector<Attribute> mAttributes;
    unsigned mStride;
};
