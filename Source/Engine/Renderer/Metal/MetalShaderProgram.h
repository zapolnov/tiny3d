#import "Engine/Renderer/IShaderProgram.h"
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>

class MetalRenderDevice;

class MetalShaderProgram : public IShaderProgram
{
public:
    MetalShaderProgram(MetalRenderDevice* device, id<MTLLibrary> library);
    ~MetalShaderProgram();

    id<MTLFunction> vertexFunction() const { return mVertexFunction; }
    id<MTLFunction> fragmentFunction() const { return mFragmentFunction; }

private:
    MetalRenderDevice* mDevice;
    id<MTLLibrary> mLibrary;
    id<MTLFunction> mVertexFunction;
    id<MTLFunction> mFragmentFunction;
};
