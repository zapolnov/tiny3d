#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>

class Engine;
class MetalRenderDevice;

@interface ViewController : NSViewController<MTKViewDelegate>
{
    MTKView* mtkView;
    MetalRenderDevice* renderDevice;
    Engine* engine;
}
-(ViewController*)initWithContentRect:(NSRect)rect;
@end
