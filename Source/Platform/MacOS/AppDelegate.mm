#import "AppDelegate.h"
#import "ViewController.h"

@implementation AppDelegate

-(void)applicationDidFinishLaunching:(NSNotification*)notification
{
    NSScreen* screen = [NSScreen mainScreen];
    float screenW = screen.frame.size.width;
    float screenH = screen.frame.size.height;

    float windowW = 1024;
    float windowH = 768;
    float windowX = (screenW - windowW) * 0.5f;
    float windowY = (screenH - windowH) * 0.5f;
    NSRect rect = NSMakeRect(windowX, windowY, windowW, windowH);

    NSUInteger styleMask = NSWindowStyleMaskTitled
                         | NSWindowStyleMaskResizable
                         | NSWindowStyleMaskClosable
                         | NSWindowStyleMaskMiniaturizable;

    window = [[NSWindow alloc] initWithContentRect:rect styleMask:styleMask backing:NSBackingStoreBuffered defer:YES];

    ViewController* controller = [[ViewController alloc] initWithContentRect:rect];
    [window setContentViewController:controller];
    [window makeFirstResponder:controller];

    [window makeKeyAndOrderFront:self];
}

-(void)applicationWillTerminate:(NSNotification*)notification
{
}

-(BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)sender
{
    return YES;
}

@end
