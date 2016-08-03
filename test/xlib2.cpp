
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

#include "dosboxxr/lib/hostcpudetect/caps.h"
#include "dosboxxr/lib/util/nr_wfpack.h"
#include "dosboxxr/lib/util/bitscan.h"
#include "dosboxxr/lib/util/rgbinfo.h"
#include "dosboxxr/lib/util/rgb_bitmap_info.h"
#include "dosboxxr/lib/util/rgb_bitmap_test_patterns.h"
#include "dosboxxr/lib/util/vinterp_tmp.h"
#include "dosboxxr/lib/graphics/stretchblt_neighbor.h"
#include "dosboxxr/lib/graphics/stretchblt_bilinear.h"
#include "dosboxxr/lib/graphics/stretchblt_bilinear_mmx.h"
#include "dosboxxr/lib/graphics/stretchblt_bilinear_sse.h"
#include "dosboxxr/lib/graphics/stretchblt_bilinear_avx.h"
#include "dosboxxr/lib/graphics/stretchblt_bilinear_arm_neon.h"

int                             method = 0;

Display*                        x_display = NULL;
Visual*                         x_visual = NULL;
Screen*                         x_screen = NULL;
int                             x_depth = 0;
Atom                            x_wmDelete;
XEvent                          x_event;
Colormap                        x_cmap;
XSetWindowAttributes            x_swa;
XWindowAttributes               x_gwa;
GC                              x_gc = 0;
XImage*                         x_image = NULL;
XShmSegmentInfo                 x_shminfo;
int                             x_using_shm = 0;
Window                          x_root_window;
Window                          x_window = (Window)0;
int                             x_screen_index = 0;
int                             x_quit = 0;
rgb_bitmap_info                 x_bitmap;
rgb_bitmap_info                 src_bitmap;

void close_bitmap();

int init_xbitmap_common_pre(unsigned int width,unsigned int height,unsigned int align,unsigned int &alloc_width) {
	close_bitmap();
    if (width == 0 || height == 0) return 0;

    alloc_width = width;
    if (align > 1) {
        alloc_width += align - 1;
        alloc_width -= alloc_width % align;
    }

	if ((x_gc=XCreateGC(x_display, (Drawable)x_window, 0, NULL)) == NULL) {
		fprintf(stderr,"Cannot create drawable\n");
		close_bitmap();
		return 0;
	}

    return 1;
}

int init_shm(unsigned int width,unsigned int height,unsigned int align=32) {
    unsigned int alloc_width;

    if (!init_xbitmap_common_pre(width,height,align,/*&*/alloc_width))
        return 0;

	x_using_shm = 1;
	memset(&x_shminfo,0,sizeof(x_shminfo));
	if ((x_image=XShmCreateImage(x_display, x_visual, x_depth, ZPixmap, NULL, &x_shminfo, alloc_width, height)) == NULL) {
		fprintf(stderr,"Cannot create SHM image\n");
		close_bitmap();
		return 0;
	}

    x_bitmap.clear();
    x_bitmap.width = width;
    x_bitmap.height = height;
    x_bitmap.stride = x_image->bytes_per_line;
    x_bitmap.bytes_per_pixel = (x_image->bits_per_pixel + 7) / 8;
    x_bitmap.update_length_from_stride_and_height();
    if (!x_bitmap.is_dim_valid()) {
		close_bitmap();
        return 0;
    }

	if ((x_shminfo.shmid=shmget(IPC_PRIVATE, x_bitmap.length, IPC_CREAT | 0777)) < 0) {
		fprintf(stderr,"Cannot get SHM ID for image\n");
		x_shminfo.shmid = 0;
		close_bitmap();
		return 0;
	}

	if ((x_shminfo.shmaddr=x_image->data=(char*)shmat(x_shminfo.shmid, 0, 0)) == MAP_FAILED) {
		fprintf(stderr,"Cannot mmap for image\n");
		x_shminfo.shmaddr = NULL;
		x_image->data = NULL;
		close_bitmap();
		return 0;
	}
	x_shminfo.readOnly = 0;
    x_bitmap.base = (unsigned char*)x_image->data;
    x_bitmap.canvas = (unsigned char*)x_image->data;

    XShmAttach(x_display, &x_shminfo);
	XSync(x_display, 0);

    x_bitmap.rgbinfo.r.setByMask(x_image->red_mask);
    x_bitmap.rgbinfo.g.setByMask(x_image->green_mask);
    x_bitmap.rgbinfo.b.setByMask(x_image->blue_mask);
    x_bitmap.rgbinfo.a.setByMask(~(x_image->red_mask+x_image->green_mask+x_image->blue_mask)); // alpha = anything not covered by R,G,B
	return 1;
}

int init_norm(unsigned int width,unsigned int height,unsigned int align=32) {
    unsigned int alloc_width;

    if (!init_xbitmap_common_pre(width,height,align,/*&*/alloc_width))
        return 0;

	x_using_shm = 0;
	if ((x_image=XCreateImage(x_display, x_visual, x_depth, ZPixmap, 0, NULL, alloc_width, height, 32, 0)) == NULL) {
		fprintf(stderr,"Cannot create image\n");
		close_bitmap();
		return 0;
	}

    x_bitmap.clear();
    x_bitmap.width = width;
    x_bitmap.height = height;
    x_bitmap.stride = x_image->bytes_per_line;
    x_bitmap.bytes_per_pixel = (x_image->bits_per_pixel + 7) / 8;
    x_bitmap.update_length_from_stride_and_height();
    if (!x_bitmap.is_dim_valid()) {
		close_bitmap();
        return 0;
    }

	if ((x_image->data=(char*)mmap(NULL,x_bitmap.length,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0)) == MAP_FAILED) {
		fprintf(stderr,"Cannot mmap for image\n");
		x_image->data = NULL;
		close_bitmap();
		return 0;
	}
    x_bitmap.base = (unsigned char*)x_image->data;
    x_bitmap.canvas = (unsigned char*)x_image->data;

    x_bitmap.rgbinfo.r.setByMask(x_image->red_mask);
    x_bitmap.rgbinfo.g.setByMask(x_image->green_mask);
    x_bitmap.rgbinfo.b.setByMask(x_image->blue_mask);
    x_bitmap.rgbinfo.a.setByMask(~(x_image->red_mask+x_image->green_mask+x_image->blue_mask)); // alpha = anything not covered by R,G,B
	return 1;
}

void close_bitmap() {
	if (x_shminfo.shmid != 0) {
		XShmDetach(x_display, &x_shminfo);
		shmctl(x_shminfo.shmid, IPC_RMID, 0); /* the buffer will persist until X closes it */
	}
	if (x_image != NULL) {
		if (x_image) {
			if (x_image->data != NULL) {
				shmdt(x_image->data);
                x_bitmap.base = x_bitmap.canvas = NULL;
				x_image->data = NULL;
			}
		}
		else {
			if (x_image->data != NULL) {
				munmap(x_image->data, x_bitmap.length);
                x_bitmap.base = x_bitmap.canvas = NULL;
				x_image->data = NULL;
			}
		}
		XDestroyImage(x_image);
		x_image = NULL;
	}
	if (x_gc != NULL) {
		XFreeGC(x_display, x_gc);
		x_gc = NULL;
	}
	x_shminfo.shmid = 0;
    x_bitmap.clear();
	x_using_shm = 0;
}

void update_to_X11() {
	if (x_image == NULL || x_bitmap.width == 0 || x_bitmap.height == 0)
		return;

	if (x_using_shm)
		XShmPutImage(x_display, x_window, x_gc, x_image, 0, 0, 0, 0, x_bitmap.width, x_bitmap.height, 0);
	else
		XPutImage(x_display, x_window, x_gc, x_image, 0, 0, 0, 0, x_bitmap.width, x_bitmap.height);
}

struct render_mode {
    const char*         name;
    void                (*render)(const rgb_bitmap_info &dbmp,const rgb_bitmap_info &sbmp);
    bool                (*can_do)(const rgb_bitmap_info &dbmp,const rgb_bitmap_info &sbmp);
};

struct render_mode render_modes[] = {
    {"bilinear",
    stretchblt_bilinear,
    stretchblt_bilinear_can_do},
#if HAVE_CPU_MMX
    {"bilinear_mmx",
    stretchblt_bilinear_mmx,
    stretchblt_bilinear_mmx_can_do},
#endif
#if HAVE_CPU_SSE2
    {"bilinear_sse",
    stretchblt_bilinear_sse,
    stretchblt_bilinear_sse_can_do},
#endif
#if HAVE_CPU_AVX2
    {"bilinear_avx",
    stretchblt_bilinear_avx,
    stretchblt_bilinear_avx_can_do},
#endif
#if HAVE_CPU_ARM_NEON
    {"bilinear_arm_neon",
    stretchblt_bilinear_arm_neon,
    stretchblt_bilinear_arm_neon_can_do},
#endif
    {"neighbor",
    stretchblt_neighbor,
    stretchblt_neighbor_can_do}
};

static inline size_t render_mode_count() {
    return sizeof(render_modes)/sizeof(render_modes[0]);
}

void rerender_out() {
    if (method <= render_mode_count())
        render_modes[method].render(/*&*/x_bitmap,/*&*/src_bitmap);
}

int main() {
	int redraw = 1;

    hostCPUcaps.detect();

	memset(&x_shminfo,0,sizeof(x_shminfo));

	/* WARNING: There are a LOT of steps involved at this level of X-windows Xlib interaction! */

	if ((x_display=XOpenDisplay(NULL)) == NULL) {
		if ((x_display=XOpenDisplay(":0")) == NULL) {
			fprintf(stderr,"Unable to open X display\n");
			return 1;
		}
	}

	x_root_window = DefaultRootWindow(x_display);
	x_screen_index = DefaultScreen(x_display);
	x_screen = XScreenOfDisplay(x_display,x_screen_index);
	x_depth = DefaultDepthOfScreen(x_screen);

	fprintf(stderr,"Root window: id=%lu depth=%lu screenindex=%d\n",
		(unsigned long)x_root_window,
		(unsigned long)x_depth,
		x_screen_index);

	/* we want to respond to WM_DELETE_WINDOW */
	x_wmDelete = XInternAtom(x_display,"WM_DELETE_WINDOW", True);

	if ((x_visual=DefaultVisualOfScreen(x_screen)) == NULL) {
		fprintf(stderr,"Cannot get default visual\n");
		return 1;
	}

	x_cmap = XCreateColormap(x_display, x_root_window, x_visual, AllocNone);

	x_swa.colormap = x_cmap;
	x_swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask; /* send us expose events, keypress/keyrelease events */

	x_window = XCreateWindow(x_display, x_root_window, 0, 0, 640, 480, 0, x_depth, InputOutput, x_visual, CWEventMask, &x_swa);

	XMapWindow(x_display, x_window);
	XMoveWindow(x_display, x_window, 0, 0);
	XStoreName(x_display, x_window, "Hello world");

	XSetWMProtocols(x_display, x_window, &x_wmDelete, 1); /* please send us WM_DELETE_WINDOW event */

	XGetWindowAttributes(x_display, x_window, &x_gwa);

	/* now we need an XImage. Try to use SHM (shared memory) mapping if possible, else, don't */
	if (!init_shm(x_gwa.width,x_gwa.height)) {
		if (!init_norm(x_gwa.width,x_gwa.height)) {
			fprintf(stderr,"Cannot alloc bitmap\n");
			return 1;
		}
	}

    assert(x_image != NULL);

    fprintf(stderr,"R/G/B/A shift/width/mask %u/%u/0x%X %u/%u/0x%X %u/%u/0x%X %u/%u/0x%X\n",
        x_bitmap.rgbinfo.r.shift, x_bitmap.rgbinfo.r.bwidth, x_bitmap.rgbinfo.r.bmask,
        x_bitmap.rgbinfo.g.shift, x_bitmap.rgbinfo.g.bwidth, x_bitmap.rgbinfo.g.bmask,
        x_bitmap.rgbinfo.b.shift, x_bitmap.rgbinfo.b.bwidth, x_bitmap.rgbinfo.b.bmask,
        x_bitmap.rgbinfo.a.shift, x_bitmap.rgbinfo.a.bwidth, x_bitmap.rgbinfo.a.bmask);

    /* src bitmap */
    src_bitmap.clear();
    src_bitmap.width = 640;
    src_bitmap.height = 480;
    src_bitmap.stride_align = 32;
    src_bitmap.rgbinfo = x_bitmap.rgbinfo;
    src_bitmap.bytes_per_pixel = x_bitmap.bytes_per_pixel;
    src_bitmap.update_stride_from_width();
    src_bitmap.update_length_from_stride_and_height();
#if HAVE_POSIX_MEMALIGN
    if (posix_memalign((void**)(&src_bitmap.base),src_bitmap.stride_align,src_bitmap.length)) {
        fprintf(stderr,"Cannot alloc src bitmap\n");
        return 1;
    }
#else
    // WARNING: malloc() is not necessarily safe for SSE and AVX use because it cannot guarantee alignment.
    // On Linux x86_64 malloc() seems to generally auto align to 16-byte boundaries which is OK for SSE but
    // can break AVX.
    src_bitmap.base = (unsigned char*)malloc(src_bitmap_length);
    if (src_bitmap.base == NULL) {
        fprintf(stderr,"Cannot alloc src bitmap\n");
        return 1;
    }
#endif
    src_bitmap.update_canvas_from_base();
    assert(src_bitmap.canvas != NULL);
    assert(src_bitmap.base != NULL);
    assert(src_bitmap.is_valid());

    /* make up something */
    test_pattern_1_render(/*&*/src_bitmap);

    method = 0;
    fprintf(stderr,"Method is %s\n",render_modes[method].name);
	rerender_out();

	/* wait for events */
	while (!x_quit) {
		/* you can skip XSync and go straight to XPending() if you are doing animation */
		if (XPending(x_display)) {
			XNextEvent(x_display, &x_event);

			if (x_event.type == Expose) {
				redraw = 1;

				/* this also seems to be a good way to detect window resize */
				{
					int pw = x_gwa.width;
					int ph = x_gwa.height;

					XGetWindowAttributes(x_display, x_window, &x_gwa);

					if (pw != x_gwa.width || ph != x_gwa.height) {
						close_bitmap();
						if (!init_shm(x_gwa.width,x_gwa.height)) {
							if (!init_norm(x_gwa.width,x_gwa.height)) {
								fprintf(stderr,"Cannot alloc bitmap\n");
								return 1;
							}
						}

						rerender_out();
					}
				}
			}
			else if (x_event.type == KeyPress) {
				char buffer[80];
				KeySym sym=0;

				XLookupString(&x_event.xkey, buffer, sizeof(buffer), &sym, NULL);

				if (sym == XK_Escape) {
					fprintf(stderr,"Exit, by ESC\n");
					x_quit = 1;
				}
                else if (sym == XK_space) {
                    do {
                        if ((++method) >= render_mode_count())
                            method = 0;

                        if (render_modes[method].can_do(/*&*/x_bitmap,/*&*/src_bitmap))
                            break;

                        fprintf(stderr,"Can't do %s, skipping\n",render_modes[method].name);
                    } while (1);

                    fprintf(stderr,"Switching to %s\n",render_modes[method].name);
                    rerender_out();
                    redraw = 1;
                }
			}
			else if (x_event.type == ClientMessage) {
				if ((Atom)x_event.xclient.data.l[0] == x_wmDelete) {
					fprintf(stderr,"Exit, by window close\n");
					x_quit = 1;
				}
			}
		}

		if (redraw) {
			update_to_X11();
			redraw = 0;
		}

		XSync(x_display,False);
	}

	/* also a lot involved for cleanup */
	close_bitmap();
    free(src_bitmap.base);
	XDestroyWindow(x_display,x_window);
	XCloseDisplay(x_display);
	return 0;
}

