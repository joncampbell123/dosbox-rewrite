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

AC_SUBST(CFLAGS)
AC_SUBST(CXXFLAGS)
AC_SUBST(LDFLAGS)

# X11
AC_SUBST(HAVE_X11)
AC_SUBST(X11_CFLAGS)
AC_SUBST(X11_LDFLAGS)
if pkg-config --atleast-version=1.5.0 x11; then
    HAVE_X11=1
    X11_CFLAGS=`pkg-config --cflags x11`
    X11_LDFLAGS=`pkg-config --libs x11`
else
    HAVE_X11=0
fi

# X11-XCB
AC_SUBST(HAVE_X11_XCB)
AC_SUBST(X11_XCB_CFLAGS)
AC_SUBST(X11_XCB_LDFLAGS)
if pkg-config --atleast-version=1.5.0 x11-xcb; then
    HAVE_X11_XCB=1
    X11_XCB_CFLAGS=`pkg-config --cflags x11-xcb`
    X11_XCB_LDFLAGS=`pkg-config --libs x11-xcb`
else
    HAVE_X11_XCB=0
fi

# XCB
AC_SUBST(HAVE_XCB)
AC_SUBST(XCB_CFLAGS)
AC_SUBST(XCB_LDFLAGS)
if pkg-config --atleast-version=1.12 xcb; then
    HAVE_XCB=1
    XCB_CFLAGS=`pkg-config --cflags xcb`
    XCB_LDFLAGS=`pkg-config --libs xcb`
else
    HAVE_XCB=0
fi

# XCB_SHM
AC_SUBST(HAVE_XCB_SHM)
AC_SUBST(XCB_SHM_CFLAGS)
AC_SUBST(XCB_SHM_LDFLAGS)
if pkg-config --atleast-version=1.12 xcb-shm; then
    HAVE_XCB_SHM=1
    XCB_SHM_CFLAGS=`pkg-config --cflags xcb-shm`
    XCB_SHM_LDFLAGS=`pkg-config --libs xcb-shm`
else
    HAVE_XCB_SHM=0
fi

# XCB_XKB
AC_SUBST(HAVE_XCB_XKB)
AC_SUBST(XCB_XKB_CFLAGS)
AC_SUBST(XCB_XKB_LDFLAGS)
if pkg-config --atleast-version=1.12 xcb-xkb; then
    HAVE_XCB_XKB=1
    XCB_XKB_CFLAGS=`pkg-config --cflags xcb-xkb`
    XCB_XKB_LDFLAGS=`pkg-config --libs xcb-xkb`
else
    HAVE_XCB_XKB=0
fi

# ALSA
AC_SUBST(HAVE_ALSA)
AC_SUBST(ALSA_CFLAGS)
AC_SUBST(ALSA_LDFLAGS)
if pkg-config --atleast-version=1.0.27.2 alsa; then
    HAVE_ALSA=1
    ALSA_CFLAGS=`pkg-config --cflags alsa`
    ALSA_LDFLAGS=`pkg-config --libs alsa`
else
    HAVE_ALSA=0
fi

# LIBDRM
AC_SUBST(HAVE_LIBDRM)
AC_SUBST(LIBDRM_CFLAGS)
AC_SUBST(LIBDRM_LDFLAGS)
if pkg-config --atleast-version=2.4.53 libdrm; then
    HAVE_LIBDRM=1
    LIBDRM_CFLAGS=`pkg-config --cflags libdrm`
    LIBDRM_LDFLAGS=`pkg-config --libs libdrm`
else
    HAVE_LIBDRM=0
fi

# OpenGL (Mesa)
AC_SUBST(HAVE_OPENGL)
AC_SUBST(OPENGL_CFLAGS)
AC_SUBST(OPENGL_LDFLAGS)
if pkg-config --atleast-version=10.0.4 gl; then
    HAVE_OPENGL=1
    OPENGL_CFLAGS=`pkg-config --cflags gl`
    OPENGL_LDFLAGS=`pkg-config --libs gl`
else
    HAVE_OPENGL=0
fi

# OpenGL (Mesa) GLU
AC_SUBST(HAVE_OPENGL_GLU)
AC_SUBST(OPENGL_GLU_CFLAGS)
AC_SUBST(OPENGL_GLU_LDFLAGS)
if pkg-config --atleast-version=9.0.0 glu; then
    HAVE_OPENGL_GLU=1
    OPENGL_GLU_CFLAGS=`pkg-config --cflags glu`
    OPENGL_GLU_LDFLAGS=`pkg-config --libs glu`
else
    HAVE_OPENGL_GLU=0
fi

# OpenGL (Mesa) GLEW
AC_SUBST(HAVE_OPENGL_GLEW)
AC_SUBST(OPENGL_GLEW_CFLAGS)
AC_SUBST(OPENGL_GLEW_LDFLAGS)
if pkg-config --atleast-version=1.6.0 glew; then
    HAVE_OPENGL_GLEW=1
    OPENGL_GLEW_CFLAGS=`pkg-config --cflags glew`
    OPENGL_GLEW_LDFLAGS=`pkg-config --libs glew`
else
    HAVE_OPENGL_GLEW=0
fi

# OpenGL (Mesa) EGL
AC_SUBST(HAVE_OPENGL_EGL)
AC_SUBST(OPENGL_EGL_CFLAGS)
AC_SUBST(OPENGL_EGL_LDFLAGS)
if pkg-config --atleast-version=10.0.4 egl; then
    HAVE_OPENGL_EGL=1
    OPENGL_EGL_CFLAGS=`pkg-config --cflags egl`
    OPENGL_EGL_LDFLAGS=`pkg-config --libs egl`
else
    HAVE_OPENGL_EGL=0
fi

# ZLIB
AC_SUBST(HAVE_ZLIB)
AC_SUBST(ZLIB_CFLAGS)
AC_SUBST(ZLIB_LDFLAGS)
if pkg-config --atleast-version=1.2.6 zlib; then
    HAVE_ZLIB=1
    ZLIB_CFLAGS=`pkg-config --cflags zlib`
    ZLIB_LDFLAGS=`pkg-config --libs zlib`
else
    HAVE_ZLIB=0
fi

# LZMA
AC_SUBST(HAVE_LZMA)
AC_SUBST(LZMA_CFLAGS)
AC_SUBST(LZMA_LDFLAGS)
if pkg-config --atleast-version=5.0.3 liblzma; then
    HAVE_LZMA=1
    LZMA_CFLAGS=`pkg-config --cflags liblzma`
    LZMA_LDFLAGS=`pkg-config --libs liblzma`
else
    HAVE_LZMA=0
fi

AC_CONFIG_FILES(Makefile)
AC_OUTPUT

