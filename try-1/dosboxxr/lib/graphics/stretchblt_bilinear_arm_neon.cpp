
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
    const int16x8_t crb = vandq_s16(cur,rmask);
    const int16x8_t cga = vandq_s16(vshrq_n_s16(cur,8),rmask);
    const int16x8_t nrb = vandq_s16(nxt,rmask);
    const int16x8_t nga = vandq_s16(vshrq_n_s16(nxt,8),rmask);
    const int16x8_t d1 = vandq_s16(vqdmulhq_n_s16(vsubq_s16(nrb,crb),mul),rmask);
    const int16x8_t d2 = vandq_s16(vqdmulhq_n_s16(vsubq_s16(nga,cga),mul),rmask);
    const int16x8_t f1 = vaddq_s16(crb,d1);
    const int16x8_t f2 = vshlq_n_s16(vaddq_s16(cga,d2),8);
    return vaddq_s16(f1,f2);
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

template <class T> void stretchblt_bilinear_arm_neon(const rgb_bitmap_info &dbmp,const rgb_bitmap_info &sbmp) {
#if HAVE_CPU_ARM_NEON
    // WARNING: This code assumes typical RGBA type packing where red and blue are NOT adjacent, and alpha and green are not adjacent
    nr_wfpack sx={0,0},sy={0,0},stepx,stepy;
    static vinterp_tmp<int16x8_t> vinterp_tmp;
    const T rbmask = (T)(dbmp.rgbinfo.r.mask+dbmp.rgbinfo.b.mask);
    const T abmask = (T)(dbmp.rgbinfo.g.mask+dbmp.rgbinfo.a.mask);
    int16x8_t rmask128,gmask128,bmask128;
    const size_t pixels_per_group =
        sizeof(int16x8_t) / sizeof(T);
    unsigned int src_bitmap_width_groups =
        (sbmp.width + pixels_per_group - 1) / pixels_per_group;
    unsigned char *drow;
    int16_t mul128;
    size_t ox,oy;
    T fshift;
    T pshift;
    T fmax;
    T mul;

    pshift = std::min(dbmp.rgbinfo.r.bwidth,std::min(dbmp.rgbinfo.g.bwidth,dbmp.rgbinfo.b.bwidth));
    fshift = (sizeof(nr_wftype) * 8) - pshift;
    fmax = 1U << pshift;

    if (sizeof(T) == 4) {
        rmask128 = (int16x8_t){0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
    }
    else {
        rmask128 = (int16x8_t){
            (int16_t)dbmp.rgbinfo.r.bmask,  (int16_t)dbmp.rgbinfo.r.bmask,
            (int16_t)dbmp.rgbinfo.r.bmask,  (int16_t)dbmp.rgbinfo.r.bmask,
            (int16_t)dbmp.rgbinfo.r.bmask,  (int16_t)dbmp.rgbinfo.r.bmask,
            (int16_t)dbmp.rgbinfo.r.bmask,  (int16_t)dbmp.rgbinfo.r.bmask
        };

        gmask128 = (int16x8_t){
            (int16_t)dbmp.rgbinfo.g.bmask,  (int16_t)dbmp.rgbinfo.g.bmask,
            (int16_t)dbmp.rgbinfo.g.bmask,  (int16_t)dbmp.rgbinfo.g.bmask,
            (int16_t)dbmp.rgbinfo.g.bmask,  (int16_t)dbmp.rgbinfo.g.bmask,
            (int16_t)dbmp.rgbinfo.g.bmask,  (int16_t)dbmp.rgbinfo.g.bmask
        };

        bmask128 = (int16x8_t){
            (int16_t)dbmp.rgbinfo.b.bmask,  (int16_t)dbmp.rgbinfo.b.bmask,
            (int16_t)dbmp.rgbinfo.b.bmask,  (int16_t)dbmp.rgbinfo.b.bmask,
            (int16_t)dbmp.rgbinfo.b.bmask,  (int16_t)dbmp.rgbinfo.b.bmask,
            (int16_t)dbmp.rgbinfo.b.bmask,  (int16_t)dbmp.rgbinfo.b.bmask
        };
    }

    render_scale_from_sd(/*&*/stepx,dbmp.width,sbmp.width);
    render_scale_from_sd(/*&*/stepy,dbmp.height,sbmp.height);
    if (dbmp.width == 0 || src_bitmap_width_groups == 0) return;

    drow = dbmp.get_scanline<uint8_t>(0);
    oy = dbmp.height;
    do {
        T *s2 = sbmp.get_scanline<T>(sy.w+1);
        T *s = sbmp.get_scanline<T>(sy.w);
        T *d = (T*)drow;

        mul = (T)(sy.f >> fshift);
        mul128 = (mul & (~1U)) << (15 - pshift); // 16-bit MMX multiply (signed bit), remove one bit to match precision

        if (mul != 0) {
            if (stepx.w != 1 || stepx.f != 0) {
                // horizontal interpolation, vertical interpolation
                if (sizeof(T) == 4)
                    stretchblt_line_bilinear_vinterp_stage_arm_neon_argb8(vinterp_tmp.tmp,(int16x8_t*)s,(int16x8_t*)s2,mul128,src_bitmap_width_groups,rmask128);
                else
                    stretchblt_line_bilinear_vinterp_stage_arm_neon_rgb16(vinterp_tmp.tmp,(int16x8_t*)s,(int16x8_t*)s2,mul128,src_bitmap_width_groups,
                        rmask128,dbmp.rgbinfo.r.shift,
                        gmask128,dbmp.rgbinfo.g.shift,
                        bmask128,dbmp.rgbinfo.b.shift);

                stretchblt_line_bilinear_hinterp_stage<T>(d,(T*)vinterp_tmp.tmp,sx,stepx,dbmp.width,rbmask,abmask,fmax,fshift,pshift);
            }
            else {
                // vertical interpolation only
                if (sizeof(T) == 4)
                    stretchblt_line_bilinear_vinterp_stage_arm_neon_argb8((int16x8_t*)d,(int16x8_t*)s,(int16x8_t*)s2,mul128,src_bitmap_width_groups,rmask128);
                else
                    stretchblt_line_bilinear_vinterp_stage_arm_neon_rgb16((int16x8_t*)d,(int16x8_t*)s,(int16x8_t*)s2,mul128,src_bitmap_width_groups,
                        rmask128,dbmp.rgbinfo.r.shift,
                        gmask128,dbmp.rgbinfo.g.shift,
                        bmask128,dbmp.rgbinfo.b.shift);
            }
        }
        else {
            if (stepx.w != 1 || stepx.f != 0) {
                // horizontal interpolation, no vertical interpolation
                stretchblt_line_bilinear_hinterp_stage<T>(d,s,sx,stepx,dbmp.width,rbmask,abmask,fmax,fshift,pshift);
            }
            else {
                // copy the scanline 1:1 no interpolation
                memcpy(d,s,dbmp.width*sizeof(T));
            }
        }

        if ((--oy) == 0) break;
        drow += dbmp.stride;
        sy += stepy;
    } while (1);
#endif
}

void stretchblt_bilinear_arm_neon(const rgb_bitmap_info &dbmp,const rgb_bitmap_info &sbmp) {
#if HAVE_CPU_ARM_NEON
    if (dbmp.bytes_per_pixel == sizeof(uint32_t))
        stretchblt_bilinear_arm_neon<uint32_t>(dbmp,sbmp);
    else if (dbmp.bytes_per_pixel == sizeof(uint16_t))
        stretchblt_bilinear_arm_neon<uint16_t>(dbmp,sbmp);
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
        // 16bpp can only handle 5/6/5 or 5/5/5 RGB/BGR
        if (dbmp.rgbinfo.r.bwidth > 6 ||
            dbmp.rgbinfo.g.bwidth > 6 ||
            dbmp.rgbinfo.b.bwidth > 6) return false;

        // green must start at bit 5
        if (dbmp.rgbinfo.g.shift != 5)
            return false;

        // red/blue must start at 0/10, 0/11, 10/0, or 11/0
        if ((dbmp.rgbinfo.r.shift == 0 && dbmp.rgbinfo.b.shift == 10) ||
            (dbmp.rgbinfo.r.shift == 0 && dbmp.rgbinfo.b.shift == 11) ||
            (dbmp.rgbinfo.r.shift == 10 && dbmp.rgbinfo.b.shift == 0) ||
            (dbmp.rgbinfo.r.shift == 11 && dbmp.rgbinfo.b.shift == 0))
            { /*OK*/ }
        else
            return false;

        // red/blue channel width must match, or else it would not be interchangeable
        if (dbmp.rgbinfo.r.bwidth != dbmp.rgbinfo.b.bwidth)
            return false;
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

