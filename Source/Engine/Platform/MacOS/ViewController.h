#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>

@interface ViewController : NSViewController<MTKViewDelegate>
{
    MTKView* mtkView;
}
-(MTKView*)createViewWithRect:(NSRect)rect;
@end
