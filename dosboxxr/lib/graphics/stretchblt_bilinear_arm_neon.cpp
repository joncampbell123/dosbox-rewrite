
#ifdef HAVE_CONFIG_H
# include "config.h" // must be first
#endif

#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "dosboxxr/lib/hostcpudetect/caps.h"
#include "dosboxxr/lib/util/nr_wfpack.h"
#include "dosboxxr/lib/util/bitscan.h"
#include "dosboxxr/lib/util/vinterp_tmp.h"
#include "dosboxxr/lib/graphics/stretchblt_bilinear.h"
#include "dosboxxr/lib/graphics/stretchblt_bilinear_arm_neon.h"

#include <algorithm>

#if HAVE_CPU_ARM_NEON
# include <arm_neon.h>

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
#endif
}

#if HAVE_CPU_ARM_NEON
template <class T> bool stretchblt_bilinear_arm_neon_can_do(const rgb_bitmap_info &dbmp,const rgb_bitmap_info &sbmp) {
    if (!dbmp.is_valid() || !sbmp.is_valid()) return false;
    if (!hostCPUcaps.neon) return false;

    // must fit in buffer
    const size_t pixels_per_group =
        sizeof(int16x8_t) / sizeof(T);

    if (sbmp.width >= (VINTERP_MAX*pixels_per_group))
        return false;

    // buffer alignment is required
    if (((size_t)sbmp.canvas & 15) != 0) return false;
    if (((size_t)dbmp.canvas & 15) != 0) return false;

    // stride alignment is required
    if (((size_t)sbmp.stride & 15) != 0) return false;
    if (((size_t)dbmp.stride & 15) != 0) return false;

    if (sizeof(T) == 4) {
        // 32bpp this code can only handle the 8-bit RGBA/ARGB case, else R/G/B fields cross 16-bit boundaries
        if (dbmp.rgbinfo.r.bwidth != 8 ||
            dbmp.rgbinfo.g.bwidth != 8 ||
            dbmp.rgbinfo.b.bwidth != 8) return false; // each field, 8 bits
        if ((dbmp.rgbinfo.r.shift&7) != 0 ||
            (dbmp.rgbinfo.g.shift&7) != 0 ||
            (dbmp.rgbinfo.b.shift&7) != 0) return false; // each field, starts on 8-bit boundaries
    }
    else {
        // 16bpp this code can handle any case
    }

    return true;
}
#endif

bool stretchblt_bilinear_arm_neon_can_do(const rgb_bitmap_info &dbmp,const rgb_bitmap_info &sbmp) {
#if HAVE_CPU_ARM_NEON
    if (dbmp.bytes_per_pixel == sizeof(uint32_t))
        return stretchblt_bilinear_arm_neon_can_do<uint32_t>(dbmp,sbmp);
    else if (dbmp.bytes_per_pixel == sizeof(uint16_t))
        return stretchblt_bilinear_arm_neon_can_do<uint16_t>(dbmp,sbmp);
#endif
    return false;
}

