#import "ViewController.h"
#import "Engine/Renderer/Metal/MetalRenderDevice.h"
#import "Engine/Core/Engine.h"
#import "Game/Game.h"

@implementation ViewController

-(ViewController*)initWithContentRect:(NSRect)rect
{
    self = [super init];
    if (!self)
        return nil;

    mtkView = [[MTKView alloc] initWithFrame:rect device:MTLCreateSystemDefaultDevice()];
    [mtkView setColorPixelFormat:MTLPixelFormatBGRA8Unorm];
    [mtkView setDepthStencilPixelFormat:MTLPixelFormatDepth24Unorm_Stencil8];
    [mtkView setClearColor:MTLClearColorMake(0.1f, 0.3f, 0.5f, 1)];
    [mtkView setDelegate:self];
    self.view = mtkView;

    renderDevice = new MetalRenderDevice(mtkView);
    engine = new Engine(renderDevice, [](Engine* engine){ return new Game(engine); });

    [self mtkView:mtkView drawableSizeWillChange:mtkView.drawableSize];

    return self;
}

-(void)dealloc
{
    delete engine;
    delete renderDevice;
}

-(void)drawInMTKView:(MTKView*)view
{
    engine->doOneFrame();
}

-(void)mtkView:(MTKView*)view drawableSizeWillChange:(CGSize)size
{
}

@end
