
noinst_LIBRARIES = \
	dosboxxr/lib/hostcpudetect/libhostcpudetect.a \
	dosboxxr/lib/text/libtext.a \
	dosboxxr/lib/cpu/libcpu.a

dosboxxr_lib_hostcpudetect_libhostcpudetect_a_SOURCES = dosboxxr/lib/hostcpudetect/caps.cpp

dosboxxr_lib_cpu_libcpu_a_SOURCES = dosboxxr/lib/cpu/ipdec_strings.cpp

dosboxxr_lib_text_libtext_a_SOURCES = dosboxxr/lib/text/chomp.c

OPCC=tools/opcc/opcc$(EXEEXT)

noinst_PROGRAMS = \
	tools/opcc/opcc$(EXEEXT) \
	test/cpumodregrm \
	test/cpumemreftypes \
	tools/cpudec/cpudec86 \
	tools/cpudetect/cpudetect

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

tools_cpudec_cpudec86_CXXFLAGS = $(AM_CXXFLAGS) -Itools/cpudec
tools_cpudec_cpudec86_DEPENDENCIES = \
	dosboxxr/lib/cpu/libcpu.a $(OPCC_H)
tools_cpudec_cpudec86_LDADD = \
	dosboxxr/lib/cpu/libcpu.a

tools_cpudetect_cpudetect_SOURCES = tools/cpudetect/cpudetect.cpp
tools_cpudetect_cpudetect_CXXFLAGS = $(AM_CXXFLAGS)
tools_cpudetect_cpudetect_DEPENDENCIES = \
	dosboxxr/lib/hostcpudetect/libhostcpudetect.a \
	dosboxxr/lib/text/libtext.a
tools_cpudetect_cpudetect_LDADD = \
	dosboxxr/lib/hostcpudetect/libhostcpudetect.a \
	dosboxxr/lib/text/libtext.a

test_cpumodregrm_SOURCES = test/cpumodregrm.cpp
test_cpumodregrm_CXXFLAGS = $(AM_CXXFLAGS)
test_cpumodregrm_DEPENDENCIES =
test_cpumodregrm_LDADD =

test_cpumemreftypes_SOURCES = test/cpumemreftypes.cpp
test_cpumemreftypes_CXXFLAGS = $(AM_CXXFLAGS)
test_cpumemreftypes_DEPENDENCIES =
test_cpumemreftypes_LDADD =

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

test-bins: testasm/test1.bin testasm/test2.bin testasm/test3.bin testasm/test3e.bin testasm/test4.bin testasm/test4e.bin testasm/test5.bin testasm/test5pro.bin testasm/test5mmx.bin testasm/testp2.bin testasm/testp3.bin testasm/testp4.bin testasm/testp4p.bin testasm/testv20.bin testasm/testc6mx.bin testasm/test3dn1.bin testasm/test3dn2.bin

tools/opcc/opcc$(EXEEXT): tools/opcc/opcc.cpp
	$(BUILD_CXX) -o $@ $<

dosboxxr/lib/cpu/core/necv20.opcodes.p: opcodes/necv20.opcodes
	$(BUILD_CPP) -I. opcodes/necv20.opcodes -o $@
dosboxxr/lib/cpu/core/necv20.c16.a16.decom.h: dosboxxr/lib/cpu/core/necv20.opcodes.p $(OPCC)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/necv20.opcodes.p -o $@ -m decompile

dosboxxr/lib/cpu/core/intel8088.opcodes.p: opcodes/intel8088.opcodes
	$(BUILD_CPP) -I. opcodes/intel8088.opcodes -o $@
dosboxxr/lib/cpu/core/intel8088.c16.a16.decom.h: dosboxxr/lib/cpu/core/intel8088.opcodes.p $(OPCC)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intel8088.opcodes.p -o $@ -m decompile

dosboxxr/lib/cpu/core/intel80186.opcodes.p: opcodes/intel80186.opcodes
	$(BUILD_CPP) -I. opcodes/intel80186.opcodes -o $@
dosboxxr/lib/cpu/core/intel80186.c16.a16.decom.h: dosboxxr/lib/cpu/core/intel80186.opcodes.p $(OPCC)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intel80186.opcodes.p -o $@ -m decompile

dosboxxr/lib/cpu/core/intel80286.opcodes.p: opcodes/intel80286.opcodes
	$(BUILD_CPP) -I. opcodes/intel80286.opcodes -o $@
dosboxxr/lib/cpu/core/intel80286.c16.a16.decom.h: dosboxxr/lib/cpu/core/intel80286.opcodes.p $(OPCC)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intel80286.opcodes.p -o $@ -m decompile

dosboxxr/lib/cpu/core/intel80386.opcodes.p: opcodes/intel80386.opcodes
	$(BUILD_CPP) -I. opcodes/intel80386.opcodes -o $@
dosboxxr/lib/cpu/core/intel80386.c16.a16.decom.h: dosboxxr/lib/cpu/core/intel80386.opcodes.p $(OPCC)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intel80386.opcodes.p -o $@ -m decompile -c 16 -a 16 -c32vars
dosboxxr/lib/cpu/core/intel80386.cxx.axx.decom.h: dosboxxr/lib/cpu/core/intel80386.opcodes.p $(OPCC)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intel80386.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars -g
dosboxxr/lib/cpu/core/intel80386.c32.a32.decom.h: dosboxxr/lib/cpu/core/intel80386.opcodes.p $(OPCC)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intel80386.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars

dosboxxr/lib/cpu/core/intel80386early.opcodes.p: opcodes/intel80386early.opcodes
	$(BUILD_CPP) -I. opcodes/intel80386early.opcodes -o $@
dosboxxr/lib/cpu/core/intel80386early.c16.a16.decom.h: dosboxxr/lib/cpu/core/intel80386early.opcodes.p $(OPCC)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intel80386early.opcodes.p -o $@ -m decompile -c 16 -a 16 -c32vars
dosboxxr/lib/cpu/core/intel80386early.cxx.axx.decom.h: dosboxxr/lib/cpu/core/intel80386early.opcodes.p $(OPCC)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intel80386early.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars -g
dosboxxr/lib/cpu/core/intel80386early.c32.a32.decom.h: dosboxxr/lib/cpu/core/intel80386early.opcodes.p $(OPCC)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intel80386early.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars

dosboxxr/lib/cpu/core/intel80486.opcodes.p: opcodes/intel80486.opcodes
	$(BUILD_CPP) -I. opcodes/intel80486.opcodes -o $@
dosboxxr/lib/cpu/core/intel80486.c16.a16.decom.h: dosboxxr/lib/cpu/core/intel80486.opcodes.p $(OPCC)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intel80486.opcodes.p -o $@ -m decompile -c 16 -a 16 -c32vars
dosboxxr/lib/cpu/core/intel80486.cxx.axx.decom.h: dosboxxr/lib/cpu/core/intel80486.opcodes.p $(OPCC)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intel80486.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars -g
dosboxxr/lib/cpu/core/intel80486.c32.a32.decom.h: dosboxxr/lib/cpu/core/intel80486.opcodes.p $(OPCC)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intel80486.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars

dosboxxr/lib/cpu/core/intel80486early.opcodes.p: opcodes/intel80486early.opcodes
	$(BUILD_CPP) -I. opcodes/intel80486early.opcodes -o $@
dosboxxr/lib/cpu/core/intel80486early.c16.a16.decom.h: dosboxxr/lib/cpu/core/intel80486early.opcodes.p $(OPCC)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intel80486early.opcodes.p -o $@ -m decompile -c 16 -a 16 -c32vars
dosboxxr/lib/cpu/core/intel80486early.cxx.axx.decom.h: dosboxxr/lib/cpu/core/intel80486early.opcodes.p $(OPCC)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intel80486early.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars -g
dosboxxr/lib/cpu/core/intel80486early.c32.a32.decom.h: dosboxxr/lib/cpu/core/intel80486early.opcodes.p $(OPCC)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intel80486early.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars

dosboxxr/lib/cpu/core/intelPentium.opcodes.p: opcodes/intelPentium.opcodes
	$(BUILD_CPP) -I. opcodes/intelPentium.opcodes -o $@
dosboxxr/lib/cpu/core/intelPentium.c16.a16.decom.h: dosboxxr/lib/cpu/core/intelPentium.opcodes.p $(OPCC)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intelPentium.opcodes.p -o $@ -m decompile -c 16 -a 16 -c32vars
dosboxxr/lib/cpu/core/intelPentium.cxx.axx.decom.h: dosboxxr/lib/cpu/core/intelPentium.opcodes.p $(OPCC)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intelPentium.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars -g
dosboxxr/lib/cpu/core/intelPentium.c32.a32.decom.h: dosboxxr/lib/cpu/core/intelPentium.opcodes.p $(OPCC)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intelPentium.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars

dosboxxr/lib/cpu/core/intelPentiumPro.opcodes.p: opcodes/intelPentiumPro.opcodes
	$(BUILD_CPP) -I. opcodes/intelPentiumPro.opcodes -o $@
dosboxxr/lib/cpu/core/intelPentiumPro.c16.a16.decom.h: dosboxxr/lib/cpu/core/intelPentiumPro.opcodes.p $(OPCC)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intelPentiumPro.opcodes.p -o $@ -m decompile -c 16 -a 16 -c32vars
dosboxxr/lib/cpu/core/intelPentiumPro.cxx.axx.decom.h: dosboxxr/lib/cpu/core/intelPentiumPro.opcodes.p $(OPCC)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intelPentiumPro.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars -g
dosboxxr/lib/cpu/core/intelPentiumPro.c32.a32.decom.h: dosboxxr/lib/cpu/core/intelPentiumPro.opcodes.p $(OPCC)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intelPentiumPro.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars

dosboxxr/lib/cpu/core/intelPentiumMMX.opcodes.p: opcodes/intelPentiumMMX.opcodes
	$(BUILD_CPP) -I. opcodes/intelPentiumMMX.opcodes -o $@
dosboxxr/lib/cpu/core/intelPentiumMMX.c16.a16.decom.h: dosboxxr/lib/cpu/core/intelPentiumMMX.opcodes.p $(OPCC)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intelPentiumMMX.opcodes.p -o $@ -m decompile -c 16 -a 16 -c32vars
dosboxxr/lib/cpu/core/intelPentiumMMX.cxx.axx.decom.h: dosboxxr/lib/cpu/core/intelPentiumMMX.opcodes.p $(OPCC)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intelPentiumMMX.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars -g
dosboxxr/lib/cpu/core/intelPentiumMMX.c32.a32.decom.h: dosboxxr/lib/cpu/core/intelPentiumMMX.opcodes.p $(OPCC)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intelPentiumMMX.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars

dosboxxr/lib/cpu/core/intelPentiumProMMX.opcodes.p: opcodes/intelPentiumProMMX.opcodes
	$(BUILD_CPP) -I. opcodes/intelPentiumProMMX.opcodes -o $@
dosboxxr/lib/cpu/core/intelPentiumProMMX.c16.a16.decom.h: dosboxxr/lib/cpu/core/intelPentiumProMMX.opcodes.p $(OPCC)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intelPentiumProMMX.opcodes.p -o $@ -m decompile -c 16 -a 16 -c32vars
dosboxxr/lib/cpu/core/intelPentiumProMMX.cxx.axx.decom.h: dosboxxr/lib/cpu/core/intelPentiumProMMX.opcodes.p $(OPCC)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intelPentiumProMMX.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars -g
dosboxxr/lib/cpu/core/intelPentiumProMMX.c32.a32.decom.h: dosboxxr/lib/cpu/core/intelPentiumProMMX.opcodes.p $(OPCC)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intelPentiumProMMX.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars

dosboxxr/lib/cpu/core/cyrix6x86MX.opcodes.p: opcodes/cyrix6x86MX.opcodes
	$(BUILD_CPP) -I. opcodes/cyrix6x86MX.opcodes -o $@
dosboxxr/lib/cpu/core/cyrix6x86MX.c16.a16.decom.h: dosboxxr/lib/cpu/core/cyrix6x86MX.opcodes.p $(OPCC)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/cyrix6x86MX.opcodes.p -o $@ -m decompile -c 16 -a 16 -c32vars
dosboxxr/lib/cpu/core/cyrix6x86MX.cxx.axx.decom.h: dosboxxr/lib/cpu/core/cyrix6x86MX.opcodes.p $(OPCC)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/cyrix6x86MX.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars -g
dosboxxr/lib/cpu/core/cyrix6x86MX.c32.a32.decom.h: dosboxxr/lib/cpu/core/cyrix6x86MX.opcodes.p $(OPCC)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/cyrix6x86MX.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars

dosboxxr/lib/cpu/core/intelPentium2.opcodes.p: opcodes/intelPentium2.opcodes
	$(BUILD_CPP) -I. opcodes/intelPentium2.opcodes -o $@
dosboxxr/lib/cpu/core/intelPentium2.c16.a16.decom.h: dosboxxr/lib/cpu/core/intelPentium2.opcodes.p $(OPCC)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intelPentium2.opcodes.p -o $@ -m decompile -c 16 -a 16 -c32vars
dosboxxr/lib/cpu/core/intelPentium2.cxx.axx.decom.h: dosboxxr/lib/cpu/core/intelPentium2.opcodes.p $(OPCC)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intelPentium2.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars -g
dosboxxr/lib/cpu/core/intelPentium2.c32.a32.decom.h: dosboxxr/lib/cpu/core/intelPentium2.opcodes.p $(OPCC)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intelPentium2.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars

dosboxxr/lib/cpu/core/intelPentium3.opcodes.p: opcodes/intelPentium3.opcodes
	$(BUILD_CPP) -I. opcodes/intelPentium3.opcodes -o $@
dosboxxr/lib/cpu/core/intelPentium3.c16.a16.decom.h: dosboxxr/lib/cpu/core/intelPentium3.opcodes.p $(OPCC)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intelPentium3.opcodes.p -o $@ -m decompile -c 16 -a 16 -c32vars
dosboxxr/lib/cpu/core/intelPentium3.cxx.axx.decom.h: dosboxxr/lib/cpu/core/intelPentium3.opcodes.p $(OPCC)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intelPentium3.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars -g
dosboxxr/lib/cpu/core/intelPentium3.c32.a32.decom.h: dosboxxr/lib/cpu/core/intelPentium3.opcodes.p $(OPCC)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intelPentium3.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars

dosboxxr/lib/cpu/core/intelPentium4.opcodes.p: opcodes/intelPentium4.opcodes
	$(BUILD_CPP) -I. opcodes/intelPentium4.opcodes -o $@
dosboxxr/lib/cpu/core/intelPentium4.c16.a16.decom.h: dosboxxr/lib/cpu/core/intelPentium4.opcodes.p $(OPCC)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intelPentium4.opcodes.p -o $@ -m decompile -c 16 -a 16 -c32vars
dosboxxr/lib/cpu/core/intelPentium4.cxx.axx.decom.h: dosboxxr/lib/cpu/core/intelPentium4.opcodes.p $(OPCC)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intelPentium4.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars -g
dosboxxr/lib/cpu/core/intelPentium4.c32.a32.decom.h: dosboxxr/lib/cpu/core/intelPentium4.opcodes.p $(OPCC)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intelPentium4.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars

dosboxxr/lib/cpu/core/intelPentium4Prescott.opcodes.p: opcodes/intelPentium4Prescott.opcodes
	$(BUILD_CPP) -I. opcodes/intelPentium4Prescott.opcodes -o $@
dosboxxr/lib/cpu/core/intelPentium4Prescott.c16.a16.decom.h: dosboxxr/lib/cpu/core/intelPentium4Prescott.opcodes.p $(OPCC)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intelPentium4Prescott.opcodes.p -o $@ -m decompile -c 16 -a 16 -c32vars
dosboxxr/lib/cpu/core/intelPentium4Prescott.cxx.axx.decom.h: dosboxxr/lib/cpu/core/intelPentium4Prescott.opcodes.p $(OPCC)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intelPentium4Prescott.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars -g
dosboxxr/lib/cpu/core/intelPentium4Prescott.c32.a32.decom.h: dosboxxr/lib/cpu/core/intelPentium4Prescott.opcodes.p $(OPCC)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/intelPentium4Prescott.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars

dosboxxr/lib/cpu/core/amdk6r2.opcodes.p: opcodes/amdk6r2.opcodes
	$(BUILD_CPP) -I. opcodes/amdk6r2.opcodes -o $@
dosboxxr/lib/cpu/core/amdk6r2.c16.a16.decom.h: dosboxxr/lib/cpu/core/amdk6r2.opcodes.p $(OPCC)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/amdk6r2.opcodes.p -o $@ -m decompile -c 16 -a 16 -c32vars
dosboxxr/lib/cpu/core/amdk6r2.cxx.axx.decom.h: dosboxxr/lib/cpu/core/amdk6r2.opcodes.p $(OPCC)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/amdk6r2.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars -g
dosboxxr/lib/cpu/core/amdk6r2.c32.a32.decom.h: dosboxxr/lib/cpu/core/amdk6r2.opcodes.p $(OPCC)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/amdk6r2.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars

dosboxxr/lib/cpu/core/amdAthlon.opcodes.p: opcodes/amdAthlon.opcodes
	$(BUILD_CPP) -I. opcodes/amdAthlon.opcodes -o $@
dosboxxr/lib/cpu/core/amdAthlon.c16.a16.decom.h: dosboxxr/lib/cpu/core/amdAthlon.opcodes.p $(OPCC)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/amdAthlon.opcodes.p -o $@ -m decompile -c 16 -a 16 -c32vars
dosboxxr/lib/cpu/core/amdAthlon.cxx.axx.decom.h: dosboxxr/lib/cpu/core/amdAthlon.opcodes.p $(OPCC)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/amdAthlon.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars -g
dosboxxr/lib/cpu/core/amdAthlon.c32.a32.decom.h: dosboxxr/lib/cpu/core/amdAthlon.opcodes.p $(OPCC)
	$(OPCC)$(EXEEXT) -i dosboxxr/lib/cpu/core/amdAthlon.opcodes.p -o $@ -m decompile -c 32 -a 32 -c32vars

# grrr how do I get automake to run opcc first before compiling other parts?
opcodes: $(OPCC_H)

