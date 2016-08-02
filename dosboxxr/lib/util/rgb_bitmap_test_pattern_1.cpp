
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

void test_pattern_1_render(rgb_bitmap_info &bitmap) {
    if (bitmap.bytes_per_pixel == sizeof(uint32_t))
        test_pattern_1_render<uint32_t>(bitmap);
    else if (bitmap.bytes_per_pixel == sizeof(uint16_t))
        test_pattern_1_render<uint16_t>(bitmap);
    else {
        fprintf(stderr,"WARNING: test_pattern_1_render() unsupported bytes per pixel %u (%u bits)\n",
            bitmap.bytes_per_pixel,bitmap.bytes_per_pixel*8);
    }
}

