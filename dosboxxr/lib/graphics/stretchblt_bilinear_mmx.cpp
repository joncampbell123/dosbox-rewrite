
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
#include "dosboxxr/lib/graphics/stretchblt_bilinear_mmx.h"

#include <algorithm>

#if HAVE_CPU_MMX
# include <mmintrin.h>

// 32bpp optimized for 8-bit ARGB/RGBA. rmask should be 0x00FF,0x00FF,... etc
static inline __m64 stretchblt_line_bilinear_pixel_blend_mmx_argb8(const __m64 cur,const __m64 nxt,const __m64 mul,const __m64 rmask) {
    __m64 rc,gc;
    __m64 rn,gn;
    __m64 d,sum;

    rc = _mm_and_si64(              cur   ,rmask);
    gc = _mm_and_si64(_mm_srli_si64(cur,8),rmask);

    rn = _mm_and_si64(              nxt   ,rmask);
    gn = _mm_and_si64(_mm_srli_si64(nxt,8),rmask);

    d = _mm_sub_pi16(rn,rc);
    sum = _mm_add_pi16(rc,_mm_mulhi_pi16(_mm_add_pi16(d,d),mul));

    d = _mm_sub_pi16(gn,gc);
    sum = _mm_add_pi16(_mm_slli_si64(_mm_add_pi16(gc,_mm_mulhi_pi16(_mm_add_pi16(d,d),mul)),8),sum);

    return sum;
}

// 16bpp general R/G/B, usually 5/6/5 or 5/5/5
static inline __m64 stretchblt_line_bilinear_pixel_blend_mmx_rgb16(const __m64 cur,const __m64 nxt,const __m64 mul,const __m64 rmask,const uint16_t rshift,const __m64 gmask,const uint16_t gshift,const __m64 bmask,const uint16_t bshift) {
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
    sum = _mm_slli_si64(_mm_add_pi16(rc,_mm_mulhi_pi16(_mm_add_pi16(d,d),mul)),rshift);

    d = _mm_sub_pi16(gn,gc);
    sum = _mm_add_pi16(_mm_slli_si64(_mm_add_pi16(gc,_mm_mulhi_pi16(_mm_add_pi16(d,d),mul)),gshift),sum);

    d = _mm_sub_pi16(bn,bc);
    sum = _mm_add_pi16(_mm_slli_si64(_mm_add_pi16(bc,_mm_mulhi_pi16(_mm_add_pi16(d,d),mul)),bshift),sum);

    return sum;
}

// case 2: 32-bit ARGB 8-bits per pixel
static inline void stretchblt_line_bilinear_vinterp_stage_mmx_argb8(__m64 *d,__m64 *s,__m64 *s2,const __m64 mul,size_t width,const __m64 rmask) {
    do {
        *d++ = stretchblt_line_bilinear_pixel_blend_mmx_argb8(*s++,*s2++,mul,rmask);
    } while ((--width) != (size_t)0);
    _mm_empty();
}

// case 1: 16-bit arbitrary masks
static inline void stretchblt_line_bilinear_vinterp_stage_mmx_rgb16(__m64 *d,__m64 *s,__m64 *s2,const __m64 mul,size_t width,const __m64 rmask,const uint16_t rshift,const __m64 gmask,const uint16_t gshift,const __m64 bmask,const uint16_t bshift) {
    do {
        *d++ = stretchblt_line_bilinear_pixel_blend_mmx_rgb16(*s++,*s2++,mul,rmask,rshift,gmask,gshift,bmask,bshift);
    } while ((--width) != (size_t)0);
}

// MMX isn't all that picky. unaligned access is allowed. the only consequence of not aligning
// the scanlines for MMX is some performance loss.
template <class T> bool stretchblt_bilinear_mmx_can_do(const rgb_bitmap_info &dbmp,const rgb_bitmap_info &sbmp) {
    if (!dbmp.is_valid() || !sbmp.is_valid()) return false;
    if (!hostCPUcaps.mmx) return false;

    // must fit in buffer
    const size_t pixels_per_group =
        sizeof(__m64) / sizeof(T);

    if (sbmp.width >= (VINTERP_MAX*pixels_per_group))
        return false;

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
#endif // HAVE_MMX

bool stretchblt_bilinear_mmx_can_do(const rgb_bitmap_info &dbmp,const rgb_bitmap_info &sbmp) {
#if HAVE_CPU_MMX
    if (dbmp.bytes_per_pixel == sizeof(uint32_t))
        return stretchblt_bilinear_mmx_can_do<uint32_t>(dbmp,sbmp);
    else if (dbmp.bytes_per_pixel == sizeof(uint16_t))
        return stretchblt_bilinear_mmx_can_do<uint16_t>(dbmp,sbmp);
#endif
    return false;
}

// MMX alignment warning:
// It is strongly recommended that the bitmap base and scan line stride are MMX aligned, meaning
// that the base of the bitmap is aligned to 8 bytes and the scan line stride is a multiple of 8.
// Failure to do so may cause a performance loss.
template <class T> void stretchblt_bilinear_mmx(const rgb_bitmap_info &dbmp,const rgb_bitmap_info &sbmp) {
#if HAVE_CPU_MMX
    // WARNING: This code assumes typical RGBA type packing where red and blue are NOT adjacent, and alpha and green are not adjacent
    nr_wfpack sx={0,0},sy={0,0},stepx,stepy;
    static vinterp_tmp<__m64> vinterp_tmp;
    const T rbmask = (T)(dbmp.rgbinfo.r.mask+dbmp.rgbinfo.b.mask);
    const T abmask = (T)(dbmp.rgbinfo.g.mask+dbmp.rgbinfo.a.mask);
    __m64 rmask64,gmask64,bmask64,mul64;
    const size_t pixels_per_group =
        sizeof(__m64) / sizeof(T);
    const unsigned int src_bitmap_width_in_groups =
        (sbmp.width + pixels_per_group - 1) / pixels_per_group;
    unsigned char *drow;
    size_t ox,oy;
    T fshift;
    T pshift;
    T fmax;
    T mul;

    pshift = std::min(dbmp.rgbinfo.r.bwidth,std::min(dbmp.rgbinfo.g.bwidth,dbmp.rgbinfo.b.bwidth));
    fshift = (sizeof(nr_wftype) * 8) - pshift;
    fmax = 1U << pshift;

    if (sizeof(T) == 4) {
        rmask64 = _mm_set_pi16(0x00FF,0x00FF,0x00FF,0x00FF);
    }
    else {
        rmask64 = _mm_set_pi16(dbmp.rgbinfo.r.bmask,dbmp.rgbinfo.r.bmask,dbmp.rgbinfo.r.bmask,dbmp.rgbinfo.r.bmask);
        gmask64 = _mm_set_pi16(dbmp.rgbinfo.g.bmask,dbmp.rgbinfo.g.bmask,dbmp.rgbinfo.g.bmask,dbmp.rgbinfo.g.bmask);
        bmask64 = _mm_set_pi16(dbmp.rgbinfo.b.bmask,dbmp.rgbinfo.b.bmask,dbmp.rgbinfo.b.bmask,dbmp.rgbinfo.b.bmask);
    }

    render_scale_from_sd(/*&*/stepx,dbmp.width,sbmp.width);
    render_scale_from_sd(/*&*/stepy,dbmp.height,sbmp.height);
    if (dbmp.width == 0 || src_bitmap_width_in_groups == 0) return;

    drow = dbmp.get_scanline<uint8_t>(0);
    oy = dbmp.height;
    do {
        T *s2 = sbmp.get_scanline<T>(sy.w+1);
        T *s = sbmp.get_scanline<T>(sy.w);
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
                    stretchblt_line_bilinear_vinterp_stage_mmx_argb8(vinterp_tmp.tmp,(__m64*)s,(__m64*)s2,mul64,src_bitmap_width_in_groups,rmask64);
                else
                    stretchblt_line_bilinear_vinterp_stage_mmx_rgb16(vinterp_tmp.tmp,(__m64*)s,(__m64*)s2,mul64,src_bitmap_width_in_groups,
                        rmask64,dbmp.rgbinfo.r.shift,
                        gmask64,dbmp.rgbinfo.g.shift,
                        bmask64,dbmp.rgbinfo.b.shift);

                stretchblt_line_bilinear_hinterp_stage<T>(d,(T*)vinterp_tmp.tmp,sx,stepx,dbmp.width,rbmask,abmask,fmax,fshift,pshift);
            }
            else {
                // vertical interpolation only
                if (sizeof(T) == 4)
                    stretchblt_line_bilinear_vinterp_stage_mmx_argb8((__m64*)d,(__m64*)s,(__m64*)s2,mul64,src_bitmap_width_in_groups,rmask64);
                else
                    stretchblt_line_bilinear_vinterp_stage_mmx_rgb16((__m64*)d,(__m64*)s,(__m64*)s2,mul64,src_bitmap_width_in_groups,
                        rmask64,dbmp.rgbinfo.r.shift,
                        gmask64,dbmp.rgbinfo.g.shift,
                        bmask64,dbmp.rgbinfo.b.shift);
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

void stretchblt_bilinear_mmx(const rgb_bitmap_info &dbmp,const rgb_bitmap_info &sbmp) {
#if HAVE_CPU_MMX
    if (dbmp.bytes_per_pixel == sizeof(uint32_t))
        stretchblt_bilinear_mmx<uint32_t>(dbmp,sbmp);
    else if (dbmp.bytes_per_pixel == sizeof(uint16_t))
        stretchblt_bilinear_mmx<uint16_t>(dbmp,sbmp);
#endif
}

