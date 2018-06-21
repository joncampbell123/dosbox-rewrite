
#include "dosboxxr/lib/util/rgbinfo.h"
#include "dosboxxr/lib/util/rgb_bitmap_info.h"

// this is relied on by other bilinear interpolation routines, and therefore declared in the header
template <class T> static inline T stretchblt_line_bilinear_pixel_blend(const T cur,const T nxt,const T mulmax,const T mul,const T rbmask,const T abmask,const T pshift) {
    T sum;

    sum  = (((uint64_t)(cur & rbmask) * (mulmax - mul)) >> pshift) & rbmask;
    sum += (((uint64_t)(cur & abmask) * (mulmax - mul)) >> pshift) & abmask;
    sum += (((uint64_t)(nxt & rbmask) *           mul ) >> pshift) & rbmask;
    sum += (((uint64_t)(nxt & abmask) *           mul ) >> pshift) & abmask;
    return sum;
}

// this is relied on by other bilinear interpolation routines, and therefore declared in the header
template <class T> static inline void stretchblt_line_bilinear_hinterp_stage(T *d,T *s,struct nr_wfpack sx,const struct nr_wfpack &stepx,size_t dwidth,const T rbmask,const T abmask,const T fmax,const T fshift,const T pshift) {
    do {
        *d++ = stretchblt_line_bilinear_pixel_blend<T>(s[sx.w],s[sx.w+1],fmax,(T)(sx.f >> (T)fshift),rbmask,abmask,pshift);
        sx += stepx;
    } while ((--dwidth) != (size_t)0);
}

template <class T> void stretchblt_bilinear(const rgb_bitmap_info &dbmp,const rgb_bitmap_info &sbmp);
void stretchblt_bilinear(const rgb_bitmap_info &dbmp,const rgb_bitmap_info &sbmp);
bool stretchblt_bilinear_can_do(const rgb_bitmap_info &dbmp,const rgb_bitmap_info &sbmp);

