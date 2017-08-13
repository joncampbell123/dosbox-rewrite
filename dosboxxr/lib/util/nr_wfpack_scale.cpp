
#ifdef HAVE_CONFIG_H
# include "config.h" // must be first
#endif

#include <stdint.h>

#include "dosboxxr/lib/util/nr_wfpack.h"

static inline void render_scale_from_sd_64towf(struct nr_wfpack &sy,const uint64_t t) {
    if (sizeof(nr_wftype) == 8) {
        // nr_wftype is 64.64
        sy.w = (nr_wftype)(t >> (uint64_t)32);
        sy.f = (nr_wftype)(t << (uint64_t)32);
    }
    else if (sizeof(nr_wftype) == 4) {
        // nr_wftype is 32.32
        sy.w = (nr_wftype)(t >> (uint64_t)32);
        sy.f = (nr_wftype)t;
    }
    else {
        sy.w = 0;
        sy.f = 0;
    }
}

// This version uses sh / dh for even scaling with nearest neighbor interpolation.
// for bilinear interpolation, use render_scale_from_sd(). using this function for
// bilinear interpolation will expose blank/junk pixels on the bottom/right edge of the frame.
void render_scale_from_sd_nearest(struct nr_wfpack &sy,const uint32_t dh,const uint32_t sh) {
    uint64_t t;

    if (dh > 0) {
        t  = (uint64_t)((uint64_t)sh << (uint64_t)32);
        t /= (uint64_t)dh;
    }
    else {
        t = 0;
    }

    render_scale_from_sd_64towf(sy,t);
}

// This version uses (sh - 1) / (dh - 1) so that bilinear interpolation does not expose junk pixels on the
// bottom/right edge of the frame. If you are using nearest neighbor scaling, don't use this version, use
// the render_scale_from_sd_nearest() version of the function.
void render_scale_from_sd(struct nr_wfpack &sy,const uint32_t dh,const uint32_t sh) {
    uint64_t t;

    if (dh > 1) {
        t  = (uint64_t)((uint64_t)(sh - uint32_t(1)) << (uint64_t)32);
        t /= (uint64_t)(dh - uint32_t(1));
    }
    else {
        t = 0;
    }

    render_scale_from_sd_64towf(sy,t);
}

