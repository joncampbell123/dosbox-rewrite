// TODO: This uses XCB with SHM extension, implement alternate mode without SHM extension

#ifdef HAVE_CONFIG_H
# include "config.h" // must be first
#endif

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <linux/fb.h>

#include <termios.h>

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <algorithm>

#include "dosboxxr/lib/util/rgbinfo.h"
#include "dosboxxr/lib/hostcpudetect/caps.h"
#include "dosboxxr/lib/util/rgb_bitmap_info.h"
#include "dosboxxr/lib/util/rgb_bitmap_test_pattern_gradients.h"
#include "dosboxxr/lib/util/rgb_bitmap_test_patterns.h"
#include "dosboxxr/lib/graphics/stretchblt_general.h"

static rgb_bitmap_info          src_bitmap;

unsigned int                    method = 0;

int                             fb_fd = -1;
unsigned char*                  fb_map = NULL;
struct fb_fix_screeninfo        fb_fix;
struct fb_var_screeninfo        fb_var;
rgb_bitmap_info                 fb_bitmap;
struct termios                  fb_old_termios;
struct termios                  fb_run_termios;

volatile int                    quit = 0;

void sigma(int x) {
    (void)x; // unused

    if ((++quit) >= 10) abort();
}

static void free_src_bitmap(void);

static int init_src_bitmap(unsigned int width=640,unsigned int height=480) {
    if (src_bitmap.base != NULL)
        return 0;
    if (!fb_bitmap.is_valid())
        return 0;

    src_bitmap.clear();
    src_bitmap.base_align = 32;
    src_bitmap.stride_align = 32;
    src_bitmap.rgbinfo = fb_bitmap.rgbinfo;
    src_bitmap.bytes_per_pixel = fb_bitmap.bytes_per_pixel;
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

int main() {
    int redraw = 1;
    char c;

    hostCPUcaps.detect();

    fb_fd = open("/dev/fb0",O_RDWR|O_EXCL);
    if (fb_fd < 0) {
        fprintf(stderr,"Unable to open framebuffer\n");
        return 1;
    }

    if (ioctl(fb_fd,FBIOGET_FSCREENINFO,&fb_fix) < 0 ||
        ioctl(fb_fd,FBIOGET_VSCREENINFO,&fb_var) < 0) {
        fprintf(stderr,"Unable to get framebuffer info\n");
        return 1;
    }

    if (fb_fix.smem_len == 0) {
        fprintf(stderr,"No memory to map\n");
        return 1;
    }

    fb_map = (unsigned char*)mmap(NULL,fb_fix.smem_len,PROT_READ|PROT_WRITE,MAP_SHARED,fb_fd,0);
    if (fb_map == MAP_FAILED) {
        fprintf(stderr,"Failed to mmap\n");
        return 1;
    }

    fb_bitmap.base = fb_map;
    fb_bitmap.canvas = fb_map;
    fb_bitmap.width = fb_var.xres;
    fb_bitmap.height = fb_var.yres;
    fb_bitmap.stride = fb_fix.line_length;
    fb_bitmap.length = fb_fix.smem_len;
    fb_bitmap.bytes_per_pixel = (fb_var.bits_per_pixel+7)/8;
    fb_bitmap.rgbinfo.r.setByShiftOffset(fb_var.red.offset,fb_var.red.length);
    fb_bitmap.rgbinfo.g.setByShiftOffset(fb_var.green.offset,fb_var.green.length);
    fb_bitmap.rgbinfo.b.setByShiftOffset(fb_var.blue.offset,fb_var.blue.length);

    if (fb_var.transp.length != 0)
        fb_bitmap.rgbinfo.a.setByShiftOffset(fb_var.transp.offset,fb_var.transp.length);
    else
        fb_bitmap.rgbinfo.a.setByMask(~(fb_bitmap.rgbinfo.r.mask+fb_bitmap.rgbinfo.g.mask+fb_bitmap.rgbinfo.b.mask));

    fprintf(stderr,"R/G/B/A shift/width/mask %u/%u/0x%X %u/%u/0x%X %u/%u/0x%X %u/%u/0x%X width=%u height=%u stride=%u length=%u bytes/pixel=%u\n",
            fb_bitmap.rgbinfo.r.shift, fb_bitmap.rgbinfo.r.bwidth, fb_bitmap.rgbinfo.r.bmask,
            fb_bitmap.rgbinfo.g.shift, fb_bitmap.rgbinfo.g.bwidth, fb_bitmap.rgbinfo.g.bmask,
            fb_bitmap.rgbinfo.b.shift, fb_bitmap.rgbinfo.b.bwidth, fb_bitmap.rgbinfo.b.bmask,
            fb_bitmap.rgbinfo.a.shift, fb_bitmap.rgbinfo.a.bwidth, fb_bitmap.rgbinfo.a.bmask,
            (unsigned int)fb_bitmap.width,
            (unsigned int)fb_bitmap.height,
            (unsigned int)fb_bitmap.stride,
            (unsigned int)fb_bitmap.length,
            (unsigned int)fb_bitmap.bytes_per_pixel);

    if (!fb_bitmap.is_valid()) {
        fprintf(stderr,"Framebuffer not valid\n");
        return 1;
    }

    if (!init_src_bitmap())
        return 1;

    test_pattern_1_render(/*&*/src_bitmap);

    signal(SIGINT,sigma);
    signal(SIGQUIT,sigma);
    signal(SIGTERM,sigma);

    tcgetattr(0/*STDIN*/,&fb_old_termios);
    fb_run_termios = fb_old_termios;
    fb_run_termios.c_lflag &= ~(ICANON|ECHO|ECHOE|ECHOK|ECHONL|ECHOCTL);
    tcsetattr(0/*STDIN*/,TCSANOW,&fb_run_termios);

    while (!quit) {
        if (redraw) {
            if (method <= stretchblt_mode_count())
                stretchblt_modes[method].render(/*&*/fb_bitmap,/*&*/src_bitmap);
            redraw = 0;
        }

        if (read(0/*STDIN*/,&c,1) < 1)
            break;

        if (c == 'w') {
            if (src_bitmap.width > 1) {
                unsigned int nw = src_bitmap.width - 1;
                unsigned int nh = src_bitmap.height;

                free_src_bitmap();
                init_src_bitmap(nw,nh);
                test_pattern_1_render(/*&*/src_bitmap);
                redraw = 1;
            }
        }
        else if (c == 'W') {
            if (src_bitmap.width < 4095) {
                unsigned int nw = src_bitmap.width + 1;
                unsigned int nh = src_bitmap.height;

                free_src_bitmap();
                init_src_bitmap(nw,nh);
                test_pattern_1_render(/*&*/src_bitmap);
                redraw = 1;
            }
        }
        else if (c == 'h') {
            if (src_bitmap.height > 1) {
                unsigned int nw = src_bitmap.width;
                unsigned int nh = src_bitmap.height - 1;

                free_src_bitmap();
                init_src_bitmap(nw,nh);
                test_pattern_1_render(/*&*/src_bitmap);
                redraw = 1;
            }
        }
        else if (c == 'H') {
            if (src_bitmap.height < 4095) {
                unsigned int nw = src_bitmap.width;
                unsigned int nh = src_bitmap.height + 1;

                free_src_bitmap();
                init_src_bitmap(nw,nh);
                test_pattern_1_render(/*&*/src_bitmap);
                redraw = 1;
            }
        }
        else if (c == ' ') {
            do {
                if ((++method) >= stretchblt_mode_count())
                    method = 0;

                if (stretchblt_modes[method].can_do(/*&*/fb_bitmap,/*&*/src_bitmap))
                    break;
                fprintf(stderr,"Can't do %s, skipping\n",stretchblt_modes[method].name);
            } while (1);

            fprintf(stderr,"Switching to %s\n",stretchblt_modes[method].name);
            redraw = 1;
        }
    }

    munmap(fb_map,fb_fix.smem_len);
    close(fb_fd);
    tcsetattr(0/*STDIN*/,TCSANOW,&fb_old_termios);
    free_src_bitmap();
    fb_bitmap.clear();
    return 0;
}

