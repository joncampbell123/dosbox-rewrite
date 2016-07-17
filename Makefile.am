
noinst_LIBRARIES = \
	dosboxxr/lib/hostcpudetect/libhostcpudetect.a \
	dosboxxr/lib/text/libtext.a

dosboxxr_lib_hostcpudetect_libhostcpudetect_a_SOURCES = dosboxxr/lib/hostcpudetect/caps.cpp

dosboxxr_lib_text_libtext_a_SOURCES = dosboxxr/lib/text/chomp.c

noinst_PROGRAMS = empty cpudetect

empty_SOURCES = empty.cpp

cpudetect_SOURCES = cpudetect.cpp
cpudetect_CXXFLAGS = $(AM_CXXFLAGS)
cpudetect_DEPENDENCIES = \
	dosboxxr/lib/hostcpudetect/libhostcpudetect.a \
	dosboxxr/lib/text/libtext.a
cpudetect_LDADD = \
	dosboxxr/lib/hostcpudetect/libhostcpudetect.a \
	dosboxxr/lib/text/libtext.a

