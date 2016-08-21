
#ifdef HAVE_CONFIG_H
# include "config.h" // must be first
#endif

#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "dosboxxr/lib/util/bitscan.h"
#include "dosboxxr/lib/util/rgbinfo.h"
#include "dosboxxr/lib/util/rgb_bitmap_info.h"
#include "dosboxxr/lib/util/rgb_bitmap_test_patterns.h"

template <class T> void test_pattern_1_render(rgb_bitmap_info &bitmap) {
    T r,g,b,*drow;
    size_t ox,oy;

    if (!bitmap.is_valid()) return;

    for (oy=0;oy < (bitmap.height/2);oy++) {
        drow = bitmap.get_scanline<T>(oy);
        for (ox=0;ox < (bitmap.width/2);ox++) {
            /* color */
            r = ((T)ox * (T)bitmap.rgbinfo.r.bmask) / (T)(bitmap.width/2);
            g = ((T)oy * (T)bitmap.rgbinfo.g.bmask) / (T)(bitmap.height/2);
            b = (T)bitmap.rgbinfo.b.bmask -
                (((T)ox * (T)bitmap.rgbinfo.b.bmask) / (T)(bitmap.width/2));

            *drow++ = (r << bitmap.rgbinfo.r.shift) + (g << bitmap.rgbinfo.g.shift) + (b << bitmap.rgbinfo.b.shift) + bitmap.rgbinfo.a.mask;
        }
    }

    for (oy=0;oy < (bitmap.height/2);oy++) {
        drow = bitmap.get_scanline<T>(oy,bitmap.width/2U);
        for (ox=(bitmap.width/2);ox < bitmap.width;ox++) {
            /* color */
            r = ((ox ^ oy) & 1) ? bitmap.rgbinfo.r.bmask : 0;
            g = ((ox ^ oy) & 1) ? bitmap.rgbinfo.g.bmask : 0;
            b = ((ox ^ oy) & 1) ? bitmap.rgbinfo.b.bmask : 0;

            *drow++ = (r << bitmap.rgbinfo.r.shift) + (g << bitmap.rgbinfo.g.shift) + (b << bitmap.rgbinfo.b.shift) + bitmap.rgbinfo.a.mask;
        }
    }

    for (oy=(bitmap.height/2);oy < bitmap.height;oy++) {
        drow = bitmap.get_scanline<T>(oy);
        for (ox=0;ox < (bitmap.width/2);ox++) {
            /* color */
            r = ((ox ^ oy) & 2) ? bitmap.rgbinfo.r.bmask : 0;
            g = ((ox ^ oy) & 2) ? bitmap.rgbinfo.g.bmask : 0;
            b = ((ox ^ oy) & 2) ? bitmap.rgbinfo.b.bmask : 0;

            *drow++ = (r << bitmap.rgbinfo.r.shift) + (g << bitmap.rgbinfo.g.shift) + (b << bitmap.rgbinfo.b.shift) + bitmap.rgbinfo.a.mask;
        }
    }

    for (oy=(bitmap.height/2);oy < bitmap.height;oy++) {
        drow = bitmap.get_scanline<T>(oy,bitmap.width/2U);
        for (ox=(bitmap.width/2);ox < bitmap.width;ox++) {
            /* color */
            r = ((ox ^ oy) & 4) ? bitmap.rgbinfo.r.bmask : 0;
            g = ((ox ^ oy) & 4) ? bitmap.rgbinfo.g.bmask : 0;
            b = ((ox ^ oy) & 4) ? bitmap.rgbinfo.b.bmask : 0;

            *drow++ = (r << bitmap.rgbinfo.r.shift) + (g << bitmap.rgbinfo.g.shift) + (b << bitmap.rgbinfo.b.shift) + bitmap.rgbinfo.a.mask;
        }
    }
}

template <> void test_pattern_1_render<rgb24bpp_t>(rgb_bitmap_info &bitmap) { // special case. 24bpp is an odd format that can't be typecast.
    rgb24bpp_t *drow;
    uint32_t r,g,b;
    size_t ox,oy;

    if (!bitmap.is_valid()) return;

    // due to the nature of this code, additional padding is required.
    // this code may write 1 byte past the last pixel.
    if ((bitmap.stride*bitmap.height) >= bitmap.length && (bitmap.width*3) >= bitmap.stride) {
        fprintf(stderr,"Test pattern 1 24bpp case: at least one pixel of padding is required. (%ux%u) >= %u && (%u*3) >= %u\n",
            (unsigned int)bitmap.stride,
            (unsigned int)bitmap.height,
            (unsigned int)bitmap.length,
            (unsigned int)bitmap.width,
            (unsigned int)bitmap.stride);
        return;
    }

    for (oy=0;oy < (bitmap.height/2);oy++) {
        drow = bitmap.get_scanline<rgb24bpp_t>(oy);
        for (ox=0;ox < (bitmap.width/2);ox++) {
            /* color */
            r = ((uint32_t)ox * (uint32_t)bitmap.rgbinfo.r.bmask) / (uint32_t)(bitmap.width/2);
            g = ((uint32_t)oy * (uint32_t)bitmap.rgbinfo.g.bmask) / (uint32_t)(bitmap.height/2);
            b = (uint32_t)bitmap.rgbinfo.b.bmask -
                (((uint32_t)ox * (uint32_t)bitmap.rgbinfo.b.bmask) / (uint32_t)(bitmap.width/2));

            *((uint32_t*)(drow++)) = (r << bitmap.rgbinfo.r.shift) + (g << bitmap.rgbinfo.g.shift) + (b << bitmap.rgbinfo.b.shift) + bitmap.rgbinfo.a.mask;
        }
    }

    for (oy=0;oy < (bitmap.height/2);oy++) {
        drow = bitmap.get_scanline<rgb24bpp_t>(oy,bitmap.width/2U);
        for (ox=(bitmap.width/2);ox < bitmap.width;ox++) {
            /* color */
            r = ((ox ^ oy) & 1) ? bitmap.rgbinfo.r.bmask : 0;
            g = ((ox ^ oy) & 1) ? bitmap.rgbinfo.g.bmask : 0;
            b = ((ox ^ oy) & 1) ? bitmap.rgbinfo.b.bmask : 0;

            *((uint32_t*)(drow++)) = (r << bitmap.rgbinfo.r.shift) + (g << bitmap.rgbinfo.g.shift) + (b << bitmap.rgbinfo.b.shift) + bitmap.rgbinfo.a.mask;
        }
    }

    for (oy=(bitmap.height/2);oy < bitmap.height;oy++) {
        drow = bitmap.get_scanline<rgb24bpp_t>(oy);
        for (ox=0;ox < (bitmap.width/2);ox++) {
            /* color */
            r = ((ox ^ oy) & 2) ? bitmap.rgbinfo.r.bmask : 0;
            g = ((ox ^ oy) & 2) ? bitmap.rgbinfo.g.bmask : 0;
            b = ((ox ^ oy) & 2) ? bitmap.rgbinfo.b.bmask : 0;

            *((uint32_t*)(drow++)) = (r << bitmap.rgbinfo.r.shift) + (g << bitmap.rgbinfo.g.shift) + (b << bitmap.rgbinfo.b.shift) + bitmap.rgbinfo.a.mask;
        }
    }

    for (oy=(bitmap.height/2);oy < bitmap.height;oy++) {
        drow = bitmap.get_scanline<rgb24bpp_t>(oy,bitmap.width/2U);
        for (ox=(bitmap.width/2);ox < bitmap.width;ox++) {
            /* color */
            r = ((ox ^ oy) & 4) ? bitmap.rgbinfo.r.bmask : 0;
            g = ((ox ^ oy) & 4) ? bitmap.rgbinfo.g.bmask : 0;
            b = ((ox ^ oy) & 4) ? bitmap.rgbinfo.b.bmask : 0;

            *((uint32_t*)(drow++)) = (r << bitmap.rgbinfo.r.shift) + (g << bitmap.rgbinfo.g.shift) + (b << bitmap.rgbinfo.b.shift) + bitmap.rgbinfo.a.mask;
        }
    }
}

void test_pattern_1_render(rgb_bitmap_info &bitmap) {
    if (bitmap.bytes_per_pixel == sizeof(uint32_t))
        test_pattern_1_render<uint32_t>(bitmap);
    else if (bitmap.bytes_per_pixel == 3)
        test_pattern_1_render<rgb24bpp_t>(bitmap);
    else if (bitmap.bytes_per_pixel == sizeof(uint16_t))
        test_pattern_1_render<uint16_t>(bitmap);
    else {
        fprintf(stderr,"WARNING: test_pattern_1_render() unsupported bytes per pixel %u (%u bits)\n",
            (unsigned int)bitmap.bytes_per_pixel,
            (unsigned int)(bitmap.bytes_per_pixel*8));
    }
}

