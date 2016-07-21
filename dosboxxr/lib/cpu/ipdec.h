
#include "dosboxxr/lib/cpu/memreftypes.h"
#include "dosboxxr/lib/cpu/x86ModRegRm.h"

enum IPDecRegClass {
    RC_REG=0,
    RC_FPUREG
};

extern const char *CPUregs16[8];
extern const char *CPUregs8[8];
extern const char *CPUregsZ[8];
extern const char **CPUregsN[5];
extern const char *CPUsregs_8086[8];
extern const char *CPUjcc7x[16];
extern const char *CPUGRP1[8];
extern const char *CPUGRP2[8];
extern const char *CPUGRP3[8];
extern const char *CPUGRP4[8];
extern const char *CPUmod0displacement16[8];
extern const char *CPU0F00ops[8];
extern const char *CPU0F01ops[8];

extern x86_offset_t             IPDecIP;
extern char                     IPDecStr[256];

static inline uint8_t IPDec8abs(uint8_t v) {
    if (v & 0x80) return 0x100 - v;
    return v;
}

const char *IPDecPrint16(const x86ModRegRm &mrm,const x86_offset_t ofs,const unsigned int sz,const IPDecRegClass regclass=RC_REG);

