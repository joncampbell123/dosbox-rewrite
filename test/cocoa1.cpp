
#ifdef HAVE_CONFIG_H
# include "config.h" // must be first
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

#include <algorithm>

#include "dosboxxr/lib/util/rgbinfo.h"
#include "dosboxxr/lib/util/rgb_bitmap_info.h"
#include "dosboxxr/lib/util/rgb_bitmap_test_pattern_gradients.h"

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
- (NSSize)windowWillResize:(NSWindow *)sender toSize:(NSSize)frameSize;
@end

@interface CocoaTestWindow : NSWindow
{
}
@end

@interface CocoaTestView : NSView
{
}

- (void)drawRect:(NSRect)dirtyRect;
- (void)keyDown:(NSEvent*)event;
@end

rgb_bitmap_info                 quartz_bitmap;
CGContextRef			cg_context = NULL;
NSApplication*			mainApplication = NULL;
CocoaTestApplicationDelegate*	mainApplicationDelegate = NULL;
CocoaTestWindowDelegate*	mainWindowDelegate = NULL;
CocoaTestWindow*		mainWindow = NULL;
CocoaTestView*			mainWindowView = NULL;
NSView*				mainWindowSubView = NULL; // do we really need this?
bool				force_abgr = false;
bool				force_bgra = false;
bool				announce_fmt = true;
bool				force_rgb555 = false;

void free_bitmap(void) {
	if (cg_context != NULL) {
		CGContextFlush(cg_context);
		CGContextRelease(cg_context);
		cg_context = NULL;
	}
	if (quartz_bitmap.base != NULL) {
		free(quartz_bitmap.base);
		quartz_bitmap.base = NULL;
	}

	quartz_bitmap.clear();
}

bool init_bitmap(unsigned int width,unsigned int height,unsigned int align=32) {
	if (quartz_bitmap.is_valid() && quartz_bitmap.width == width && quartz_bitmap.height == height)
		return true;

	enum CGImageAlphaInfo cgAlphaMode = kCGImageAlphaNoneSkipLast;
	unsigned int bits_per_channel = 8;

	// TODO: Figure out how to represent RGB 5/5/5 16-bit rgb in OS X

	if (force_abgr)
		cgAlphaMode = kCGImageAlphaNoneSkipLast;
	else if (force_bgra)
		cgAlphaMode = kCGImageAlphaNoneSkipFirst;

	free_bitmap();

	// FIXME: Considering Mac OS X target audience and aethetics, it's best to assume 32bpp ARGB 8-bit/channel
	//        rather than deal with deprecated APIs to figure out bit depth. Although the CG API does support
	//        16bpp 5/6/5 RGB I doubt any OS X system would ship with the desktop set that way. In any case
	//        should OS X default to something like 32bpp 10/10/10 ARGB it doesn't matter because Quartz uses
	//        the GPU to render the desktop.
	//
	//        BUT: It would be fun to allow the user to override bit depth to see what else OS X will render from.

	quartz_bitmap.width = width;
	quartz_bitmap.height = height;
	quartz_bitmap.bytes_per_pixel = 4; // 32bpp
	quartz_bitmap.stride_align = align;
	quartz_bitmap.update_stride_from_width();
	quartz_bitmap.update_length_from_stride_and_height();

	if (cgAlphaMode == kCGImageAlphaNoneSkipLast) { // "If the total size of the pixel is greater than space required, LSB (last byte?) is ignored"
		if (quartz_bitmap.bytes_per_pixel == 4) {
			quartz_bitmap.rgbinfo.r.setByMask(0x000000FFUL);
			quartz_bitmap.rgbinfo.g.setByMask(0x0000FF00UL);
			quartz_bitmap.rgbinfo.b.setByMask(0x00FF0000UL);
			quartz_bitmap.rgbinfo.a.setByMask(0xFF000000UL);
		}
		else {
			fprintf(stderr,"Unknown mapping\n");
			free_bitmap();
			return false;
		}
	}
	else if (cgAlphaMode == kCGImageAlphaNoneSkipFirst) { // "If the total size of the pixel is greater than space required, MSB (first byte?) is ignored"
		if (quartz_bitmap.bytes_per_pixel == 4) {
			quartz_bitmap.rgbinfo.r.setByMask(0x0000FF00UL);
			quartz_bitmap.rgbinfo.g.setByMask(0x00FF0000UL);
			quartz_bitmap.rgbinfo.b.setByMask(0xFF000000UL);
			quartz_bitmap.rgbinfo.a.setByMask(0x000000FFUL);
		}
		else if (quartz_bitmap.bytes_per_pixel == 2) {
			quartz_bitmap.rgbinfo.r.setByMask(0x7C00UL);
			quartz_bitmap.rgbinfo.g.setByMask(0x03E0UL);
			quartz_bitmap.rgbinfo.b.setByMask(0x001FUL);
			quartz_bitmap.rgbinfo.a.setByMask(0x8000UL);
		}
		else {
			fprintf(stderr,"Unknown mapping\n");
			free_bitmap();
			return false;
		}
	}
	else {
		fprintf(stderr,"Unknown mapping\n");
		free_bitmap();
		return false;
	}

#if HAVE_POSIX_MEMALIGN // OS X "El Capitan" has it!
	if (posix_memalign((void**)(&quartz_bitmap.base),align,quartz_bitmap.length)) {
		fprintf(stderr,"Failed to alloc bitmap\n");
		free_bitmap();
		return false;
	}
	quartz_bitmap.canvas = quartz_bitmap.base;
#else
# error malloc case not implemented
#endif

	if (!quartz_bitmap.is_valid()) {
		fprintf(stderr,"Constructed bitmap is not valid\n");
		free_bitmap();
		return false;
	}

	CGColorSpaceRef cgColorspace = CGColorSpaceCreateDeviceRGB(); // TODO: What if this fails?

	cg_context = CGBitmapContextCreate(quartz_bitmap.canvas,
		quartz_bitmap.width,quartz_bitmap.height,bits_per_channel,
		quartz_bitmap.stride,cgColorspace,
		cgAlphaMode);
	
	CGColorSpaceRelease(cgColorspace);

	if (cg_context == NULL) {
		fprintf(stderr,"Failed to create bitmap\n");
		free_bitmap();
		return false;
	}

	CGContextSetBlendMode(cg_context, kCGBlendModeCopy);
	CGContextTranslateCTM(cg_context, 0, -quartz_bitmap.height);
	CGContextScaleCTM(cg_context, 1.0, -1.0);

	if (announce_fmt) {
		announce_fmt = false;
		fprintf(stderr,"R/G/B/A shift/width/mask %u/%u/0x%X %u/%u/0x%X %u/%u/0x%X %u/%u/0x%X\n",
				quartz_bitmap.rgbinfo.r.shift, quartz_bitmap.rgbinfo.r.bwidth, quartz_bitmap.rgbinfo.r.bmask,
				quartz_bitmap.rgbinfo.g.shift, quartz_bitmap.rgbinfo.g.bwidth, quartz_bitmap.rgbinfo.g.bmask,
				quartz_bitmap.rgbinfo.b.shift, quartz_bitmap.rgbinfo.b.bwidth, quartz_bitmap.rgbinfo.b.bmask,
				quartz_bitmap.rgbinfo.a.shift, quartz_bitmap.rgbinfo.a.bwidth, quartz_bitmap.rgbinfo.a.bmask);
	}

	return true;
}

bool init_bitmap_from_main_window(void) {
	NSSize sz = [[mainWindow contentView] frame].size;

	fprintf(stderr,"Window size %.3f x %.3f\n",sz.width,sz.height);

	return init_bitmap(ceil(sz.width),ceil(sz.height));
}

bool init_bitmap_from_main_window(NSSize sz) {
	fprintf(stderr,"Window size %.3f x %.3f\n",sz.width,sz.height);

	return init_bitmap(ceil(sz.width),ceil(sz.height));
}

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

	if (!init_bitmap_from_main_window())
		fprintf(stderr,"Failed to init bitmap\n");

	render_test_pattern_rgb_gradients(quartz_bitmap);

	[ mainWindowView setNeedsDisplay:YES ];
	[ mainWindowSubView setNeedsDisplay:YES ];
}
@end

@implementation CocoaTestWindowDelegate
- (BOOL)windowShouldClose:(id)sender
{
	fprintf(stderr,"should window close? yes!\n");
	[ NSApp terminate: self ];
	return YES;
}

- (NSSize)windowWillResize:(NSWindow *)sender toSize:(NSSize)frameSize
{
	NSRect contentRect = [ sender contentRectForFrameRect: NSMakeRect(0,0,frameSize.width,frameSize.height) ];
	NSSize contentSize = NSMakeSize(contentRect.size.width,contentRect.size.height);

	fprintf(stderr,"Window resize\n");

	if (!init_bitmap_from_main_window(contentSize))
		fprintf(stderr,"Failed to init bitmap\n");

	render_test_pattern_rgb_gradients(quartz_bitmap);

	[ mainWindowView setNeedsDisplay:YES ];
	[ mainWindowSubView setNeedsDisplay:YES ];
	return frameSize;
}
@end

@implementation CocoaTestWindow
@end

@implementation CocoaTestView
- (void)drawRect:(NSRect)dirtyRect
{
	if (mainWindow != NULL && cg_context != NULL) {
		fprintf(stderr,"Update\n");
		CGContextRef ctx = (CGContextRef)[[NSGraphicsContext currentContext] graphicsPort];
		CGImageRef image = CGBitmapContextCreateImage(cg_context);
		CGRect rect = CGRectMake(0,0,quartz_bitmap.width,quartz_bitmap.height);
		CGContextDrawImage(ctx,rect,image);
		CGImageRelease(image);
		CGContextFlush(ctx);
	}
}

- (void)keyDown:(NSEvent*)event
{
	NSString* ch = [event characters];
	char tmp[512];

	[ch getCString:tmp maxLength:sizeof(tmp)];

	if (!strcmp(tmp,"\x1B")) {
		[ NSApp terminate: self ];
	}
}
@end

int main(int argc,char **argv) {
	NSRect contentRect;
	NSString* nstr;
	char *a;
	int i;

	for (i=1;i < argc;) {
		a = argv[i++];

		if (*a == '-') {
			do { a++; } while (*a == '-');

			if (!strcmp(a,"bgra")) {
				force_bgra = true;
			}
			else if (!strcmp(a,"abgr")) {
				force_abgr = true;
			}
			else if (!strcmp(a,"rgb555")) {
				force_rgb555 = true;
			}
			else {
				fprintf(stderr,"Unhandled switch %s\n",a);
				return 1;
			}
		}
		else {
			fprintf(stderr,"Unhandled arg %s\n",a);
			return 1;
		}
	}

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
	[ mainWindow setOpaque: YES ];

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
	[ mainWindow makeKeyWindow ];
	[ mainWindow makeFirstResponder: mainWindowSubView ];

	// main event loop
	[ NSApp activateIgnoringOtherApps:YES ];
	[ NSApp run ];

	return 0;
}

