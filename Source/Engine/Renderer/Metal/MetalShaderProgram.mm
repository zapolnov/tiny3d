#import "MetalShaderProgram.h"
#import "MetalRenderDevice.h"

MetalShaderProgram::MetalShaderProgram(MetalRenderDevice* device, id<MTLLibrary> library)
    : mDevice(device)
    , mLibrary(library)
{
    mVertexFunction = [mLibrary newFunctionWithName:@"vertexShader"];
    if (!mVertexFunction)
        NSLog(@"Vertex function was not found in shader library.");

    mFragmentFunction = [mLibrary newFunctionWithName:@"fragmentShader"];
    if (!mFragmentFunction)
        NSLog(@"Fragment function was not found in shader library.");
}

MetalShaderProgram::~MetalShaderProgram()
{
}
