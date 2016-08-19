
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
#include "dosboxxr/lib/util/rgbinfo.h"
#include "dosboxxr/lib/util/rgb_bitmap_info.h"
#include "dosboxxr/lib/util/rgb_bitmap_test_patterns.h"
#include "dosboxxr/lib/util/rgb_bitmap_test_pattern_gradients.h"
#include "dosboxxr/lib/graphics/stretchblt_general.h"
#include "dosboxxr/lib/xlib/demo.h"

static rgb_bitmap_info          src_bitmap;
static XlibDemo                 demo;
static size_t                   method = 0;
static bool                     resize_src_mode = false;

static void free_src_bitmap(void);

static int init_src_bitmap(unsigned int width=640,unsigned int height=480) {
    if (src_bitmap.base != NULL)
        return 0;
    if (!demo.bitmap.is_valid())
        return 0;

    src_bitmap.clear();
    src_bitmap.base_align = 32;
    src_bitmap.stride_align = 32;
    src_bitmap.rgbinfo = demo.bitmap.rgbinfo;
    src_bitmap.bytes_per_pixel = demo.bitmap.bytes_per_pixel;
    src_bitmap.width = width;
    src_bitmap.height = height;
    src_bitmap.update_stride_from_width();
    src_bitmap.update_length_from_stride_and_height();

#if HAVE_POSIX_MEMALIGN
    if (posix_memalign((void**)(&src_bitmap.base),src_bitmap.base_align,src_bitmap.length)) {
        fprintf(stderr,"Unable to alloc src bitmap\n");
        assert(src_bitmap.base == NULL);
        free_src_bitmap();
        return 0;
    }
    assert(src_bitmap.base != NULL);
    src_bitmap.canvas = src_bitmap.base;
#else
    src_bitmap.base = malloc(src_bitmap.length + (src_bitmap.base_align*2));
    if (src_bitmap.base == NULL) {
        fprintf(stderr,"Unable to alloc src bitmap\n");
        assert(src_bitmap.base == NULL);
        free_src_bitmap();
        return 0;
    }
    src_bitmap.canvas = (unsigned char*)(((uintptr_t)src_bitmap.base + src_bitmap.base_align - 1U) & ~((uintptr_t)(src_bitmap.base_align - 1U)));
#endif

    assert(src_bitmap.is_valid());
    return 1;
}

static void free_src_bitmap(void) {
    if (src_bitmap.base != NULL) {
        free(src_bitmap.base);
        src_bitmap.base = NULL;
    }

    src_bitmap.clear();
}

static void help() {
    fprintf(stderr,"  -shm       Use SHM extensions (shared memory)\n");
    fprintf(stderr,"  -no-shm    Don't use SHM extensions\n");
}

static int parse_argv(int argc,char **argv) {
    char *a;
    int i;

    for (i=1;i < argc;) {
        a = argv[i++];

        if (*a == '-') {
            do { a++; } while (*a == '-');

            if (!strcmp(a,"h") || !strcmp(a,"help")) {
                help();
                return 1;
            }
            else if (!strcmp(a,"no-shm")) {
                demo.enable_shm = false;
            }
            else if (!strcmp(a,"shm")) {
                demo.enable_shm = true;
            }
            else {
                fprintf(stderr,"Unknown switch %s\n",a);
                return 1;
            }
        }
        else {
            fprintf(stderr,"Unhandled arg %s\n",a);
            return 1;
        }
    }

    return 0;
}

int main(int argc,char **argv) {
    if (parse_argv(argc,argv))
        return 1;

    hostCPUcaps.detect();

    if (!demo.init())
        return 1;
    if (!demo.set_bitmap_size())
        return 1;
    if (!init_src_bitmap())
        return 1;

    demo.dump_rgba_format();
    test_pattern_1_render(/*&*/src_bitmap);

    /* wait for events */
    while (!demo.quit) {
        demo.common_handle_events();
        if (demo.quit) break;

        if (demo.got_event) {
            XEvent &event = demo.event;

            if (event.type == KeyPress) {
                char buffer[80];
                KeySym sym=0;

                XLookupString(&event.xkey, buffer, sizeof(buffer), &sym, NULL);

                if (sym == XK_s) {
                    resize_src_mode = !resize_src_mode;
                    demo.need_redraw();
                }
                else if (sym == XK_space) {
                    do {
                        if ((++method) >= stretchblt_mode_count())
                            method = 0;

                        if (stretchblt_modes[method].can_do(/*&*/demo.bitmap,/*&*/src_bitmap))
                            break;
                        fprintf(stderr,"Can't do %s, skipping\n",stretchblt_modes[method].name);
                    } while (1);

                    fprintf(stderr,"Switching to %s\n",stretchblt_modes[method].name);
                    demo.need_redraw();
                }
            }
 
            demo.got_event = false;
        }

        if (demo.redraw) {
            if (resize_src_mode) {
                free_src_bitmap();
                if (!init_src_bitmap(demo.bitmap.width,demo.bitmap.height))
                    break;
                test_pattern_1_render(/*&*/src_bitmap);
            }

            if (method <= stretchblt_mode_count())
                stretchblt_modes[method].render(/*&*/demo.bitmap,/*&*/src_bitmap);

            demo.redraw = 0;
            demo.need_update();
        }
        if (demo.update) {
            demo.update_to_X11();
            demo.update = 0;
        }

        // NTS: Use XFlush() if doing animation.
        //      XSync() flushes AND waits for X server to complete.
        XSync(demo.display,False);
    }

    /* also a lot involved for cleanup */
    free_src_bitmap();
    demo.exit();
    return 0;
}

