
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

#if HAVE_CPU_ARM_NEON
# include <arm_neon.h>
#endif

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

#if HAVE_CPU_ARM_NEON
// 32bpp optimized for 8-bit ARGB/RGBA. rmask should be 0x00FF,0x00FF,... etc
static inline int16x8_t stretchblt_line_bilinear_pixel_blend_arm_neon_argb8(const int16x8_t cur,const int16x8_t nxt,const int16_t mul,const int16x8_t rmask) {
#if 0 // FIXME: I can't develop this code until I can figure out how to get X on my Raspberry Pi 2 to come up in 32-bit ARGB mode
    int16x8_t d1,d2,d3,d4;

    d1 = _mm256_and_si256(_mm256_mulhi_epi16(_mm256_sub_epi16(_mm256_and_si256(nxt,rmask),_mm256_and_si256(cur,rmask)),mul),rmask);
    d2 = _mm256_slli_si256(_mm256_and_si256(_mm256_mulhi_epi16(_mm256_sub_epi16(_mm256_and_si256(_mm256_srli_si256(nxt,1/*bytes!*/),rmask),_mm256_and_si256(_mm256_srli_si256(cur,1/*bytes!*/),rmask)),mul),rmask),1/*bytes!*/);
    d3 = _mm256_add_epi8(d1,d2);
    d4 = _mm256_add_epi8(d3,d3);
    return _mm256_add_epi8(d4,cur);
#endif
}

template <const uint8_t shf> static inline int16x8_t stretchblt_line_bilinear_pixel_blend_arm_neon_rgb16channel(const int16x8_t cur,const int16x8_t nxt,const int16_t mul,const int16x8_t cmask) {
    // WARNING: The reason for shf != 0 conditional shifting by template param 'shf' is that
    //          the ARMv7 assembler behind GCC 4.8 on the Raspberry Pi won't allow you to specify a
    //          constant shift of 0. It will complain "invalid constant", though in a way that
    //          is confusing that causes GCC to point to some unrelated closing bracket farther
    //          down this source code. The ternary expression is meant to bypass the shift
    //          entirely if shf == 0 to avoid that error. That is also why the template parameter
    //          is declared const.
    const int16x8_t cir = (shf != 0) ? vshrq_n_s16(cur,shf) : cur;
    const int16x8_t nir = (shf != 0) ? vshrq_n_s16(nxt,shf) : nxt;
    const int16x8_t rc = vandq_s16(cir,cmask);
    const int16x8_t rn = vandq_s16(nir,cmask);
    const int16x8_t d = vsubq_s16(rn,rc);
    const int16x8_t f = vaddq_s16(rc,vandq_s16(vqdmulhq_n_s16(d,mul),cmask));
    return (shf != 0) ? vshlq_n_s16(f,shf) : f;
}

// 16bpp 5/6/5 (FIXME: Add another variant for 5/5/5 when the need arises)
static inline int16x8_t stretchblt_line_bilinear_pixel_blend_arm_neon_rgb16(const int16x8_t cur,const int16x8_t nxt,const int16_t mul,const int16x8_t rmask,const uint16_t rshift,const int16x8_t gmask,const uint16_t gshift,const int16x8_t bmask,const uint16_t bshift) {
    int16x8_t sr,sg,sb;

    // WARNING: ARMv7 NEON shift intrinsics demand that the shift bit count is a constant. It cannot
    //          be a variable. So in the interest of getting this to work, we assume (for now)
    //          that it is the RGB 5/6/5 format that my Raspberry Pi 2 comes up in when starting X.
    //
    //          Note that it doesn't matter whether the 5/6/5 bit fields are R/G/B or B/G/R, what
    //          matters is that the bit fields are 5-bit/6-bit/5-bit.

    sr = stretchblt_line_bilinear_pixel_blend_arm_neon_rgb16channel<uint8_t(11)>(cur,nxt,mul,rmask);
    sg = stretchblt_line_bilinear_pixel_blend_arm_neon_rgb16channel<uint8_t(5)>(cur,nxt,mul,gmask);
    sb = stretchblt_line_bilinear_pixel_blend_arm_neon_rgb16channel<uint8_t(0)>(cur,nxt,mul,bmask);
    return vaddq_s16(vaddq_s16(sr,sg),sb);
}
#endif

#if HAVE_CPU_ARM_NEON
// case 2: 32-bit ARGB 8-bits per pixel
static inline void stretchblt_line_bilinear_vinterp_stage_arm_neon_argb8(int16x8_t *d,int16x8_t *s,int16x8_t *s2,const int16_t mul,size_t width,const int16x8_t rmask) {
    do {
        *d++ = stretchblt_line_bilinear_pixel_blend_arm_neon_argb8(*s++,*s2++,mul,rmask);
    } while ((--width) != (size_t)0);
}

// case 1: 16-bit arbitrary masks
static inline void stretchblt_line_bilinear_vinterp_stage_arm_neon_rgb16(int16x8_t *d,int16x8_t *s,int16x8_t *s2,const int16_t mul,size_t width,const int16x8_t rmask,const uint16_t rshift,const int16x8_t gmask,const uint16_t gshift,const int16x8_t bmask,const uint16_t bshift) {
    do {
        *d++ = stretchblt_line_bilinear_pixel_blend_arm_neon_rgb16(*s++,*s2++,mul,rmask,rshift,gmask,gshift,bmask,bshift);
    } while ((--width) != (size_t)0);
}
#endif

template <class T> void stretchblt_bilinear_arm_neon() {
#if HAVE_CPU_ARM_NEON
    // WARNING: This code assumes typical RGBA type packing where red and blue are NOT adjacent, and alpha and green are not adjacent
    nr_wfpack sx={0,0},sy={0,0},stepx,stepy;
    static vinterp_tmp<int16x8_t> vinterp_tmp;
    const T alpha = 
        (T)(~(x_image->red_mask+x_image->green_mask+x_image->blue_mask));
    const T rbmask = (T)(x_image->red_mask+x_image->blue_mask);
    const T abmask = (T)x_image->green_mask + alpha;
    int16x8_t rmask128,gmask128,bmask128;
    const size_t pixels_per_group =
        sizeof(int16x8_t) / sizeof(T);
    unsigned char *drow;
    uint16_t rm,gm,bm;
    uint8_t rs,gs,bs;
    int16_t mul128;
    size_t ox,oy;
    T fshift;
    T pshift;
    T fmax;
    T mul;

    // do not run this function if NEON extensions are not present
    if (!hostCPUcaps.neon) {
        fprintf(stderr,"CPU does not support NEON\n");
        return;
    }

    rs = bitscan_forward(x_image->red_mask,0);
    rm = bitscan_count(x_image->red_mask,rs) - rs;

    gs = bitscan_forward(x_image->green_mask,0);
    gm = bitscan_count(x_image->green_mask,gs) - gs;

    bs = bitscan_forward(x_image->blue_mask,0);
    bm = bitscan_count(x_image->blue_mask,bs) - bs;

    fshift = std::min(rm,std::min(gm,bm));
    pshift = fshift;
    fshift = (sizeof(nr_wftype) * 8) - fshift;

    // this code WILL fault if base or stride are not multiple of 16!
    if ((src_bitmap_stride & 15) != 0 || ((size_t)src_bitmap & 15) != 0) {
        fprintf(stderr,"Source bitmap not NEON usable (base=%p stride=0x%x\n",(void*)src_bitmap,(unsigned int)src_bitmap_stride);
        return;
    }
    if (((size_t)x_image->data & 15) != 0 || (x_image->bytes_per_line & 15) != 0) {
        fprintf(stderr,"Target bitmap not NEON usable (base=%p stride=0x%x\n",(void*)x_image->data,(unsigned int)x_image->bytes_per_line);
        return;
    }

    if (sizeof(T) == 4) {
        // 32bpp this code can only handle the 8-bit RGBA/ARGB case, else R/G/B fields cross 16-bit boundaries
        if (pshift != 8) return;
        if (bm != 8 || gm != 8 || rm != 8) return; // each field, 8 bits
        if ((rs&7) != 0 || (gs&7) != 0 || (bs&7) != 0) return; // each field, starts on 8-bit boundaries

        rmask128 = (int16x8_t){0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
    }
    else {
        // 16bpp this code can handle any case
        if (pshift > 15) return;

        rmask128 = (int16x8_t){
            (int16_t)((1 << rm) - 1),   (int16_t)((1 << rm) - 1),
            (int16_t)((1 << rm) - 1),   (int16_t)((1 << rm) - 1),
            (int16_t)((1 << rm) - 1),   (int16_t)((1 << rm) - 1),
            (int16_t)((1 << rm) - 1),   (int16_t)((1 << rm) - 1)
        };

        gmask128 = (int16x8_t){
            (int16_t)((1 << gm) - 1),   (int16_t)((1 << gm) - 1),
            (int16_t)((1 << gm) - 1),   (int16_t)((1 << gm) - 1),
            (int16_t)((1 << gm) - 1),   (int16_t)((1 << gm) - 1),
            (int16_t)((1 << gm) - 1),   (int16_t)((1 << gm) - 1)
        };

        bmask128 = (int16x8_t){
            (int16_t)((1 << bm) - 1),   (int16_t)((1 << bm) - 1),
            (int16_t)((1 << bm) - 1),   (int16_t)((1 << bm) - 1),
            (int16_t)((1 << bm) - 1),   (int16_t)((1 << bm) - 1),
            (int16_t)((1 << bm) - 1),   (int16_t)((1 << bm) - 1)
        };
    }

    fmax = 1U << pshift;

    render_scale_from_sd(/*&*/stepx,bitmap_width,src_bitmap_width);
    render_scale_from_sd(/*&*/stepy,bitmap_height,src_bitmap_height);

    unsigned int src_bitmap_width_groups = (src_bitmap_width + pixels_per_group - 1) / pixels_per_group;

    if (bitmap_width == 0 || src_bitmap_width_groups == 0) return;

    drow = (unsigned char*)x_image->data;
    oy = bitmap_height;
    do {
        T *s2 = src_bitmap.get_scanline<T>(sy.w+1);
        T *s = src_bitmap.get_scanline<T>(sy.w);
        T *d = (T*)drow;

        mul = (T)(sy.f >> fshift);
        mul128 = (mul & (~1U)) << (15 - pshift); // 16-bit MMX multiply (signed bit), remove one bit to match precision

        if (mul != 0) {
            if (stepx.w != 1 || stepx.f != 0) {
                // horizontal interpolation, vertical interpolation
                if (sizeof(T) == 4)
                    rerender_line_bilinear_vinterp_stage_arm_neon_argb8(vinterp_tmp.tmp,(int16x8_t*)s,(int16x8_t*)s2,mul128,src_bitmap_width_groups,rmask128);
                else
                    rerender_line_bilinear_vinterp_stage_arm_neon_rgb16(vinterp_tmp.tmp,(int16x8_t*)s,(int16x8_t*)s2,mul128,src_bitmap_width_groups,
                        rmask128,rs,gmask128,gs,bmask128,bs);

                rerender_line_bilinear_hinterp_stage<T>(d,(T*)vinterp_tmp.tmp,sx,stepx,bitmap_width,rbmask,abmask,fmax,fshift,pshift);
            }
            else {
                // vertical interpolation only
                if (sizeof(T) == 4)
                    rerender_line_bilinear_vinterp_stage_arm_neon_argb8((int16x8_t*)d,(int16x8_t*)s,(int16x8_t*)s2,mul128,src_bitmap_width_groups,rmask128);
                else
                    rerender_line_bilinear_vinterp_stage_arm_neon_rgb16((int16x8_t*)d,(int16x8_t*)s,(int16x8_t*)s2,mul128,src_bitmap_width_groups,
                        rmask128,rs,gmask128,gs,bmask128,bs);
            }
        }
        else {
            if (stepx.w != 1 || stepx.f != 0) {
                // horizontal interpolation, no vertical interpolation
                rerender_line_bilinear_hinterp_stage<T>(d,s,sx,stepx,bitmap_width,rbmask,abmask,fmax,fshift,pshift);
            }
            else {
                // copy the scanline 1:1 no interpolation
                memcpy(d,s,bitmap_width*sizeof(T));
            }
        }

        if ((--oy) == 0) break;
        drow += x_image->bytes_per_line;
        sy += stepy;
    } while (1);
#else
    fprintf(stderr,"Compiler does not support ARM NEON\n");
#endif
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

