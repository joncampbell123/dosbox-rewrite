
#include <sys/mman.h>

#include <sys/ipc.h>
#include <sys/shm.h>

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XShm.h>

#include <algorithm>

Display*			x_display = NULL;
Visual*				x_visual = NULL;
Screen*				x_screen = NULL;
int				x_depth = 0;
Atom				x_wmDelete;
XEvent				x_event;
Colormap			x_cmap;
XSetWindowAttributes		x_swa;
XWindowAttributes		x_gwa;
GC				x_gc = 0;
XImage*				x_image = NULL;
XShmSegmentInfo			x_shminfo;
size_t				x_image_length = 0;
int				x_using_shm = 0;
Window				x_root_window;
Window				x_window = (Window)0;
int				x_screen_index = 0;
int				x_quit = 0;
int				bitmap_width = 0;
int				bitmap_height = 0;

void close_bitmap();

int init_shm() {
	close_bitmap();

	if ((x_gc=XCreateGC(x_display, (Drawable)x_window, 0, NULL)) == NULL) {
		fprintf(stderr,"Cannot create drawable\n");
		close_bitmap();
		return 0;
	}

	x_using_shm = 1;
	memset(&x_shminfo,0,sizeof(x_shminfo));
	bitmap_width = (x_gwa.width+15)&(~15);
	bitmap_height = (x_gwa.height+15)&(~15);
	if ((x_image=XShmCreateImage(x_display, x_visual, x_depth, ZPixmap, NULL, &x_shminfo, bitmap_width, bitmap_height)) == NULL) {
		fprintf(stderr,"Cannot create SHM image\n");
		close_bitmap();
		return 0;
	}

	x_image_length = x_image->bytes_per_line * (x_image->height + 1);
	if ((x_shminfo.shmid=shmget(IPC_PRIVATE, x_image_length, IPC_CREAT | 0777)) < 0) {
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
	XShmAttach(x_display, &x_shminfo);
	XSync(x_display, 0);

	memset(x_image->data,0x00,x_image_length);

	bitmap_width = x_gwa.width;
	bitmap_height = x_gwa.height;
	fprintf(stderr,"SHM-based XImage created %ux%u\n",bitmap_width,bitmap_height);
	return 1;
}

int init_norm() {
	close_bitmap();

	if ((x_gc=XCreateGC(x_display, (Drawable)x_window, 0, NULL)) == NULL) {
		fprintf(stderr,"Cannot create drawable\n");
		close_bitmap();
		return 0;
	}

	x_using_shm = 0;
	bitmap_width = (x_gwa.width+15)&(~15);
	bitmap_height = (x_gwa.height+15)&(~15);
	if ((x_image=XCreateImage(x_display, x_visual, x_depth, ZPixmap, 0, NULL, bitmap_width, bitmap_height, 32, 0)) == NULL) {
		fprintf(stderr,"Cannot create image\n");
		close_bitmap();
		return 0;
	}

	x_image_length = x_image->bytes_per_line * (x_image->height + 1);
	if ((x_image->data=(char*)mmap(NULL,x_image_length,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0)) == MAP_FAILED) {
		fprintf(stderr,"Cannot mmap for image\n");
		x_image->data = NULL;
		close_bitmap();
		return 0;
	}

	memset(x_image->data,0x00,x_image_length);

	bitmap_width = x_gwa.width;
	bitmap_height = x_gwa.height;
	fprintf(stderr,"Normal (non-SHM) XImage created %ux%u\n",bitmap_width,bitmap_height);
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
				x_image->data = NULL;
				x_image_length = 0;
			}
		}
		else {
			if (x_image->data != NULL) {
				munmap(x_image->data, x_image_length);
				x_image->data = NULL;
				x_image_length = 0;
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
	x_using_shm = 0;
}

void update_to_X11() {
	if (x_image == NULL || bitmap_width == 0 || bitmap_height == 0)
		return;

	if (x_using_shm)
		XShmPutImage(x_display, x_window, x_gc, x_image, 0, 0, 0, 0, bitmap_width, bitmap_height, 0);
	else
		XPutImage(x_display, x_window, x_gc, x_image, 0, 0, 0, 0, bitmap_width, bitmap_height);
}

unsigned int bitscan_forward(const uint32_t v,unsigned int bit) {
    while (bit < 32) {
        if (!(v & (1U << bit)))
            bit++;
        else
            return bit;
    }

    return bit;
}

unsigned int bitscan_count(const uint32_t v,unsigned int bit) {
    while (bit < 32) {
        if (v & (1U << bit))
            bit++;
        else
            return bit;
    }

    return bit;
}

/* render image to XImage.
 * stretch fit using crude nearest neighbor scaling */
void rerender_out() {
	int ox,oy;

    if (x_image->bits_per_pixel == 32) {
        const uint32_t alpha =
            (~(x_image->red_mask+x_image->green_mask+x_image->blue_mask));
        uint32_t rm,gm,bm;
        uint8_t rs,gs,bs;
        uint32_t r,g,b;
        uint32_t *drow;

        rs = bitscan_forward(x_image->red_mask,0);
        rm = bitscan_count(x_image->red_mask,rs) - rs;
        rm = (1U << rm) - 1U;

        gs = bitscan_forward(x_image->green_mask,0);
        gm = bitscan_count(x_image->green_mask,gs) - gs;
        gm = (1U << gm) - 1U;

        bs = bitscan_forward(x_image->blue_mask,0);
        bm = bitscan_count(x_image->blue_mask,bs) - bs;
        bm = (1U << bm) - 1U;

        fprintf(stderr,"R/G/B shift/mask %u/0x%X %u/0x%X %u/0x%X\n",rs,rm,gs,gm,bs,bm);

        for (oy=0;oy < bitmap_height;oy++) {
            drow = (uint32_t*)((uint8_t*)x_image->data + (x_image->bytes_per_line * oy));
            for (ox=0;ox < bitmap_width;ox++) {
                /* color */
                r = (ox * rm) / bitmap_width;
                g = (oy * gm) / bitmap_height;
                b = bm - ((ox * bm) / bitmap_width);

                *drow++ = (r << rs) + (g << gs) + (b << bs) + alpha;
            }
        }
    }
    else if (x_image->bits_per_pixel == 16) {
        const uint16_t alpha =
            (~(x_image->red_mask+x_image->green_mask+x_image->blue_mask));
        uint16_t rm,gm,bm;
        uint8_t rs,gs,bs;
        uint16_t r,g,b;
        uint16_t *drow;

        rs = bitscan_forward(x_image->red_mask,0);
        rm = bitscan_count(x_image->red_mask,rs) - rs;
        rm = (1U << rm) - 1U;

        gs = bitscan_forward(x_image->green_mask,0);
        gm = bitscan_count(x_image->green_mask,gs) - gs;
        gm = (1U << gm) - 1U;

        bs = bitscan_forward(x_image->blue_mask,0);
        bm = bitscan_count(x_image->blue_mask,bs) - bs;
        bm = (1U << bm) - 1U;

        fprintf(stderr,"R/G/B shift/mask %u/0x%X %u/0x%X %u/0x%X\n",rs,rm,gs,gm,bs,bm);

        for (oy=0;oy < bitmap_height;oy++) {
            drow = (uint16_t*)((uint8_t*)x_image->data + (x_image->bytes_per_line * oy));
            for (ox=0;ox < bitmap_width;ox++) {
                /* color */
                r = (ox * rm) / bitmap_width;
                g = (oy * gm) / bitmap_height;
                b = bm - ((ox * bm) / bitmap_width);

                *drow++ = (r << rs) + (g << gs) + (b << bs) + alpha;
            }
        }
    }
    else {
        fprintf(stderr,"WARNING: unsupported bit depth %u/bpp\n",
            x_image->bits_per_pixel);
    }
}

int main() {
	int redraw = 1;

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
	if (!init_shm()) {
		if (!init_norm()) {
			fprintf(stderr,"Cannot alloc bitmap\n");
			return 1;
		}
	}

	rerender_out();

	/* wait for events */
	while (!x_quit) {
		/* you can skip XSync and go straight to XPending() if you are doing animation */
		if (XPending(x_display)) {
			XNextEvent(x_display, &x_event);

			if (x_event.type == Expose) {
				fprintf(stderr,"Expose event\n");
				redraw = 1;

				/* this also seems to be a good way to detect window resize */
				{
					int pw = x_gwa.width;
					int ph = x_gwa.height;

					XGetWindowAttributes(x_display, x_window, &x_gwa);

					if (pw != x_gwa.width || ph != x_gwa.height) {
						close_bitmap();
						if (!init_shm()) {
							if (!init_norm()) {
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
	XDestroyWindow(x_display,x_window);
	XCloseDisplay(x_display);
	return 0;
}

