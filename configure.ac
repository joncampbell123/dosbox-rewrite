AC_INIT([DOSBox-X-rewrite], 0.01)
AM_INIT_AUTOMAKE
AC_PROG_CC
AC_PROG_CXX

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

AC_CONFIG_FILES(Makefile)
AC_OUTPUT

