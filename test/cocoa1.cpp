
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

NSWindow*		mainWindow = NULL;

int main(int argc,char **argv) {
	NSRect contentRect;

	contentRect = NSMakeRect(0,0,640,480);

	mainWindow = [
		[ NSWindow alloc ] initWithContentRect: contentRect /* <- method */
		styleMask: NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask /* <- param */
		backing: NSBackingStoreBuffered /* <- param */
		defer: NO /* <- param */
	];

	if (mainWindow == NULL) {
		fprintf(stderr,"Failed to make main window\n");
		return 1;
	}

	// TODO: how to show window?

	// TODO: event loop?

	// TODO: How to free window?
	return 0;
}

