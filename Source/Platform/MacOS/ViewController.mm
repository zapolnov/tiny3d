#import "ViewController.h"
#import <Carbon/Carbon.h>
#import "Engine/Renderer/Metal/MetalRenderDevice.h"
#import "Engine/Core/Engine.h"
#import "Engine/Input/InputManager.h"
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
    renderDevice->onDrawableSizeChanged(size.width, size.height);
}

static Key mapKey(int code)
{
    switch (code) {
        case kVK_LeftArrow: return KeyLeft;
        case kVK_RightArrow: return KeyRight;
        case kVK_UpArrow: return KeyUp;
        case kVK_DownArrow: return KeyDown;
    }

    return KeyNone;
}

-(BOOL)acceptsFirstResponder
{
    return YES;
}

-(void)keyDown:(NSEvent*)event
{
    Key key = mapKey(event.keyCode);
    if (key != KeyNone)
        engine->inputManager()->injectKeyPress(key);
}

-(void)keyUp:(NSEvent*)event
{
    Key key = mapKey(event.keyCode);
    if (key != KeyNone)
        engine->inputManager()->injectKeyRelease(key);
}

@end
