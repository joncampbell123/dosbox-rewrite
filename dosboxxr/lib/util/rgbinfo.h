
#ifndef DOSBOXXR_LIB_UTIL_RGBINFO_H
#define DOSBOXXR_LIB_UTIL_RGBINFO_H

#include "dosboxxr/lib/util/bitscan.h"

// utility struct to store bit position, width, and mask of pixels.
// T is either uint16_t or uint32_t. This is ONLY for use with RGB
// pixel formats.
template <class T> struct rgbchannelinfo {
    uint8_t         shift;              // raw value = pixel << shift
    uint8_t         bwidth;             // width of the pixel value, in bits
    T               bmask;              // pmask = (1U << bwidth) - 1
    T               mask;               // mask = pmask << shift
public:
    rgbchannelinfo() : shift(0), bwidth(0), bmask(0), mask(0) { }
    void clear() {
        shift = 0;
        bwidth = 0;
        bmask = 0;
        mask = 0;
    }
    void setByMask(const T m) { // initialize this struct by mask (i.e. rgb mask provided by X windows)
        shift = bitscan_forward(m,0);
        bwidth = bitscan_count(m,shift) - shift;
        bmask = (uint32_t(1U) << uint32_t(bwidth)) - uint32_t(1U);
        mask = bmask << shift;
    }
};

template <class T> struct rgbinfo {
    struct rgbchannelinfo<T>    r,g,b,a;
public:
    void clear() {
        r.clear();
        g.clear();
        b.clear();
        a.clear();
    }
};

// default to uint32_t because ARGB 32-bit is very common today (8-bit),
// and some hardware like Intel chipsets also offer a 10-bit ARGB.
// 16-bit RGB RGB565 is not as common as it used to be.
typedef rgbchannelinfo<uint32_t>        rgbchannelinfo_t;
typedef rgbinfo<uint32_t>               rgbinfo_t;

#endif

