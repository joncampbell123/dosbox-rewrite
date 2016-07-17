
noinst_LIBRARIES = dosboxxr/lib/hostcpudetect/libhostcpudetect.a

dosboxxr_lib_hostcpudetect_libhostcpudetect_a_SOURCES = dosboxxr/lib/hostcpudetect/caps.cpp

noinst_PROGRAMS = empty cpudetect

empty_SOURCES = empty.cpp

cpudetect_SOURCES = cpudetect.cpp
cpudetect_CXXFLAGS = $(AM_CXXFLAGS)
cpudetect_DEPENDENCIES = dosboxxr/lib/hostcpudetect/libhostcpudetect.a
cpudetect_LDADD = dosboxxr/lib/hostcpudetect/libhostcpudetect.a

