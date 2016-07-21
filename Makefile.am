
noinst_LIBRARIES = \
	dosboxxr/lib/hostcpudetect/libhostcpudetect.a \
	dosboxxr/lib/text/libtext.a \
	dosboxxr/lib/cpu/libcpu.a

dosboxxr_lib_hostcpudetect_libhostcpudetect_a_SOURCES = dosboxxr/lib/hostcpudetect/caps.cpp

dosboxxr_lib_cpu_libcpu_a_SOURCES = dosboxxr/lib/cpu/ipdec_strings.cpp

dosboxxr_lib_text_libtext_a_SOURCES = dosboxxr/lib/text/chomp.c

noinst_PROGRAMS = \
	empty \
	cpudetect \
	cpumodregrm \
	cpumemreftypes \
	cpudec86

empty_SOURCES = empty.cpp

cpudec86_SOURCES = cpudec86.cpp
cpudec86_CXXFLAGS = $(AM_CXXFLAGS)
cpudec86_DEPENDENCIES = \
	dosboxxr/lib/cpu/libcpu.a
cpudec86_LDADD = \
	dosboxxr/lib/cpu/libcpu.a

cpudetect_SOURCES = cpudetect.cpp
cpudetect_CXXFLAGS = $(AM_CXXFLAGS)
cpudetect_DEPENDENCIES = \
	dosboxxr/lib/hostcpudetect/libhostcpudetect.a \
	dosboxxr/lib/text/libtext.a
cpudetect_LDADD = \
	dosboxxr/lib/hostcpudetect/libhostcpudetect.a \
	dosboxxr/lib/text/libtext.a

cpumodregrm_SOURCES = cpumodregrm.cpp
cpumodregrm_CXXFLAGS = $(AM_CXXFLAGS)
cpumodregrm_DEPENDENCIES =
cpumodregrm_LDADD =

cpumemreftypes_SOURCES = cpumemreftypes.cpp
cpumemreftypes_CXXFLAGS = $(AM_CXXFLAGS)
cpumemreftypes_DEPENDENCIES =
cpumemreftypes_LDADD =

test1.bin: test1.asm
	yasm -o $@ -f bin $<

test-bins: test1.bin

