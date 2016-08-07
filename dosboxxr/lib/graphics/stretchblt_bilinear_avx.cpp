
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
#include "dosboxxr/lib/util/rgbinfo.h"
#include "dosboxxr/lib/util/vinterp_tmp.h"
#include "dosboxxr/lib/graphics/stretchblt_bilinear.h"
#include "dosboxxr/lib/graphics/stretchblt_bilinear_avx.h"

#include <algorithm>

#if HAVE_CPU_AVX2
# include <immintrin.h>

// 32bpp optimized for 8-bit ARGB/RGBA. rmask should be 0x00FF,0x00FF,... etc
static inline __m256i stretchblt_line_bilinear_pixel_blend_avx_argb8(const __m256i cur,const __m256i nxt,const __m256i mul,const __m256i rmask) {
    __m256i rc,gc;
    __m256i rn,gn;
    __m256i d,sum;

    rc = _mm256_and_si256(                  cur   ,rmask);
    gc = _mm256_and_si256(_mm256_srli_epi16(cur,8),rmask);

    rn = _mm256_and_si256(               nxt   ,rmask);
    gn = _mm256_and_si256(_mm256_srli_epi16(nxt,8),rmask);

    d = _mm256_sub_epi16(rn,rc);
    sum = _mm256_add_epi16(rc,_mm256_mulhi_epi16(_mm256_add_epi16(d,d),mul));

    d = _mm256_sub_epi16(gn,gc);
    sum = _mm256_add_epi16(_mm256_slli_epi16(_mm256_add_epi16(gc,_mm256_mulhi_epi16(_mm256_add_epi16(d,d),mul)),8),sum);

    return sum;
}

// 16bpp general R/G/B, usually 5/6/5 or 5/5/5
static inline __m256i stretchblt_line_bilinear_pixel_blend_avx_rgb16(const __m256i cur,const __m256i nxt,const __m256i mul,const __m256i rmask,const uint16_t rshift,const __m256i gmask,const uint16_t gshift,const __m256i bmask,const uint16_t bshift) {
    __m256i rc,gc,bc;
    __m256i rn,gn,bn;
    __m256i d,sum;

    rc = _mm256_and_si256(_mm256_srli_epi16(cur,rshift),rmask);
    gc = _mm256_and_si256(_mm256_srli_epi16(cur,gshift),gmask);
    bc = _mm256_and_si256(_mm256_srli_epi16(cur,bshift),bmask);

    rn = _mm256_and_si256(_mm256_srli_epi16(nxt,rshift),rmask);
    gn = _mm256_and_si256(_mm256_srli_epi16(nxt,gshift),gmask);
    bn = _mm256_and_si256(_mm256_srli_epi16(nxt,bshift),bmask);

    d = _mm256_sub_epi16(rn,rc);
    sum = _mm256_slli_epi16(_mm256_add_epi16(rc,_mm256_mulhi_epi16(_mm256_add_epi16(d,d),mul)),rshift);

    d = _mm256_sub_epi16(gn,gc);
    sum = _mm256_add_epi16(_mm256_slli_epi16(_mm256_add_epi16(gc,_mm256_mulhi_epi16(_mm256_add_epi16(d,d),mul)),gshift),sum);

    d = _mm256_sub_epi16(bn,bc);
    sum = _mm256_add_epi16(_mm256_slli_epi16(_mm256_add_epi16(bc,_mm256_mulhi_epi16(_mm256_add_epi16(d,d),mul)),bshift),sum);

    return sum;
}

// case 2: 32-bit ARGB 8-bits per pixel
static inline void stretchblt_line_bilinear_vinterp_stage_avx_argb8(__m256i *d,__m256i *s,__m256i *s2,const __m256i mul,size_t width,const __m256i rmask) {
    do {
        *d++ = stretchblt_line_bilinear_pixel_blend_avx_argb8(*s++,*s2++,mul,rmask);
    } while ((--width) != (size_t)0);
}

// case 1: 16-bit arbitrary masks
static inline void stretchblt_line_bilinear_vinterp_stage_avx_rgb16(__m256i *d,__m256i *s,__m256i *s2,const __m256i mul,size_t width,const __m256i rmask,const uint16_t rshift,const __m256i gmask,const uint16_t gshift,const __m256i bmask,const uint16_t bshift) {
    do {
        *d++ = stretchblt_line_bilinear_pixel_blend_avx_rgb16(*s++,*s2++,mul,rmask,rshift,gmask,gshift,bmask,bshift);
    } while ((--width) != (size_t)0);
}
#endif

// AVX alignment warning:
// Do not use this routine unless the scan lines in the bitmap are all aligned to AVX boundaries,
// meaning that the bitmap base is aligned to a 32 byte boundary and the scan line "stride" is a
// multiple of 32 bytes. Besides the performance loss of unaligned access, program testing on
// current Intel hardware says that unaligned access triggers a fault from the processor.
//
// NOTE: Experience on my development laptop says this isn't much faster than SSE. However
//       I'm going to assume that's just my laptop, and that maybe in the future, AVX will
//       get faster.
template <class T> void stretchblt_bilinear_avx(const rgb_bitmap_info &dbmp,const rgb_bitmap_info &sbmp) {
#if HAVE_CPU_AVX2
    // WARNING: This code assumes typical RGBA type packing where red and blue are NOT adjacent, and alpha and green are not adjacent
    nr_wfpack sx={0,0},sy={0,0},stepx,stepy;
    static vinterp_tmp<__m256i> vinterp_tmp;
    const T rbmask = (T)(dbmp.rgbinfo.r.mask+dbmp.rgbinfo.b.mask);
    const T abmask = (T)(dbmp.rgbinfo.g.mask+dbmp.rgbinfo.a.mask);
    __m256i rmask256,gmask256,bmask256,mul256;
    const size_t pixels_per_group =
        sizeof(__m256i) / sizeof(T);
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
        rmask256 = _mm256_set_epi16(
            0x00FF,0x00FF,0x00FF,0x00FF,0x00FF,0x00FF,0x00FF,0x00FF,
            0x00FF,0x00FF,0x00FF,0x00FF,0x00FF,0x00FF,0x00FF,0x00FF);
    }
    else {
        rmask256 = _mm256_set_epi16(
            dbmp.rgbinfo.r.bmask,dbmp.rgbinfo.r.bmask,dbmp.rgbinfo.r.bmask,dbmp.rgbinfo.r.bmask,
            dbmp.rgbinfo.r.bmask,dbmp.rgbinfo.r.bmask,dbmp.rgbinfo.r.bmask,dbmp.rgbinfo.r.bmask,
            dbmp.rgbinfo.r.bmask,dbmp.rgbinfo.r.bmask,dbmp.rgbinfo.r.bmask,dbmp.rgbinfo.r.bmask,
            dbmp.rgbinfo.r.bmask,dbmp.rgbinfo.r.bmask,dbmp.rgbinfo.r.bmask,dbmp.rgbinfo.r.bmask);
        gmask256 = _mm256_set_epi16(
            dbmp.rgbinfo.g.bmask,dbmp.rgbinfo.g.bmask,dbmp.rgbinfo.g.bmask,dbmp.rgbinfo.g.bmask,
            dbmp.rgbinfo.g.bmask,dbmp.rgbinfo.g.bmask,dbmp.rgbinfo.g.bmask,dbmp.rgbinfo.g.bmask,
            dbmp.rgbinfo.g.bmask,dbmp.rgbinfo.g.bmask,dbmp.rgbinfo.g.bmask,dbmp.rgbinfo.g.bmask,
            dbmp.rgbinfo.g.bmask,dbmp.rgbinfo.g.bmask,dbmp.rgbinfo.g.bmask,dbmp.rgbinfo.g.bmask);
        bmask256 = _mm256_set_epi16(
            dbmp.rgbinfo.b.bmask,dbmp.rgbinfo.b.bmask,dbmp.rgbinfo.b.bmask,dbmp.rgbinfo.b.bmask,
            dbmp.rgbinfo.b.bmask,dbmp.rgbinfo.b.bmask,dbmp.rgbinfo.b.bmask,dbmp.rgbinfo.b.bmask,
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
            mul256 = _mm256_set_epi16(
                m,m,m,m,m,m,m,m,
                m,m,m,m,m,m,m,m);
        }

        if (mul != 0) {
            if (stepx.w != 1 || stepx.f != 0) {
                // horizontal interpolation, vertical interpolation
                if (sizeof(T) == 4)
                    stretchblt_line_bilinear_vinterp_stage_avx_argb8(vinterp_tmp.tmp,(__m256i*)s,(__m256i*)s2,mul256,src_bitmap_width_in_groups,rmask256);
                else
                    stretchblt_line_bilinear_vinterp_stage_avx_rgb16(vinterp_tmp.tmp,(__m256i*)s,(__m256i*)s2,mul256,src_bitmap_width_in_groups,
                        rmask256,dbmp.rgbinfo.r.shift,
                        gmask256,dbmp.rgbinfo.g.shift,
                        bmask256,dbmp.rgbinfo.b.shift);

                stretchblt_line_bilinear_hinterp_stage<T>(d,(T*)vinterp_tmp.tmp,sx,stepx,dbmp.width,rbmask,abmask,fmax,fshift,pshift);
            }
            else {
                // vertical interpolation only
                if (sizeof(T) == 4)
                    stretchblt_line_bilinear_vinterp_stage_avx_argb8((__m256i*)d,(__m256i*)s,(__m256i*)s2,mul256,src_bitmap_width_in_groups,rmask256);
                else
                    stretchblt_line_bilinear_vinterp_stage_avx_rgb16((__m256i*)d,(__m256i*)s,(__m256i*)s2,mul256,src_bitmap_width_in_groups,
                        rmask256,dbmp.rgbinfo.r.shift,
                        gmask256,dbmp.rgbinfo.g.shift,
                        bmask256,dbmp.rgbinfo.b.shift);
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

void stretchblt_bilinear_avx(const rgb_bitmap_info &dbmp,const rgb_bitmap_info &sbmp) {
#if HAVE_CPU_AVX2
    if (dbmp.bytes_per_pixel == sizeof(uint32_t))
        stretchblt_bilinear_avx<uint32_t>(dbmp,sbmp);
    else if (dbmp.bytes_per_pixel == sizeof(uint16_t))
        stretchblt_bilinear_avx<uint16_t>(dbmp,sbmp);
#endif
}

#if HAVE_CPU_AVX2
// AVX2 alignment is required.
template <class T> bool stretchblt_bilinear_avx_can_do(const rgb_bitmap_info &dbmp,const rgb_bitmap_info &sbmp) {
    if (!dbmp.is_valid() || !sbmp.is_valid()) return false;
    if (!hostCPUcaps.avx2) return false;

# if IS_WINDOWS && IS_MINGW && defined(__x86_64__)
    // FIXME: I'm having trouble getting this code to work in 64-bit Windows without crashing in the vinterp stage.
    //        It doesn't seem to be stack alignment, in fact GDB confirms %rbp is 32-byte aligned when it happens.
    //        The problem seems to be the compiler allocating __m256i variables at misaligned addresses and then
    //        accessing them. I will remove this if later updates to MinGW fix this issue.
    return false;
# endif

    // must fit in buffer
    const size_t pixels_per_group =
        sizeof(__m256i) / sizeof(T);

    if (sbmp.width >= (VINTERP_MAX*pixels_per_group))
        return false;

    // buffer alignment is required, else AVX will fault
    if (((size_t)sbmp.canvas & 31) != 0) return false;
    if (((size_t)dbmp.canvas & 31) != 0) return false;

    // stride alignment is required, else AVX will fault
    if (((size_t)sbmp.stride & 31) != 0) return false;
    if (((size_t)dbmp.stride & 31) != 0) return false;

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

bool stretchblt_bilinear_avx_can_do(const rgb_bitmap_info &dbmp,const rgb_bitmap_info &sbmp) {
#if HAVE_CPU_AVX2
    if (dbmp.bytes_per_pixel == sizeof(uint32_t))
        return stretchblt_bilinear_avx_can_do<uint32_t>(dbmp,sbmp);
    else if (dbmp.bytes_per_pixel == sizeof(uint16_t))
        return stretchblt_bilinear_avx_can_do<uint16_t>(dbmp,sbmp);
#endif
    return false;
}

