#import "AppDelegate.h"
#import "common.h"
#import "Freefall.h"

@interface AppDelegate ()

@property (weak) IBOutlet NSWindow *window;

@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
	std::vector<std::string> args;
	/*for(int i = 0; i < argc; ++i) {
		args.emplace_back(argv[i]);
	}*/

	Polar engine(args);
	auto app = Freefall(engine);
	
	[NSApp performSelector:@selector(terminate:) withObject:nil afterDelay:0.0];
}

- (void)applicationWillTerminate:(NSNotification *)aNotification {

}

@end
