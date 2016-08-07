
#ifdef HAVE_CONFIG_H
# include "config.h" // must be first
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

#if !defined(__OBJC__)
# error you need to compile this code as Objective C++
#endif

#import <Cocoa/Cocoa.h>
#import <Carbon/Carbon.h>
#import <AppKit/AppKit.h>
#import <CoreData/CoreData.h>
#import <Foundation/Foundation.h>

@interface CocoaTestApplicationDelegate : NSObject <NSApplicationDelegate>
{
}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender;
- (void)applicationWillFinishLaunching:(NSNotification *)aNotification;
- (void)applicationDidFinishLaunching:(NSNotification *)aNotification;
@end

@interface CocoaTestWindowDelegate : NSObject <NSWindowDelegate>
{
}

- (BOOL)windowShouldClose:(id)sender;
@end

@interface CocoaTestWindow : NSWindow
{
}
@end

@interface CocoaTestView : NSView
{
}
@end

NSApplication*			mainApplication = NULL;
CocoaTestApplicationDelegate*	mainApplicationDelegate = NULL;
CocoaTestWindowDelegate*	mainWindowDelegate = NULL;
CocoaTestWindow*		mainWindow = NULL;
CocoaTestView*			mainWindowView = NULL;
NSView*				mainWindowSubView = NULL; // do we really need this?

@implementation CocoaTestApplicationDelegate
- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender
{
	fprintf(stderr,"App should terminate?\n");
	return NSTerminateNow;
}

- (void)applicationWillFinishLaunching:(NSNotification *)aNotification {
	fprintf(stderr,"App will finish launching\n");
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
	fprintf(stderr,"App did finish launching\n");
}
@end

@implementation CocoaTestWindowDelegate
- (BOOL)windowShouldClose:(id)sender
{
	fprintf(stderr,"should window close? yes!\n");
	[ NSApp terminate: self ];
	return YES;
}
@end

@implementation CocoaTestWindow
@end

@implementation CocoaTestView
@end

int main(int argc,char **argv) {
	NSRect contentRect;
	NSString* nstr;

	contentRect = NSMakeRect(0,0,640,480);

	// connect to the GUI
	[NSAutoreleasePool new];
	mainApplication = [ NSApplication sharedApplication ];
	// Okay, so apparently, since we're not using NIBs, we have to do this to make Apple show our damn main menu in the menu bar.
	[ NSApp setActivationPolicy:NSApplicationActivationPolicyRegular ];

	// please tell me app events
	mainApplicationDelegate = [
		[ CocoaTestApplicationDelegate alloc ] init ];

	[ NSApp setDelegate: mainApplicationDelegate ];

	// the menu!
	// NTS: Apparently enforced in Mac OS X is the scheme where the first menu in the menu bar
	//      is always named whatever your process is called, and you can't override it. Fine >:(
	id menubar = [[NSMenu new] autorelease];
	[NSApp setMainMenu:menubar];

	id appMenuItem = [[NSMenuItem alloc] initWithTitle:@"File" action:nil keyEquivalent:@""];
	[menubar addItem:appMenuItem];

	id appMenu = [[NSMenu alloc] initWithTitle:@"File"];
	[appMenuItem setSubmenu:appMenu];

	id hideMenuItem = [[[NSMenuItem alloc] initWithTitle:@"Hide"
		action:@selector(hide:) keyEquivalent:@"h"] autorelease];
	[appMenu addItem:hideMenuItem];

	[appMenu addItem:[NSMenuItem separatorItem]];

	id quitMenuItem = [[[NSMenuItem alloc] initWithTitle:@"Quit"
		action:@selector(terminate:) keyEquivalent:@"q"] autorelease];
	[appMenu addItem:quitMenuItem];

	// create window and delegate for Window events
	mainWindowDelegate = [
		[ CocoaTestWindowDelegate alloc ] init ];

	mainWindow = [
		[ CocoaTestWindow alloc ] initWithContentRect: contentRect /* <- method */
		styleMask: NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask | NSResizableWindowMask /* <- param */
		backing: NSBackingStoreBuffered /* <- param */
		defer: NO /* <- param */
		screen: [NSScreen mainScreen]
	];

	[ mainWindow setTitle: @"Test program 1" ];
	[ mainWindow setDelegate: mainWindowDelegate ];
	[ mainWindow setAcceptsMouseMovedEvents: YES ];
	[ mainWindow setViewsNeedDisplay: NO ];

	// now the view
	mainWindowView = [
		[ CocoaTestView alloc ] initWithFrame: contentRect
	];

	[ mainWindowView setAutoresizingMask: NSViewWidthSizable | NSViewHeightSizable ];

	// now the subview
	mainWindowSubView = [
		[ NSView alloc ] initWithFrame: contentRect ];
	[ mainWindowSubView setAutoresizingMask: NSViewWidthSizable | NSViewHeightSizable ];
	[ mainWindowView addSubview: mainWindowSubView ];
	[ mainWindow setContentView: mainWindowView ];

	[ mainWindow makeKeyAndOrderFront:nil ];

	// main event loop
	[ NSApp activateIgnoringOtherApps:YES ];
	[ NSApp run ];

	return 0;
}

