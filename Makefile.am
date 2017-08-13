
CLEANFILES =

noinst_LIBRARIES = \
	dosboxxr/lib/hostcpudetect/libhostcpudetect.a \
	dosboxxr/lib/graphics/libstretchblt.a \
	dosboxxr/lib/util/libutil.a \
	dosboxxr/lib/text/libtext.a \
	dosboxxr/lib/cpu/libcpu.a

if HAVE_X11
noinst_LIBRARIES += \
	dosboxxr/lib/xlib/libxlibdemo.a
endif

dosboxxr_lib_hostcpudetect_libhostcpudetect_a_SOURCES = dosboxxr/lib/hostcpudetect/caps.cpp

dosboxxr_lib_cpu_libcpu_a_SOURCES = dosboxxr/lib/cpu/ipdec_strings.cpp

dosboxxr_lib_text_libtext_a_SOURCES = dosboxxr/lib/text/chomp.c

dosboxxr_lib_util_libutil_a_SOURCES = \
	dosboxxr/lib/util/nr_wfpack_scale.cpp \
	dosboxxr/lib/util/rgb_bitmap_test_pattern_1.cpp \
	dosboxxr/lib/util/rgb_bitmap_test_pattern_gradients.cpp

dosboxxr_lib_xlib_libxlibdemo_a_SOURCES = \
	dosboxxr/lib/xlib/demo.cpp

# NTS: This is done manually to allow each stretchblt to be compiled with the appropriate CFLAGS.
#      Compiling all with the -mmmx -msse etc flags only causes problems because it then tells GCC
#      it's ALWAYS ok to use those extended instruction sets, which is NOT what we want! That causes
#      problems because then it means our SSE code crashes attempting to use AVX instructions on a
#      CPU that doesn't support it because GCC thought it was faster to use them in the prologue.
dosboxxr/lib/graphics/libstretchblt.a: dosboxxr/lib/graphics/stretchblt_neighbor.o dosboxxr/lib/graphics/stretchblt_bilinear.o dosboxxr/lib/graphics/stretchblt_bilinear_mmx.o dosboxxr/lib/graphics/stretchblt_bilinear_sse.o dosboxxr/lib/graphics/stretchblt_bilinear_avx.o dosboxxr/lib/graphics/stretchblt_bilinear_arm_neon.o dosboxxr/lib/graphics/stretchblt_general.o
	$(RM) dosboxxr/lib/graphics/libstretchblt.a
	$(AR) r dosboxxr/lib/graphics/libstretchblt.a dosboxxr/lib/graphics/stretchblt_neighbor.o dosboxxr/lib/graphics/stretchblt_bilinear.o dosboxxr/lib/graphics/stretchblt_bilinear_mmx.o dosboxxr/lib/graphics/stretchblt_bilinear_sse.o dosboxxr/lib/graphics/stretchblt_bilinear_avx.o dosboxxr/lib/graphics/stretchblt_bilinear_arm_neon.o dosboxxr/lib/graphics/stretchblt_general.o

CLEANFILES += dosboxxr/lib/graphics/libstretchblt.a dosboxxr/lib/graphics/libstretchblt.a dosboxxr/lib/graphics/stretchblt_neighbor.o dosboxxr/lib/graphics/stretchblt_bilinear.o dosboxxr/lib/graphics/stretchblt_bilinear_mmx.o dosboxxr/lib/graphics/stretchblt_bilinear_sse.o dosboxxr/lib/graphics/stretchblt_bilinear_avx.o dosboxxr/lib/graphics/stretchblt_bilinear_arm_neon.o dosboxxr/lib/graphics/stretchblt_general.o

dosboxxr/lib/graphics/stretchblt_neighbor.o: dosboxxr/lib/graphics/stretchblt_neighbor.cpp
	$(CXX) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(AM_CXXFLAGS) $(CPPFLAGS) -c -o $@ $<

dosboxxr/lib/graphics/stretchblt_bilinear.o: dosboxxr/lib/graphics/stretchblt_bilinear.cpp
	$(CXX) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(AM_CXXFLAGS) $(CPPFLAGS) -c -o $@ $<

dosboxxr/lib/graphics/stretchblt_bilinear_mmx.o: dosboxxr/lib/graphics/stretchblt_bilinear_mmx.cpp
	$(CXX) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(AM_CXXFLAGS) $(CPPFLAGS) $(CPU_MMX_CFLAGS) -c -o $@ $<

dosboxxr/lib/graphics/stretchblt_bilinear_sse.o: dosboxxr/lib/graphics/stretchblt_bilinear_sse.cpp
	$(CXX) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(AM_CXXFLAGS) $(CPPFLAGS) $(CPU_SSE_CFLAGS) $(CPU_SSE2_CFLAGS) -c -o $@ $<

dosboxxr/lib/graphics/stretchblt_bilinear_avx.o: dosboxxr/lib/graphics/stretchblt_bilinear_avx.cpp
	$(CXX) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(AM_CXXFLAGS) $(CPPFLAGS) $(CPU_SSE_CFLAGS) $(CPU_SSE2_CFLAGS) $(CPU_AVX_CFLAGS) $(CPU_AVX2_CFLAGS) -c -o $@ $<

dosboxxr/lib/graphics/stretchblt_bilinear_arm_neon.o: dosboxxr/lib/graphics/stretchblt_bilinear_arm_neon.cpp
	$(CXX) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(AM_CXXFLAGS) $(CPPFLAGS) $(CPU_ARM_NEON_CFLAGS) -c -o $@ $<

dosboxxr/lib/graphics/stretchblt_general.o: dosboxxr/lib/graphics/stretchblt_general.cpp
	$(CXX) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(AM_CXXFLAGS) $(CPPFLAGS) -c -o $@ $<

OPCC=bin/tools/opcc

noinst_PROGRAMS = \
	bin/tools/opcc \
	bin/tools/cpudec86 \
	bin/tools/cpudec86e \
	bin/tools/cpudetect \
	bin/test/cpumodregrm \
	bin/test/cpumemreftypes \
	bin/tools/cpustep86

if HAVE_WINDOWS_H
noinst_PROGRAMS += \
	bin/test/wingdi1 \
	bin/test/wingdi2
endif

if HAVE_WINDOWS_H
noinst_PROGRAMS += \
	bin/test/windx1_1 \
    bin/test/windx2_1 \
    bin/test/windx4_1 \
    bin/test/windx7_1
endif

if IS_MAC_OSX
if HAS_OBJCXX
noinst_PROGRAMS += \
	bin/test/cocoa1 \
	bin/test/cocoa2
endif
endif

if HAVE_X11
if HAVE_OPENGL
if HAVE_OPENGL_GLU
noinst_PROGRAMS += \
	bin/test/xlibopengl \
	bin/test/xlibopengl2
endif
endif
endif

if HAVE_X11
if HAVE_X11_XCB
if HAVE_OPENGL
if HAVE_OPENGL_GLU
noinst_PROGRAMS += \
	bin/test/xlibxcbopengl
endif
endif
endif
endif

if HAVE_X11
if HAVE_XEXT
noinst_PROGRAMS += \
	bin/test/xlib1 \
	bin/test/xlib2
endif
endif

if HAVE_X11
if HAVE_XEXT
if HAVE_XCB_SHM
if HAVE_X11_XCB
noinst_PROGRAMS += \
	bin/test/xcb1 \
	bin/test/xcb2
endif
endif
endif
endif

if HAVE_LINUX_FB_H
if HAVE_TERMIOS_H
noinst_PROGRAMS += \
	bin/test/fb1 \
	bin/test/fb2
endif
endif

bin_tools_cpustep86_SOURCES = \
	tools/cpustep/cpustep86.cpp \
	tools/cpustep/cpudec86_everything.cpp \
	tools/cpustep/cpustep86_everything.cpp
bin_tools_cpustep86_CXXFLAGS = $(AM_CXXFLAGS) -Itools/cpustep
bin_tools_cpustep86_DEPENDENCIES = \
	dosboxxr/lib/cpu/libcpu.a $(OPCC_DECOM_H)
bin_tools_cpustep86_LDADD = \
	dosboxxr/lib/cpu/libcpu.a

bin_tools_cpudec86_SOURCES = \
	tools/cpudec/cpudec86.cpp \
	tools/cpudec/cpudec86_8086.cpp \
	tools/cpudec/cpudec86_necv20.cpp \
	tools/cpudec/cpudec86_80186.cpp \
	tools/cpudec/cpudec86_80286.cpp \
	tools/cpudec/cpudec86_80386.cpp \
	tools/cpudec/cpudec86_80386_generic.cpp \
	tools/cpudec/cpudec86_80386early.cpp \
	tools/cpudec/cpudec86_80386early_generic.cpp \
	tools/cpudec/cpudec86_80486early.cpp \
	tools/cpudec/cpudec86_80486early_generic.cpp \
	tools/cpudec/cpudec86_80486.cpp \
	tools/cpudec/cpudec86_80486_generic.cpp \
	tools/cpudec/cpudec86_pentium.cpp \
	tools/cpudec/cpudec86_pentium_generic.cpp \
	tools/cpudec/cpudec86_pentiumpro.cpp \
	tools/cpudec/cpudec86_pentiumpro_generic.cpp \
	tools/cpudec/cpudec86_pentiummmx.cpp \
	tools/cpudec/cpudec86_pentiummmx_generic.cpp \
	tools/cpudec/cpudec86_pentiumprommx.cpp \
	tools/cpudec/cpudec86_pentiumprommx_generic.cpp \
	tools/cpudec/cpudec86_cyrix6x86mx.cpp \
	tools/cpudec/cpudec86_cyrix6x86mx_generic.cpp \
	tools/cpudec/cpudec86_pentium2.cpp \
	tools/cpudec/cpudec86_pentium2_generic.cpp \
	tools/cpudec/cpudec86_pentium3.cpp \
	tools/cpudec/cpudec86_pentium3_generic.cpp \
	tools/cpudec/cpudec86_pentium4.cpp \
	tools/cpudec/cpudec86_pentium4_generic.cpp \
	tools/cpudec/cpudec86_pentium4prescott.cpp \
	tools/cpudec/cpudec86_pentium4prescott_generic.cpp \
	tools/cpudec/cpudec86_amdk6r2.cpp \
	tools/cpudec/cpudec86_amdk6r2_generic.cpp \
	tools/cpudec/cpudec86_amdathlon.cpp \
	tools/cpudec/cpudec86_amdathlon_generic.cpp \
	tools/cpudec/cpudec86_intelcore.cpp \
	tools/cpudec/cpudec86_intelcore_generic.cpp \
	tools/cpudec/cpudec86_everything.cpp \
	tools/cpudec/cpudec86_everything_generic.cpp

bin_tools_cpudec86e_SOURCES = \
	tools/cpudec/cpudec86.cpp \
	tools/cpudec/cpudec86_everything.cpp \
	tools/cpudec/cpudec86_everything_generic.cpp

bin_tools_cpudec86_CXXFLAGS = $(AM_CXXFLAGS) -Itools/cpudec
bin_tools_cpudec86_DEPENDENCIES = \
	dosboxxr/lib/cpu/libcpu.a $(OPCC_DECOM_H)
bin_tools_cpudec86_LDADD = \
	dosboxxr/lib/cpu/libcpu.a

bin_tools_cpudec86e_CXXFLAGS = $(AM_CXXFLAGS) -Itools/cpudec -DONLY_EVERYTHING=1
bin_tools_cpudec86e_DEPENDENCIES = \
	dosboxxr/lib/cpu/libcpu.a $(OPCC_DECOM_H)
bin_tools_cpudec86e_LDADD = \
	dosboxxr/lib/cpu/libcpu.a

bin_tools_cpudetect_SOURCES = tools/cpudetect/cpudetect.cpp
bin_tools_cpudetect_CXXFLAGS = $(AM_CXXFLAGS)
bin_tools_cpudetect_DEPENDENCIES = \
	dosboxxr/lib/hostcpudetect/libhostcpudetect.a \
	dosboxxr/lib/text/libtext.a
bin_tools_cpudetect_LDADD = \
	dosboxxr/lib/hostcpudetect/libhostcpudetect.a \
	dosboxxr/lib/text/libtext.a

bin_test_xlibopengl_SOURCES = test/xlibopengl.cpp
bin_test_xlibopengl_CXXFLAGS = $(AM_CXXFLAGS) $(X11_CFLAGS) $(OPENGL_CFLAGS) $(OPENGL_GLU_CFLAGS)
bin_test_xlibopengl_DEPENDENCIES =
bin_test_xlibopengl_LDADD = $(X11_LDFLAGS) $(OPENGL_LDFLAGS) $(OPENGL_GLU_LDFLAGS)

bin_test_xlibopengl2_SOURCES = test/xlibopengl2.cpp
bin_test_xlibopengl2_CXXFLAGS = $(AM_CXXFLAGS) $(X11_CFLAGS) $(OPENGL_CFLAGS) $(OPENGL_GLU_CFLAGS)
bin_test_xlibopengl2_DEPENDENCIES = \
	dosboxxr/lib/hostcpudetect/libhostcpudetect.a \
	dosboxxr/lib/util/libutil.a \
	dosboxxr/lib/text/libtext.a \
	dosboxxr/lib/xlib/libxlibdemo.a
bin_test_xlibopengl2_LDADD = $(X11_LDFLAGS) $(OPENGL_LDFLAGS) $(OPENGL_GLU_LDFLAGS) \
	dosboxxr/lib/hostcpudetect/libhostcpudetect.a \
	dosboxxr/lib/util/libutil.a \
	dosboxxr/lib/text/libtext.a \
	dosboxxr/lib/xlib/libxlibdemo.a

bin_test_xlibxcbopengl_SOURCES = test/xlibxcbopengl.cpp
bin_test_xlibxcbopengl_CXXFLAGS = $(AM_CXXFLAGS) $(X11_CFLAGS) $(OPENGL_CFLAGS) $(OPENGL_GLU_CFLAGS) $(X11_XCB_CFLAGS)
bin_test_xlibxcbopengl_DEPENDENCIES =
bin_test_xlibxcbopengl_LDADD = $(X11_LDFLAGS) $(OPENGL_LDFLAGS) $(OPENGL_GLU_LDFLAGS) $(X11_XCB_LDFLAGS)

bin_test_xlib1_SOURCES = test/xlib1.cpp
bin_test_xlib1_CXXFLAGS = $(AM_CXXFLAGS) $(X11_CFLAGS) $(XEXT_CFLAGS)
bin_test_xlib1_DEPENDENCIES = \
	dosboxxr/lib/util/libutil.a \
	dosboxxr/lib/xlib/libxlibdemo.a
bin_test_xlib1_LDADD = $(X11_LDFLAGS) $(XEXT_LDFLAGS) \
	dosboxxr/lib/util/libutil.a \
	dosboxxr/lib/xlib/libxlibdemo.a

bin_test_xlib2_SOURCES = test/xlib2.cpp
bin_test_xlib2_CXXFLAGS = $(AM_CXXFLAGS) $(X11_CFLAGS) $(XEXT_CFLAGS)
bin_test_xlib2_DEPENDENCIES = \
	dosboxxr/lib/hostcpudetect/libhostcpudetect.a \
	dosboxxr/lib/graphics/libstretchblt.a \
	dosboxxr/lib/util/libutil.a \
	dosboxxr/lib/text/libtext.a \
	dosboxxr/lib/xlib/libxlibdemo.a
bin_test_xlib2_LDADD = $(X11_LDFLAGS) $(XEXT_LDFLAGS) \
	dosboxxr/lib/graphics/libstretchblt.a \
	dosboxxr/lib/hostcpudetect/libhostcpudetect.a \
	dosboxxr/lib/text/libtext.a \
	dosboxxr/lib/util/libutil.a \
	dosboxxr/lib/xlib/libxlibdemo.a

bin_test_xcb1_SOURCES = test/xcb1.cpp
bin_test_xcb1_CXXFLAGS = $(AM_CXXFLAGS) $(X11_CFLAGS) $(XEXT_CFLAGS) $(X11_XCB_CFLAGS) $(XCB_SHM_CFLAGS)
bin_test_xcb1_DEPENDENCIES = \
	dosboxxr/lib/util/libutil.a
bin_test_xcb1_LDADD = $(X11_LDFLAGS) $(XEXT_LDFLAGS) $(X11_XCB_LDFLAGS) $(XCB_SHM_LDFLAGS) \
	dosboxxr/lib/util/libutil.a

bin_test_xcb2_SOURCES = test/xcb2.cpp
bin_test_xcb2_CXXFLAGS = $(AM_CXXFLAGS) $(X11_CFLAGS) $(XEXT_CFLAGS) $(X11_XCB_CFLAGS) $(XCB_SHM_CFLAGS)
bin_test_xcb2_DEPENDENCIES = \
	dosboxxr/lib/hostcpudetect/libhostcpudetect.a \
	dosboxxr/lib/graphics/libstretchblt.a \
	dosboxxr/lib/util/libutil.a \
	dosboxxr/lib/text/libtext.a \
	dosboxxr/lib/xlib/libxlibdemo.a
bin_test_xcb2_LDADD = $(X11_LDFLAGS) $(XEXT_LDFLAGS) $(X11_XCB_LDFLAGS) $(XCB_SHM_LDFLAGS) \
	dosboxxr/lib/graphics/libstretchblt.a \
	dosboxxr/lib/hostcpudetect/libhostcpudetect.a \
	dosboxxr/lib/text/libtext.a \
	dosboxxr/lib/util/libutil.a \
	dosboxxr/lib/xlib/libxlibdemo.a

bin_test_fb1_SOURCES = test/fb1.cpp
bin_test_fb1_CXXFLAGS = $(AM_CXXFLAGS)
bin_test_fb1_DEPENDENCIES = \
	dosboxxr/lib/util/libutil.a
bin_test_fb1_LDADD = \
	dosboxxr/lib/util/libutil.a

bin_test_fb2_SOURCES = test/fb2.cpp
bin_test_fb2_CXXFLAGS = $(AM_CXXFLAGS)
bin_test_fb2_DEPENDENCIES = \
	dosboxxr/lib/graphics/libstretchblt.a \
	dosboxxr/lib/hostcpudetect/libhostcpudetect.a \
	dosboxxr/lib/text/libtext.a \
	dosboxxr/lib/util/libutil.a \
	dosboxxr/lib/xlib/libxlibdemo.a
bin_test_fb2_LDADD = \
	dosboxxr/lib/graphics/libstretchblt.a \
	dosboxxr/lib/hostcpudetect/libhostcpudetect.a \
	dosboxxr/lib/text/libtext.a \
	dosboxxr/lib/util/libutil.a \
	dosboxxr/lib/xlib/libxlibdemo.a

bin_test_cocoa1_SOURCES = test/cocoa1.cpp
bin_test_cocoa1_CXXFLAGS = $(AM_CXXFLAGS) $(OBJCXXFLAGS)
bin_test_cocoa1_DEPENDENCIES = \
	dosboxxr/lib/util/libutil.a
bin_test_cocoa1_LDADD = \
	dosboxxr/lib/util/libutil.a
bin_test_cocoa1_LDFLAGS = $(AM_LDFLAGS) -framework CoreFoundation -framework CoreServices -framework Cocoa -framework Carbon

bin_test_cocoa2_SOURCES = test/cocoa2.cpp
bin_test_cocoa2_CXXFLAGS = $(AM_CXXFLAGS) $(OBJCXXFLAGS)
bin_test_cocoa2_DEPENDENCIES = \
	dosboxxr/lib/graphics/libstretchblt.a \
	dosboxxr/lib/hostcpudetect/libhostcpudetect.a \
	dosboxxr/lib/text/libtext.a \
	dosboxxr/lib/util/libutil.a \
	dosboxxr/lib/xlib/libxlibdemo.a
bin_test_cocoa2_LDADD = \
	dosboxxr/lib/graphics/libstretchblt.a \
	dosboxxr/lib/hostcpudetect/libhostcpudetect.a \
	dosboxxr/lib/text/libtext.a \
	dosboxxr/lib/util/libutil.a \
	dosboxxr/lib/xlib/libxlibdemo.a
bin_test_cocoa2_LDFLAGS = $(AM_LDFLAGS) -framework CoreFoundation -framework CoreServices -framework Cocoa -framework Carbon

bin_test_windx1_1_SOURCES = test/windx1_1.cpp
bin_test_windx1_1_CXXFLAGS = $(AM_CXXFLAGS) -Idosboxxr/ext/directx
bin_test_windx1_1_DEPENDENCIES = \
	dosboxxr/lib/util/libutil.a
bin_test_windx1_1_LDADD = $(X11_LDFLAGS) \
	dosboxxr/lib/util/libutil.a -lkernel32 -lgdi32 -lddraw

bin_test_windx2_1_SOURCES = test/windx2_1.cpp
bin_test_windx2_1_CXXFLAGS = $(AM_CXXFLAGS) -Idosboxxr/ext/directx
bin_test_windx2_1_DEPENDENCIES = \
	dosboxxr/lib/util/libutil.a
bin_test_windx2_1_LDADD = $(X11_LDFLAGS) \
	dosboxxr/lib/util/libutil.a -lkernel32 -lgdi32 -lddraw -ldxguid

bin_test_windx4_1_SOURCES = test/windx4_1.cpp
bin_test_windx4_1_CXXFLAGS = $(AM_CXXFLAGS) -Idosboxxr/ext/directx
bin_test_windx4_1_DEPENDENCIES = \
	dosboxxr/lib/util/libutil.a
bin_test_windx4_1_LDADD = $(X11_LDFLAGS) \
	dosboxxr/lib/util/libutil.a -lkernel32 -lgdi32 -lddraw -ldxguid

bin_test_windx7_1_SOURCES = test/windx7_1.cpp
bin_test_windx7_1_CXXFLAGS = $(AM_CXXFLAGS) -Idosboxxr/ext/directx
bin_test_windx7_1_DEPENDENCIES = \
	dosboxxr/lib/util/libutil.a
bin_test_windx7_1_LDADD = $(X11_LDFLAGS) \
	dosboxxr/lib/util/libutil.a -lkernel32 -lgdi32 -lddraw -ldxguid

bin_test_wingdi1_SOURCES = test/wingdi1.cpp
bin_test_wingdi1_CXXFLAGS = $(AM_CXXFLAGS)
bin_test_wingdi1_DEPENDENCIES = \
	dosboxxr/lib/util/libutil.a
bin_test_wingdi1_LDADD = $(X11_LDFLAGS) \
	dosboxxr/lib/util/libutil.a -lkernel32 -lgdi32

bin_test_wingdi2_SOURCES = test/wingdi2.cpp
bin_test_wingdi2_CXXFLAGS = $(AM_CXXFLAGS)
bin_test_wingdi2_DEPENDENCIES = \
	dosboxxr/lib/graphics/libstretchblt.a \
	dosboxxr/lib/hostcpudetect/libhostcpudetect.a \
	dosboxxr/lib/text/libtext.a \
	dosboxxr/lib/util/libutil.a
bin_test_wingdi2_LDADD = $(X11_LDFLAGS) \
	dosboxxr/lib/graphics/libstretchblt.a \
	dosboxxr/lib/hostcpudetect/libhostcpudetect.a \
	dosboxxr/lib/text/libtext.a \
	dosboxxr/lib/util/libutil.a \
	-lkernel32 -lgdi32

bin_test_cpumodregrm_SOURCES = test/cpumodregrm.cpp
bin_test_cpumodregrm_CXXFLAGS = $(AM_CXXFLAGS)
bin_test_cpumodregrm_DEPENDENCIES =
bin_test_cpumodregrm_LDADD =

bin_test_cpumemreftypes_SOURCES = test/cpumemreftypes.cpp
bin_test_cpumemreftypes_CXXFLAGS = $(AM_CXXFLAGS)
bin_test_cpumemreftypes_DEPENDENCIES =
bin_test_cpumemreftypes_LDADD =

OPCC_DECOM_H = \
	dosboxxr/lib/cpu/core/intel8088.c16.a16.decom.h \
	\
	dosboxxr/lib/cpu/core/necv20.c16.a16.decom.h \
	\
	dosboxxr/lib/cpu/core/intel80186.c16.a16.decom.h \
	\
	dosboxxr/lib/cpu/core/intel80286.c16.a16.decom.h \
	\
	dosboxxr/lib/cpu/core/intel80386.cxx.axx.decom.h \
	\
	dosboxxr/lib/cpu/core/intel80386early.cxx.axx.decom.h \
	\
	dosboxxr/lib/cpu/core/intel80486.cxx.axx.decom.h \
	\
	dosboxxr/lib/cpu/core/intel80486early.cxx.axx.decom.h \
	\
	dosboxxr/lib/cpu/core/intelPentium.cxx.axx.decom.h \
	\
	dosboxxr/lib/cpu/core/intelPentiumPro.cxx.axx.decom.h \
	\
	dosboxxr/lib/cpu/core/intelPentiumMMX.cxx.axx.decom.h \
	\
	dosboxxr/lib/cpu/core/intelPentiumProMMX.cxx.axx.decom.h \
	\
	dosboxxr/lib/cpu/core/cyrix6x86MX.cxx.axx.decom.h \
	\
	dosboxxr/lib/cpu/core/intelPentium2.cxx.axx.decom.h \
	\
	dosboxxr/lib/cpu/core/intelPentium3.cxx.axx.decom.h \
	\
	dosboxxr/lib/cpu/core/intelPentium4.cxx.axx.decom.h \
	\
	dosboxxr/lib/cpu/core/intelPentium4Prescott.cxx.axx.decom.h \
	\
	dosboxxr/lib/cpu/core/amdk6r2.cxx.axx.decom.h \
	\
	dosboxxr/lib/cpu/core/amdAthlon.cxx.axx.decom.h \
	\
	dosboxxr/lib/cpu/core/intelCore.cxx.axx.decom.h \
	\
	dosboxxr/lib/cpu/core/everything.cxx.axx.decom.h

OPCC_EMU_H = \
	dosboxxr/lib/cpu/core/intel8088.c16.a16.emu.h \
	\
	dosboxxr/lib/cpu/core/everything.c16.a16.emu.h \
	dosboxxr/lib/cpu/core/everything.cxx.axx.emu.h \
	dosboxxr/lib/cpu/core/everything.c32.a32.emu.h

testasm/test1.bin: testasm/test1.asm
	yasm -o $@ -f bin $<

testasm/test2.bin: testasm/test2.asm
	yasm -o $@ -f bin $<

testasm/test3.bin: testasm/test3.asm
	yasm -o $@ -f bin $<

testasm/test3e.bin: testasm/test3e.asm
	yasm -o $@ -f bin $<

testasm/test4.bin: testasm/test4.asm
	yasm -o $@ -f bin $<

testasm/test4e.bin: testasm/test4e.asm
	yasm -o $@ -f bin $<

testasm/test5.bin: testasm/test5.asm
	yasm -o $@ -f bin $<

testasm/test5pro.bin: testasm/test5pro.asm
	yasm -o $@ -f bin $<

testasm/test5mmx.bin: testasm/test5mmx.asm
	yasm -o $@ -f bin $<

testasm/testc6mx.bin: testasm/testc6mx.asm
	yasm -o $@ -f bin $<

testasm/testp2.bin: testasm/testp2.asm
	yasm -o $@ -f bin $<

testasm/testp3.bin: testasm/testp3.asm
	yasm -o $@ -f bin $<

testasm/testp4.bin: testasm/testp4.asm
	yasm -o $@ -f bin $<

testasm/testp4p.bin: testasm/testp4p.asm
	yasm -o $@ -f bin $<

testasm/testv20.bin: testasm/testv20.asm
	yasm -o $@ -f bin $<

testasm/test3dn1.bin: testasm/test3dn1.asm
	yasm -o $@ -f bin $<

testasm/test3dn2.bin: testasm/test3dn2.asm
	yasm -o $@ -f bin $<

testasm/testssse3.bin: testasm/testssse3.asm
	yasm -o $@ -f bin $<

testasm/testevry.bin: testasm/testevry.asm
	yasm -o $@ -f bin $<

testasm/testemu1.bin: testasm/testemu1.asm
	yasm -o $@ -f bin $<

test-bins: testasm/test1.bin testasm/test2.bin testasm/test3.bin testasm/test3e.bin testasm/test4.bin testasm/test4e.bin testasm/test5.bin testasm/test5pro.bin testasm/test5mmx.bin testasm/testp2.bin testasm/testp3.bin testasm/testp4.bin testasm/testp4p.bin testasm/testv20.bin testasm/testc6mx.bin testasm/test3dn1.bin testasm/test3dn2.bin testasm/testssse3.bin testasm/testevry.bin testasm/testemu1.bin

bin/tools/opcc$(EXEEXT): tools/opcc/opcc.cpp
	$(BUILD_CXX) -o $@ $<

# NTS: I would use cpp -E -I. <infile> -o <outfile> except that for whatever reason,
#      Clang 7.3.0 on Mac OS X is too brain-dead to recognize it even though --help says
#      that we can use it. But there's a lesser-known <infile> <outfile> syntax that works. *sigh*

dosboxxr/lib/cpu/core/necv20.opcodes.p: opcodes/necv20.opcodes
	$(BUILD_CPP) -E -I. opcodes/necv20.opcodes $@
dosboxxr/lib/cpu/core/necv20.c16.a16.decom.h: dosboxxr/lib/cpu/core/necv20.opcodes.p $(OPCC)$(EXEEXT)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/necv20.opcodes.p -o $@ -m decompile -gc1632

dosboxxr/lib/cpu/core/intel8088.opcodes.p: opcodes/intel8088.opcodes
	$(BUILD_CPP) -E -I. opcodes/intel8088.opcodes $@
dosboxxr/lib/cpu/core/intel8088.c16.a16.decom.h: dosboxxr/lib/cpu/core/intel8088.opcodes.p $(OPCC)$(EXEEXT)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intel8088.opcodes.p -o $@ -m decompile -gc1632
dosboxxr/lib/cpu/core/intel8088.c16.a16.emu.h: dosboxxr/lib/cpu/core/intel8088.opcodes.p $(OPCC)$(EXEEXT)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intel8088.opcodes.p -o $@ -m execute -gc1632

dosboxxr/lib/cpu/core/intel80186.opcodes.p: opcodes/intel80186.opcodes
	$(BUILD_CPP) -E -I. opcodes/intel80186.opcodes $@
dosboxxr/lib/cpu/core/intel80186.c16.a16.decom.h: dosboxxr/lib/cpu/core/intel80186.opcodes.p $(OPCC)$(EXEEXT)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intel80186.opcodes.p -o $@ -m decompile -gc1632

dosboxxr/lib/cpu/core/intel80286.opcodes.p: opcodes/intel80286.opcodes
	$(BUILD_CPP) -E -I. opcodes/intel80286.opcodes $@
dosboxxr/lib/cpu/core/intel80286.c16.a16.decom.h: dosboxxr/lib/cpu/core/intel80286.opcodes.p $(OPCC)$(EXEEXT)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intel80286.opcodes.p -o $@ -m decompile -gc1632

dosboxxr/lib/cpu/core/intel80386.opcodes.p: opcodes/intel80386.opcodes
	$(BUILD_CPP) -E -I. opcodes/intel80386.opcodes $@
dosboxxr/lib/cpu/core/intel80386.c16.a16.decom.h: dosboxxr/lib/cpu/core/intel80386.opcodes.p $(OPCC)$(EXEEXT)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intel80386.opcodes.p -o $@ -m decompile -c 16 -a 16 -c32vars -gc1632
dosboxxr/lib/cpu/core/intel80386.cxx.axx.decom.h: dosboxxr/lib/cpu/core/intel80386.opcodes.p $(OPCC)$(EXEEXT)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intel80386.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars -g
dosboxxr/lib/cpu/core/intel80386.c32.a32.decom.h: dosboxxr/lib/cpu/core/intel80386.opcodes.p $(OPCC)$(EXEEXT)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intel80386.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars -gc1632

dosboxxr/lib/cpu/core/intel80386early.opcodes.p: opcodes/intel80386early.opcodes
	$(BUILD_CPP) -E -I. opcodes/intel80386early.opcodes $@
dosboxxr/lib/cpu/core/intel80386early.c16.a16.decom.h: dosboxxr/lib/cpu/core/intel80386early.opcodes.p $(OPCC)$(EXEEXT)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intel80386early.opcodes.p -o $@ -m decompile -c 16 -a 16 -c32vars -gc1632
dosboxxr/lib/cpu/core/intel80386early.cxx.axx.decom.h: dosboxxr/lib/cpu/core/intel80386early.opcodes.p $(OPCC)$(EXEEXT)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intel80386early.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars -g
dosboxxr/lib/cpu/core/intel80386early.c32.a32.decom.h: dosboxxr/lib/cpu/core/intel80386early.opcodes.p $(OPCC)$(EXEEXT)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intel80386early.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars -gc1632

dosboxxr/lib/cpu/core/intel80486.opcodes.p: opcodes/intel80486.opcodes
	$(BUILD_CPP) -E -I. opcodes/intel80486.opcodes $@
dosboxxr/lib/cpu/core/intel80486.c16.a16.decom.h: dosboxxr/lib/cpu/core/intel80486.opcodes.p $(OPCC)$(EXEEXT)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intel80486.opcodes.p -o $@ -m decompile -c 16 -a 16 -c32vars -gc1632
dosboxxr/lib/cpu/core/intel80486.cxx.axx.decom.h: dosboxxr/lib/cpu/core/intel80486.opcodes.p $(OPCC)$(EXEEXT)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intel80486.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars -g
dosboxxr/lib/cpu/core/intel80486.c32.a32.decom.h: dosboxxr/lib/cpu/core/intel80486.opcodes.p $(OPCC)$(EXEEXT)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intel80486.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars -gc1632

dosboxxr/lib/cpu/core/intel80486early.opcodes.p: opcodes/intel80486early.opcodes
	$(BUILD_CPP) -E -I. opcodes/intel80486early.opcodes $@
dosboxxr/lib/cpu/core/intel80486early.c16.a16.decom.h: dosboxxr/lib/cpu/core/intel80486early.opcodes.p $(OPCC)$(EXEEXT)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intel80486early.opcodes.p -o $@ -m decompile -c 16 -a 16 -c32vars -gc1632
dosboxxr/lib/cpu/core/intel80486early.cxx.axx.decom.h: dosboxxr/lib/cpu/core/intel80486early.opcodes.p $(OPCC)$(EXEEXT)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intel80486early.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars -g
dosboxxr/lib/cpu/core/intel80486early.c32.a32.decom.h: dosboxxr/lib/cpu/core/intel80486early.opcodes.p $(OPCC)$(EXEEXT)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intel80486early.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars -gc1632

dosboxxr/lib/cpu/core/intelPentium.opcodes.p: opcodes/intelPentium.opcodes
	$(BUILD_CPP) -E -I. opcodes/intelPentium.opcodes $@
dosboxxr/lib/cpu/core/intelPentium.c16.a16.decom.h: dosboxxr/lib/cpu/core/intelPentium.opcodes.p $(OPCC)$(EXEEXT)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intelPentium.opcodes.p -o $@ -m decompile -c 16 -a 16 -c32vars -gc1632
dosboxxr/lib/cpu/core/intelPentium.cxx.axx.decom.h: dosboxxr/lib/cpu/core/intelPentium.opcodes.p $(OPCC)$(EXEEXT)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intelPentium.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars -g
dosboxxr/lib/cpu/core/intelPentium.c32.a32.decom.h: dosboxxr/lib/cpu/core/intelPentium.opcodes.p $(OPCC)$(EXEEXT)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intelPentium.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars -gc1632

dosboxxr/lib/cpu/core/intelPentiumPro.opcodes.p: opcodes/intelPentiumPro.opcodes
	$(BUILD_CPP) -E -I. opcodes/intelPentiumPro.opcodes $@
dosboxxr/lib/cpu/core/intelPentiumPro.c16.a16.decom.h: dosboxxr/lib/cpu/core/intelPentiumPro.opcodes.p $(OPCC)$(EXEEXT)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intelPentiumPro.opcodes.p -o $@ -m decompile -c 16 -a 16 -c32vars -gc1632
dosboxxr/lib/cpu/core/intelPentiumPro.cxx.axx.decom.h: dosboxxr/lib/cpu/core/intelPentiumPro.opcodes.p $(OPCC)$(EXEEXT)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intelPentiumPro.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars -g
dosboxxr/lib/cpu/core/intelPentiumPro.c32.a32.decom.h: dosboxxr/lib/cpu/core/intelPentiumPro.opcodes.p $(OPCC)$(EXEEXT)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intelPentiumPro.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars -gc1632

dosboxxr/lib/cpu/core/intelPentiumMMX.opcodes.p: opcodes/intelPentiumMMX.opcodes
	$(BUILD_CPP) -E -I. opcodes/intelPentiumMMX.opcodes $@
dosboxxr/lib/cpu/core/intelPentiumMMX.c16.a16.decom.h: dosboxxr/lib/cpu/core/intelPentiumMMX.opcodes.p $(OPCC)$(EXEEXT)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intelPentiumMMX.opcodes.p -o $@ -m decompile -c 16 -a 16 -c32vars -gc1632
dosboxxr/lib/cpu/core/intelPentiumMMX.cxx.axx.decom.h: dosboxxr/lib/cpu/core/intelPentiumMMX.opcodes.p $(OPCC)$(EXEEXT)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intelPentiumMMX.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars -g
dosboxxr/lib/cpu/core/intelPentiumMMX.c32.a32.decom.h: dosboxxr/lib/cpu/core/intelPentiumMMX.opcodes.p $(OPCC)$(EXEEXT)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intelPentiumMMX.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars -gc1632

dosboxxr/lib/cpu/core/intelPentiumProMMX.opcodes.p: opcodes/intelPentiumProMMX.opcodes
	$(BUILD_CPP) -E -I. opcodes/intelPentiumProMMX.opcodes $@
dosboxxr/lib/cpu/core/intelPentiumProMMX.c16.a16.decom.h: dosboxxr/lib/cpu/core/intelPentiumProMMX.opcodes.p $(OPCC)$(EXEEXT)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intelPentiumProMMX.opcodes.p -o $@ -m decompile -c 16 -a 16 -c32vars -gc1632
dosboxxr/lib/cpu/core/intelPentiumProMMX.cxx.axx.decom.h: dosboxxr/lib/cpu/core/intelPentiumProMMX.opcodes.p $(OPCC)$(EXEEXT)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intelPentiumProMMX.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars -g
dosboxxr/lib/cpu/core/intelPentiumProMMX.c32.a32.decom.h: dosboxxr/lib/cpu/core/intelPentiumProMMX.opcodes.p $(OPCC)$(EXEEXT)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intelPentiumProMMX.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars -gc1632

dosboxxr/lib/cpu/core/cyrix6x86MX.opcodes.p: opcodes/cyrix6x86MX.opcodes
	$(BUILD_CPP) -E -I. opcodes/cyrix6x86MX.opcodes $@
dosboxxr/lib/cpu/core/cyrix6x86MX.c16.a16.decom.h: dosboxxr/lib/cpu/core/cyrix6x86MX.opcodes.p $(OPCC)$(EXEEXT)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/cyrix6x86MX.opcodes.p -o $@ -m decompile -c 16 -a 16 -c32vars -gc1632
dosboxxr/lib/cpu/core/cyrix6x86MX.cxx.axx.decom.h: dosboxxr/lib/cpu/core/cyrix6x86MX.opcodes.p $(OPCC)$(EXEEXT)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/cyrix6x86MX.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars -g
dosboxxr/lib/cpu/core/cyrix6x86MX.c32.a32.decom.h: dosboxxr/lib/cpu/core/cyrix6x86MX.opcodes.p $(OPCC)$(EXEEXT)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/cyrix6x86MX.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars -gc1632

dosboxxr/lib/cpu/core/intelPentium2.opcodes.p: opcodes/intelPentium2.opcodes
	$(BUILD_CPP) -E -I. opcodes/intelPentium2.opcodes $@
dosboxxr/lib/cpu/core/intelPentium2.c16.a16.decom.h: dosboxxr/lib/cpu/core/intelPentium2.opcodes.p $(OPCC)$(EXEEXT)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intelPentium2.opcodes.p -o $@ -m decompile -c 16 -a 16 -c32vars -gc1632
dosboxxr/lib/cpu/core/intelPentium2.cxx.axx.decom.h: dosboxxr/lib/cpu/core/intelPentium2.opcodes.p $(OPCC)$(EXEEXT)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intelPentium2.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars -g
dosboxxr/lib/cpu/core/intelPentium2.c32.a32.decom.h: dosboxxr/lib/cpu/core/intelPentium2.opcodes.p $(OPCC)$(EXEEXT)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intelPentium2.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars -gc1632

dosboxxr/lib/cpu/core/intelPentium3.opcodes.p: opcodes/intelPentium3.opcodes
	$(BUILD_CPP) -E -I. opcodes/intelPentium3.opcodes $@
dosboxxr/lib/cpu/core/intelPentium3.c16.a16.decom.h: dosboxxr/lib/cpu/core/intelPentium3.opcodes.p $(OPCC)$(EXEEXT)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intelPentium3.opcodes.p -o $@ -m decompile -c 16 -a 16 -c32vars -gc1632
dosboxxr/lib/cpu/core/intelPentium3.cxx.axx.decom.h: dosboxxr/lib/cpu/core/intelPentium3.opcodes.p $(OPCC)$(EXEEXT)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intelPentium3.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars -g
dosboxxr/lib/cpu/core/intelPentium3.c32.a32.decom.h: dosboxxr/lib/cpu/core/intelPentium3.opcodes.p $(OPCC)$(EXEEXT)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intelPentium3.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars -gc1632

dosboxxr/lib/cpu/core/intelPentium4.opcodes.p: opcodes/intelPentium4.opcodes
	$(BUILD_CPP) -E -I. opcodes/intelPentium4.opcodes $@
dosboxxr/lib/cpu/core/intelPentium4.c16.a16.decom.h: dosboxxr/lib/cpu/core/intelPentium4.opcodes.p $(OPCC)$(EXEEXT)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intelPentium4.opcodes.p -o $@ -m decompile -c 16 -a 16 -c32vars -gc1632
dosboxxr/lib/cpu/core/intelPentium4.cxx.axx.decom.h: dosboxxr/lib/cpu/core/intelPentium4.opcodes.p $(OPCC)$(EXEEXT)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intelPentium4.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars -g
dosboxxr/lib/cpu/core/intelPentium4.c32.a32.decom.h: dosboxxr/lib/cpu/core/intelPentium4.opcodes.p $(OPCC)$(EXEEXT)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intelPentium4.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars -gc1632

dosboxxr/lib/cpu/core/intelPentium4Prescott.opcodes.p: opcodes/intelPentium4Prescott.opcodes
	$(BUILD_CPP) -E -I. opcodes/intelPentium4Prescott.opcodes $@
dosboxxr/lib/cpu/core/intelPentium4Prescott.c16.a16.decom.h: dosboxxr/lib/cpu/core/intelPentium4Prescott.opcodes.p $(OPCC)$(EXEEXT)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intelPentium4Prescott.opcodes.p -o $@ -m decompile -c 16 -a 16 -c32vars -gc1632
dosboxxr/lib/cpu/core/intelPentium4Prescott.cxx.axx.decom.h: dosboxxr/lib/cpu/core/intelPentium4Prescott.opcodes.p $(OPCC)$(EXEEXT)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intelPentium4Prescott.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars -g
dosboxxr/lib/cpu/core/intelPentium4Prescott.c32.a32.decom.h: dosboxxr/lib/cpu/core/intelPentium4Prescott.opcodes.p $(OPCC)$(EXEEXT)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intelPentium4Prescott.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars -gc1632

dosboxxr/lib/cpu/core/amdk6r2.opcodes.p: opcodes/amdk6r2.opcodes
	$(BUILD_CPP) -E -I. opcodes/amdk6r2.opcodes $@
dosboxxr/lib/cpu/core/amdk6r2.c16.a16.decom.h: dosboxxr/lib/cpu/core/amdk6r2.opcodes.p $(OPCC)$(EXEEXT)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/amdk6r2.opcodes.p -o $@ -m decompile -c 16 -a 16 -c32vars -gc1632
dosboxxr/lib/cpu/core/amdk6r2.cxx.axx.decom.h: dosboxxr/lib/cpu/core/amdk6r2.opcodes.p $(OPCC)$(EXEEXT)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/amdk6r2.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars -g
dosboxxr/lib/cpu/core/amdk6r2.c32.a32.decom.h: dosboxxr/lib/cpu/core/amdk6r2.opcodes.p $(OPCC)$(EXEEXT)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/amdk6r2.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars -gc1632

dosboxxr/lib/cpu/core/amdAthlon.opcodes.p: opcodes/amdAthlon.opcodes
	$(BUILD_CPP) -E -I. opcodes/amdAthlon.opcodes $@
dosboxxr/lib/cpu/core/amdAthlon.c16.a16.decom.h: dosboxxr/lib/cpu/core/amdAthlon.opcodes.p $(OPCC)$(EXEEXT)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/amdAthlon.opcodes.p -o $@ -m decompile -c 16 -a 16 -c32vars -gc1632
dosboxxr/lib/cpu/core/amdAthlon.cxx.axx.decom.h: dosboxxr/lib/cpu/core/amdAthlon.opcodes.p $(OPCC)$(EXEEXT)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/amdAthlon.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars -g
dosboxxr/lib/cpu/core/amdAthlon.c32.a32.decom.h: dosboxxr/lib/cpu/core/amdAthlon.opcodes.p $(OPCC)$(EXEEXT)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/amdAthlon.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars -gc1632

dosboxxr/lib/cpu/core/intelCore.opcodes.p: opcodes/intelCore.opcodes
	$(BUILD_CPP) -E -I. opcodes/intelCore.opcodes $@
dosboxxr/lib/cpu/core/intelCore.c16.a16.decom.h: dosboxxr/lib/cpu/core/intelCore.opcodes.p $(OPCC)$(EXEEXT)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intelCore.opcodes.p -o $@ -m decompile -c 16 -a 16 -c32vars -gc1632
dosboxxr/lib/cpu/core/intelCore.cxx.axx.decom.h: dosboxxr/lib/cpu/core/intelCore.opcodes.p $(OPCC)$(EXEEXT)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intelCore.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars -g
dosboxxr/lib/cpu/core/intelCore.c32.a32.decom.h: dosboxxr/lib/cpu/core/intelCore.opcodes.p $(OPCC)$(EXEEXT)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intelCore.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars -gc1632

dosboxxr/lib/cpu/core/everything.opcodes.p: opcodes/everything.opcodes
	$(BUILD_CPP) -E -I. opcodes/everything.opcodes $@
dosboxxr/lib/cpu/core/everything.c16.a16.decom.h: dosboxxr/lib/cpu/core/everything.opcodes.p $(OPCC)$(EXEEXT)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/everything.opcodes.p -o $@ -m decompile -c 16 -a 16 -c32vars -gc1632
dosboxxr/lib/cpu/core/everything.cxx.axx.decom.h: dosboxxr/lib/cpu/core/everything.opcodes.p $(OPCC)$(EXEEXT)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/everything.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars -g
dosboxxr/lib/cpu/core/everything.c32.a32.decom.h: dosboxxr/lib/cpu/core/everything.opcodes.p $(OPCC)$(EXEEXT)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/everything.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars -gc1632
dosboxxr/lib/cpu/core/everything.c16.a16.emu.h: dosboxxr/lib/cpu/core/everything.opcodes.p $(OPCC)$(EXEEXT)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/everything.opcodes.p -o $@ -m execute -c 16 -a 16 -c32vars -gc1632
dosboxxr/lib/cpu/core/everything.cxx.axx.emu.h: dosboxxr/lib/cpu/core/everything.opcodes.p $(OPCC)$(EXEEXT)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/everything.opcodes.p -o $@ -m execute -c 32 -a 32 -c32vars -g
dosboxxr/lib/cpu/core/everything.c32.a32.emu.h: dosboxxr/lib/cpu/core/everything.opcodes.p $(OPCC)$(EXEEXT)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/everything.opcodes.p -o $@ -m execute -c 32 -a 32 -c32vars -gc1632

# grrr how do I get automake to run opcc first before compiling other parts?
opcodes: opcodes-dec opcodes-emu

opcodes-dec: $(OPCC_DECOM_H)

opcodes-emu: $(OPCC_EMU_H)

