#import "MetalTexture.h"
#import "MetalRenderDevice.h"

MetalTexture::MetalTexture(MetalRenderDevice* device, id<MTLTexture> texture)
    : mDevice(device)
    , mTexture(texture)
{
}

MetalTexture::~MetalTexture()
{
}
