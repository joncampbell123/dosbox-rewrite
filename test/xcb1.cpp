
#include <sys/mman.h>

#include <sys/ipc.h>
#include <sys/shm.h>

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <xcb/xcb.h>
#include <xcb/shm.h>

xcb_connection_t*		xcb_connection = NULL;
xcb_screen_t*			xcb_screen = NULL;
xcb_window_t			xcb_window = 0;
xcb_generic_event_t*		xcb_event = NULL;
const xcb_setup_t*		xcb_setup = NULL;
int				xcb_screen_num = -1;
xcb_get_geometry_reply_t*	xcb_geo = NULL;
xcb_gcontext_t			xcb_gc = 0;
xcb_format_t*			xcb_fmt = NULL;
xcb_visualtype_t*		xcb_visual = NULL;

int				bitmap_width = 0,bitmap_height = 0;
int				redraw = 1;

xcb_shm_seg_t			xcb_shm = 0;
int				bitmap_shmid = -1;
unsigned char*			bitmap = NULL;
size_t				bitmap_stride = 0;
size_t				bitmap_size = 0;

unsigned char*			image=NULL; /* 24bpp image */
size_t				image_stride;
size_t				image_width,image_height;

void free_bitmap();

void init_bitmap() {
	if (xcb_geo == NULL)
		return;
	if (bitmap_shmid >= 0)
		return;

	bitmap_width = xcb_geo->width;
	bitmap_height = xcb_geo->height;

	/* local bitmap */
	bitmap_stride = bitmap_width * xcb_fmt->bits_per_pixel;
	bitmap_stride += xcb_fmt->scanline_pad - 1;
	bitmap_stride -= bitmap_stride % xcb_fmt->scanline_pad;
	bitmap_stride >>= 3;
	bitmap_size = bitmap_stride * bitmap_height;

	fprintf(stderr,"BMP is %u x %u stride=%zu size=%zu depth=%u\n",
		bitmap_width, bitmap_height, bitmap_stride, bitmap_size, xcb_fmt->depth);

	if ((bitmap_shmid=shmget(IPC_PRIVATE, bitmap_size, IPC_CREAT | 0777)) < 0) {
		fprintf(stderr,"Cannot get SHM ID for image\n");
		free_bitmap();
		return;
	}

	if ((bitmap=(unsigned char*)shmat(bitmap_shmid, 0, 0)) == MAP_FAILED) {
		fprintf(stderr,"Cannot mmap for image\n");
		bitmap = NULL;
		free_bitmap();
		return;
	}

	xcb_shm = xcb_generate_id(xcb_connection);
	xcb_shm_attach(xcb_connection, xcb_shm, bitmap_shmid, 0);
}

void free_bitmap() {
	if (xcb_shm) {
		xcb_shm_detach(xcb_connection, xcb_shm);
		xcb_shm = 0;
	}
	if (bitmap) {
		shmdt(bitmap);
		bitmap = NULL;
		bitmap_size = 0;
	}
	if (bitmap_shmid >= 0) {
		shmctl(bitmap_shmid, IPC_RMID, 0); /* the buffer will persist until X closes it */
		bitmap_shmid = -1;
	}
}

void update_xcb_geo() {
	xcb_get_geometry_cookie_t x;

	if (xcb_geo != NULL) {
		free(xcb_geo);
		xcb_geo = NULL;
	}

	x = xcb_get_geometry(xcb_connection, xcb_window);
	xcb_geo = xcb_get_geometry_reply(xcb_connection, x, NULL);

	if (xcb_geo) {
		fprintf(stderr,"Window is now %u x %u\n",xcb_geo->width,xcb_geo->height);
	}
}

void freeall() {
	free_bitmap();
	if (xcb_geo != NULL) {
		free(xcb_geo);
		xcb_geo = NULL;
	}
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

#if 0
		if (xcb_fmt->bits_per_pixel == 32) {
			if (xcb_visual->blue_mask == 0x000000FF) {/*most common, ARGB*/
#endif

    if (xcb_fmt->bits_per_pixel == 32) {
        const uint32_t alpha =
            (~(xcb_visual->red_mask+xcb_visual->green_mask+xcb_visual->blue_mask));
        uint32_t rm,gm,bm;
        uint8_t rs,gs,bs;
        uint32_t r,g,b;
        uint32_t *drow;

        rs = bitscan_forward(xcb_visual->red_mask,0);
        rm = bitscan_count(xcb_visual->red_mask,rs) - rs;
        rm = (1U << rm) - 1U;

        gs = bitscan_forward(xcb_visual->green_mask,0);
        gm = bitscan_count(xcb_visual->green_mask,gs) - gs;
        gm = (1U << gm) - 1U;

        bs = bitscan_forward(xcb_visual->blue_mask,0);
        bm = bitscan_count(xcb_visual->blue_mask,bs) - bs;
        bm = (1U << bm) - 1U;

        fprintf(stderr,"R/G/B shift/mask %u/0x%X %u/0x%X %u/0x%X\n",rs,rm,gs,gm,bs,bm);

        for (oy=0;oy < bitmap_height;oy++) {
            drow = (uint32_t*)((uint8_t*)bitmap + (bitmap_stride * oy));
            for (ox=0;ox < bitmap_width;ox++) {
                /* color */
                r = (ox * rm) / bitmap_width;
                g = (oy * gm) / bitmap_height;
                b = bm - ((ox * bm) / bitmap_width);

                *drow++ = (r << rs) + (g << gs) + (b << bs) + alpha;
            }
        }
    }
    else if (xcb_fmt->bits_per_pixel == 16) {
        const uint16_t alpha =
            (~(xcb_visual->red_mask+xcb_visual->green_mask+xcb_visual->blue_mask));
        uint16_t rm,gm,bm;
        uint8_t rs,gs,bs;
        uint16_t r,g,b;
        uint16_t *drow;

        rs = bitscan_forward(xcb_visual->red_mask,0);
        rm = bitscan_count(xcb_visual->red_mask,rs) - rs;
        rm = (1U << rm) - 1U;

        gs = bitscan_forward(xcb_visual->green_mask,0);
        gm = bitscan_count(xcb_visual->green_mask,gs) - gs;
        gm = (1U << gm) - 1U;

        bs = bitscan_forward(xcb_visual->blue_mask,0);
        bm = bitscan_count(xcb_visual->blue_mask,bs) - bs;
        bm = (1U << bm) - 1U;

        fprintf(stderr,"R/G/B shift/mask %u/0x%X %u/0x%X %u/0x%X\n",rs,rm,gs,gm,bs,bm);

        for (oy=0;oy < bitmap_height;oy++) {
            drow = (uint16_t*)((uint8_t*)bitmap + (bitmap_stride * oy));
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
            xcb_fmt->bits_per_pixel);
    }
}

int main() {
	/* WARNING: a lot is involved at the XCB low level interface */

	if ((xcb_connection=xcb_connect(NULL, &xcb_screen_num)) == NULL) {
		if ((xcb_connection=xcb_connect(":0", &xcb_screen_num)) == NULL) {
			fprintf(stderr,"Unable to create xcb connection\n");
			return 1;
		}
	}

	if ((xcb_setup=xcb_get_setup(xcb_connection)) == NULL) { /* "the result must not be freed" */
		fprintf(stderr,"Cannot get XCB setup\n");
		return 1;
	}

	if ((xcb_screen=(xcb_setup_roots_iterator(xcb_setup)).data) == NULL) {
		fprintf(stderr,"Cannot get XCB screen\n");
		return 1;
	}

	/* dump the values. we're guessing with this shit. nobody fucking documents what the fuck "bitmap_format_scanline_unit" means. >:( */
	fprintf(stderr,"xcb_screen:\n");
	fprintf(stderr,"   bitmap_format_scanline_unit = %u\n",xcb_setup->bitmap_format_scanline_unit);
	fprintf(stderr,"   bitmap_format_scanline_pad = %u\n",xcb_setup->bitmap_format_scanline_pad);

	/* we need the visual to tell us how the RGB is formatted */
	{
		xcb_depth_iterator_t depth_iter = xcb_screen_allowed_depths_iterator(xcb_screen);

		for (;depth_iter.rem;xcb_depth_next(&depth_iter)) {
			xcb_visualtype_iterator_t visual_iter = xcb_depth_visuals_iterator(depth_iter.data);

			for (;visual_iter.rem;xcb_visualtype_next(&visual_iter)) {
				if (xcb_screen->root_visual == visual_iter.data->visual_id) {
					xcb_visual = visual_iter.data;
					break;
				}
			}
		}

		if (xcb_visual == NULL) {
			fprintf(stderr,"Unable to determine visual\n");
			return 1;
		}
	}

	/* pick a format */
	{
		xcb_format_t *f = xcb_setup_pixmap_formats(xcb_setup);
		xcb_format_t *fend = f + xcb_setup_pixmap_formats_length(xcb_setup); /* !!! Wait, is the "length" in entries or in bytes?? xcb_image devs don't remember C/C++ pointer math type rules?? */
		for (;f < fend;f++) {
			fprintf(stderr,"  format bpp=%u scanline_pad=%u depth=%u\n",
				f->bits_per_pixel,
				f->scanline_pad,
				f->depth);

			/* despite all the formats listed, only the same depth as the screen will work. */
			/* note that even for modern screens you'll see entries listed like "depth=24 bpp=32 scanline_pad=32".
			 * you'll also see an entry for depth=32. don't use it. it doesn't work (not that I could accomplish anyway). */
			if (f->depth == xcb_screen->root_depth)
				xcb_fmt = f;
		}
	}

	xcb_window = xcb_generate_id(xcb_connection);

	{
		uint32_t mask;
		uint32_t values[2];

		mask = XCB_CW_EVENT_MASK;
		values[0] =	XCB_EVENT_MASK_EXPOSURE       | XCB_EVENT_MASK_BUTTON_PRESS   |
				XCB_EVENT_MASK_BUTTON_RELEASE | XCB_EVENT_MASK_POINTER_MOTION |
				XCB_EVENT_MASK_ENTER_WINDOW   | XCB_EVENT_MASK_LEAVE_WINDOW   |
				XCB_EVENT_MASK_KEY_PRESS      | XCB_EVENT_MASK_KEY_RELEASE;
		values[1] = 0;

		xcb_create_window(xcb_connection, /*depth*/0, /*window id*/xcb_window, /*parent window*/xcb_screen->root, /*x*/0, /*y*/ 0,
			640, 480, /*border width*/10, XCB_WINDOW_CLASS_INPUT_OUTPUT, xcb_screen->root_visual, mask, values);
	}

	{
		const char *title = "Hello world";

		xcb_change_property(xcb_connection, XCB_PROP_MODE_REPLACE, xcb_window,
			XCB_ATOM_WM_NAME, XCB_ATOM_STRING, /*format=*/8/*<-FIXME: What does this mean?? 8 bytes/char?*/,
			strlen(title), title);
	}

	xcb_map_window(xcb_connection, xcb_window);
	xcb_flush(xcb_connection);
	update_xcb_geo();

	{
		uint32_t mask;
		uint32_t values[3];

		mask = XCB_GC_FOREGROUND | XCB_GC_BACKGROUND;
		values[0] = xcb_screen->black_pixel;
		values[1] = xcb_screen->white_pixel;
		values[2] = 0;

		xcb_gc = xcb_generate_id(xcb_connection);
		xcb_create_gc(xcb_connection, xcb_gc, xcb_window, mask, values);
	}

	init_bitmap();
	rerender_out();
	redraw = 1;

	/* use xcb_poll_event() if you want to do animation at the same time */
	while ((xcb_event=xcb_wait_for_event(xcb_connection)) != NULL) {
		/* FIXME: can anyone explain why all the XCB samples recommend checking not xcb_event->response_type, but
		 *        instead (xcb_event->response_type & ~0x80)? */
		unsigned char etype = xcb_event->response_type & ~0x80;

		if (etype == XCB_EXPOSE) {
			int pw = xcb_geo->width;
			int ph = xcb_geo->height;

			fprintf(stderr,"Expose event\n");

			update_xcb_geo();

			if (pw != xcb_geo->width || ph != xcb_geo->height) {
				free_bitmap();
				init_bitmap();
				rerender_out();
				redraw = 1;
			}
		}
		else if (etype == XCB_KEY_PRESS) {
			xcb_key_press_event_t *ev = (xcb_key_press_event_t*)xcb_event;

			/* Okay, here's my biggest gripe about the XCB library. The keyboard events have a vague "detail"
			 * constant that represents the keyboard key, and NOBODY FUCKING DOCUMENTS WHAT IT MEANS OR HOW
			 * TO MAKE SENSE OF IT. That's BAD. Really BAD. Not even Microsoft Windows is that bad for something
			 * so fundamental and basic. For fucks sake MS-DOS and Windows 3.0 had keyboard constants you could
			 * make sense of keyboard input from and that's considered ancient unstable crashy shit in comparison
			 * to Linux/X11. And let me tell you XCB library developers something: A lot of the weird backwards
			 * compat shit that Microsoft had to support in Windows came from such an environment of guessing and
			 * poor documentation. Do you want to maintain a library that must support the guesses of thousands
			 * of developers forever more? Do you want the hell of having to maintain stupid code layouts and
			 * weird values because your poor documentation and lack of constants forced developers to hardcode
			 * key codes? No? Then get your shit straight. */
			if (ev->detail == 9/*FIXME: Where are the keyboard scan code defines???*/) {
				fprintf(stderr,"KEY_PRESS/escape\n");
				free(xcb_event);
				xcb_event = NULL;
				break;
			}
		}
		else if (etype == 0) {
			/* FIXME: Is this right?? Do I just typecast as xcb_generic_error_t?? What does error->error_code mean??? Is it the same as the
			 * error codes in X11? Is error code 8 == BadMatch or does XCB have it's own error codes???
			 * Nobody fucking documents this shit. So as far as I know, this is how you read back error codes. */
			xcb_generic_error_t *error = (xcb_generic_error_t*)xcb_event;
			fprintf(stderr,"Error err=%u major=%u minor=%u\n",error->error_code,
				error->major_code,error->minor_code);
		}

		free(xcb_event);
		xcb_event = NULL;

		if (redraw) {
			fprintf(stderr,"Redraw\n");
			if (bitmap_shmid >= 0) {
				/* FIXME: Z_PIXMAP?!?!?? Why not XY_PIXMAP???
				 *        "Z_PIXMAP" leads me to believe it's something to do with a Z buffer!
				 *        If you try XY_PIXMAP the function will just render it like a 1-bit monochromatic
				 *        bitmap! What fucking sense does that make?!?!?!?!? AAARRRRRGH!
				 *        It doesn't help that NONE of this shit is documented! */
				xcb_shm_put_image(xcb_connection, xcb_window, xcb_gc,
					/*total width*/bitmap_width, /*total height*/bitmap_height, /*src x*/0, /*src y*/0,
					/*src width*/bitmap_width, /*src height*/bitmap_height, /*dst x*/0, /*dst y*/0,
					xcb_fmt->depth, XCB_IMAGE_FORMAT_Z_PIXMAP, /*send event (image write is complete)*/0, xcb_shm, /*offset*/0);
				xcb_flush(xcb_connection);
			}

			redraw = 0;
		}
	}

	freeall();
	free(image);
	xcb_free_gc(xcb_connection, xcb_gc);
	xcb_disconnect(xcb_connection);
	return 0;
}

