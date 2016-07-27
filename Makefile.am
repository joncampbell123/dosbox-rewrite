
noinst_LIBRARIES = \
	dosboxxr/lib/hostcpudetect/libhostcpudetect.a \
	dosboxxr/lib/text/libtext.a \
	dosboxxr/lib/cpu/libcpu.a

dosboxxr_lib_hostcpudetect_libhostcpudetect_a_SOURCES = dosboxxr/lib/hostcpudetect/caps.cpp

dosboxxr_lib_cpu_libcpu_a_SOURCES = dosboxxr/lib/cpu/ipdec_strings.cpp

dosboxxr_lib_text_libtext_a_SOURCES = dosboxxr/lib/text/chomp.c

noinst_PROGRAMS = \
	opcc$(EXEEXT) \
	empty \
	cpudetect \
	cpumodregrm \
	cpumemreftypes \
	cpudec86

empty_SOURCES = empty.cpp

cpudec86_SOURCES = cpudec86.cpp
cpudec86_CXXFLAGS = $(AM_CXXFLAGS)
cpudec86_DEPENDENCIES = \
	dosboxxr/lib/cpu/libcpu.a $(OPCC_H)
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

OPCC_H = \
	dosboxxr/lib/cpu/core/intel8088.c16.a16.decom.h \
	\
	dosboxxr/lib/cpu/core/intel80286.c16.a16.decom.h \
	\
	dosboxxr/lib/cpu/core/intel80386.c16.a16.decom.h \
	dosboxxr/lib/cpu/core/intel80386.cxx.axx.decom.h \
	dosboxxr/lib/cpu/core/intel80386.c32.a32.decom.h \
	\
	dosboxxr/lib/cpu/core/intel80486.c16.a16.decom.h \
	dosboxxr/lib/cpu/core/intel80486.cxx.axx.decom.h \
	dosboxxr/lib/cpu/core/intel80486.c32.a32.decom.h \
	\
	dosboxxr/lib/cpu/core/intelPentium.c16.a16.decom.h \
	dosboxxr/lib/cpu/core/intelPentium.cxx.axx.decom.h \
	dosboxxr/lib/cpu/core/intelPentium.c32.a32.decom.h \
	\
	dosboxxr/lib/cpu/core/intelPentiumPro.c16.a16.decom.h \
	dosboxxr/lib/cpu/core/intelPentiumPro.cxx.axx.decom.h \
	dosboxxr/lib/cpu/core/intelPentiumPro.c32.a32.decom.h \
	\
	dosboxxr/lib/cpu/core/intelPentiumMMX.c16.a16.decom.h \
	dosboxxr/lib/cpu/core/intelPentiumMMX.cxx.axx.decom.h \
	dosboxxr/lib/cpu/core/intelPentiumMMX.c32.a32.decom.h

test1.bin: test1.asm
	yasm -o $@ -f bin $<

test2.bin: test2.asm
	yasm -o $@ -f bin $<

test3.bin: test3.asm
	yasm -o $@ -f bin $<

test4.bin: test4.asm
	yasm -o $@ -f bin $<

test5.bin: test5.asm
	yasm -o $@ -f bin $<

test5pro.bin: test5pro.asm
	yasm -o $@ -f bin $<

test5mmx.bin: test5mmx.asm
	yasm -o $@ -f bin $<

test-bins: test1.bin test2.bin test3.bin test4.bin test5.bin test5pro.bin test5mmx.bin

opcc$(EXEEXT): opcc.cpp
	$(BUILD_CXX) -o $@ $<

dosboxxr/lib/cpu/core/intel8088.opcodes.p: opcodes/intel8088.opcodes
	$(BUILD_CPP) -I. opcodes/intel8088.opcodes -o $@
dosboxxr/lib/cpu/core/intel8088.c16.a16.decom.h: dosboxxr/lib/cpu/core/intel8088.opcodes.p opcc$(EXEEXT)
	./opcc$(EXEEXT) -i dosboxxr/lib/cpu/core/intel8088.opcodes.p -o $@ -m decompile

dosboxxr/lib/cpu/core/intel80286.opcodes.p: opcodes/intel80286.opcodes
	$(BUILD_CPP) -I. opcodes/intel80286.opcodes -o $@
dosboxxr/lib/cpu/core/intel80286.c16.a16.decom.h: dosboxxr/lib/cpu/core/intel80286.opcodes.p opcc$(EXEEXT)
	./opcc$(EXEEXT) -i dosboxxr/lib/cpu/core/intel80286.opcodes.p -o $@ -m decompile

dosboxxr/lib/cpu/core/intel80386.opcodes.p: opcodes/intel80386.opcodes
	$(BUILD_CPP) -I. opcodes/intel80386.opcodes -o $@
dosboxxr/lib/cpu/core/intel80386.c16.a16.decom.h: dosboxxr/lib/cpu/core/intel80386.opcodes.p opcc$(EXEEXT)
	./opcc$(EXEEXT) -i dosboxxr/lib/cpu/core/intel80386.opcodes.p -o $@ -m decompile -c 16 -a 16 -c32vars
dosboxxr/lib/cpu/core/intel80386.cxx.axx.decom.h: dosboxxr/lib/cpu/core/intel80386.opcodes.p opcc$(EXEEXT)
	./opcc$(EXEEXT) -i dosboxxr/lib/cpu/core/intel80386.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars -g
dosboxxr/lib/cpu/core/intel80386.c32.a32.decom.h: dosboxxr/lib/cpu/core/intel80386.opcodes.p opcc$(EXEEXT)
	./opcc$(EXEEXT) -i dosboxxr/lib/cpu/core/intel80386.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars

dosboxxr/lib/cpu/core/intel80486.opcodes.p: opcodes/intel80486.opcodes
	$(BUILD_CPP) -I. opcodes/intel80486.opcodes -o $@
dosboxxr/lib/cpu/core/intel80486.c16.a16.decom.h: dosboxxr/lib/cpu/core/intel80486.opcodes.p opcc$(EXEEXT)
	./opcc$(EXEEXT) -i dosboxxr/lib/cpu/core/intel80486.opcodes.p -o $@ -m decompile -c 16 -a 16 -c32vars
dosboxxr/lib/cpu/core/intel80486.cxx.axx.decom.h: dosboxxr/lib/cpu/core/intel80486.opcodes.p opcc$(EXEEXT)
	./opcc$(EXEEXT) -i dosboxxr/lib/cpu/core/intel80486.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars -g
dosboxxr/lib/cpu/core/intel80486.c32.a32.decom.h: dosboxxr/lib/cpu/core/intel80486.opcodes.p opcc$(EXEEXT)
	./opcc$(EXEEXT) -i dosboxxr/lib/cpu/core/intel80486.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars

dosboxxr/lib/cpu/core/intelPentium.opcodes.p: opcodes/intelPentium.opcodes
	$(BUILD_CPP) -I. opcodes/intelPentium.opcodes -o $@
dosboxxr/lib/cpu/core/intelPentium.c16.a16.decom.h: dosboxxr/lib/cpu/core/intelPentium.opcodes.p opcc$(EXEEXT)
	./opcc$(EXEEXT) -i dosboxxr/lib/cpu/core/intelPentium.opcodes.p -o $@ -m decompile -c 16 -a 16 -c32vars
dosboxxr/lib/cpu/core/intelPentium.cxx.axx.decom.h: dosboxxr/lib/cpu/core/intelPentium.opcodes.p opcc$(EXEEXT)
	./opcc$(EXEEXT) -i dosboxxr/lib/cpu/core/intelPentium.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars -g
dosboxxr/lib/cpu/core/intelPentium.c32.a32.decom.h: dosboxxr/lib/cpu/core/intelPentium.opcodes.p opcc$(EXEEXT)
	./opcc$(EXEEXT) -i dosboxxr/lib/cpu/core/intelPentium.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars

dosboxxr/lib/cpu/core/intelPentiumPro.opcodes.p: opcodes/intelPentiumPro.opcodes
	$(BUILD_CPP) -I. opcodes/intelPentiumPro.opcodes -o $@
dosboxxr/lib/cpu/core/intelPentiumPro.c16.a16.decom.h: dosboxxr/lib/cpu/core/intelPentiumPro.opcodes.p opcc$(EXEEXT)
	./opcc$(EXEEXT) -i dosboxxr/lib/cpu/core/intelPentiumPro.opcodes.p -o $@ -m decompile -c 16 -a 16 -c32vars
dosboxxr/lib/cpu/core/intelPentiumPro.cxx.axx.decom.h: dosboxxr/lib/cpu/core/intelPentiumPro.opcodes.p opcc$(EXEEXT)
	./opcc$(EXEEXT) -i dosboxxr/lib/cpu/core/intelPentiumPro.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars -g
dosboxxr/lib/cpu/core/intelPentiumPro.c32.a32.decom.h: dosboxxr/lib/cpu/core/intelPentiumPro.opcodes.p opcc$(EXEEXT)
	./opcc$(EXEEXT) -i dosboxxr/lib/cpu/core/intelPentiumPro.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars

dosboxxr/lib/cpu/core/intelPentiumMMX.opcodes.p: opcodes/intelPentiumMMX.opcodes
	$(BUILD_CPP) -I. opcodes/intelPentiumMMX.opcodes -o $@
dosboxxr/lib/cpu/core/intelPentiumMMX.c16.a16.decom.h: dosboxxr/lib/cpu/core/intelPentiumMMX.opcodes.p opcc$(EXEEXT)
	./opcc$(EXEEXT) -i dosboxxr/lib/cpu/core/intelPentiumMMX.opcodes.p -o $@ -m decompile -c 16 -a 16 -c32vars
dosboxxr/lib/cpu/core/intelPentiumMMX.cxx.axx.decom.h: dosboxxr/lib/cpu/core/intelPentiumMMX.opcodes.p opcc$(EXEEXT)
	./opcc$(EXEEXT) -i dosboxxr/lib/cpu/core/intelPentiumMMX.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars -g
dosboxxr/lib/cpu/core/intelPentiumMMX.c32.a32.decom.h: dosboxxr/lib/cpu/core/intelPentiumMMX.opcodes.p opcc$(EXEEXT)
	./opcc$(EXEEXT) -i dosboxxr/lib/cpu/core/intelPentiumMMX.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars

# grrr how do I get automake to run opcc first before compiling other parts?
opcodes: $(OPCC_H)

