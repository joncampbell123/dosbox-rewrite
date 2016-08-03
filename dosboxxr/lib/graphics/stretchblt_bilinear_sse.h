
#include "dosboxxr/lib/util/rgbinfo.h"
#include "dosboxxr/lib/util/rgb_bitmap_info.h"

bool stretchblt_bilinear_sse_can_do(const rgb_bitmap_info &dbmp,const rgb_bitmap_info &sbmp);
template <class T> void stretchblt_bilinear_sse(const rgb_bitmap_info &dbmp,const rgb_bitmap_info &sbmp);
void stretchblt_bilinear_sse(const rgb_bitmap_info &dbmp,const rgb_bitmap_info &sbmp);

