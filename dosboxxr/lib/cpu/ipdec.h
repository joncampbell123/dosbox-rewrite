
#include "dosboxxr/lib/cpu/memreftypes.h"
#include "dosboxxr/lib/cpu/x86ModRegRm.h"
#include "dosboxxr/lib/cpu/x86ScaleIndexBase.h"

enum IPDecRegClass {
    RC_REG=0,
    RC_FPUREG,
    RC_MMXREG,
    RC_SSEREG,
    RC_AVXREG
};

extern const char *CPUregs32[8];
extern const char *CPUregs16[8];
extern const char *CPUregs8[8];
extern const char *CPUregsZ[8];
extern const char **CPUregsN[5];
extern const char *CPUsregs_8086[8];
extern const char *CPUsregs_80386[8];
extern const char *CPUjcc7x[16];
extern const char *CPUjcc0F8x[16];
extern const char *CPUGRP1[8];
extern const char *CPUGRP2[8];
extern const char *CPUGRP3[8];
extern const char *CPUGRP4[8];
extern const char *CPUmod0displacement16[8];
extern const char *CPU0F00ops[8];
extern const char *CPU0F01ops[8];
extern const char *CPUsetcc0F9x[16];
extern const char *sizesuffix[12];
extern const char *sizespec[12];

extern x86_offset_t             IPDecIP;
extern char                     IPDecStr[256];

static inline uint8_t IPDec8abs(uint8_t v) {
    if (v & 0x80) return 0x100 - v;
    return v;
}

static inline uint16_t IPDec16abs(uint16_t v) {
    if (v & 0x8000UL) return 0x10000UL - v;
    return v;
}

static inline uint32_t IPDec32abs(uint32_t v) {
    if (v & 0x80000000ULL) return 0x100000000ULL - v;
    return v;
}

const char *IPDecPrint16(const x86ModRegRm &mrm,const x86_offset_t ofs,const unsigned int sz,const IPDecRegClass regclass,const char *suffix);
const char *IPDecPrint32(const x86ModRegRm &mrm,const x86ScaleIndexBase &sib,const x86_offset_t ofs,const unsigned int sz,const IPDecRegClass regclass,const char *suffix);
const char *IPDecPrint1632(const bool addr32,const x86ModRegRm &mrm,const x86ScaleIndexBase &sib,const x86_offset_t ofs,const unsigned int sz,const IPDecRegClass regclass,const char *suffix);

