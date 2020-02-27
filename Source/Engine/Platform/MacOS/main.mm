#import <Cocoa/Cocoa.h>
#import "AppDelegate.h"

int main(int argc, char** argv)
{
    @autoreleasepool {
        NSApplication* app = [NSApplication sharedApplication];
        AppDelegate* appDelegate = [[AppDelegate alloc] init];
        [app setDelegate:appDelegate];
        [app run];
    }
    return 0;
}
