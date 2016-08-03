
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

static XlibDemo                 demo;

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

    if (!demo.init())
        return 1;
    if (!demo.set_bitmap_size())
        return 1;

    demo.dump_rgba_format();

	/* wait for events */
	while (!demo.quit) {
        demo.common_handle_events();
        if (demo.quit) break;

		if (demo.redraw) {
            render_test_pattern_rgb_gradients(demo.bitmap);
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
    demo.exit();
	return 0;
}

