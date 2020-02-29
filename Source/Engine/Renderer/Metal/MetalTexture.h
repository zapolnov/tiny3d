#import "Engine/Renderer/ITexture.h"
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>

class MetalRenderDevice;

class MetalTexture : public ITexture
{
public:
    MetalTexture(MetalRenderDevice* device, id<MTLTexture> texture);
    ~MetalTexture();

    id<MTLTexture> nativeTexture() const { return mTexture; }

private:
    MetalRenderDevice* mDevice;
    id<MTLTexture> mTexture;
};
