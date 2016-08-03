
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

class XlibDemo {
    public:
        XlibDemo();
        ~XlibDemo();
    public:
        void                    close_bitmap(void);
        void                    update_to_X11(void);
        int                     set_bitmap_size(void);
        bool                    common_handle_events(void);
        int                     set_bitmap_size(unsigned int width,unsigned int height);
        void                    update_window_attributes(void);
        void                    dump_rgba_format(void);
        bool                    is_idle(void);
        int                     init(void);
        void                    exit(void);
    private:
        int                     init_shm(unsigned int width,unsigned int height,unsigned int align=32);
        int                     init_norm(unsigned int width,unsigned int height,unsigned int align=32);
        int                     init_xbitmap_common_pre(unsigned int width,unsigned int height,unsigned int align,unsigned int &alloc_width);
    public:
        inline void need_redraw(void) {
            redraw = 1;
        }
        inline void need_update(void) {
            update = 1;
        }
        inline void set_quit(void) {
            quit = 1;
        }
    public:
        Display*                display;
        Visual*                 visual;
        Screen*                 screen;
        int                     depth;
        Atom                    wmDelete;
        XEvent                  event;
        Colormap                cmap;
        XSetWindowAttributes    swa;
        XWindowAttributes       gwa;
        GC                      gc;
        XImage*                 image;
        XShmSegmentInfo         shminfo;
        Window                  root_window;
        Window                  window;
        int                     screen_index;
        rgb_bitmap_info         bitmap;
        bool                    using_shm;
        bool                    got_event;
        bool                    redraw;
        bool                    update;
        bool                    quit;
    public: /* options */
        bool                    enable_shm;
};

