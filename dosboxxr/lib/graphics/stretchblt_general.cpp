
#ifdef HAVE_CONFIG_H
# include "config.h" // must be first
#endif

#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <algorithm>

#include "dosboxxr/lib/hostcpudetect/caps.h"
#include "dosboxxr/lib/util/nr_wfpack.h"
#include "dosboxxr/lib/util/bitscan.h"
#include "dosboxxr/lib/util/rgbinfo.h"
#include "dosboxxr/lib/util/rgb_bitmap_info.h"
#include "dosboxxr/lib/util/rgb_bitmap_test_patterns.h"
#include "dosboxxr/lib/util/vinterp_tmp.h"

#include "dosboxxr/lib/graphics/stretchblt_general.h"

#include "dosboxxr/lib/graphics/stretchblt_neighbor.h"
#include "dosboxxr/lib/graphics/stretchblt_bilinear.h"
#include "dosboxxr/lib/graphics/stretchblt_bilinear_mmx.h"
#include "dosboxxr/lib/graphics/stretchblt_bilinear_sse.h"
#include "dosboxxr/lib/graphics/stretchblt_bilinear_avx.h"
#include "dosboxxr/lib/graphics/stretchblt_bilinear_arm_neon.h"

struct stretchblt_mode stretchblt_modes[] = {
    {"bilinear",
    stretchblt_bilinear,
    stretchblt_bilinear_can_do},
#if HAVE_CPU_MMX
    {"bilinear_mmx",
    stretchblt_bilinear_mmx,
    stretchblt_bilinear_mmx_can_do},
#endif
#if HAVE_CPU_SSE2
    {"bilinear_sse",
    stretchblt_bilinear_sse,
    stretchblt_bilinear_sse_can_do},
#endif
#if HAVE_CPU_AVX2
    {"bilinear_avx",
    stretchblt_bilinear_avx,
    stretchblt_bilinear_avx_can_do},
#endif
#if HAVE_CPU_ARM_NEON
    {"bilinear_arm_neon",
    stretchblt_bilinear_arm_neon,
    stretchblt_bilinear_arm_neon_can_do},
#endif
    {"neighbor",
    stretchblt_neighbor,
    stretchblt_neighbor_can_do}
};

size_t stretchblt_mode_count() {
    return sizeof(stretchblt_modes)/sizeof(stretchblt_modes[0]);
}

