
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
#include "dosboxxr/lib/graphics/stretchblt_neighbor.h"

template <class T> void stretchblt_neighbor(const rgb_bitmap_info &dbmp,const rgb_bitmap_info &sbmp) {
    nr_wfpack sx,sxi={0,0},sy={0,0},stepx,stepy;
    uint8_t *drow;
    size_t ox,oy;

    render_scale_from_sd_nearest(/*&*/stepx,dbmp.width,sbmp.width);
    render_scale_from_sd_nearest(/*&*/stepy,dbmp.height,sbmp.height);

    drow = dbmp.get_scanline<uint8_t>(0);
    oy = dbmp.height;
    do {
        T *s = sbmp.get_scanline<T>(sy.w);
        T *d = (T*)drow;

        ox = dbmp.width;
        sx = sxi;
        do {
            *d++ = s[sx.w];
            if ((--ox) == 0) break;
            sx += stepx;
        } while (1);

        if ((--oy) == 0) break;
        drow += dbmp.stride;
        sy += stepy;
    } while (1);
}

void stretchblt_neighbor(const rgb_bitmap_info &dbmp,const rgb_bitmap_info &sbmp) {
    if (dbmp.bytes_per_pixel == sizeof(uint32_t))
        stretchblt_neighbor<uint32_t>(dbmp,sbmp);
    else if (dbmp.bytes_per_pixel == 3)
        stretchblt_neighbor<rgb24bpp_t>(dbmp,sbmp);
    else if (dbmp.bytes_per_pixel == sizeof(uint16_t))
        stretchblt_neighbor<uint16_t>(dbmp,sbmp);
    else if (dbmp.bytes_per_pixel == sizeof(uint8_t))
        stretchblt_neighbor<uint8_t>(dbmp,sbmp);
}

bool stretchblt_neighbor_can_do(const rgb_bitmap_info &dbmp,const rgb_bitmap_info &sbmp) {
    if (!dbmp.is_valid() || !sbmp.is_valid()) return false;
    return true;
}

