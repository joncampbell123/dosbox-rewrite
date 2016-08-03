
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

#include <algorithm>

template <class T> inline void stretchblt_line_bilinear_vinterp_stage(T *d,T *s,T *s2,const T mul,size_t width,const T rbmask,const T abmask,const T fmax,const T pshift) {
    do {
        *d++ = stretchblt_line_bilinear_pixel_blend<T>(*s++,*s2++,fmax,mul,rbmask,abmask,pshift);
    } while ((--width) != (size_t)0);
}

template <class T> void stretchblt_bilinear(const rgb_bitmap_info &dbmp,const rgb_bitmap_info &sbmp) {
    // WARNING: This code assumes typical RGBA type packing where red and blue are NOT adjacent, and alpha and green are not adjacent
    const T rbmask = (T)(dbmp.rgbinfo.r.mask+dbmp.rgbinfo.b.mask);
    const T abmask = (T)(dbmp.rgbinfo.g.mask+dbmp.rgbinfo.a.mask);
    nr_wfpack sx={0,0},sy={0,0},stepx,stepy;
    static vinterp_tmp<T> vinterp_tmp;
    unsigned char *drow;
    size_t ox,oy;
    T fshift;
    T pshift;
    T fmax;
    T mul;

    pshift = std::min(dbmp.rgbinfo.r.bwidth,std::min(dbmp.rgbinfo.g.bwidth,dbmp.rgbinfo.b.bwidth));
    fshift = (sizeof(nr_wftype) * 8) - pshift;
    fmax = 1U << pshift;

    render_scale_from_sd(/*&*/stepx,dbmp.width,sbmp.width);
    render_scale_from_sd(/*&*/stepy,dbmp.height,sbmp.height);

    if (dbmp.width == 0 || sbmp.width == 0) return;

    drow = dbmp.get_scanline<uint8_t>(0);
    oy = dbmp.height;
    do {
        T *s2 = sbmp.get_scanline<T>(sy.w+1);
        T *s = sbmp.get_scanline<T>(sy.w);
        T *d = (T*)drow;

        mul = (T)(sy.f >> fshift);
        if (mul != 0) {
            if (stepx.w != 1 || stepx.f != 0) {
                // horizontal interpolation, vertical interpolation
                stretchblt_line_bilinear_vinterp_stage<T>(vinterp_tmp.tmp,s,s2,mul,sbmp.width,rbmask,abmask,fmax,pshift);
                stretchblt_line_bilinear_hinterp_stage<T>(d,vinterp_tmp.tmp,sx,stepx,dbmp.width,rbmask,abmask,fmax,fshift,pshift);
            }
            else {
                // vertical interpolation only
                stretchblt_line_bilinear_vinterp_stage<T>(d,s,s2,mul,sbmp.width,rbmask,abmask,fmax,pshift);
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
}

void stretchblt_bilinear(const rgb_bitmap_info &dbmp,const rgb_bitmap_info &sbmp) {
    if (dbmp.bytes_per_pixel == sizeof(uint32_t))
        stretchblt_bilinear<uint32_t>(dbmp,sbmp);
    else if (dbmp.bytes_per_pixel == sizeof(uint16_t))
        stretchblt_bilinear<uint16_t>(dbmp,sbmp);
}

template <class T> bool stretchblt_bilinear_can_do(const rgb_bitmap_info &dbmp,const rgb_bitmap_info &sbmp) {
    if (!dbmp.is_valid() || !sbmp.is_valid()) return false;

    if (sbmp.width >= VINTERP_MAX)
        return false;

    return true;
}

bool stretchblt_bilinear_can_do(const rgb_bitmap_info &dbmp,const rgb_bitmap_info &sbmp) {
    if (dbmp.bytes_per_pixel == sizeof(uint32_t))
        return stretchblt_bilinear_can_do<uint32_t>(dbmp,sbmp);
    else if (dbmp.bytes_per_pixel == sizeof(uint16_t))
        return stretchblt_bilinear_can_do<uint16_t>(dbmp,sbmp);
    return true;
}

