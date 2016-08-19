// TODO: This uses XCB with SHM extension, implement alternate mode without SHM extension

#ifdef HAVE_CONFIG_H
# include "config.h" // must be first
#endif

#include <sys/mman.h>

#include <sys/ipc.h>
#include <sys/shm.h>

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <xcb/xcb.h>
#include <xcb/shm.h>

#include <algorithm>

#include "dosboxxr/lib/util/rgbinfo.h"
#include "dosboxxr/lib/util/rgb_bitmap_info.h"
#include "dosboxxr/lib/util/rgb_bitmap_test_pattern_gradients.h"

xcb_connection_t*               xcb_connection = NULL;
xcb_screen_t*                   xcb_screen = NULL;
xcb_window_t                    xcb_window = 0;
xcb_generic_event_t*            xcb_event = NULL;
const xcb_setup_t*              xcb_setup = NULL;
int                             xcb_screen_num = -1;
xcb_get_geometry_reply_t*       xcb_geo = NULL;
xcb_gcontext_t                  xcb_gc = 0;
xcb_format_t*                   xcb_fmt = NULL;
xcb_visualtype_t*               xcb_visual = NULL;
int                             xcb_bitmap_shmid = -1;
xcb_shm_seg_t                   xcb_shm = 0;
rgb_bitmap_info                 xcb_bitmap;

void free_bitmap();

void init_bitmap(unsigned int width,unsigned int height,unsigned int align=32) {
    if (xcb_geo == NULL)
        return;
    if (xcb_bitmap_shmid >= 0)
        return;

    (void)align; // unused. we should be using this though.

    xcb_bitmap.clear();
    xcb_bitmap.width = width;
    xcb_bitmap.height = height;
    xcb_bitmap.bytes_per_pixel = (xcb_fmt->bits_per_pixel + 7) / 8;

    // we must obey XCB scanline padding requirements.
    xcb_bitmap.stride = xcb_bitmap.width * xcb_fmt->bits_per_pixel; /* stride in bits */
    xcb_bitmap.stride += xcb_fmt->scanline_pad - 1;/* round up */
    xcb_bitmap.stride -= xcb_bitmap.stride % xcb_fmt->scanline_pad/*bits*/;
    xcb_bitmap.stride /= 8; /* back to bytes */

    xcb_bitmap.update_length_from_stride_and_height();

    if ((xcb_bitmap_shmid=shmget(IPC_PRIVATE, xcb_bitmap.length, IPC_CREAT | 0777)) < 0) {
        fprintf(stderr,"Cannot get SHM ID for image\n");
        free_bitmap();
        return;
    }

    if ((xcb_bitmap.base=(unsigned char*)shmat(xcb_bitmap_shmid, 0, 0)) == MAP_FAILED) {
        fprintf(stderr,"Cannot mmap for image\n");
        xcb_bitmap.base = NULL;
        free_bitmap();
        return;
    }
    xcb_bitmap.canvas = xcb_bitmap.base;

    xcb_shm = xcb_generate_id(xcb_connection);
    xcb_shm_attach(xcb_connection, xcb_shm, xcb_bitmap_shmid, 0);

    xcb_bitmap.rgbinfo.r.setByMask(xcb_visual->red_mask);
    xcb_bitmap.rgbinfo.g.setByMask(xcb_visual->green_mask);
    xcb_bitmap.rgbinfo.b.setByMask(xcb_visual->blue_mask);
    xcb_bitmap.rgbinfo.a.setByMask(~(xcb_visual->red_mask+xcb_visual->green_mask+xcb_visual->blue_mask)); // alpha = anything not covered by R,G,B
}

void free_bitmap() {
    if (xcb_shm) {
        xcb_shm_detach(xcb_connection, xcb_shm);
        xcb_shm = 0;
    }
    if (xcb_bitmap.base) {
        shmdt(xcb_bitmap.base);
        xcb_bitmap.base = NULL;
    }
    if (xcb_bitmap_shmid >= 0) {
        shmctl(xcb_bitmap_shmid, IPC_RMID, 0); /* the buffer will persist until X closes it */
        xcb_bitmap_shmid = -1;
    }
    xcb_bitmap.clear();
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

int main() {
    int redraw = 1;

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
        values[0] =
            XCB_EVENT_MASK_EXPOSURE       | XCB_EVENT_MASK_BUTTON_PRESS   |
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

    init_bitmap(xcb_geo->width,xcb_geo->height);
    render_test_pattern_rgb_gradients(xcb_bitmap);
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
                init_bitmap(xcb_geo->width,xcb_geo->height);
                render_test_pattern_rgb_gradients(xcb_bitmap);
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
            if (xcb_bitmap_shmid >= 0) {
                /* FIXME: Z_PIXMAP?!?!?? Why not XY_PIXMAP???
                 *        "Z_PIXMAP" leads me to believe it's something to do with a Z buffer!
                 *        If you try XY_PIXMAP the function will just render it like a 1-bit monochromatic
                 *        bitmap! What fucking sense does that make?!?!?!?!? AAARRRRRGH!
                 *        It doesn't help that NONE of this shit is documented! */
                xcb_shm_put_image(xcb_connection, xcb_window, xcb_gc,
                    /*total width*/xcb_bitmap.width, /*total height*/xcb_bitmap.height, /*src x*/0, /*src y*/0,
                    /*src width*/xcb_bitmap.width, /*src height*/xcb_bitmap.height, /*dst x*/0, /*dst y*/0,
                    xcb_fmt->depth, XCB_IMAGE_FORMAT_Z_PIXMAP, /*send event (image write is complete)*/0, xcb_shm, /*offset*/0);
                xcb_flush(xcb_connection);
            }

            redraw = 0;
        }
    }

    freeall();
    xcb_free_gc(xcb_connection, xcb_gc);
    xcb_disconnect(xcb_connection);
    return 0;
}

