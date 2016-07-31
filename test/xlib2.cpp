
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

#if HAVE_CPU_MMX
# include <mmintrin.h>
#endif

int                 method = 0;

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

unsigned char*      src_bitmap = NULL;
unsigned int        src_bitmap_width = 0;
unsigned int        src_bitmap_align = 0;
unsigned int        src_bitmap_height = 0;
size_t              src_bitmap_length = 0;
size_t              src_bitmap_stride = 0;

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

template <class T> void src_bitmap_render() {
    const T alpha =
        (~(x_image->red_mask+x_image->green_mask+x_image->blue_mask));
    T rm,gm,bm,r,g,b,*drow;
    uint8_t rs,gs,bs;
    int ox,oy;

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

    for (oy=0;oy < (src_bitmap_height/2);oy++) {
        drow = (T*)((uint8_t*)src_bitmap + (src_bitmap_stride * oy));
        for (ox=0;ox < (src_bitmap_width/2);ox++) {
            /* color */
            r = (ox * rm) / (src_bitmap_width/2);
            g = (oy * gm) / (src_bitmap_height/2);
            b = bm - ((ox * bm) / (src_bitmap_width/2));

            *drow++ = (r << rs) + (g << gs) + (b << bs) + alpha;
        }
    }

    for (oy=0;oy < (src_bitmap_height/2);oy++) {
        drow = ((T*)((uint8_t*)src_bitmap + (src_bitmap_stride * oy))) + (src_bitmap_width/2);
        for (ox=(src_bitmap_width/2);ox < src_bitmap_width;ox++) {
            /* color */
            r = ((ox ^ oy) & 1) ? rm : 0;
            g = ((ox ^ oy) & 1) ? gm : 0;
            b = ((ox ^ oy) & 1) ? bm : 0;

            *drow++ = (r << rs) + (g << gs) + (b << bs) + alpha;
        }
    }

    for (oy=(src_bitmap_height/2);oy < src_bitmap_height;oy++) {
        drow = ((T*)((uint8_t*)src_bitmap + (src_bitmap_stride * oy)));
        for (ox=0;ox < (src_bitmap_width/2);ox++) {
            /* color */
            r = ((ox ^ oy) & 2) ? rm : 0;
            g = ((ox ^ oy) & 2) ? gm : 0;
            b = ((ox ^ oy) & 2) ? bm : 0;

            *drow++ = (r << rs) + (g << gs) + (b << bs) + alpha;
        }
    }

    for (oy=(src_bitmap_height/2);oy < src_bitmap_height;oy++) {
        drow = ((T*)((uint8_t*)src_bitmap + (src_bitmap_stride * oy))) + (src_bitmap_width/2);
        for (ox=(src_bitmap_width/2);ox < src_bitmap_width;ox++) {
            /* color */
            r = ((ox ^ oy) & 4) ? rm : 0;
            g = ((ox ^ oy) & 4) ? gm : 0;
            b = ((ox ^ oy) & 4) ? bm : 0;

            *drow++ = (r << rs) + (g << gs) + (b << bs) + alpha;
        }
    }
}

void src_bitmap_render() {
    if (x_image->bits_per_pixel == 32)
        src_bitmap_render<uint32_t>();
    else if (x_image->bits_per_pixel == 16)
        src_bitmap_render<uint16_t>();
    else {
        fprintf(stderr,"WARNING: unsupported bit depth %u/bpp\n",
            x_image->bits_per_pixel);
    }
}

// wftype: must match the size of a CPU register to do fixed point math without shifting.
// the rationale is that, from experience with x86 processors past the Pentium III,
// processors are faster at addition, subtraction, and multiplication than they are with
// logical operators like AND, OR, and shift. therefore, we can speed up horizontal
// interpolation by managing the whole + fractional parts in a way that eliminates
// the shift operation when reading the whole part. But to accomplish this, we have to
// define the base datatype as one the exact width of a CPU register.
#if defined(__i386__) // x86
typedef uint32_t			nr_wftype;	// datatype the size of a CPU register
#elif defined(__x86_64__) // x86_64
typedef uint64_t			nr_wftype;	// datatype the size of a CPU register
#else
typedef unsigned int			nr_wftype;	// datatype the size of a CPU register
#endif

// number of bits in wftype
static const unsigned char				nr_wfbits = (sizeof(nr_wftype) * 8);

// what to mask the wftype by to obtain the upper 8 bits (i.e. checking whether the fractional part is effectively zero)
static const nr_wftype					nr_top8bitmask = ((nr_wftype)0xFF << (nr_wftype)(nr_wfbits - 8));

// what to mask the fraction by if comparing to "close enough" to 1:1
static const nr_wftype					nr_wfscsigbits = ((nr_wftype)0xFFFF << (nr_wftype)(nr_wfbits - 16));
static const nr_wftype					nr_wfscsigbits_add = ((nr_wftype)1 << (nr_wftype)(nr_wfbits - (16 + 1)));
static const nr_wftype					nr_wfhalfmax = ((nr_wftype)1 << (nr_wftype)(nr_wfbits - 1));

// wftype pack to contain whole & fractional parts
struct nr_wfpack {
public:
	nr_wftype			w,f;
public:
	// add to THIS struct another struct. inline for performance!
	inline void add(const struct nr_wfpack &b) {
#if defined(__x86_64__) && defined(nr_ALLOW_ASM) // x86_64 optimized version because GCC can't figure out what we're trying to do with carry
		// f += b.f
		// w += b.w + carry
		__asm__ (	"addq	%2,%0\n"
				"adcq	%3,%1\n" :
				/* outputs */ "+rme" (f), "+rme" (w) : /* +rme to mean we read/modify/write the outputs */
				/* inputs */ "ri" (b.f), "ri" (b.w) : /* make input params registers, add cannot take two memory operands */
				/* clobbered */ "cc" /* modifies flags */);
#elif defined(__i386__) && defined(nr_ALLOW_ASM) // i686 optimized version because GCC can't figure out what we're trying to do with carry
		// f += b.f
		// w += b.w + carry
		__asm__ (	"addl	%2,%0\n"
				"adcl	%3,%1\n" :
				/* outputs */ "+rme" (f), "+rme" (w) : /* +rme to mean we read/modify/write the outputs */
				/* inputs */ "ri" (b.f), "ri" (b.w) : /* make input params registers, add cannot take two memory operands */
				/* clobbered */ "cc" /* modifies flags */);
#else // generic version. may be slower than optimized because GCC probably won't figure out we want i686 equiv. of add eax,ebx followed by adc ecx,edx, the carry part)
		nr_wftype ov = f;
		w += b.w+(((f += b.f) < ov)?1:0);
		// what we want: add fractions. add whole part, with carry from fractions.
#endif
	}
	// C++ convenience for add()
	inline struct nr_wfpack& operator+=(const struct nr_wfpack &b) {
		add(b);
		return *this;
	}
	inline struct nr_wfpack operator+(const struct nr_wfpack &b) {
		struct nr_wfpack r = *this;
		return (r += b);
	}

	// inline for performance!
	inline unsigned int mul_h(const unsigned int b,const unsigned int a) {
		// NTS: i686 handwritten version is slower than the generic
#if 0
#else
		// NTS: Despite the fact modern CPUs are faster at ADD than SHR, this is faster
		//      than typecasting to long long then shifting down. By quite a bit.
		return a + (unsigned int)((((int)b - (int)a) * (f >> (nr_wfbits - 8))) >> 8);
#endif
	}
};

void render_scale_from_sd(struct nr_wfpack &sy,const uint32_t dh,const uint32_t sh) {
	uint64_t t;

    t  = (uint64_t)((uint64_t)sh << (uint64_t)32);
    t /= (uint64_t)dh;

	if (sizeof(nr_wftype) == 8) {
		// nr_wftype is 64.64
		sy.w = (nr_wftype)(t >> (uint64_t)32);
		sy.f = (nr_wftype)(t << (uint64_t)32);
	}
	else if (sizeof(nr_wftype) == 4) {
		// nr_wftype is 32.32
		sy.w = (nr_wftype)(t >> (uint64_t)32);
		sy.f = (nr_wftype)t;
	}
	else {
		abort();
	}
}

/* render image to XImage.
 * stretch fit using crude nearest neighbor scaling */

template <class T> void rerender_out_neighbor() {
    nr_wfpack sx,sxi={0,0},sy={0,0},stepx,stepy;
    unsigned char *drow;
    size_t ox,oy;

    render_scale_from_sd(/*&*/stepx,bitmap_width,src_bitmap_width);
    render_scale_from_sd(/*&*/stepy,bitmap_height,src_bitmap_height);

    drow = (unsigned char*)x_image->data;
    oy = bitmap_height;
    do {
        T *s = (T*)((uint8_t*)src_bitmap + (src_bitmap_stride*sy.w));
        T *d = (T*)drow;

        ox = bitmap_width;
        sx = sxi;
        do {
            *d++ = s[sx.w];
            if ((--ox) == 0) break;
            sx += stepx;
        } while (1);

        if ((--oy) == 0) break;
        drow += x_image->bytes_per_line;
        sy += stepy;
    } while (1);
}



#define VINTERP_MAX 4096

// WARNING: This code assumes typical RGBA type packing where red and blue are NOT adjacent, and alpha and green are not adjacent
template <class T> inline T rerender_line_bilinear_pixel_blend(const T cur,const T nxt,const T mulmax,const T mul,const T rbmask,const T abmask,const T pshift) {
    T sum;

    sum  = (((uint64_t)(cur & rbmask) * (mulmax - mul)) >> pshift) & rbmask;
    sum += (((uint64_t)(cur & abmask) * (mulmax - mul)) >> pshift) & abmask;
    sum += (((uint64_t)(nxt & rbmask) *           mul ) >> pshift) & rbmask;
    sum += (((uint64_t)(nxt & abmask) *           mul ) >> pshift) & abmask;
    return sum;
}

#if HAVE_CPU_MMX
// 32bpp optimized for 8-bit ARGB/RGBA. rmask should be 0x00FF,0x00FF,... etc
static inline __m64 rerender_line_bilinear_pixel_blend_mmx_argb8(const __m64 cur,const __m64 nxt,const __m64 mul,const __m64 rmask) {
    __m64 d1,d2,d3,d4;

    d1 = _mm_and_si64(_mm_mulhi_pi16(_mm_sub_pi16(_mm_and_si64(nxt,rmask),_mm_and_si64(cur,rmask)),mul),rmask);
    d2 = _mm_slli_si64(_mm_and_si64(_mm_mulhi_pi16(_mm_sub_pi16(_mm_and_si64(_mm_srli_si64(nxt,8),rmask),_mm_and_si64(_mm_srli_si64(cur,8),rmask)),mul),rmask),8);
    d3 = _mm_add_pi8(d1,d2);
    d4 = _mm_add_pi8(d3,d3);
    return _mm_add_pi8(d4,cur);
}

// 16bpp general R/G/B, usually 5/6/5 or 5/5/5
static inline __m64 rerender_line_bilinear_pixel_blend_mmx_rgb16(const __m64 cur,const __m64 nxt,const __m64 mul,const __m64 rmask,const uint16_t rshift,const __m64 gmask,const uint16_t gshift,const __m64 bmask,const uint16_t bshift) {
    __m64 rc,gc,bc;
    __m64 rn,gn,bn;
    __m64 d,sum;

    rc = _mm_and_si64(_mm_srli_si64(cur,rshift),rmask);
    gc = _mm_and_si64(_mm_srli_si64(cur,gshift),gmask);
    bc = _mm_and_si64(_mm_srli_si64(cur,bshift),bmask);

    rn = _mm_and_si64(_mm_srli_si64(nxt,rshift),rmask);
    gn = _mm_and_si64(_mm_srli_si64(nxt,gshift),gmask);
    bn = _mm_and_si64(_mm_srli_si64(nxt,bshift),bmask);

    d = _mm_sub_pi16(rn,rc);
    sum = _mm_slli_si64(_mm_add_pi16(rc,_mm_and_si64(_mm_mulhi_pi16(_mm_add_pi16(d,d),mul),rmask)),rshift);

    d = _mm_sub_pi16(gn,gc);
    sum = _mm_add_pi16(_mm_slli_si64(_mm_add_pi16(gc,_mm_and_si64(_mm_mulhi_pi16(_mm_add_pi16(d,d),mul),gmask)),gshift),sum);

    d = _mm_sub_pi16(bn,bc);
    sum = _mm_add_pi16(_mm_slli_si64(_mm_add_pi16(bc,_mm_and_si64(_mm_mulhi_pi16(_mm_add_pi16(d,d),mul),bmask)),bshift),sum);

    return sum;
}
#endif

template <class T> inline void rerender_line_bilinear_hinterp_stage(T *d,T *s,struct nr_wfpack sx,const struct nr_wfpack &stepx,size_t dwidth,const T rbmask,const T abmask,const T fmax,const T fshift,const T pshift) {
    do {
        *d++ = rerender_line_bilinear_pixel_blend<T>(s[sx.w],s[sx.w+1],fmax,(T)(sx.f >> (T)fshift),rbmask,abmask,pshift);
        sx += stepx;
    } while ((--dwidth) != (size_t)0);
}

#if HAVE_CPU_MMX
// case 2: 32-bit ARGB 8-bits per pixel
static inline void rerender_line_bilinear_vinterp_stage_mmx_argb8(__m64 *d,__m64 *s,__m64 *s2,const __m64 mul,size_t width,const __m64 rmask) {
    do {
        *d++ = rerender_line_bilinear_pixel_blend_mmx_argb8(*s++,*s2++,mul,rmask);
    } while ((--width) != (size_t)0);
    _mm_empty();
}

// case 1: 16-bit arbitrary masks
static inline void rerender_line_bilinear_vinterp_stage_mmx_rgb16(__m64 *d,__m64 *s,__m64 *s2,const __m64 mul,size_t width,const __m64 rmask,const uint16_t rshift,const __m64 gmask,const uint16_t gshift,const __m64 bmask,const uint16_t bshift) {
    do {
        *d++ = rerender_line_bilinear_pixel_blend_mmx_rgb16(*s++,*s2++,mul,rmask,rshift,gmask,gshift,bmask,bshift);
    } while ((--width) != (size_t)0);
}
#endif

template <class T> inline void rerender_line_bilinear_vinterp_stage(T *d,T *s,T *s2,const T mul,size_t width,const T rbmask,const T abmask,const T fmax,const T pshift) {
    do {
        *d++ = rerender_line_bilinear_pixel_blend<T>(*s++,*s2++,fmax,mul,rbmask,abmask,pshift);
    } while ((--width) != (size_t)0);
}

template <class T> class vinterp_tmp {
public:
    T               tmp[VINTERP_MAX];
};

template <class T> void rerender_out_bilinear() {
    // WARNING: This code assumes typical RGBA type packing where red and blue are NOT adjacent, and alpha and green are not adjacent
    nr_wfpack sx={0,0},sy={0,0},stepx,stepy;
    static vinterp_tmp<T> vinterp_tmp;
    const T alpha = 
        (T)(~(x_image->red_mask+x_image->green_mask+x_image->blue_mask));
    const T rbmask = (T)(x_image->red_mask+x_image->blue_mask);
    const T abmask = (T)x_image->green_mask + alpha;
    unsigned char *drow;
    uint32_t rm,gm,bm;
    uint8_t rs,gs,bs;
    size_t ox,oy;
    T fshift;
    T pshift;
    T fmax;
    T mul;

    rs = bitscan_forward(x_image->red_mask,0);
    rm = bitscan_count(x_image->red_mask,rs) - rs;

    gs = bitscan_forward(x_image->green_mask,0);
    gm = bitscan_count(x_image->green_mask,gs) - gs;

    bs = bitscan_forward(x_image->blue_mask,0);
    bm = bitscan_count(x_image->blue_mask,bs) - bs;

    fshift = std::min(rm,std::min(gm,bm));
    pshift = fshift;
    fshift = (sizeof(nr_wftype) * 8) - fshift;

    fmax = 1U << pshift;

    render_scale_from_sd(/*&*/stepx,bitmap_width,src_bitmap_width);
    render_scale_from_sd(/*&*/stepy,bitmap_height,src_bitmap_height);

    if (bitmap_width == 0 || src_bitmap_width == 0) return;

    drow = (unsigned char*)x_image->data;
    oy = bitmap_height;
    do {
        T *s2 = (T*)((uint8_t*)src_bitmap + (src_bitmap_stride*(sy.w+1)));
        T *s = (T*)((uint8_t*)src_bitmap + (src_bitmap_stride*sy.w));
        T *d = (T*)drow;

        mul = (T)(sy.f >> fshift);

        if (mul != 0) {
            if (stepx.w != 1 || stepx.f != 0) {
                // horizontal interpolation, vertical interpolation
                rerender_line_bilinear_vinterp_stage<T>(vinterp_tmp.tmp,s,s2,mul,src_bitmap_width,rbmask,abmask,fmax,pshift);
                rerender_line_bilinear_hinterp_stage<T>(d,vinterp_tmp.tmp,sx,stepx,bitmap_width,rbmask,abmask,fmax,fshift,pshift);
            }
            else {
                // vertical interpolation only
                rerender_line_bilinear_vinterp_stage<T>(d,s,s2,mul,src_bitmap_width,rbmask,abmask,fmax,pshift);
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
}

template <class T> void rerender_out_bilinear_mmx() {
#if HAVE_CPU_MMX
    // WARNING: This code assumes typical RGBA type packing where red and blue are NOT adjacent, and alpha and green are not adjacent
    nr_wfpack sx={0,0},sy={0,0},stepx,stepy;
    static vinterp_tmp<__m64> vinterp_tmp;
    const T alpha = 
        (T)(~(x_image->red_mask+x_image->green_mask+x_image->blue_mask));
    const T rbmask = (T)(x_image->red_mask+x_image->blue_mask);
    const T abmask = (T)x_image->green_mask + alpha;
    __m64 rmask64,gmask64,bmask64,mul64;
    const size_t pixels_per_mmx =
        sizeof(__m64) / sizeof(T);
    unsigned char *drow;
    uint32_t rm,gm,bm;
    uint8_t rs,gs,bs;
    size_t ox,oy;
    T fshift;
    T pshift;
    T fmax;
    T mul;

    // do not run this function if MMX extensions are not present
    if (!hostCPUcaps.mmx) return;

    rs = bitscan_forward(x_image->red_mask,0);
    rm = bitscan_count(x_image->red_mask,rs) - rs;

    gs = bitscan_forward(x_image->green_mask,0);
    gm = bitscan_count(x_image->green_mask,gs) - gs;

    bs = bitscan_forward(x_image->blue_mask,0);
    bm = bitscan_count(x_image->blue_mask,bs) - bs;

    fshift = std::min(rm,std::min(gm,bm));
    pshift = fshift;
    fshift = (sizeof(nr_wftype) * 8) - fshift;

    if (sizeof(T) == 4) {
        // 32bpp this code can only handle the 8-bit RGBA/ARGB case, else R/G/B fields cross 16-bit boundaries
        if (pshift != 8) return;
        if (bm != 8 || gm != 8 || rm != 8) return; // each field, 8 bits
        if ((rs&7) != 0 || (gs&7) != 0 || (bs&7) != 0) return; // each field, starts on 8-bit boundaries

        rmask64 = _mm_set_pi16(0x00FF,0x00FF,0x00FF,0x00FF);
    }
    else {
        // 16bpp this code can handle any case
        if (pshift > 15) return;

        rmask64 = _mm_set_pi16((1U << rm) - 1,(1U << rm) - 1,(1U << rm) - 1,(1U << rm) - 1);
        gmask64 = _mm_set_pi16((1U << gm) - 1,(1U << gm) - 1,(1U << gm) - 1,(1U << gm) - 1);
        bmask64 = _mm_set_pi16((1U << bm) - 1,(1U << bm) - 1,(1U << bm) - 1,(1U << bm) - 1);
    }

    fmax = 1U << pshift;

    render_scale_from_sd(/*&*/stepx,bitmap_width,src_bitmap_width);
    render_scale_from_sd(/*&*/stepy,bitmap_height,src_bitmap_height);

    unsigned int src_bitmap_width_m64 = (src_bitmap_width + pixels_per_mmx - 1) / pixels_per_mmx;

    if (bitmap_width == 0 || src_bitmap_width_m64 == 0) return;

    drow = (unsigned char*)x_image->data;
    oy = bitmap_height;
    do {
        T *s2 = (T*)((uint8_t*)src_bitmap + (src_bitmap_stride*(sy.w+1)));
        T *s = (T*)((uint8_t*)src_bitmap + (src_bitmap_stride*sy.w));
        T *d = (T*)drow;

        mul = (T)(sy.f >> fshift);

        {
            unsigned int m = (mul & (~1U)) << (15 - pshift); // 16-bit MMX multiply (signed bit), remove one bit to match precision
            mul64 = _mm_set_pi16(m,m,m,m);
        }

        if (mul != 0) {
            if (stepx.w != 1 || stepx.f != 0) {
                // horizontal interpolation, vertical interpolation
                if (sizeof(T) == 4)
                    rerender_line_bilinear_vinterp_stage_mmx_argb8(vinterp_tmp.tmp,(__m64*)s,(__m64*)s2,mul64,src_bitmap_width_m64,rmask64);
                else
                    rerender_line_bilinear_vinterp_stage_mmx_rgb16(vinterp_tmp.tmp,(__m64*)s,(__m64*)s2,mul64,src_bitmap_width_m64,
                        rmask64,rs,gmask64,gs,bmask64,bs);

                rerender_line_bilinear_hinterp_stage<T>(d,(T*)vinterp_tmp.tmp,sx,stepx,bitmap_width,rbmask,abmask,fmax,fshift,pshift);
            }
            else {
                // vertical interpolation only
                if (sizeof(T) == 4)
                    rerender_line_bilinear_vinterp_stage_mmx_argb8((__m64*)d,(__m64*)s,(__m64*)s2,mul64,src_bitmap_width_m64,rmask64);
                else
                    rerender_line_bilinear_vinterp_stage_mmx_rgb16((__m64*)d,(__m64*)s,(__m64*)s2,mul64,src_bitmap_width_m64,
                        rmask64,rs,gmask64,gs,bmask64,bs);
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
#endif
}

void rerender_out() {
    if (method == 0) {
        fprintf(stderr,"Neighbor\n");
        if (x_image->bits_per_pixel == 32)
            rerender_out_neighbor<uint32_t>();
        else if (x_image->bits_per_pixel == 16)
            rerender_out_neighbor<uint16_t>();
        else if (x_image->bits_per_pixel == 8)
            rerender_out_neighbor<uint8_t>();
    }
    else if (method == 1) {
        fprintf(stderr,"Basic bilinear\n");
        if (x_image->bits_per_pixel == 32)
            rerender_out_bilinear<uint32_t>();
        else if (x_image->bits_per_pixel == 16)
            rerender_out_bilinear<uint16_t>();
    }
    else if (method == 2) {
        fprintf(stderr,"Basic bilinear MMX\n");
        if (x_image->bits_per_pixel == 32)
            rerender_out_bilinear_mmx<uint32_t>();
        else if (x_image->bits_per_pixel == 16)
            rerender_out_bilinear_mmx<uint16_t>();
    }
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
	if (!init_shm()) {
		if (!init_norm()) {
			fprintf(stderr,"Cannot alloc bitmap\n");
			return 1;
		}
	}

    /* src bitmap */
    src_bitmap_width = 640;
    src_bitmap_height = 480;
    src_bitmap_align = 32; // bytes
    src_bitmap_stride = ((x_image->bits_per_pixel+7)/8) * src_bitmap_width;
    src_bitmap_stride = (src_bitmap_stride + (src_bitmap_align * 2) - 1) & (~(src_bitmap_align - 1U));
    src_bitmap_length = src_bitmap_stride * (src_bitmap_height + 1);
    src_bitmap = (unsigned char*)malloc(src_bitmap_length);
    if (src_bitmap == NULL) {
        fprintf(stderr,"Cannot alloc src bitmap\n");
        return 1;
    }

    /* make up something */
    src_bitmap_render();

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
                else if (sym == XK_space) {
                    if ((++method) >= 3)
                        method = 0;

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
    free(src_bitmap);
	XDestroyWindow(x_display,x_window);
	XCloseDisplay(x_display);
	return 0;
}

