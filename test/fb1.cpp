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
#include "dosboxxr/lib/util/rgb_bitmap_info.h"
#include "dosboxxr/lib/util/rgb_bitmap_test_pattern_gradients.h"

int                             fb_fd = -1;
unsigned char*                  fb_map = NULL;
struct fb_fix_screeninfo        fb_fix;
struct fb_var_screeninfo        fb_var;
rgb_bitmap_info                 fb_bitmap;
struct termios                  fb_old_termios;
struct termios                  fb_run_termios;

volatile int                    quit = 0;

void free_bitmap();

void sigma(int x) {
    if ((++quit) >= 10) abort();
}

void init_bitmap(unsigned int width,unsigned int height,unsigned int align=32) {
}

void free_bitmap() {
    fb_bitmap.clear();
}

void freeall() {
	free_bitmap();
}

int main() {
    int redraw = 1;

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
            fb_bitmap.width,           fb_bitmap.height,           fb_bitmap.stride,
            fb_bitmap.length,          fb_bitmap.bytes_per_pixel);

    if (!fb_bitmap.is_valid()) {
        fprintf(stderr,"Framebuffer not valid\n");
        return 1;
    }

    signal(SIGINT,sigma);
    signal(SIGQUIT,sigma);
    signal(SIGTERM,sigma);

    tcgetattr(0/*STDIN*/,&fb_old_termios);
    fb_run_termios = fb_old_termios;
    fb_run_termios.c_lflag &= ~(ICANON|ECHO|ECHOE|ECHOK|ECHONL|ECHOCTL);
    tcsetattr(0/*STDIN*/,TCSANOW,&fb_run_termios);

    while (!quit) {
        if (redraw) {
	        render_test_pattern_rgb_gradients(fb_bitmap);
            redraw = 0;
        }
    }

    munmap(fb_map,fb_fix.smem_len);
    close(fb_fd);
    tcsetattr(0/*STDIN*/,TCSANOW,&fb_old_termios);
	return 0;
}

