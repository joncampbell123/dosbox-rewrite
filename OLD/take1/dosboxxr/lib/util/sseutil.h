
#ifndef DOSBOXXR_LIB_UTIL_SSEUTIL_H
#define DOSBOXXR_LIB_UTIL_SSEUTIL_H

/* MinGW/Windows: Some functions need this attribute to force GCC stack alignment.
   This is important when using SSE intrinsics especially when done from a WndProc callback. */
#if IS_WINDOWS && IS_MINGW
# define SSE_REALIGN __attribute__((force_align_arg_pointer))
#else
# define SSE_REALIGN /*nothing*/
#endif

#endif

