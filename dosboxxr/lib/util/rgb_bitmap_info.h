
#ifndef DOSBOXXR_LIB_UTIL_RGB_BITMAP_INFO_H
#define DOSBOXXR_LIB_UTIL_RGB_BITMAP_INFO_H

#include <stdint.h>

#include "dosboxxr/lib/util/rgbinfo.h"

struct rgb_bitmap_info {
    // describe bitmap by base, width, height, alignment, length (of buffer), and stride.
    // this does NOT describe where it's allocated from, how to free it, etc. BY DESIGN.
    unsigned char*              base;           // base of the memory block
    unsigned char*              canvas;         // where to draw from
    size_t                      width,height;   // dimensions of frame
    size_t                      length;         // length of the buffer (last byte = base+length-1)
    size_t                      stride;         // bytes per scanline
    size_t                      bytes_per_pixel;// bytes per pixel
    size_t                      stride_align;   // required alignment per scanline
    size_t                      base_align;     // required alignment for base
    size_t                      stride_padding; // additional padding in pixels per scanline
    size_t                      height_padding; // additional padding in pixels, vertically
    rgbinfo_t                   rgbinfo;
public:
    rgb_bitmap_info() : base(NULL), canvas(NULL), width(0), height(0), length(0), stride(0), bytes_per_pixel(0), stride_align(0), base_align(0),
        stride_padding(0), height_padding(0), rgbinfo() { }
    bool inline is_valid() const /* does not change class members */ {
        if (base == NULL) return false;
        if (canvas == NULL) return false;
        if (length == 0) return false;
        if (stride == 0) return false;
        if (width == 0 || height == 0) return false;
        if (bytes_per_pixel == 0 || bytes_per_pixel > 8) return false;
        if ((width*bytes_per_pixel) > stride) return false;
        if ((height*stride) > length) return false;
        return true;
    }
    template <class T> inline T* get_scanline(const size_t y,const size_t x=0) const { /* WARNING: does not range-check 'y', assumes canvas != NULL */
        return (T*)(canvas + (y * stride) + (x * bytes_per_pixel));
    }
    void update_stride_from_width() {
        stride = ((width + stride_padding) * bytes_per_pixel);
        if (stride_align > 0) {
            size_t a = stride % stride_align;
            if (a != 0) stride += stride_align - a;
        }
    }
    void update_length_from_stride_and_height() {
        length = stride * (height + height_padding);
    }
    void update_canvas_from_base() {
        canvas = base;
    }
};

#endif

