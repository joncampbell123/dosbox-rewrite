
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
#include "dosboxxr/lib/graphics/stretchblt_bilinear_sse.h"

#include <algorithm>

#if HAVE_CPU_SSE2
# include <xmmintrin.h>

// 32bpp optimized for 8-bit ARGB/RGBA. rmask should be 0x00FF,0x00FF,... etc
static inline __m128i stretchblt_line_bilinear_pixel_blend_sse_argb8(const __m128i cur,const __m128i nxt,const __m128i mul,const __m128i rmask) {
    __m128i d1,d2,d3,d4;

    d1 = _mm_and_si128(_mm_mulhi_epi16(_mm_sub_epi16(_mm_and_si128(nxt,rmask),_mm_and_si128(cur,rmask)),mul),rmask);
    d2 = _mm_slli_si128(_mm_and_si128(_mm_mulhi_epi16(_mm_sub_epi16(_mm_and_si128(_mm_srli_si128(nxt,1/*bytes!*/),rmask),_mm_and_si128(_mm_srli_si128(cur,1/*bytes!*/),rmask)),mul),rmask),1/*bytes!*/);
    d3 = _mm_add_epi8(d1,d2);
    d4 = _mm_add_epi8(d3,d3);
    return _mm_add_epi8(d4,cur);
}

// 16bpp general R/G/B, usually 5/6/5 or 5/5/5
static inline __m128i stretchblt_line_bilinear_pixel_blend_sse_rgb16(const __m128i cur,const __m128i nxt,const __m128i mul,const __m128i rmask,const uint16_t rshift,const __m128i gmask,const uint16_t gshift,const __m128i bmask,const uint16_t bshift) {
    __m128i rc,gc,bc;
    __m128i rn,gn,bn;
    __m128i d,sum;

    rc = _mm_and_si128(_mm_srli_epi16(cur,rshift),rmask);
    gc = _mm_and_si128(_mm_srli_epi16(cur,gshift),gmask);
    bc = _mm_and_si128(_mm_srli_epi16(cur,bshift),bmask);

    rn = _mm_and_si128(_mm_srli_epi16(nxt,rshift),rmask);
    gn = _mm_and_si128(_mm_srli_epi16(nxt,gshift),gmask);
    bn = _mm_and_si128(_mm_srli_epi16(nxt,bshift),bmask);

    d = _mm_sub_epi16(rn,rc);
    sum = _mm_slli_epi16(_mm_add_epi16(rc,_mm_and_si128(_mm_mulhi_epi16(_mm_add_epi16(d,d),mul),rmask)),rshift);

    d = _mm_sub_epi16(gn,gc);
    sum = _mm_add_epi16(_mm_slli_epi16(_mm_add_epi16(gc,_mm_and_si128(_mm_mulhi_epi16(_mm_add_epi16(d,d),mul),gmask)),gshift),sum);

    d = _mm_sub_epi16(bn,bc);
    sum = _mm_add_epi16(_mm_slli_epi16(_mm_add_epi16(bc,_mm_and_si128(_mm_mulhi_epi16(_mm_add_epi16(d,d),mul),bmask)),bshift),sum);

    return sum;
}

// case 2: 32-bit ARGB 8-bits per pixel
static inline void stretchblt_line_bilinear_vinterp_stage_sse_argb8(__m128i *d,__m128i *s,__m128i *s2,const __m128i mul,size_t width,const __m128i rmask) {
    do {
        *d++ = stretchblt_line_bilinear_pixel_blend_sse_argb8(*s++,*s2++,mul,rmask);
    } while ((--width) != (size_t)0);
}

// case 1: 16-bit arbitrary masks
static inline void stretchblt_line_bilinear_vinterp_stage_sse_rgb16(__m128i *d,__m128i *s,__m128i *s2,const __m128i mul,size_t width,const __m128i rmask,const uint16_t rshift,const __m128i gmask,const uint16_t gshift,const __m128i bmask,const uint16_t bshift) {
    do {
        *d++ = stretchblt_line_bilinear_pixel_blend_sse_rgb16(*s++,*s2++,mul,rmask,rshift,gmask,gshift,bmask,bshift);
    } while ((--width) != (size_t)0);
}

// SSE2 alignment is required. While modern processors (Intel Core 2, etc) will tolerate
// some unaligned SSE2 the older SSE2 enabled processors (Pentium 4) are stricter about it.
template <class T> bool stretchblt_bilinear_sse_can_do(const rgb_bitmap_info &dbmp,const rgb_bitmap_info &sbmp) {
    if (!dbmp.is_valid() || !sbmp.is_valid()) return false;
    if (!hostCPUcaps.sse2) return false;

    // must fit in buffer
    const size_t pixels_per_group =
        sizeof(__m128i) / sizeof(T);

    if (sbmp.width >= (VINTERP_MAX*pixels_per_group))
        return false;

    // buffer alignment is required, else SSE will fault
    if (((size_t)sbmp.canvas & 15) != 0) return false;
    if (((size_t)dbmp.canvas & 15) != 0) return false;

    // stride alignment is required, else SSE will fault
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

bool stretchblt_bilinear_sse_can_do(const rgb_bitmap_info &dbmp,const rgb_bitmap_info &sbmp) {
#if HAVE_CPU_SSE2
    if (dbmp.bytes_per_pixel == sizeof(uint32_t))
        return stretchblt_bilinear_sse_can_do<uint32_t>(dbmp,sbmp);
    else if (dbmp.bytes_per_pixel == sizeof(uint16_t))
        return stretchblt_bilinear_sse_can_do<uint16_t>(dbmp,sbmp);
#endif
    return false;
}

// SSE alignment warning:
// Do not use this routine unless the scan lines in the bitmap are all aligned to SSE boundaries,
// meaning that the bitmap base is aligned to a 16 byte boundary and the scan line "stride" is a
// multiple of 16 bytes. Modern processors (like Intel Core 2 and such) tolerate misaligned SSE
// but older processors (like the Pentium 4) will throw an exception if SSE instructions are
// executed on misaligned memory. Besides that, there may be performance loss if SSE operations
// are misaligned.
template <class T> void stretchblt_bilinear_sse(const rgb_bitmap_info &dbmp,const rgb_bitmap_info &sbmp) {
#if HAVE_CPU_SSE2
    // WARNING: This code assumes typical RGBA type packing where red and blue are NOT adjacent, and alpha and green are not adjacent
    nr_wfpack sx={0,0},sy={0,0},stepx,stepy;
    static vinterp_tmp<__m128i> vinterp_tmp;
    const T rbmask = (T)(dbmp.rgbinfo.r.mask+dbmp.rgbinfo.b.mask);
    const T abmask = (T)(dbmp.rgbinfo.g.mask+dbmp.rgbinfo.a.mask);
    __m128i rmask128,gmask128,bmask128,mul128;
    const size_t pixels_per_group =
        sizeof(__m128i) / sizeof(T);
    unsigned int src_bitmap_width_in_groups =
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
        rmask128 = _mm_set_epi16(0x00FF,0x00FF,0x00FF,0x00FF,0x00FF,0x00FF,0x00FF,0x00FF);
    }
    else {
        rmask128 = _mm_set_epi16(
            dbmp.rgbinfo.r.bmask,dbmp.rgbinfo.r.bmask,dbmp.rgbinfo.r.bmask,dbmp.rgbinfo.r.bmask,
            dbmp.rgbinfo.r.bmask,dbmp.rgbinfo.r.bmask,dbmp.rgbinfo.r.bmask,dbmp.rgbinfo.r.bmask);
        gmask128 = _mm_set_epi16(
            dbmp.rgbinfo.g.bmask,dbmp.rgbinfo.g.bmask,dbmp.rgbinfo.g.bmask,dbmp.rgbinfo.g.bmask,
            dbmp.rgbinfo.g.bmask,dbmp.rgbinfo.g.bmask,dbmp.rgbinfo.g.bmask,dbmp.rgbinfo.g.bmask);
        bmask128 = _mm_set_epi16(
            dbmp.rgbinfo.b.bmask,dbmp.rgbinfo.b.bmask,dbmp.rgbinfo.b.bmask,dbmp.rgbinfo.b.bmask,
            dbmp.rgbinfo.b.bmask,dbmp.rgbinfo.b.bmask,dbmp.rgbinfo.b.bmask,dbmp.rgbinfo.b.bmask);
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
            mul128 = _mm_set_epi16(m,m,m,m,m,m,m,m);
        }

        if (mul != 0) {
            if (stepx.w != 1 || stepx.f != 0) {
                // horizontal interpolation, vertical interpolation
                if (sizeof(T) == 4)
                    stretchblt_line_bilinear_vinterp_stage_sse_argb8(vinterp_tmp.tmp,(__m128i*)s,(__m128i*)s2,mul128,src_bitmap_width_in_groups,rmask128);
                else
                    stretchblt_line_bilinear_vinterp_stage_sse_rgb16(vinterp_tmp.tmp,(__m128i*)s,(__m128i*)s2,mul128,src_bitmap_width_in_groups,
                        rmask128,dbmp.rgbinfo.r.shift,
                        gmask128,dbmp.rgbinfo.g.shift,
                        bmask128,dbmp.rgbinfo.b.shift);

                stretchblt_line_bilinear_hinterp_stage<T>(d,(T*)vinterp_tmp.tmp,sx,stepx,dbmp.width,rbmask,abmask,fmax,fshift,pshift);
            }
            else {
                // vertical interpolation only
                if (sizeof(T) == 4)
                    stretchblt_line_bilinear_vinterp_stage_sse_argb8((__m128i*)d,(__m128i*)s,(__m128i*)s2,mul128,src_bitmap_width_in_groups,rmask128);
                else
                    stretchblt_line_bilinear_vinterp_stage_sse_rgb16((__m128i*)d,(__m128i*)s,(__m128i*)s2,mul128,src_bitmap_width_in_groups,
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

void stretchblt_bilinear_sse(const rgb_bitmap_info &dbmp,const rgb_bitmap_info &sbmp) {
#if HAVE_CPU_SSE2
    if (dbmp.bytes_per_pixel == sizeof(uint32_t))
        stretchblt_bilinear_sse<uint32_t>(dbmp,sbmp);
    else if (dbmp.bytes_per_pixel == sizeof(uint16_t))
        stretchblt_bilinear_sse<uint16_t>(dbmp,sbmp);
#endif
}

