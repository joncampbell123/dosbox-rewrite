AC_INIT([DOSBox-X-rewrite], 0.01)
AM_INIT_AUTOMAKE
AC_PROG_CC
AC_PROG_CXX
AC_CONFIG_HEADER([config.h])
AC_PREFIX_DEFAULT(/usr/local)

# largefile support.
# NTS: creation of config.h means you must include config.h to get _FILE_OFFSET_BITS=64 or whatever your platform needs.
#      Failure to do so will prevent that code from accessing files larger than 2GB on 32-bit systems (where off_t is 32-bit wide)
AC_SYS_LARGEFILE

# this code uses POSIX functions
POSIX_DEF=" -D_POSIX_C_SOURCE=200809L"
CFLAGS+="$POSIX_DEF"
CPPFLAGS+="$POSIX_DEF"
CXXFLAGS+="$POSIX_DEF"

# this code uses C++11 features (TODO: gnu++11 if GCC, else c++11)
c11=0
AC_MSG_CHECKING([to make sure your C++ compiler supports C++11])
old_CXXFLAGS=$CXXFLAGS
old_CFLAGS=$CFLAGS
CXXFLAGS+=" -std=gnu++11"
CFLAGS+=" -std=gnu11"
AC_TRY_COMPILE([ ], [ ],
    [
        AC_MSG_RESULT([yes])
        c11=1
    ],
    [
        AC_MSG_RESULT([no])
        CXXFLAGS=$old_CXXFLAGS
        CFLAGS=$old_CFLAGS
    ])

# maybe gnu++0x?
if test $c11 = 0; then
    AC_MSG_CHECKING([to make sure your C++ compiler supports C++11 (gnu++0x)])
    CXXFLAGS+=" -std=gnu++0x"
    CFLAGS+=" -std=gnu1x"
    AC_TRY_COMPILE([ ], [ ],
        [
            AC_MSG_RESULT([yes])
            c11=1
        ],
        [
            AC_MSG_RESULT([no])
            CXXFLAGS=$old_CXXFLAGS
            CFLAGS=$old_CFLAGS
        ])
fi

# sorry
if test $c11 = 0; then
    AC_MSG_ERROR([This code base requires a C++11 capable compiler])
fi

# use MMX, SSE, SSE2, AVX if available (TODO: Only if target is i686 or x86_64)
# not for ALL code, just the optimized rendering code.
old_CXXFLAGS=$CXXFLAGS
old_CFLAGS=$CFLAGS
for ext in mmx sse sse2 sse3 avx avx2; do
    AC_MSG_CHECKING([to make sure your C++ compiler supports the $ext instruction set])
    EXT=`echo $ext | tr [[:lower:]] [[:upper:]]`
    CXXFLAGS+=" -m$ext"
    CFLAGS+=" -m$ext"
    AC_TRY_COMPILE([ ],[ ],
        [
            AC_MSG_RESULT([yes])
            _nm=HAVE_CPU_$EXT
            eval $_nm=1
        ],
        [
            AC_MSG_RESULT([no])
            _nm=HAVE_CPU_$EXT
            eval $_nm=0
        ])
    CXXFLAGS=$old_CXXFLAGS
    CFLAGS=$old_CFLAGS
done
AC_SUBST(HAVE_CPU_MMX)
AC_SUBST(CPU_MMX_CFLAGS)
AM_CONDITIONAL([HAVE_CPU_MMX],[test x$HAVE_CPU_MMX = 1])
AC_DEFINE_UNQUOTED([HAVE_CPU_MMX],[$HAVE_CPU_MMX],[Has MMX CPU extension])
if test $HAVE_CPU_MMX = 1; then CPU_MMX_CFLAGS+=-mmmx; fi

AC_SUBST(HAVE_CPU_SSE)
AC_SUBST(CPU_SSE_CFLAGS)
AM_CONDITIONAL([HAVE_CPU_SSE],[test x$HAVE_CPU_SSE = 1])
AC_DEFINE_UNQUOTED([HAVE_CPU_SSE],[$HAVE_CPU_SSE],[Has SSE CPU extension])
if test $HAVE_CPU_SSE = 1; then CPU_SSE_CFLAGS+=-msse; fi

AC_SUBST(HAVE_CPU_SSE2)
AC_SUBST(CPU_SSE2_CFLAGS)
AM_CONDITIONAL([HAVE_CPU_SSE2],[test x$HAVE_CPU_SSE2 = 1])
AC_DEFINE_UNQUOTED([HAVE_CPU_SSE2],[$HAVE_CPU_SSE2],[Has SSE2 CPU extension])
if test $HAVE_CPU_SSE2 = 1; then CPU_SSE2_CFLAGS+=-msse2; fi

AC_SUBST(HAVE_CPU_SSE3)
AC_SUBST(CPU_SSE3_CFLAGS)
AM_CONDITIONAL([HAVE_CPU_SSE3],[test x$HAVE_CPU_SSE3 = 1])
AC_DEFINE_UNQUOTED([HAVE_CPU_SSE3],[$HAVE_CPU_SSE3],[Has SSE3 CPU extension])
if test $HAVE_CPU_SSE3 = 1; then CPU_SSE3_CFLAGS+=-msse3; fi

AC_SUBST(HAVE_CPU_AVX)
AC_SUBST(CPU_AVX_CFLAGS)
AM_CONDITIONAL([HAVE_CPU_AVX],[test x$HAVE_CPU_AVX = 1])
AC_DEFINE_UNQUOTED([HAVE_CPU_AVX],[$HAVE_CPU_AVX],[Has AVX CPU extension])
if test $HAVE_CPU_AVX = 1; then CPU_AVX_CFLAGS+=-mavx; fi

AC_SUBST(HAVE_CPU_AVX2)
AC_SUBST(CPU_AVX2_CFLAGS)
AM_CONDITIONAL([HAVE_CPU_AVX2],[test x$HAVE_CPU_AVX2 = 1])
AC_DEFINE_UNQUOTED([HAVE_CPU_AVX2],[$HAVE_CPU_AVX2],[Has AVX2 CPU extension])
if test $HAVE_CPU_AVX2 = 1; then CPU_AVX2_CFLAGS+=-mavx2; fi

AC_SUBST(CFLAGS)
AC_SUBST(CXXFLAGS)
AC_SUBST(LDFLAGS)

# important functions
AC_CHECK_FUNCS([nanosleep usleep lseek64 lseeki64 truncate ftruncate openat renameat symlink link stat fstat fstatat statfs fstatfs realpath sysconf mmap munmap getpagesize mincore mlock munlock mprotect mremap msync getrlimit setrlimit shmat shm_open remap_file_pages fallocate readlink readlinkat])
AC_CHECK_DECLS([isinf isnan],[],[],[[#include <math.h>]])

# GCC __builtin_cpu_init and other functions
HAVE_GCC_BUILTIN_CPU=0
AC_SUBST(HAVE_GCC_BUILTIN_CPU)
AC_MSG_CHECKING([for GCC builtin_cpu functions])
AC_TRY_LINK([#include <cpuid.h>],
    [
        __builtin_cpu_init();
        __builtin_cpu_supports("sse2");
    ],
    [
        AC_MSG_RESULT([yes])
        HAVE_GCC_BUILTIN_CPU=1
    ],
    [
        AC_MSG_RESULT([no])
    ])
AM_CONDITIONAL([HAVE_GCC_BUILTIN_CPU],[test x$HAVE_GCC_BUILTIN_CPU = 1])
AC_DEFINE_UNQUOTED([HAVE_GCC_BUILTIN_CPU],[$HAVE_GCC_BUILTIN_CPU],[Has GCC builtin_cpu functions])

# we have to work at checking for clock_gettime
# note that on Linux this requires librt.
old_CXXFLAGS=$CXXFLAGS
old_CFLAGS=$CFLAGS
CFLAGS+=" -lrt"
CXXFLAGS+=" -lrt"
HAVE_CLOCK_GETTIME=0
AC_SUBST(HAVE_CLOCK_GETTIME)
AC_MSG_CHECKING([for clock_gettime])
AC_TRY_LINK([#include <time.h>],
    [
            struct timespec tv;
            clock_gettime(CLOCK_MONOTONIC,&tv);
    ],
    [
        AC_MSG_RESULT([yes])
        HAVE_CLOCK_GETTIME=1
    ],
    [
        AC_MSG_RESULT([no])
        old_CXXFLAGS=$CXXFLAGS
        old_CFLAGS=$CFLAGS
    ])
AM_CONDITIONAL([HAVE_CLOCK_GETTIME],[test x$HAVE_CLOCK_GETTIME = 1])
AC_DEFINE_UNQUOTED([HAVE_CLOCK_GETTIME],[$HAVE_CLOCK_GETTIME],[Has clock_gettime])

# Does Linux have sys/vm86.h for virtual 8086 mode?
HAVE_SYS_VM86_H=0
AC_SUBST(HAVE_SYS_VM86_H)
AC_CHECK_HEADER([sys/vm86.h],[HAVE_SYS_VM86_H=1],[HAVE_SYS_VM86_H=0])
AM_CONDITIONAL([HAVE_SYS_VM86_H],[test x$HAVE_SYS_VM86_H = 1])
AC_DEFINE_UNQUOTED([HAVE_SYS_VM86_H],[$HAVE_SYS_VM86_H],[Has sys/vm86.h])

# Does Linux have sys/ptrace.h?
HAVE_SYS_PTRACE_H=0
AC_SUBST(HAVE_SYS_PTRACE_H)
AC_CHECK_HEADER([sys/ptrace.h],[HAVE_SYS_PTRACE_H=1],[HAVE_SYS_PTRACE_H=0])
AM_CONDITIONAL([HAVE_SYS_PTRACE_H],[test x$HAVE_SYS_PTRACE_H = 1])
AC_DEFINE_UNQUOTED([HAVE_SYS_PTRACE_H],[$HAVE_SYS_PTRACE_H],[Has sys/ptrace.h])

# Does Linux have asm/ldt.h for the modify_ldt syscall?
HAVE_ASM_LDT_H=0
AC_SUBST(HAVE_ASM_LDT_H)
AC_CHECK_HEADER([asm/ldt.h],[HAVE_ASM_LDT_H=1],[HAVE_ASM_LDT_H=0])
AM_CONDITIONAL([HAVE_ASM_LDT_H],[test x$HAVE_ASM_LDT_H = 1])
AC_DEFINE_UNQUOTED([HAVE_ASM_LDT_H],[$HAVE_ASM_LDT_H],[Has asm/ldt.h])

# Does Linux have linux/kvm.h for KVM support?
HAVE_LINUX_KVM_H=0
AC_SUBST(HAVE_LINUX_KVM_H)
AC_CHECK_HEADER([linux/kvm.h],[HAVE_LINUX_KVM_H=1],[HAVE_LINUX_KVM_H=0])
AM_CONDITIONAL([HAVE_LINUX_KVM_H],[test x$HAVE_LINUX_KVM_H = 1])
AC_DEFINE_UNQUOTED([HAVE_LINUX_KVM_H],[$HAVE_LINUX_KVM_H],[Has linux/kvm.h])

# X11
HAVE_X11=0
AC_ARG_WITH(x11,AS_HELP_STRING([--with-x11],[Enable X11 support]),[],[with_x11=yes])
AC_SUBST(HAVE_X11)
AC_SUBST(X11_CFLAGS)
AC_SUBST(X11_LDFLAGS)
if test "$with_x11" = "yes"; then
    AC_MSG_CHECKING([for X11])
    if pkg-config --atleast-version=1.5.0 x11; then
        AC_MSG_RESULT([yes])
        HAVE_X11=1
        X11_CFLAGS=`pkg-config --cflags x11`
        X11_LDFLAGS=`pkg-config --libs x11`
    else
        AC_MSG_RESULT([no])
    fi
fi
AC_DEFINE_UNQUOTED([HAVE_X11],[$HAVE_X11],[Has X11])
AM_CONDITIONAL([HAVE_X11],[test x$HAVE_X11 = 1])

# X11-XCB
HAVE_X11_XCB=0
AC_ARG_WITH(x11-xcb,AS_HELP_STRING([--with-x11-xcb],[Enable X11-XCB support]),[],[with_x11_xcb=yes])
AC_SUBST(HAVE_X11_XCB)
AC_SUBST(X11_XCB_CFLAGS)
AC_SUBST(X11_XCB_LDFLAGS)
if test "$with_x11_xcb" = "yes"; then
    AC_MSG_CHECKING([for X11-XCB])
    if pkg-config --atleast-version=1.5.0 x11-xcb; then
        AC_MSG_RESULT([yes])
        HAVE_X11_XCB=1
        X11_XCB_CFLAGS=`pkg-config --cflags x11-xcb`
        X11_XCB_LDFLAGS=`pkg-config --libs x11-xcb`
    else
        AC_MSG_RESULT([no])
    fi
fi
AC_DEFINE_UNQUOTED([HAVE_X11_XCB],[$HAVE_X11_XCB],[Has X11 XCB])
AM_CONDITIONAL([HAVE_X11_XCB],[test x$HAVE_X11_XCB = 1])

# XCB
HAVE_XCB=0
AC_ARG_WITH(xcb,AS_HELP_STRING([--with-xcb],[Enable XCB support]),[],[with_xcb=yes])
AC_SUBST(HAVE_XCB)
AC_SUBST(XCB_CFLAGS)
AC_SUBST(XCB_LDFLAGS)
if test "$with_xcb" = "yes"; then
    AC_MSG_CHECKING([for XCB])
    if pkg-config --atleast-version=1.12 xcb; then
        AC_MSG_RESULT([yes])
        HAVE_XCB=1
        XCB_CFLAGS=`pkg-config --cflags xcb`
        XCB_LDFLAGS=`pkg-config --libs xcb`
    else
        AC_MSG_RESULT([no])
    fi
fi
AC_DEFINE_UNQUOTED([HAVE_XCB],[$HAVE_XCB],[Has XCB])
AM_CONDITIONAL([HAVE_XCB],[test x$HAVE_XCB = 1])

# XCB-XV
HAVE_XCB_XV=0
AC_ARG_WITH(xcb-xv,AS_HELP_STRING([--with-xcb-xv],[Enable XCB-XV support]),[],[with_xcb_xv=yes])
AC_SUBST(HAVE_XCB_XV)
AC_SUBST(XCB_XV_CFLAGS)
AC_SUBST(XCB_XV_LDFLAGS)
if test "$with_xcb_xv" = "yes"; then
    AC_MSG_CHECKING([for XCB xv])
    if pkg-config --atleast-version=1.12 xcb-xv; then
        AC_MSG_RESULT([yes])
        HAVE_XCB_XV=1
        XCB_CFLAGS=`pkg-config --cflags xcb-xv`
        XCB_LDFLAGS=`pkg-config --libs xcb-xv`
    else
        AC_MSG_RESULT([no])
    fi
fi
AC_DEFINE_UNQUOTED([HAVE_XCB_XV],[$HAVE_XCB_XV],[Has XCB xv])
AM_CONDITIONAL([HAVE_XCB_XV],[test x$HAVE_XCB_XV = 1])

# XCB_SHM
HAVE_XCB_SHM=0
AC_ARG_WITH(xcb-shm,AS_HELP_STRING([--with-xcb-shm],[Enable XCB SHM support]),[],[with_xcb_shm=yes])
AC_SUBST(HAVE_XCB_SHM)
AC_SUBST(XCB_SHM_CFLAGS)
AC_SUBST(XCB_SHM_LDFLAGS)
if test "$with_xcb_shm" = "yes"; then
    AC_MSG_CHECKING([for XCB SHM])
    if pkg-config --atleast-version=1.12 xcb-shm; then
        AC_MSG_RESULT([yes])
        HAVE_XCB_SHM=1
        XCB_SHM_CFLAGS=`pkg-config --cflags xcb-shm`
        XCB_SHM_LDFLAGS=`pkg-config --libs xcb-shm`
    else
        AC_MSG_RESULT([no])
    fi
fi
AC_DEFINE_UNQUOTED([HAVE_XCB_SHM],[$HAVE_XCB_SHM],[Has XCB SHM])
AM_CONDITIONAL([HAVE_XCB_SHM],[test x$HAVE_XCB_SHM = 1])

# XCB_XKB
HAVE_XCB_XKB=0
AC_ARG_WITH(xcb-xkb,AS_HELP_STRING([--with-xcb-xkb],[Enable XCB XKB support]),[],[with_xcb_xkb=yes])
AC_SUBST(HAVE_XCB_XKB)
AC_SUBST(XCB_XKB_CFLAGS)
AC_SUBST(XCB_XKB_LDFLAGS)
if test "$with_xcb_xkb" = "yes"; then
    AC_MSG_CHECKING([for XCB XKB])
    if pkg-config --atleast-version=1.12 xcb-xkb; then
        AC_MSG_RESULT([yes])
        HAVE_XCB_XKB=1
        XCB_XKB_CFLAGS=`pkg-config --cflags xcb-xkb`
        XCB_XKB_LDFLAGS=`pkg-config --libs xcb-xkb`
    else
        AC_MSG_RESULT([no])
    fi
fi
AC_DEFINE_UNQUOTED([HAVE_XCB_XKB],[$HAVE_XCB_XKB],[Has XCB XKB])
AM_CONDITIONAL([HAVE_XCB_XKB],[test x$HAVE_XCB_XKB = 1])

# ALSA
HAVE_ALSA=0
AC_ARG_WITH(alsa,AS_HELP_STRING([--with-alsa],[Enable ALSA support]),[],[with_alsa=yes])
AC_SUBST(HAVE_ALSA)
AC_SUBST(ALSA_CFLAGS)
AC_SUBST(ALSA_LDFLAGS)
if test "$with_alsa" = "yes"; then
    AC_MSG_CHECKING([for ALSA])
    if pkg-config --atleast-version=1.0.27.2 alsa; then
        AC_MSG_RESULT([yes])
        HAVE_ALSA=1
        ALSA_CFLAGS=`pkg-config --cflags alsa`
        ALSA_LDFLAGS=`pkg-config --libs alsa`
    else
        AC_MSG_RESULT([no])
    fi
fi
AC_DEFINE_UNQUOTED([HAVE_ALSA],[$HAVE_ALSA],[Has ALSA])
AM_CONDITIONAL([HAVE_ALSA],[test x$HAVE_ALSA = 1])

# LIBDRM
HAVE_LIBDRM=0
AC_ARG_WITH(libdrm,AS_HELP_STRING([--with-libdrm],[Enable DRM (Direct Rendering Manager) support]),[],[with_libdrm=yes])
AC_SUBST(HAVE_LIBDRM)
AC_SUBST(LIBDRM_CFLAGS)
AC_SUBST(LIBDRM_LDFLAGS)
if test "$with_libdrm" = "yes"; then
    AC_MSG_CHECKING([for libdrm])
    if pkg-config --atleast-version=2.4.53 libdrm; then
        AC_MSG_RESULT([yes])
        HAVE_LIBDRM=1
        LIBDRM_CFLAGS=`pkg-config --cflags libdrm`
        LIBDRM_LDFLAGS=`pkg-config --libs libdrm`
    else
        AC_MSG_RESULT([no])
    fi
fi
AC_DEFINE_UNQUOTED([HAVE_LIBDRM],[$HAVE_LIBDRM],[Has libdrm])
AM_CONDITIONAL([HAVE_LIBDRM],[test x$HAVE_LIBDRM = 1])

# OpenGL
HAVE_OPENGL=0
AC_ARG_WITH(gl,AS_HELP_STRING([--with-gl],[Enable OpenGL support]),[],[with_gl=yes])
AC_SUBST(HAVE_OPENGL)
AC_SUBST(OPENGL_CFLAGS)
AC_SUBST(OPENGL_LDFLAGS)
if test "$with_gl" = "yes"; then
    AC_MSG_CHECKING([for OpenGL GL])
    if pkg-config --atleast-version=10.0.4 gl; then
        AC_MSG_RESULT([yes])
        HAVE_OPENGL=1
        OPENGL_CFLAGS=`pkg-config --cflags gl`
        OPENGL_LDFLAGS=`pkg-config --libs gl`
    else
        AC_MSG_RESULT([no])
    fi
fi
AC_DEFINE_UNQUOTED([HAVE_OPENGL],[$HAVE_OPENGL],[Has OpenGL])
AM_CONDITIONAL([HAVE_OPENGL],[test x$HAVE_OPENGL = 1])

# OpenGL GLU
HAVE_OPENGL_GLU=0
AC_ARG_WITH(glu,AS_HELP_STRING([--with-glu],[Enable OpenGL GLU support]),[],[with_glu=yes])
AC_SUBST(HAVE_OPENGL_GLU)
AC_SUBST(OPENGL_GLU_CFLAGS)
AC_SUBST(OPENGL_GLU_LDFLAGS)
if test "$with_glu" = "yes"; then
    AC_MSG_CHECKING([for OpenGL GLU])
    if pkg-config --atleast-version=9.0.0 glu; then
        AC_MSG_RESULT([yes])
        HAVE_OPENGL_GLU=1
        OPENGL_GLU_CFLAGS=`pkg-config --cflags glu`
        OPENGL_GLU_LDFLAGS=`pkg-config --libs glu`
    else
        AC_MSG_RESULT([no])
    fi
fi
AC_DEFINE_UNQUOTED([HAVE_OPENGL_GLU],[$HAVE_OPENGL_GLU],[Has OpenGL GLU])
AM_CONDITIONAL([HAVE_OPENGL_GLU],[test x$HAVE_OPENGL_GLU = 1])

# OpenGL GLEW
HAVE_OPENGL_GLEW=0
AC_ARG_WITH(glew,AS_HELP_STRING([--with-glew],[Enable OpenGL GLEW support]),[],[with_glew=yes])
AC_SUBST(HAVE_OPENGL_GLEW)
AC_SUBST(OPENGL_GLEW_CFLAGS)
AC_SUBST(OPENGL_GLEW_LDFLAGS)
if test "$with_glew" = "yes"; then
    AC_MSG_CHECKING([for OpenGL GLEW])
    if pkg-config --atleast-version=1.6.0 glew; then
        AC_MSG_RESULT([yes])
        HAVE_OPENGL_GLEW=1
        OPENGL_GLEW_CFLAGS=`pkg-config --cflags glew`
        OPENGL_GLEW_LDFLAGS=`pkg-config --libs glew`
    else
        AC_MSG_RESULT([no])
    fi
fi
AC_DEFINE_UNQUOTED([HAVE_OPENGL_GLEW],[$HAVE_OPENGL_GLEW],[Has OpenGL GLEW])
AM_CONDITIONAL([HAVE_OPENGL_GLEW],[test x$HAVE_OPENGL_GLEW = 1])

# OpenGL EGL
HAVE_OPENGL_EGL=0
AC_ARG_WITH(egl,AS_HELP_STRING([--with-egl],[Enable OpenGL EGL support]),[],[with_egl=yes])
AC_SUBST(HAVE_OPENGL_EGL)
AC_SUBST(OPENGL_EGL_CFLAGS)
AC_SUBST(OPENGL_EGL_LDFLAGS)
if test "$with_egl" = "yes"; then
    AC_MSG_CHECKING([for OpenGL EGL])
    if pkg-config --atleast-version=10.0.4 egl; then
        AC_MSG_RESULT([yes])
        HAVE_OPENGL_EGL=1
        OPENGL_EGL_CFLAGS=`pkg-config --cflags egl`
        OPENGL_EGL_LDFLAGS=`pkg-config --libs egl`
    else
        AC_MSG_RESULT([no])
    fi
fi
AC_DEFINE_UNQUOTED([HAVE_OPENGL_EGL],[$HAVE_OPENGL_EGL],[Has OpenGL EGL])
AM_CONDITIONAL([HAVE_OPENGL_EGL],[test x$HAVE_OPENGL_EGL = 1])

# ZLIB
HAVE_ZLIB=0
AC_ARG_WITH(zlib,AS_HELP_STRING([--with-zlib],[Enable ZLIB support]),[],[with_zlib=yes])
AC_SUBST(HAVE_ZLIB)
AC_SUBST(ZLIB_CFLAGS)
AC_SUBST(ZLIB_LDFLAGS)
if test "$with_zlib" = "yes"; then
    AC_MSG_CHECKING([for ZLIB])
    if pkg-config --atleast-version=1.2.6 zlib; then
        AC_MSG_RESULT([yes])
        HAVE_ZLIB=1
        ZLIB_CFLAGS=`pkg-config --cflags zlib`
        ZLIB_LDFLAGS=`pkg-config --libs zlib`
    else
        AC_MSG_RESULT([no])
    fi
fi
AC_DEFINE_UNQUOTED([HAVE_ZLIB],[$HAVE_ZLIB],[Has ZLIB])
AM_CONDITIONAL([HAVE_ZLIB],[test x$HAVE_ZLIB = 1])

# LZMA
HAVE_LZMA=0
AC_ARG_WITH(lzma,AS_HELP_STRING([--with-lzma],[Enable LZMA support]),[],[with_lzma=yes])
AC_SUBST(HAVE_LZMA)
AC_SUBST(LZMA_CFLAGS)
AC_SUBST(LZMA_LDFLAGS)
if test "$with_lzma" = "yes"; then
    AC_MSG_CHECKING([for LZMA])
    if pkg-config --atleast-version=5.0.3 liblzma; then
        AC_MSG_RESULT([yes])
        HAVE_LZMA=1
        LZMA_CFLAGS=`pkg-config --cflags liblzma`
        LZMA_LDFLAGS=`pkg-config --libs liblzma`
    else
        AC_MSG_RESULT([no])
    fi
fi
AC_DEFINE_UNQUOTED([HAVE_LZMA],[$HAVE_LZMA],[Has LZMA])
AM_CONDITIONAL([HAVE_LZMA],[test x$HAVE_LZMA = 1])

# FFMPEG avcodec
HAVE_FFMPEG_AVCODEC=0
AC_ARG_WITH(ffmpeg,AS_HELP_STRING([--with-ffmpeg],[Enable FFMPEG support]),[],[with_ffmpeg=yes])
AC_SUBST(HAVE_FFMPEG_AVCODEC)
AC_SUBST(FFMPEG_AVCODEC_CFLAGS)
AC_SUBST(FFMPEG_AVCODEC_LDFLAGS)
if test "$with_ffmpeg" = "yes"; then
    AC_MSG_CHECKING([for FFMPEG avcodec])
    if pkg-config --atleast-version=57.24.102 libavcodec; then
        AC_MSG_RESULT([yes])
        HAVE_FFMPEG_AVCODEC=1
        FFMPEG_AVCODEC_CFLAGS=`pkg-config --cflags libavcodec`
        FFMPEG_AVCODEC_LDFLAGS=`pkg-config --libs libavcodec`
    else
        AC_MSG_RESULT([no])
    fi
fi
AC_DEFINE_UNQUOTED([HAVE_FFMPEG_AVCODEC],[$HAVE_FFMPEG_AVCODEC],[Has FFMPEG_AVCODEC])
AM_CONDITIONAL([HAVE_FFMPEG_AVCODEC],[test x$HAVE_FFMPEG_AVCODEC = 1])

# FFMPEG avformat
HAVE_FFMPEG_AVFORMAT=0
AC_SUBST(HAVE_FFMPEG_AVFORMAT)
AC_SUBST(FFMPEG_AVFORMAT_CFLAGS)
AC_SUBST(FFMPEG_AVFORMAT_LDFLAGS)
if test "$with_ffmpeg" = "yes"; then
    AC_MSG_CHECKING([for FFMPEG avformat])
    if pkg-config --atleast-version=57.25.100 libavformat; then
        AC_MSG_RESULT([yes])
        HAVE_FFMPEG_AVFORMAT=1
        FFMPEG_AVFORMAT_CFLAGS=`pkg-config --cflags libavformat`
        FFMPEG_AVFORMAT_LDFLAGS=`pkg-config --libs libavformat`
    else
        AC_MSG_RESULT([no])
    fi
fi
AC_DEFINE_UNQUOTED([HAVE_FFMPEG_AVFORMAT],[$HAVE_FFMPEG_AVFORMAT],[Has FFMPEG_AVFORMAT])
AM_CONDITIONAL([HAVE_FFMPEG_AVFORMAT],[test x$HAVE_FFMPEG_AVFORMAT = 1])

# FFMPEG avutil
HAVE_FFMPEG_AVUTIL=0
AC_SUBST(HAVE_FFMPEG_AVUTIL)
AC_SUBST(FFMPEG_AVUTIL_CFLAGS)
AC_SUBST(FFMPEG_AVUTIL_LDFLAGS)
if test "$with_ffmpeg" = "yes"; then
    AC_MSG_CHECKING([for FFMPEG avutil])
    if pkg-config --atleast-version=55.17.103 libavutil; then
        AC_MSG_RESULT([yes])
        HAVE_FFMPEG_AVUTIL=1
        FFMPEG_AVUTIL_CFLAGS=`pkg-config --cflags libavutil`
        FFMPEG_AVUTIL_LDFLAGS=`pkg-config --libs libavutil`
    else
        AC_MSG_RESULT([no])
    fi
fi
AC_DEFINE_UNQUOTED([HAVE_FFMPEG_AVUTIL],[$HAVE_FFMPEG_AVUTIL],[Has FFMPEG_AVUTIL])
AM_CONDITIONAL([HAVE_FFMPEG_AVUTIL],[test x$HAVE_FFMPEG_AVUTIL = 1])

# FFMPEG swscale
HAVE_FFMPEG_SWSCALE=0
AC_ARG_WITH(ffmpeg-swscale,AS_HELP_STRING([--with-ffmpeg-swscale],[Enable FFMPEG swscale support]),[],[with_ffmpeg_swscale=yes])
AC_SUBST(HAVE_FFMPEG_SWSCALE)
AC_SUBST(FFMPEG_SWSCALE_CFLAGS)
AC_SUBST(FFMPEG_SWSCALE_LDFLAGS)
if test "$with_ffmpeg_swscale" = "yes"; then
    AC_MSG_CHECKING([for FFMPEG swscale])
    if pkg-config --atleast-version=4.0.100 libswscale; then
        AC_MSG_RESULT([yes])
        HAVE_FFMPEG_SWSCALE=1
        FFMPEG_SWSCALE_CFLAGS=`pkg-config --cflags libswscale`
        FFMPEG_SWSCALE_LDFLAGS=`pkg-config --libs libswscale`
    else
        AC_MSG_RESULT([no])
    fi
fi
AC_DEFINE_UNQUOTED([HAVE_FFMPEG_SWSCALE],[$HAVE_FFMPEG_SWSCALE],[Has FFMPEG_SWSCALE])
AM_CONDITIONAL([HAVE_FFMPEG_SWSCALE],[test x$HAVE_FFMPEG_SWSCALE = 1])

# Simple Directmedia Layer 1.x
HAVE_SDL1=0
AC_ARG_WITH(sdl1,AS_HELP_STRING([--with-sdl1],[Enable SDL 1.x]),[],[with_sdl1=yes])
AC_SUBST(HAVE_SDL1)
AC_SUBST(SDL1_CFLAGS)
AC_SUBST(SDL1_LDFLAGS)
if test "$with_sdl1" = "yes"; then
    AC_MSG_CHECKING([for SDL 1.x])
    if pkg-config --atleast-version=1.2.15 sdl; then
        AC_MSG_RESULT([yes])
        HAVE_SDL1=1
        SDL1_CFLAGS=`pkg-config --cflags sdl`
        SDL1_LDFLAGS=`pkg-config --libs sdl`
    else
        AC_MSG_RESULT([no])
    fi
fi
AC_DEFINE_UNQUOTED([HAVE_SDL1],[$HAVE_SDL1],[Has SDL1])
AM_CONDITIONAL([HAVE_SDL1],[test x$HAVE_SDL1 = 1])

# Simple Directmedia Layer 2.x
HAVE_SDL2=0
AC_ARG_WITH(sdl2,AS_HELP_STRING([--with-sdl2],[Enable SDL 2.x]),[],[with_sdl2=yes])
AC_SUBST(HAVE_SDL2)
AC_SUBST(SDL2_CFLAGS)
AC_SUBST(SDL2_LDFLAGS)
if test "$with_sdl2" = "yes"; then
    AC_MSG_CHECKING([for SDL 2.x])
    if pkg-config --atleast-version=2.0.3 sdl2; then
        AC_MSG_RESULT([yes])
        HAVE_SDL2=1
        SDL2_CFLAGS=`pkg-config --cflags sdl2`
        SDL2_LDFLAGS=`pkg-config --libs sdl2`
    else
        AC_MSG_RESULT([no])
    fi
fi
AC_DEFINE_UNQUOTED([HAVE_SDL2],[$HAVE_SDL2],[Has SDL2])
AM_CONDITIONAL([HAVE_SDL2],[test x$HAVE_SDL2 = 1])

# libpng
HAVE_LIBPNG=0
AC_ARG_WITH(libpng,AS_HELP_STRING([--with-libpng],[Enable libpng]),[],[with_libpng=yes])
AC_SUBST(HAVE_LIBPNG)
AC_SUBST(LIBPNG_CFLAGS)
AC_SUBST(LIBPNG_LDFLAGS)
if test "$with_libpng" = "yes"; then
    AC_MSG_CHECKING([for libpng])
    if pkg-config --atleast-version=1.2.46 libpng; then
        AC_MSG_RESULT([yes])
        HAVE_LIBPNG=1
        LIBPNG_CFLAGS=`pkg-config --cflags libpng`
        LIBPNG_LDFLAGS=`pkg-config --libs libpng`
    else
        AC_MSG_RESULT([no])
    fi
fi
AC_DEFINE_UNQUOTED([HAVE_LIBPNG],[$HAVE_LIBPNG],[Has LIBPNG])
AM_CONDITIONAL([HAVE_LIBPNG],[test x$HAVE_LIBPNG = 1])

# libjpeg
HAVE_LIBJPEG=0
AC_ARG_WITH(libjpeg,AS_HELP_STRING([--with-libjpeg],[Enable libjpeg]),[],[with_libjpeg=yes])
AC_SUBST(HAVE_LIBJPEG)
AC_SUBST(LIBJPEG_CFLAGS)
AC_SUBST(LIBJPEG_LDFLAGS)
if test "$with_libjpeg" = "yes"; then
    AC_MSG_CHECKING([for libjpeg])
    if pkg-config --atleast-version=1.0.0 jpeg; then
        AC_MSG_RESULT([yes])
        HAVE_LIBJPEG=1
        LIBJPEG_CFLAGS=`pkg-config --cflags jpeg`
        LIBJPEG_LDFLAGS=`pkg-config --libs jpeg`
    elif [[[ -f /usr/lib/libjpeg.a || -f /usr/lib64/libjpeg.a ]]] ; then
        AC_MSG_RESULT([yes])
        HAVE_LIBJPEG=1
        LIBJPEG_CFLAGS=""
        LIBJPEG_LDFLAGS="-ljpeg";
    else
        AC_MSG_RESULT([no])
    fi
fi
AC_DEFINE_UNQUOTED([HAVE_LIBJPEG],[$HAVE_LIBJPEG],[Has LIBJPEG])
AM_CONDITIONAL([HAVE_LIBJPEG],[test x$HAVE_LIBJPEG = 1])

# freetype2
HAVE_FREETYPE2=0
AC_ARG_WITH(freetype2,AS_HELP_STRING([--with-freetype2],[Enable freetype 2]),[],[with_freetype2=yes])
AC_SUBST(HAVE_FREETYPE2)
AC_SUBST(FREETYPE2_CFLAGS)
AC_SUBST(FREETYPE2_LDFLAGS)
if test "$with_freetype2" = "yes"; then
    AC_MSG_CHECKING([for freetype2])
    if pkg-config --atleast-version=9.20.3 freetype2; then
        AC_MSG_RESULT([yes])
        HAVE_FREETYPE2=1
        FREETYPE2_CFLAGS=`pkg-config --cflags freetype2`
        FREETYPE2_LDFLAGS=`pkg-config --libs freetype2`
    else
        AC_MSG_RESULT([no])
    fi
fi
AC_DEFINE_UNQUOTED([HAVE_FREETYPE2],[$HAVE_FREETYPE2],[Has FREETYPE2])
AM_CONDITIONAL([HAVE_FREETYPE2],[test x$HAVE_FREETYPE2 = 1])

# fontconfig
HAVE_FONTCONFIG=0
AC_ARG_WITH(fontconfig,AS_HELP_STRING([--with-fontconfig],[Enable fontconfig]),[],[with_fontconfig=yes])
AC_SUBST(HAVE_FONTCONFIG)
AC_SUBST(FONTCONFIG_CFLAGS)
AC_SUBST(FONTCONFIG_LDFLAGS)
if test "$with_fontconfig" = "yes"; then
    AC_MSG_CHECKING([for fontconfig])
    if pkg-config --atleast-version=2.7.3 fontconfig; then
        AC_MSG_RESULT([yes])
        HAVE_FONTCONFIG=1
        FONTCONFIG_CFLAGS=`pkg-config --cflags fontconfig`
        FONTCONFIG_LDFLAGS=`pkg-config --libs fontconfig`
    else
        AC_MSG_RESULT([no])
    fi
fi
AC_DEFINE_UNQUOTED([HAVE_FONTCONFIG],[$HAVE_FONTCONFIG],[Has FONTCONFIG])
AM_CONDITIONAL([HAVE_FONTCONFIG],[test x$HAVE_FONTCONFIG = 1])

# cairo
HAVE_CAIRO=0
AC_ARG_WITH(cairo,AS_HELP_STRING([--with-cairo],[Enable cairo]),[],[with_cairo=yes])
AC_SUBST(HAVE_CAIRO)
AC_SUBST(CAIRO_CFLAGS)
AC_SUBST(CAIRO_LDFLAGS)
if test "$with_cairo" = "yes"; then
    AC_MSG_CHECKING([for cairo])
    if pkg-config --atleast-version=1.11.2 cairo; then
        AC_MSG_RESULT([yes])
        HAVE_CAIRO=1
        CAIRO_CFLAGS=`pkg-config --cflags cairo`
        CAIRO_LDFLAGS=`pkg-config --libs cairo`
    else
        AC_MSG_RESULT([no])
    fi
fi
AC_DEFINE_UNQUOTED([HAVE_CAIRO],[$HAVE_CAIRO],[Has CAIRO])
AM_CONDITIONAL([HAVE_CAIRO],[test x$HAVE_CAIRO = 1])

AC_CONFIG_FILES(Makefile)
AC_OUTPUT

