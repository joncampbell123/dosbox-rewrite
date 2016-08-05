
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
    void clear() {
        base = NULL;
        canvas = NULL;
        width = 0;
        height = 0;
        length = 0;
        stride = 0;
        bytes_per_pixel = 0;
        stride_align = 0;
        base_align = 0;
        stride_padding = 0;
        height_padding = 0;
        rgbinfo.clear();
    }
    bool inline is_dim_valid() const {
        if (length == 0) return false;
        if (stride == 0) return false;
        if (width == 0 || height == 0) return false;
        if (bytes_per_pixel == 0 || bytes_per_pixel > 8) return false;
        if ((width*bytes_per_pixel) > stride) return false;
        if ((height*stride) > length) return false;
        return true;
    }
    bool inline is_valid() const /* does not change class members */ {
        if (base == NULL) return false;
        if (canvas == NULL) return false;
        if (!is_dim_valid()) return false;
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

    // Additional note regarding 24bpp RGB.
    // This structure can support and represent 24 bits/pixel RGB formats, even though it is uncommon and not often used
    // in video playback and graphics support code. However, the code in this project imposes one additional requirement
    // regarding the stride and length of the bitmap.
    //
    // For performance reasons, a 24bpp pixel is typecast as uint32_t which then (on little endian processors) places the
    // 24bpp pixel in the lower 24 bits of the 32-bit unsigned int where this code can work on it. Because of the 32-bit
    // typedef, it is easily possible for such code to read 1 byte past the last pixel, and possibly 1 byte past the end
    // of the bitmap. For this reason, 24bpp rendering code in this project will demand that you pad the stride of the bitmap
    // such that:
    //
    //      stride > (width * 3)
    //
    // Meaning that each scan line must have at least 1 additional byte of padding to cover for the 32-bit memory access
    // per pixel without segfaulting or accessing memory out of bounds. Such code *by design* will refuse to render 24bpp
    // unless this condition is met!
    //
    // Because of the odd byte count per pixel in 24bpp, not every graphics routine in this project will implement or
    // support 24bpp RGB. The routines that do will always be a subset of the overall graphics rendering library.
    //
    // For the historical record, 24bpp was once a common framebuffer format in the early to mid 1990's when SVGA graphics
    // cards were becoming popular. However, it became more efficient both for hardware and software to instead implement
    // "true color" as 32-bit XRGB since 32-bit (4 bytes) is a power of 2 size and it lines up nicely with the 32-bit
    // integer datatype of the CPU. Today, most video cards implement the 32bpp, 16bpp, or 8bpp paletted modes, and they 
    // leave out the 24bpp mode entirely.
};

#endif

