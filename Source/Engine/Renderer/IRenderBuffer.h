#pragma once

class IRenderBuffer
{
public:
    virtual ~IRenderBuffer() = default;
    virtual unsigned uploadData(const void* data) = 0;
};
