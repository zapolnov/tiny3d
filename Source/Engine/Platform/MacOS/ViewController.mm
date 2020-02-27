#import "ViewController.h"

@implementation ViewController

-(MTKView*)createViewWithRect:(NSRect)rect
{
    mtkView = [[MTKView alloc] initWithFrame:rect device:MTLCreateSystemDefaultDevice()];
    [mtkView setColorPixelFormat:MTLPixelFormatBGRA8Unorm];
    [mtkView setDepthStencilPixelFormat:MTLPixelFormatDepth24Unorm_Stencil8];
    [mtkView setClearColor:MTLClearColorMake(0.1f, 0.3f, 0.5f, 1)];
    [mtkView setDelegate:self];
    self.view = mtkView;
    return mtkView;
}

-(void)drawInMTKView:(MTKView*)view
{
}

-(void)mtkView:(MTKView*)view drawableSizeWillChange:(CGSize)size
{
}

@end
