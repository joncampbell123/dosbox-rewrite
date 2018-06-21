
#include "dosboxxr/lib/util/rgbinfo.h"
#include "dosboxxr/lib/util/rgb_bitmap_info.h"

bool stretchblt_bilinear_sse_can_do(const rgb_bitmap_info &dbmp,const rgb_bitmap_info &sbmp);
template <class T> void stretchblt_bilinear_sse(const rgb_bitmap_info &dbmp,const rgb_bitmap_info &sbmp);
void stretchblt_bilinear_sse(const rgb_bitmap_info &dbmp,const rgb_bitmap_info &sbmp);

// MinGW + SSE WARNING:
// When targeting Windows, remember that Windows is built around a 4-byte stack alignment, while
// MinGW/GCC defaults to a 16-bit stack alignment. This also means that any code that uses SSE
// intrinsics and declares SSE values on the stack will CRASH if the stack pointer gets misaligned
// by Windows in any way. When GCC stack alignment is 16 bytes and you declare SSE __m128i variables
// on the stack, GCC assumes that they will be aligned when your function is called. If the stack
// pointer is not aligned, then those SSE variables are not aligned, and your program will crash
// when trying to use them.
//
// The most common way that this happens is from callback functions in Windows, the most likely
// culprit being your WindowProc() callback for handling window events. To allow the SSE intrinsics
// to work properly regardless, you will need to mark your WindowProc() function with GCC's
// __attribute__((force_align_arg_ptr)) attribute to tell the compiler that the prologue of your
// WindowProc() needs to fix up the stack alignment. A more general marker is provided in the
// "dosboxxr/lib/util/sseutil.h" header file as SSE_REALIGN, which will become this attribute if
// MinGW+Windows or will become nothing otherwise.
