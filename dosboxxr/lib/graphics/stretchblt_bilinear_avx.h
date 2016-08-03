
#include "dosboxxr/lib/util/rgb_bitmap_info.h"
#include "dosboxxr/lib/util/rgb_bitmap_test_patterns.h"

template <class T> void stretchblt_bilinear_avx(const rgb_bitmap_info &dbmp,const rgb_bitmap_info &sbmp);
void stretchblt_bilinear_avx(const rgb_bitmap_info &dbmp,const rgb_bitmap_info &sbmp);
bool stretchblt_bilinear_avx_can_do(const rgb_bitmap_info &dbmp,const rgb_bitmap_info &sbmp);

