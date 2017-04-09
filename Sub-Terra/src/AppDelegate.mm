#import "AppDelegate.h"
#import "common.h"
#import "Freefall.h"

@interface AppDelegate ()

@property (weak) IBOutlet NSWindow *window;

@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
	const std::vector<std::string> args;
	Freefall().Run(args);
	[NSApp performSelector:@selector(terminate:) withObject:nil afterDelay:0.0];
}

- (void)applicationWillTerminate:(NSNotification *)aNotification {

}

@end
