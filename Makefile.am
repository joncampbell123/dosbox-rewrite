
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
	tools/cpudec/cpudec86

empty_SOURCES = empty.cpp

tools_cpudec_cpudec86_SOURCES = \
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
	tools/cpudec/cpudec86_amdathlon_generic.cpp

tools_cpudec_cpudec86_CXXFLAGS = $(AM_CXXFLAGS)
tools_cpudec_cpudec86_DEPENDENCIES = \
	dosboxxr/lib/cpu/libcpu.a $(OPCC_H)
tools_cpudec_cpudec86_LDADD = \
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
	dosboxxr/lib/cpu/core/necv20.c16.a16.decom.h \
	\
	dosboxxr/lib/cpu/core/intel80186.c16.a16.decom.h \
	\
	dosboxxr/lib/cpu/core/intel80286.c16.a16.decom.h \
	\
	dosboxxr/lib/cpu/core/intel80386.c16.a16.decom.h \
	dosboxxr/lib/cpu/core/intel80386.cxx.axx.decom.h \
	dosboxxr/lib/cpu/core/intel80386.c32.a32.decom.h \
	\
	dosboxxr/lib/cpu/core/intel80386early.c16.a16.decom.h \
	dosboxxr/lib/cpu/core/intel80386early.cxx.axx.decom.h \
	dosboxxr/lib/cpu/core/intel80386early.c32.a32.decom.h \
	\
	dosboxxr/lib/cpu/core/intel80486.c16.a16.decom.h \
	dosboxxr/lib/cpu/core/intel80486.cxx.axx.decom.h \
	dosboxxr/lib/cpu/core/intel80486.c32.a32.decom.h \
	\
	dosboxxr/lib/cpu/core/intel80486early.c16.a16.decom.h \
	dosboxxr/lib/cpu/core/intel80486early.cxx.axx.decom.h \
	dosboxxr/lib/cpu/core/intel80486early.c32.a32.decom.h \
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
	dosboxxr/lib/cpu/core/intelPentiumMMX.c32.a32.decom.h \
	\
	dosboxxr/lib/cpu/core/intelPentiumProMMX.c16.a16.decom.h \
	dosboxxr/lib/cpu/core/intelPentiumProMMX.cxx.axx.decom.h \
	dosboxxr/lib/cpu/core/intelPentiumProMMX.c32.a32.decom.h \
	\
	dosboxxr/lib/cpu/core/cyrix6x86MX.c16.a16.decom.h \
	dosboxxr/lib/cpu/core/cyrix6x86MX.cxx.axx.decom.h \
	dosboxxr/lib/cpu/core/cyrix6x86MX.c32.a32.decom.h \
	\
	dosboxxr/lib/cpu/core/intelPentium2.c16.a16.decom.h \
	dosboxxr/lib/cpu/core/intelPentium2.cxx.axx.decom.h \
	dosboxxr/lib/cpu/core/intelPentium2.c32.a32.decom.h \
	\
	dosboxxr/lib/cpu/core/intelPentium3.c16.a16.decom.h \
	dosboxxr/lib/cpu/core/intelPentium3.cxx.axx.decom.h \
	dosboxxr/lib/cpu/core/intelPentium3.c32.a32.decom.h \
	\
	dosboxxr/lib/cpu/core/intelPentium4.c16.a16.decom.h \
	dosboxxr/lib/cpu/core/intelPentium4.cxx.axx.decom.h \
	dosboxxr/lib/cpu/core/intelPentium4.c32.a32.decom.h \
	\
	dosboxxr/lib/cpu/core/intelPentium4Prescott.c16.a16.decom.h \
	dosboxxr/lib/cpu/core/intelPentium4Prescott.cxx.axx.decom.h \
	dosboxxr/lib/cpu/core/intelPentium4Prescott.c32.a32.decom.h \
	\
	dosboxxr/lib/cpu/core/amdk6r2.c16.a16.decom.h \
	dosboxxr/lib/cpu/core/amdk6r2.cxx.axx.decom.h \
	dosboxxr/lib/cpu/core/amdk6r2.c32.a32.decom.h \
	\
	dosboxxr/lib/cpu/core/amdAthlon.c16.a16.decom.h \
	dosboxxr/lib/cpu/core/amdAthlon.cxx.axx.decom.h \
	dosboxxr/lib/cpu/core/amdAthlon.c32.a32.decom.h

test1.bin: test1.asm
	yasm -o $@ -f bin $<

test2.bin: test2.asm
	yasm -o $@ -f bin $<

test3.bin: test3.asm
	yasm -o $@ -f bin $<

test3e.bin: test3e.asm
	yasm -o $@ -f bin $<

test4.bin: test4.asm
	yasm -o $@ -f bin $<

test4e.bin: test4e.asm
	yasm -o $@ -f bin $<

test5.bin: test5.asm
	yasm -o $@ -f bin $<

test5pro.bin: test5pro.asm
	yasm -o $@ -f bin $<

test5mmx.bin: test5mmx.asm
	yasm -o $@ -f bin $<

testc6mx.bin: testc6mx.asm
	yasm -o $@ -f bin $<

testp2.bin: testp2.asm
	yasm -o $@ -f bin $<

testp3.bin: testp3.asm
	yasm -o $@ -f bin $<

testp4.bin: testp4.asm
	yasm -o $@ -f bin $<

testp4p.bin: testp4p.asm
	yasm -o $@ -f bin $<

testv20.bin: testv20.asm
	yasm -o $@ -f bin $<

test3dn1.bin: test3dn1.asm
	yasm -o $@ -f bin $<

test3dn2.bin: test3dn2.asm
	yasm -o $@ -f bin $<

test-bins: test1.bin test2.bin test3.bin test3e.bin test4.bin test4e.bin test5.bin test5pro.bin test5mmx.bin testp2.bin testp3.bin testp4.bin testp4p.bin testv20.bin testc6mx.bin test3dn1.bin test3dn2.bin

opcc$(EXEEXT): opcc.cpp
	$(BUILD_CXX) -o $@ $<

dosboxxr/lib/cpu/core/necv20.opcodes.p: opcodes/necv20.opcodes
	$(BUILD_CPP) -I. opcodes/necv20.opcodes -o $@
dosboxxr/lib/cpu/core/necv20.c16.a16.decom.h: dosboxxr/lib/cpu/core/necv20.opcodes.p opcc$(EXEEXT)
	./opcc$(EXEEXT) -i dosboxxr/lib/cpu/core/necv20.opcodes.p -o $@ -m decompile

dosboxxr/lib/cpu/core/intel8088.opcodes.p: opcodes/intel8088.opcodes
	$(BUILD_CPP) -I. opcodes/intel8088.opcodes -o $@
dosboxxr/lib/cpu/core/intel8088.c16.a16.decom.h: dosboxxr/lib/cpu/core/intel8088.opcodes.p opcc$(EXEEXT)
	./opcc$(EXEEXT) -i dosboxxr/lib/cpu/core/intel8088.opcodes.p -o $@ -m decompile

dosboxxr/lib/cpu/core/intel80186.opcodes.p: opcodes/intel80186.opcodes
	$(BUILD_CPP) -I. opcodes/intel80186.opcodes -o $@
dosboxxr/lib/cpu/core/intel80186.c16.a16.decom.h: dosboxxr/lib/cpu/core/intel80186.opcodes.p opcc$(EXEEXT)
	./opcc$(EXEEXT) -i dosboxxr/lib/cpu/core/intel80186.opcodes.p -o $@ -m decompile

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

dosboxxr/lib/cpu/core/intel80386early.opcodes.p: opcodes/intel80386early.opcodes
	$(BUILD_CPP) -I. opcodes/intel80386early.opcodes -o $@
dosboxxr/lib/cpu/core/intel80386early.c16.a16.decom.h: dosboxxr/lib/cpu/core/intel80386early.opcodes.p opcc$(EXEEXT)
	./opcc$(EXEEXT) -i dosboxxr/lib/cpu/core/intel80386early.opcodes.p -o $@ -m decompile -c 16 -a 16 -c32vars
dosboxxr/lib/cpu/core/intel80386early.cxx.axx.decom.h: dosboxxr/lib/cpu/core/intel80386early.opcodes.p opcc$(EXEEXT)
	./opcc$(EXEEXT) -i dosboxxr/lib/cpu/core/intel80386early.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars -g
dosboxxr/lib/cpu/core/intel80386early.c32.a32.decom.h: dosboxxr/lib/cpu/core/intel80386early.opcodes.p opcc$(EXEEXT)
	./opcc$(EXEEXT) -i dosboxxr/lib/cpu/core/intel80386early.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars

dosboxxr/lib/cpu/core/intel80486.opcodes.p: opcodes/intel80486.opcodes
	$(BUILD_CPP) -I. opcodes/intel80486.opcodes -o $@
dosboxxr/lib/cpu/core/intel80486.c16.a16.decom.h: dosboxxr/lib/cpu/core/intel80486.opcodes.p opcc$(EXEEXT)
	./opcc$(EXEEXT) -i dosboxxr/lib/cpu/core/intel80486.opcodes.p -o $@ -m decompile -c 16 -a 16 -c32vars
dosboxxr/lib/cpu/core/intel80486.cxx.axx.decom.h: dosboxxr/lib/cpu/core/intel80486.opcodes.p opcc$(EXEEXT)
	./opcc$(EXEEXT) -i dosboxxr/lib/cpu/core/intel80486.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars -g
dosboxxr/lib/cpu/core/intel80486.c32.a32.decom.h: dosboxxr/lib/cpu/core/intel80486.opcodes.p opcc$(EXEEXT)
	./opcc$(EXEEXT) -i dosboxxr/lib/cpu/core/intel80486.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars

dosboxxr/lib/cpu/core/intel80486early.opcodes.p: opcodes/intel80486early.opcodes
	$(BUILD_CPP) -I. opcodes/intel80486early.opcodes -o $@
dosboxxr/lib/cpu/core/intel80486early.c16.a16.decom.h: dosboxxr/lib/cpu/core/intel80486early.opcodes.p opcc$(EXEEXT)
	./opcc$(EXEEXT) -i dosboxxr/lib/cpu/core/intel80486early.opcodes.p -o $@ -m decompile -c 16 -a 16 -c32vars
dosboxxr/lib/cpu/core/intel80486early.cxx.axx.decom.h: dosboxxr/lib/cpu/core/intel80486early.opcodes.p opcc$(EXEEXT)
	./opcc$(EXEEXT) -i dosboxxr/lib/cpu/core/intel80486early.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars -g
dosboxxr/lib/cpu/core/intel80486early.c32.a32.decom.h: dosboxxr/lib/cpu/core/intel80486early.opcodes.p opcc$(EXEEXT)
	./opcc$(EXEEXT) -i dosboxxr/lib/cpu/core/intel80486early.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars

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

dosboxxr/lib/cpu/core/intelPentiumProMMX.opcodes.p: opcodes/intelPentiumProMMX.opcodes
	$(BUILD_CPP) -I. opcodes/intelPentiumProMMX.opcodes -o $@
dosboxxr/lib/cpu/core/intelPentiumProMMX.c16.a16.decom.h: dosboxxr/lib/cpu/core/intelPentiumProMMX.opcodes.p opcc$(EXEEXT)
	./opcc$(EXEEXT) -i dosboxxr/lib/cpu/core/intelPentiumProMMX.opcodes.p -o $@ -m decompile -c 16 -a 16 -c32vars
dosboxxr/lib/cpu/core/intelPentiumProMMX.cxx.axx.decom.h: dosboxxr/lib/cpu/core/intelPentiumProMMX.opcodes.p opcc$(EXEEXT)
	./opcc$(EXEEXT) -i dosboxxr/lib/cpu/core/intelPentiumProMMX.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars -g
dosboxxr/lib/cpu/core/intelPentiumProMMX.c32.a32.decom.h: dosboxxr/lib/cpu/core/intelPentiumProMMX.opcodes.p opcc$(EXEEXT)
	./opcc$(EXEEXT) -i dosboxxr/lib/cpu/core/intelPentiumProMMX.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars

dosboxxr/lib/cpu/core/cyrix6x86MX.opcodes.p: opcodes/cyrix6x86MX.opcodes
	$(BUILD_CPP) -I. opcodes/cyrix6x86MX.opcodes -o $@
dosboxxr/lib/cpu/core/cyrix6x86MX.c16.a16.decom.h: dosboxxr/lib/cpu/core/cyrix6x86MX.opcodes.p opcc$(EXEEXT)
	./opcc$(EXEEXT) -i dosboxxr/lib/cpu/core/cyrix6x86MX.opcodes.p -o $@ -m decompile -c 16 -a 16 -c32vars
dosboxxr/lib/cpu/core/cyrix6x86MX.cxx.axx.decom.h: dosboxxr/lib/cpu/core/cyrix6x86MX.opcodes.p opcc$(EXEEXT)
	./opcc$(EXEEXT) -i dosboxxr/lib/cpu/core/cyrix6x86MX.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars -g
dosboxxr/lib/cpu/core/cyrix6x86MX.c32.a32.decom.h: dosboxxr/lib/cpu/core/cyrix6x86MX.opcodes.p opcc$(EXEEXT)
	./opcc$(EXEEXT) -i dosboxxr/lib/cpu/core/cyrix6x86MX.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars

dosboxxr/lib/cpu/core/intelPentium2.opcodes.p: opcodes/intelPentium2.opcodes
	$(BUILD_CPP) -I. opcodes/intelPentium2.opcodes -o $@
dosboxxr/lib/cpu/core/intelPentium2.c16.a16.decom.h: dosboxxr/lib/cpu/core/intelPentium2.opcodes.p opcc$(EXEEXT)
	./opcc$(EXEEXT) -i dosboxxr/lib/cpu/core/intelPentium2.opcodes.p -o $@ -m decompile -c 16 -a 16 -c32vars
dosboxxr/lib/cpu/core/intelPentium2.cxx.axx.decom.h: dosboxxr/lib/cpu/core/intelPentium2.opcodes.p opcc$(EXEEXT)
	./opcc$(EXEEXT) -i dosboxxr/lib/cpu/core/intelPentium2.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars -g
dosboxxr/lib/cpu/core/intelPentium2.c32.a32.decom.h: dosboxxr/lib/cpu/core/intelPentium2.opcodes.p opcc$(EXEEXT)
	./opcc$(EXEEXT) -i dosboxxr/lib/cpu/core/intelPentium2.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars

dosboxxr/lib/cpu/core/intelPentium3.opcodes.p: opcodes/intelPentium3.opcodes
	$(BUILD_CPP) -I. opcodes/intelPentium3.opcodes -o $@
dosboxxr/lib/cpu/core/intelPentium3.c16.a16.decom.h: dosboxxr/lib/cpu/core/intelPentium3.opcodes.p opcc$(EXEEXT)
	./opcc$(EXEEXT) -i dosboxxr/lib/cpu/core/intelPentium3.opcodes.p -o $@ -m decompile -c 16 -a 16 -c32vars
dosboxxr/lib/cpu/core/intelPentium3.cxx.axx.decom.h: dosboxxr/lib/cpu/core/intelPentium3.opcodes.p opcc$(EXEEXT)
	./opcc$(EXEEXT) -i dosboxxr/lib/cpu/core/intelPentium3.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars -g
dosboxxr/lib/cpu/core/intelPentium3.c32.a32.decom.h: dosboxxr/lib/cpu/core/intelPentium3.opcodes.p opcc$(EXEEXT)
	./opcc$(EXEEXT) -i dosboxxr/lib/cpu/core/intelPentium3.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars

dosboxxr/lib/cpu/core/intelPentium4.opcodes.p: opcodes/intelPentium4.opcodes
	$(BUILD_CPP) -I. opcodes/intelPentium4.opcodes -o $@
dosboxxr/lib/cpu/core/intelPentium4.c16.a16.decom.h: dosboxxr/lib/cpu/core/intelPentium4.opcodes.p opcc$(EXEEXT)
	./opcc$(EXEEXT) -i dosboxxr/lib/cpu/core/intelPentium4.opcodes.p -o $@ -m decompile -c 16 -a 16 -c32vars
dosboxxr/lib/cpu/core/intelPentium4.cxx.axx.decom.h: dosboxxr/lib/cpu/core/intelPentium4.opcodes.p opcc$(EXEEXT)
	./opcc$(EXEEXT) -i dosboxxr/lib/cpu/core/intelPentium4.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars -g
dosboxxr/lib/cpu/core/intelPentium4.c32.a32.decom.h: dosboxxr/lib/cpu/core/intelPentium4.opcodes.p opcc$(EXEEXT)
	./opcc$(EXEEXT) -i dosboxxr/lib/cpu/core/intelPentium4.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars

dosboxxr/lib/cpu/core/intelPentium4Prescott.opcodes.p: opcodes/intelPentium4Prescott.opcodes
	$(BUILD_CPP) -I. opcodes/intelPentium4Prescott.opcodes -o $@
dosboxxr/lib/cpu/core/intelPentium4Prescott.c16.a16.decom.h: dosboxxr/lib/cpu/core/intelPentium4Prescott.opcodes.p opcc$(EXEEXT)
	./opcc$(EXEEXT) -i dosboxxr/lib/cpu/core/intelPentium4Prescott.opcodes.p -o $@ -m decompile -c 16 -a 16 -c32vars
dosboxxr/lib/cpu/core/intelPentium4Prescott.cxx.axx.decom.h: dosboxxr/lib/cpu/core/intelPentium4Prescott.opcodes.p opcc$(EXEEXT)
	./opcc$(EXEEXT) -i dosboxxr/lib/cpu/core/intelPentium4Prescott.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars -g
dosboxxr/lib/cpu/core/intelPentium4Prescott.c32.a32.decom.h: dosboxxr/lib/cpu/core/intelPentium4Prescott.opcodes.p opcc$(EXEEXT)
	./opcc$(EXEEXT) -i dosboxxr/lib/cpu/core/intelPentium4Prescott.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars

dosboxxr/lib/cpu/core/amdk6r2.opcodes.p: opcodes/amdk6r2.opcodes
	$(BUILD_CPP) -I. opcodes/amdk6r2.opcodes -o $@
dosboxxr/lib/cpu/core/amdk6r2.c16.a16.decom.h: dosboxxr/lib/cpu/core/amdk6r2.opcodes.p opcc$(EXEEXT)
	./opcc$(EXEEXT) -i dosboxxr/lib/cpu/core/amdk6r2.opcodes.p -o $@ -m decompile -c 16 -a 16 -c32vars
dosboxxr/lib/cpu/core/amdk6r2.cxx.axx.decom.h: dosboxxr/lib/cpu/core/amdk6r2.opcodes.p opcc$(EXEEXT)
	./opcc$(EXEEXT) -i dosboxxr/lib/cpu/core/amdk6r2.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars -g
dosboxxr/lib/cpu/core/amdk6r2.c32.a32.decom.h: dosboxxr/lib/cpu/core/amdk6r2.opcodes.p opcc$(EXEEXT)
	./opcc$(EXEEXT) -i dosboxxr/lib/cpu/core/amdk6r2.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars

dosboxxr/lib/cpu/core/amdAthlon.opcodes.p: opcodes/amdAthlon.opcodes
	$(BUILD_CPP) -I. opcodes/amdAthlon.opcodes -o $@
dosboxxr/lib/cpu/core/amdAthlon.c16.a16.decom.h: dosboxxr/lib/cpu/core/amdAthlon.opcodes.p opcc$(EXEEXT)
	./opcc$(EXEEXT) -i dosboxxr/lib/cpu/core/amdAthlon.opcodes.p -o $@ -m decompile -c 16 -a 16 -c32vars
dosboxxr/lib/cpu/core/amdAthlon.cxx.axx.decom.h: dosboxxr/lib/cpu/core/amdAthlon.opcodes.p opcc$(EXEEXT)
	./opcc$(EXEEXT) -i dosboxxr/lib/cpu/core/amdAthlon.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars -g
dosboxxr/lib/cpu/core/amdAthlon.c32.a32.decom.h: dosboxxr/lib/cpu/core/amdAthlon.opcodes.p opcc$(EXEEXT)
	./opcc$(EXEEXT) -i dosboxxr/lib/cpu/core/amdAthlon.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars

# grrr how do I get automake to run opcc first before compiling other parts?
opcodes: $(OPCC_H)

