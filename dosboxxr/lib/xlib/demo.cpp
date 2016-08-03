
#ifdef HAVE_CONFIG_H
# include "config.h" // must be first
#endif

#include <sys/mman.h>

#include <sys/ipc.h>
#include <sys/shm.h>

#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XShm.h>

#include <algorithm>

#include "dosboxxr/lib/util/rgbinfo.h"
#include "dosboxxr/lib/util/rgb_bitmap_info.h"
#include "dosboxxr/lib/util/rgb_bitmap_test_pattern_gradients.h"
#include "dosboxxr/lib/xlib/demo.h"

XlibDemo::XlibDemo() {
    display = NULL;
    visual = NULL;
    screen = NULL;
    depth = 0;
    got_event = 0;
    wmDelete = 0;
    cmap = 0;
    redraw = 0;
    update = 0;
    memset(&swa,0,sizeof(swa));
    memset(&gwa,0,sizeof(gwa));
    gc = 0;
    image = NULL;
    memset(&shminfo,0,sizeof(shminfo));
    root_window = (Window)0;
    window = (Window)0;
    screen_index = -1;
    using_shm = false;
    quit = false;

    enable_shm = true;
}

XlibDemo::~XlibDemo() {
    exit();
}

bool XlibDemo::is_idle(void) {
    return (!redraw && !update && !quit);
}

void XlibDemo::update_window_attributes(void) {
	XGetWindowAttributes(display, window, &gwa);
}

int XlibDemo::init_xbitmap_common_pre(unsigned int width,unsigned int height,unsigned int align,unsigned int &alloc_width) {
	close_bitmap();
    if (width == 0 || height == 0) return 0;

    alloc_width = width;
    if (align > 1) {
        alloc_width += align - 1;
        alloc_width -= alloc_width % align;
    }

	if ((gc=XCreateGC(display, (Drawable)window, 0, NULL)) == NULL) {
		fprintf(stderr,"Cannot create drawable\n");
		close_bitmap();
		return 0;
	}

    return 1;
}

int XlibDemo::init_shm(unsigned int width,unsigned int height,unsigned int align) {
    unsigned int alloc_width;

    if (!enable_shm)
        return 0;

    if (!init_xbitmap_common_pre(width,height,align,/*&*/alloc_width))
        return 0;

	using_shm = 1;
	memset(&shminfo,0,sizeof(shminfo));
	if ((image=XShmCreateImage(display, visual, depth, ZPixmap, NULL, &shminfo, alloc_width, height)) == NULL) {
		fprintf(stderr,"Cannot create SHM image\n");
		close_bitmap();
		return 0;
	}

    bitmap.clear();
    bitmap.width = width;
    bitmap.height = height;
    bitmap.stride = image->bytes_per_line;
    bitmap.bytes_per_pixel = (image->bits_per_pixel + 7) / 8;
    bitmap.update_length_from_stride_and_height();
    if (!bitmap.is_dim_valid()) {
		close_bitmap();
        return 0;
    }

	if ((shminfo.shmid=shmget(IPC_PRIVATE, bitmap.length, IPC_CREAT | 0777)) < 0) {
		fprintf(stderr,"Cannot get SHM ID for image\n");
		shminfo.shmid = 0;
		close_bitmap();
		return 0;
	}

	if ((shminfo.shmaddr=image->data=(char*)shmat(shminfo.shmid, 0, 0)) == MAP_FAILED) {
		fprintf(stderr,"Cannot mmap for image\n");
		shminfo.shmaddr = NULL;
		image->data = NULL;
		close_bitmap();
		return 0;
	}
	shminfo.readOnly = 0;
    bitmap.base = (unsigned char*)image->data;
    bitmap.canvas = (unsigned char*)image->data;

    XShmAttach(display, &shminfo);
	XSync(display, 0);

    bitmap.rgbinfo.r.setByMask(image->red_mask);
    bitmap.rgbinfo.g.setByMask(image->green_mask);
    bitmap.rgbinfo.b.setByMask(image->blue_mask);
    bitmap.rgbinfo.a.setByMask(~(image->red_mask+image->green_mask+image->blue_mask)); // alpha = anything not covered by R,G,B
	return 1;
}

int XlibDemo::init_norm(unsigned int width,unsigned int height,unsigned int align) {
    unsigned int alloc_width;

    if (!init_xbitmap_common_pre(width,height,align,/*&*/alloc_width))
        return 0;

	using_shm = 0;
	if ((image=XCreateImage(display, visual, depth, ZPixmap, 0, NULL, alloc_width, height, 32, 0)) == NULL) {
		fprintf(stderr,"Cannot create image\n");
		close_bitmap();
		return 0;
	}

    bitmap.clear();
    bitmap.width = width;
    bitmap.height = height;
    bitmap.stride = image->bytes_per_line;
    bitmap.bytes_per_pixel = (image->bits_per_pixel + 7) / 8;
    bitmap.update_length_from_stride_and_height();
    if (!bitmap.is_dim_valid()) {
		close_bitmap();
        return 0;
    }

	if ((image->data=(char*)mmap(NULL,bitmap.length,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0)) == MAP_FAILED) {
		fprintf(stderr,"Cannot mmap for image\n");
		image->data = NULL;
		close_bitmap();
		return 0;
	}
    bitmap.base = (unsigned char*)image->data;
    bitmap.canvas = (unsigned char*)image->data;

    bitmap.rgbinfo.r.setByMask(image->red_mask);
    bitmap.rgbinfo.g.setByMask(image->green_mask);
    bitmap.rgbinfo.b.setByMask(image->blue_mask);
    bitmap.rgbinfo.a.setByMask(~(image->red_mask+image->green_mask+image->blue_mask)); // alpha = anything not covered by R,G,B
	return 1;
}

void XlibDemo::close_bitmap(void) {
	if (shminfo.shmid != 0) {
		XShmDetach(display, &shminfo);
		shmctl(shminfo.shmid, IPC_RMID, 0); /* the buffer will persist until X closes it */
	}
	if (image != NULL) {
		if (using_shm) {
			if (image->data != NULL) {
				shmdt(image->data);
				image->data = NULL;
			}
		}
		else {
			if (image->data != NULL) {
				munmap(image->data, bitmap.length);
				image->data = NULL;
			}
		}
		XDestroyImage(image);
		image = NULL;
	}
	if (gc != NULL) {
		XFreeGC(display, gc);
		gc = NULL;
	}
	shminfo.shmid = 0;
    bitmap.clear();
	using_shm = 0;
}

int XlibDemo::set_bitmap_size(void) {
    return set_bitmap_size(gwa.width,gwa.height);
}

int XlibDemo::set_bitmap_size(unsigned int width,unsigned int height) {
    if (width == bitmap.width && height == bitmap.height && bitmap.is_valid())
        return 1;

    close_bitmap();
    if (width == 0 || height == 0)
        return 0;

    if (!init_shm(width,height)) {
        if (!init_norm(width,height))
            return 0;
    }

    need_redraw();
    need_update();

    assert(bitmap.is_valid());
    return 1;
}

void XlibDemo::dump_rgba_format(void) {
    fprintf(stderr,"R/G/B/A shift/width/mask %u/%u/0x%X %u/%u/0x%X %u/%u/0x%X %u/%u/0x%X using_shm=%u\n",
            bitmap.rgbinfo.r.shift, bitmap.rgbinfo.r.bwidth, bitmap.rgbinfo.r.bmask,
            bitmap.rgbinfo.g.shift, bitmap.rgbinfo.g.bwidth, bitmap.rgbinfo.g.bmask,
            bitmap.rgbinfo.b.shift, bitmap.rgbinfo.b.bwidth, bitmap.rgbinfo.b.bmask,
            bitmap.rgbinfo.a.shift, bitmap.rgbinfo.a.bwidth, bitmap.rgbinfo.a.bmask,
            using_shm);
}

void XlibDemo::update_to_X11(void) {
	if (image == NULL || bitmap.width == 0 || bitmap.height == 0)
		return;

	if (using_shm)
		XShmPutImage(display, window, gc, image, 0, 0, 0, 0, bitmap.width, bitmap.height, 0);
	else
		XPutImage(display, window, gc, image, 0, 0, 0, 0, bitmap.width, bitmap.height);
}

int XlibDemo::init(void) {
    if (display != NULL)
        return 1;

    if ((display=XOpenDisplay(NULL)) == NULL) {
        if ((display=XOpenDisplay(":0")) == NULL) {
            fprintf(stderr,"Unable to open X display\n");
            exit();
            return 0;
        }
    }

    root_window = DefaultRootWindow(display);
    screen_index = DefaultScreen(display);
    screen = XScreenOfDisplay(display,screen_index);
    depth = DefaultDepthOfScreen(screen);

    fprintf(stderr,"Root window: id=%lu depth=%lu screenindex=%d\n",
            (unsigned long)root_window,
            (unsigned long)depth,
            screen_index);

	if ((visual=DefaultVisualOfScreen(screen)) == NULL) {
		fprintf(stderr,"Cannot get default visual\n");
        exit();
		return 1;
	}

	cmap = XCreateColormap(display, root_window, visual, AllocNone);

	swa.colormap = cmap;
	swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask; /* send us expose events, keypress/keyrelease events */

	window = XCreateWindow(display, root_window, 0, 0, 640, 480, 0, depth, InputOutput, visual, CWEventMask, &swa);

	XMapWindow(display, window);
	XMoveWindow(display, window, 0, 0);
	XStoreName(display, window, "Hello world");
    update_window_attributes();

	/* we want to respond to WM_DELETE_WINDOW */
	wmDelete = XInternAtom(display,"WM_DELETE_WINDOW", True);
    if (wmDelete != None) XSetWMProtocols(display, window, &wmDelete, 1); /* please send us WM_DELETE_WINDOW event */

    return 1;
}

void XlibDemo::exit(void) {
    if (display != NULL) {
        close_bitmap();
        XDestroyWindow(display,window);
        XCloseDisplay(display);
        display = NULL;
    }
}

bool XlibDemo::common_handle_events(void) {
    if (is_idle() || XPending(display)) {
        XNextEvent(display, &event);
        got_event = true;

        if (event.type == Expose) {
            update_window_attributes();
            need_update();
            if (!set_bitmap_size()) {
                set_quit();
                fprintf(stderr,"Cannot alloc bitmap\n");
                return true;
            }
        }
        else if (event.type == KeyPress) {
            char buffer[80];
            KeySym sym=0;

            XLookupString(&event.xkey, buffer, sizeof(buffer), &sym, NULL);

            if (sym == XK_Escape) {
                fprintf(stderr,"Exit, by ESC\n");
                set_quit();
                return true;
            }
        }
        else if (event.type == ClientMessage) {
            if (wmDelete != None && (Atom)event.xclient.data.l[0] == wmDelete) {
                fprintf(stderr,"Exit, by window close\n");
                set_quit();
                return true;
            }
        }
    }

    return false;
}

