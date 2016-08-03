
#ifdef HAVE_CONFIG_H
# include "config.h" // must be first
#endif

#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "dosboxxr/lib/util/rgbinfo.h"
#include "dosboxxr/lib/util/rgb_bitmap_info.h"
#include "dosboxxr/lib/util/rgb_bitmap_test_pattern_gradients.h"

template <class T> void render_test_pattern_rgb_gradients(rgb_bitmap_info &bmp) {
    size_t ox,oy;
    T r,g,b;
    T *drow;

    for (oy=0;oy < bmp.height;oy++) {
        drow = bmp.get_scanline<T>(oy);
        for (ox=0;ox < bmp.width;ox++) {
            /* color */
            r = (ox * bmp.rgbinfo.r.bmask) / bmp.width;
            g = (oy * bmp.rgbinfo.g.bmask) / bmp.height;
            b = bmp.rgbinfo.b.bmask - ((ox * bmp.rgbinfo.b.bmask) / bmp.width);

            *drow++ = (r << bmp.rgbinfo.r.shift) +
                (g << bmp.rgbinfo.g.shift) +
                (b << bmp.rgbinfo.b.shift) +
                bmp.rgbinfo.a.mask;
        }
    }
}

void render_test_pattern_rgb_gradients(rgb_bitmap_info &bmp) {
    if (bmp.bytes_per_pixel == sizeof(uint32_t))
        render_test_pattern_rgb_gradients<uint32_t>(bmp);
    else if (bmp.bytes_per_pixel == sizeof(uint16_t))
        render_test_pattern_rgb_gradients<uint16_t>(bmp);
}

