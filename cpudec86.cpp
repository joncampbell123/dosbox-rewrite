
#ifdef HAVE_CONFIG_H
# include "config.h" // must be first
#endif

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>

#include "dosboxxr/lib/cpu/x86ModRegRm.h"
#include "dosboxxr/lib/cpu/memreftypes.h"

x86_offset_t            exe_ip = 0;
unsigned char*          exe_ip_ptr = NULL;
unsigned char*          exe_image = NULL;
unsigned char*          exe_image_fence = NULL;

#include <endian.h>

#define case_span_2(x) \
    case (x)+0: case (x)+1
#define case_span_4(x) \
    case_span_2(x): case_span_2((x)+2)
#define case_span_8(x) \
    case_span_4(x): case_span_4((x)+4)
#define case_span_16(x) \
    case_span_8(x): case_span_8((x)+8)

// this macro covers one whole 8-value block where in MOD/REG/RM MOD == mod and REG == reg for any value of R/M
#define case_span_by_mod_reg(mod,reg) \
        case_span_8(((mod) << 6) + ((reg) << 3))

static const char *CPUregs16[8] = {
    "AX","CX","DX","BX", "SP","BP","SI","DI"
};

static const char *CPUregs8[8] = {
    "AL","CL","DL","BL", "AH","CH","DH","BH"
};

static const char *CPUregsZ[8] = {
    "","","","", "","","",""
};

static const char **CPUregsN[5] = {
    CPUregsZ,                       // sz=0
    CPUregs8,                       // sz=1
    CPUregs16,                      // sz=2
    CPUregsZ,                       // sz=3
    CPUregsZ                        // sz=4
};

static const char *CPUsregs[8] = {
    "ES","CS","SS","DS","","","",""
};

static const char *CPUjcc7x[16] = {
    "JO","JNO",
    "JB","JNB",
    "JZ","JNZ",
    "JBE","JA",
    "JS","JNS",
    "JPE","JPO",
    "JL","JGE",
    "JLE","JG"
};

static const char *CPUGRP1[8] = {
    "ADD","OR",
    "ADC","SBB",
    "AND","SUB",
    "XOR","CMP"
};

static const char *CPUGRP2[8] = {
    "ROL","ROR",
    "RCL","RCR",
    "SHL","SHR",
    "SAL","SAR"
};

static const char *CPUGRP3[8] = {
    "TEST","TEST", // according to x86 opcode map geek edition, reg==1 is undefined alias of reg==0
    "NOT","NEG",
    "MUL","IMUL",
    "DIV","IDIV"
};

static const char *CPUGRP4[8] = {
    "INC","DEC",
    "CALL","CALLF", // reg==2 (CALL) or higher invalid unless opcode == 0xFF
    "JMP","JMPF",
    "PUSH","???"    // reg==7 illegal
};

static const char *CPUmod0displacement16[8] = {
    "BX+SI","BX+DI","BP+SI","BP+DI",
    "SI",   "DI",   "BP",   "BX"
};

//// CPU CORE
#define DECOMPILEMODE

static x86_offset_t IPDecIP;
static char IPDecStr[256];

static inline bool IPcontinue(void) {
    return (exe_ip_ptr < exe_image_fence);
}

static inline x86_offset_t IPval(void) {
    return exe_ip;
}

static inline uint8_t IPFB(void) {
    const uint8_t r = *((const uint8_t*)exe_ip_ptr);
    exe_ip_ptr += 1;
    exe_ip += 1;
    return r;
}

static inline int32_t IPFBsigned(void) {
    return (int32_t)((int8_t)IPFB());
}

static inline void IPFModRegRm(x86ModRegRm &m) {
    m.byte = IPFB();
}

static inline uint16_t IPFW(void) {
    const uint16_t r = le16toh(*((const uint16_t*)exe_ip_ptr));
    exe_ip_ptr += 2;
    exe_ip += 2;
    return r;
}

static inline int32_t IPFWsigned(void) {
    return (int32_t)((int16_t)IPFW());
}

static inline uint32_t IPFDW(void) {
    const uint32_t r = le32toh(*((const uint32_t*)exe_ip_ptr));
    exe_ip_ptr += 4;
    exe_ip += 4;
    return r;
}

// given mod/reg/rm fetch displacement (16-bit code)
x86_offset_t IPFmrmdisplace16(x86ModRegRm &mrm) {
    switch (mrm.mod()) {
        case 0:
            if (mrm.rm() == 6) return IPFW();
            return 0;
        case 1:
            return IPFBsigned();
        case 2:
            return IPFW();
        case 3:
            return 0;
    };
}

static inline uint8_t IPDec8abs(uint8_t v) {
    if (v & 0x80) return 0x100 - v;
    return v;
}

enum IPDecRegClass {
    RC_REG=0,
    RC_FPUREG
};

// print 16-bit code form of mod/reg/rm with displacement
const char *IPDecPrint16(const x86ModRegRm &mrm,const x86_offset_t ofs,const unsigned int sz,const IPDecRegClass regclass=RC_REG) {
    static char tmp[64];
    char *w=tmp,*wf=tmp+sizeof(tmp)-1;

    switch (mrm.mod()) {
        case 0: // [indirect] or [displacement]
            if (mrm.rm() == 6)
                w += snprintf(w,(size_t)(wf-w),"[%04lxh]",(unsigned long)ofs);
            else
                w += snprintf(w,(size_t)(wf-w),"[%s]",CPUmod0displacement16[mrm.rm()]);
            break;
        case 1: // [indirect+disp8]
            w += snprintf(w,(size_t)(wf-w),"[%s%c%02Xh]",CPUmod0displacement16[mrm.rm()],ofs&0x80?'-':'+',IPDec8abs((uint8_t)ofs));
            break;
        case 2: // [indirect+disp16]
            w += snprintf(w,(size_t)(wf-w),"[%s+%04Xh]",CPUmod0displacement16[mrm.rm()],(uint16_t)ofs);
            break;
        case 3: // register
            switch (regclass) {
                case RC_REG:
                    w += snprintf(w,(size_t)(wf-w),"%s",CPUregsN[sz][mrm.rm()]);
                    break;
                case RC_FPUREG: // Floating point registers
                    w += snprintf(w,(size_t)(wf-w),"ST(%u)",mrm.rm());
                    break;
            };
            break;
    }

    return tmp;
}

void IPDec(x86_offset_t ip) {
    char *w = IPDecStr,*wf = IPDecStr+sizeof(IPDecStr)-1;
    x86_offset_t disp;
    x86ModRegRm mrm;
    uint8_t op1,v8;
    uint16_t v16b;
    uint16_t v16;

    {
#ifdef DECOMPILEMODE
        /* one instruction only */
        IPDecStr[0] = 0;
        IPDecIP = ip;
#else
        while (IPcontinue())
#endif
        {
after_prefix:
            switch (op1=IPFB()) {
                case 0x00: // ADD r/m,reg byte size
                    mrm.set(IPFB());
                    disp = IPFmrmdisplace16(/*&*/mrm);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"ADDb %s,%s",IPDecPrint16(mrm,disp,1),CPUregs8[mrm.reg()]);
#endif
                    break;
                case 0x01: // ADD r/m,reg word size
                    mrm.set(IPFB());
                    disp = IPFmrmdisplace16(/*&*/mrm);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"ADDw %s,%s",IPDecPrint16(mrm,disp,2),CPUregs16[mrm.reg()]);
#endif
                    break;
                case 0x02: // ADD reg,r/m byte size
                    mrm.set(IPFB());
                    disp = IPFmrmdisplace16(/*&*/mrm);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"ADDb %s,%s",CPUregs8[mrm.reg()],IPDecPrint16(mrm,disp,1));
#endif
                    break;
                case 0x03: // ADD reg,r/m word size
                    mrm.set(IPFB());
                    disp = IPFmrmdisplace16(/*&*/mrm);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"ADDw %s,%s",CPUregs16[mrm.reg()],IPDecPrint16(mrm,disp,2));
#endif
                    break;
                case 0x04: // ADD AL,imm8
                    v8 = IPFB();
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"ADDb AL,%02Xh",v8);
#endif
                    break;
                case 0x05: // ADD AX,imm16
                    v16 = IPFW();
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"ADDw AX,%04Xh",v16);
#endif
                    break;
                case 0x06:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"PUSHw ES");
#endif
                    break;
                case 0x07:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"POPw ES");
#endif
                    break;
                case 0x08: // OR r/m,reg byte size
                    mrm.set(IPFB());
                    disp = IPFmrmdisplace16(/*&*/mrm);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"ORb %s,%s",IPDecPrint16(mrm,disp,1),CPUregs8[mrm.reg()]);
#endif
                    break;
                case 0x09: // OR r/m,reg word size
                    mrm.set(IPFB());
                    disp = IPFmrmdisplace16(/*&*/mrm);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"ORw %s,%s",IPDecPrint16(mrm,disp,2),CPUregs16[mrm.reg()]);
#endif
                    break;
                case 0x0A: // OR reg,r/m byte size
                    mrm.set(IPFB());
                    disp = IPFmrmdisplace16(/*&*/mrm);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"ORb %s,%s",CPUregs8[mrm.reg()],IPDecPrint16(mrm,disp,1));
#endif
                    break;
                case 0x0B: // OR reg,r/m word size
                    mrm.set(IPFB());
                    disp = IPFmrmdisplace16(/*&*/mrm);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"ORw %s,%s",CPUregs16[mrm.reg()],IPDecPrint16(mrm,disp,2));
#endif
                    break;
                case 0x0C: // OR AL,imm8
                    v8 = IPFB();
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"ORb AL,%02Xh",v8);
#endif
                    break;
                case 0x0D: // OR AX,imm16
                    v16 = IPFW();
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"ORw AX,%04Xh",v16);
#endif
                    break;
                case 0x0E:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"PUSHw CS");
#endif
                    break;
                case 0x0F: // 8086 only!
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"POPw CS");
#endif
                    break;
                case 0x10: // ADC r/m,reg byte size
                    mrm.set(IPFB());
                    disp = IPFmrmdisplace16(/*&*/mrm);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"ADCb %s,%s",IPDecPrint16(mrm,disp,1),CPUregs8[mrm.reg()]);
#endif
                    break;
                case 0x11: // ADC r/m,reg word size
                    mrm.set(IPFB());
                    disp = IPFmrmdisplace16(/*&*/mrm);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"ADCw %s,%s",IPDecPrint16(mrm,disp,2),CPUregs16[mrm.reg()]);
#endif
                    break;
                case 0x12: // ADC reg,r/m byte size
                    mrm.set(IPFB());
                    disp = IPFmrmdisplace16(/*&*/mrm);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"ADCb %s,%s",CPUregs8[mrm.reg()],IPDecPrint16(mrm,disp,1));
#endif
                    break;
                case 0x13: // ADC reg,r/m word size
                    mrm.set(IPFB());
                    disp = IPFmrmdisplace16(/*&*/mrm);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"ADCw %s,%s",CPUregs16[mrm.reg()],IPDecPrint16(mrm,disp,2));
#endif
                    break;
                case 0x14: // ADC AL,imm8
                    v8 = IPFB();
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"ADCb AL,%02Xh",v8);
#endif
                    break;
                case 0x15: // ADC AX,imm16
                    v16 = IPFW();
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"ADCw AX,%04Xh",v16);
#endif
                    break;
                case 0x16:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"PUSHw SS");
#endif
                    break;
                case 0x17:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"POPw SS");
#endif
                    break;
                case 0x18: // SBB r/m,reg byte size
                    mrm.set(IPFB());
                    disp = IPFmrmdisplace16(/*&*/mrm);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"SBBb %s,%s",IPDecPrint16(mrm,disp,1),CPUregs8[mrm.reg()]);
#endif
                    break;
                case 0x19: // SBB r/m,reg word size
                    mrm.set(IPFB());
                    disp = IPFmrmdisplace16(/*&*/mrm);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"SBBw %s,%s",IPDecPrint16(mrm,disp,2),CPUregs16[mrm.reg()]);
#endif
                    break;
                case 0x1A: // SBB reg,r/m byte size
                    mrm.set(IPFB());
                    disp = IPFmrmdisplace16(/*&*/mrm);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"SBBb %s,%s",CPUregs8[mrm.reg()],IPDecPrint16(mrm,disp,1));
#endif
                    break;
                case 0x1B: // SBB reg,r/m word size
                    mrm.set(IPFB());
                    disp = IPFmrmdisplace16(/*&*/mrm);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"SBBw %s,%s",CPUregs16[mrm.reg()],IPDecPrint16(mrm,disp,2));
#endif
                    break;
                case 0x1C: // SBB AL,imm8
                    v8 = IPFB();
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"SBBb AL,%02Xh",v8);
#endif
                    break;
                case 0x1D: // SBB AX,imm16
                    v16 = IPFW();
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"SBBw AX,%04Xh",v16);
#endif
                    break;
                case 0x1E:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"PUSHw DS");
#endif
                    break;
                case 0x1F:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"POPw DS");
#endif
                    break;
                case 0x20: // AND r/m,reg byte size
                    mrm.set(IPFB());
                    disp = IPFmrmdisplace16(/*&*/mrm);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"ANDb %s,%s",IPDecPrint16(mrm,disp,1),CPUregs8[mrm.reg()]);
#endif
                    break;
                case 0x21: // AND r/m,reg word size
                    mrm.set(IPFB());
                    disp = IPFmrmdisplace16(/*&*/mrm);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"ANDw %s,%s",IPDecPrint16(mrm,disp,2),CPUregs16[mrm.reg()]);
#endif
                    break;
                case 0x22: // AND reg,r/m byte size
                    mrm.set(IPFB());
                    disp = IPFmrmdisplace16(/*&*/mrm);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"ANDb %s,%s",CPUregs8[mrm.reg()],IPDecPrint16(mrm,disp,1));
#endif
                    break;
                case 0x23: // AND reg,r/m word size
                    mrm.set(IPFB());
                    disp = IPFmrmdisplace16(/*&*/mrm);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"ANDw %s,%s",CPUregs16[mrm.reg()],IPDecPrint16(mrm,disp,2));
#endif
                    break;
                case 0x24: // AND AL,imm8
                    v8 = IPFB();
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"ANDb AL,%02Xh",v8);
#endif
                    break;
                case 0x25: // AND AX,imm16
                    v16 = IPFW();
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"ANDw AX,%04Xh",v16);
#endif
                    break;
                case 0x26:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"ES: ");
#endif
                    goto after_prefix;
                case 0x27:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"DAA");
#endif
                    break;
                case 0x28: // SUB r/m,reg byte size
                    mrm.set(IPFB());
                    disp = IPFmrmdisplace16(/*&*/mrm);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"SUBb %s,%s",IPDecPrint16(mrm,disp,1),CPUregs8[mrm.reg()]);
#endif
                    break;
                case 0x29: // SUB r/m,reg word size
                    mrm.set(IPFB());
                    disp = IPFmrmdisplace16(/*&*/mrm);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"SUBw %s,%s",IPDecPrint16(mrm,disp,2),CPUregs16[mrm.reg()]);
#endif
                    break;
                case 0x2A: // SUB reg,r/m byte size
                    mrm.set(IPFB());
                    disp = IPFmrmdisplace16(/*&*/mrm);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"SUBb %s,%s",CPUregs8[mrm.reg()],IPDecPrint16(mrm,disp,1));
#endif
                    break;
                case 0x2B: // SUB reg,r/m word size
                    mrm.set(IPFB());
                    disp = IPFmrmdisplace16(/*&*/mrm);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"SUBw %s,%s",CPUregs16[mrm.reg()],IPDecPrint16(mrm,disp,2));
#endif
                    break;
                case 0x2C: // SUB AL,imm8
                    v8 = IPFB();
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"SUBb AL,%02Xh",v8);
#endif
                    break;
                case 0x2D: // SUB AX,imm16
                    v16 = IPFW();
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"SUBw AX,%04Xh",v16);
#endif
                    break;
                case 0x2E:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"CS: ");
#endif
                    goto after_prefix;
                case 0x2F:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"DAS");
#endif
                    break;
                case 0x30: // XOR r/m,reg byte size
                    mrm.set(IPFB());
                    disp = IPFmrmdisplace16(/*&*/mrm);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"XORb %s,%s",IPDecPrint16(mrm,disp,1),CPUregs8[mrm.reg()]);
#endif
                    break;
                case 0x31: // XOR r/m,reg word size
                    mrm.set(IPFB());
                    disp = IPFmrmdisplace16(/*&*/mrm);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"XORw %s,%s",IPDecPrint16(mrm,disp,2),CPUregs16[mrm.reg()]);
#endif
                    break;
                case 0x32: // XOR reg,r/m byte size
                    mrm.set(IPFB());
                    disp = IPFmrmdisplace16(/*&*/mrm);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"XORb %s,%s",CPUregs8[mrm.reg()],IPDecPrint16(mrm,disp,1));
#endif
                    break;
                case 0x33: // XOR reg,r/m word size
                    mrm.set(IPFB());
                    disp = IPFmrmdisplace16(/*&*/mrm);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"XORw %s,%s",CPUregs16[mrm.reg()],IPDecPrint16(mrm,disp,2));
#endif
                    break;
                case 0x34: // XOR AL,imm8
                    v8 = IPFB();
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"XORb AL,%02Xh",v8);
#endif
                    break;
                case 0x35: // XOR AX,imm16
                    v16 = IPFW();
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"XORw AX,%04Xh",v16);
#endif
                    break;
                case 0x36:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"SS: ");
#endif
                    goto after_prefix;
                case 0x37:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"AAA");
#endif
                    break;
                case 0x38: // CMP r/m,reg byte size
                    mrm.set(IPFB());
                    disp = IPFmrmdisplace16(/*&*/mrm);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"CMPb %s,%s",IPDecPrint16(mrm,disp,1),CPUregs8[mrm.reg()]);
#endif
                    break;
                case 0x39: // CMP r/m,reg word size
                    mrm.set(IPFB());
                    disp = IPFmrmdisplace16(/*&*/mrm);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"CMPw %s,%s",IPDecPrint16(mrm,disp,2),CPUregs16[mrm.reg()]);
#endif
                    break;
                case 0x3A: // CMP reg,r/m byte size
                    mrm.set(IPFB());
                    disp = IPFmrmdisplace16(/*&*/mrm);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"CMPb %s,%s",CPUregs8[mrm.reg()],IPDecPrint16(mrm,disp,1));
#endif
                    break;
                case 0x3B: // CMP reg,r/m word size
                    mrm.set(IPFB());
                    disp = IPFmrmdisplace16(/*&*/mrm);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"CMPw %s,%s",CPUregs16[mrm.reg()],IPDecPrint16(mrm,disp,2));
#endif
                    break;
                case 0x3C: // CMP AL,imm8
                    v8 = IPFB();
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"CMPb AL,%02Xh",v8);
#endif
                    break;
                case 0x3D: // CMP AX,imm16
                    v16 = IPFW();
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"CMPw AX,%04Xh",v16);
#endif
                    break;
                case 0x3E:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"DS: ");
#endif
                    goto after_prefix;
                case 0x3F:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"AAS");
#endif
                    break;
                case_span_8(0x40): // 0x40-0x47
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"INCw %s",CPUregs16[op1&7]);
#endif
                    break;
                case_span_8(0x48): // 0x48-0x4F
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"DECw %s",CPUregs16[op1&7]);
#endif
                    break;
                case_span_8(0x50): // 0x50-0x57
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"PUSHw %s",CPUregs16[op1&7]);
#endif
                    break;
                case_span_8(0x58): // 0x58-0x5F
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"POPw %s",CPUregs16[op1&7]);
#endif
                    break;
                // ALIAS 8086
                // 8086: 60-6F not documented, but found to be aliases of 70-7F
                // according to http://www.os2museum.com/wp/undocumented-8086-opcodes/
                // TODO: Pull out the old IBM 5150 and check that these are alias
                case_span_16(0x60): // 0x60-0x6F
                // END ALIAS
                case_span_16(0x70): // 0x70-0x7F
                    v16 = (uint16_t)IPFBsigned();
                    v16 = (v16 + IPval()) & 0xFFFFU;
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"%sw %04Xh",CPUjcc7x[op1&15],v16);
#endif
                    break;
                case 0x80: // GRP1 byte r/m, imm8
                case 0x82: // GRP1 byte r/m, imm8 alias
                    mrm.set(IPFB());
                    disp = IPFmrmdisplace16(/*&*/mrm);
                    v8 = IPFB();
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"%sb %s,%02Xh",CPUGRP1[mrm.reg()],IPDecPrint16(mrm,disp,1),v8);
#endif
                    break;
                case 0x81: // GRP1 word r/m, imm16
                    mrm.set(IPFB());
                    disp = IPFmrmdisplace16(/*&*/mrm);
                    v16 = IPFW();
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"%sw %s,%04Xh",CPUGRP1[mrm.reg()],IPDecPrint16(mrm,disp,2),v16);
#endif
                    break;
                case 0x83: // GRP1 word r/m, imm8 sign extended
                    mrm.set(IPFB());
                    disp = IPFmrmdisplace16(/*&*/mrm);
                    v16 = (uint16_t)IPFBsigned();
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"%sw %s,%04Xh",CPUGRP1[mrm.reg()],IPDecPrint16(mrm,disp,2),v16);
#endif
                    break;
                case 0x84: // TEST r/m,reg byte size
                    mrm.set(IPFB());
                    disp = IPFmrmdisplace16(/*&*/mrm);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"TESTb %s,%s",IPDecPrint16(mrm,disp,1),CPUregs8[mrm.reg()]);
#endif
                    break;
                case 0x85: // TEST r/m,reg word size
                    mrm.set(IPFB());
                    disp = IPFmrmdisplace16(/*&*/mrm);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"TESTw %s,%s",IPDecPrint16(mrm,disp,2),CPUregs16[mrm.reg()]);
#endif
                    break;
                case 0x86: // XCHG r/m,reg byte size
                    mrm.set(IPFB());
                    disp = IPFmrmdisplace16(/*&*/mrm);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"XCHGb %s,%s",CPUregs8[mrm.reg()],IPDecPrint16(mrm,disp,1));
#endif
                    break;
                case 0x87: // XCHG r/m,reg word size
                    mrm.set(IPFB());
                    disp = IPFmrmdisplace16(/*&*/mrm);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"XCHGw %s,%s",CPUregs16[mrm.reg()],IPDecPrint16(mrm,disp,2));
#endif
                    break;
                case 0x88: // MOV r/m,reg byte size
                    mrm.set(IPFB());
                    disp = IPFmrmdisplace16(/*&*/mrm);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"MOVb %s,%s",IPDecPrint16(mrm,disp,1),CPUregs8[mrm.reg()]);
#endif
                    break;
                case 0x89: // MOV r/m,reg word size
                    mrm.set(IPFB());
                    disp = IPFmrmdisplace16(/*&*/mrm);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"MOVw %s,%s",IPDecPrint16(mrm,disp,2),CPUregs16[mrm.reg()]);
#endif
                    break;
                case 0x8A: // MOV reg,r/m byte size
                    mrm.set(IPFB());
                    disp = IPFmrmdisplace16(/*&*/mrm);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"MOVb %s,%s",CPUregs8[mrm.reg()],IPDecPrint16(mrm,disp,1));
#endif
                    break;
                case 0x8B: // MOV reg,r/m word size
                    mrm.set(IPFB());
                    disp = IPFmrmdisplace16(/*&*/mrm);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"MOVw %s,%s",CPUregs16[mrm.reg()],IPDecPrint16(mrm,disp,2));
#endif
                    break;
                case 0x8C: // MOV r/m,sreg word size
                    mrm.set(IPFB());
                    disp = IPFmrmdisplace16(/*&*/mrm);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"MOVw %s,%s",IPDecPrint16(mrm,disp,2),CPUsregs[mrm.reg()]);
#endif
                    break;
                case 0x8D: // LEA reg,r/m word size
                    mrm.set(IPFB());
                    disp = IPFmrmdisplace16(/*&*/mrm);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"LEAw %s,%s",CPUregs16[mrm.reg()],IPDecPrint16(mrm,disp,2));
#endif
                    break;
                case 0x8E: // MOV sreg,r/m word size
                    mrm.set(IPFB());
                    disp = IPFmrmdisplace16(/*&*/mrm);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"MOVw %s,%s",CPUsregs[mrm.reg()],IPDecPrint16(mrm,disp,2));
#endif
                    break;
                case 0x8F: // POP r/m word size
                    mrm.set(IPFB());
                    disp = IPFmrmdisplace16(/*&*/mrm);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"POPw %s",IPDecPrint16(mrm,disp,2));
#endif
                    break;
                case 0x90:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"NOP");
#endif
                    break;
                case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"XCHGw %s,AX",CPUregs16[op1&7]);
#endif
                    break;
                case 0x98:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"CBWb");
#endif
                    break;
                case 0x99:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"CWDw");
#endif
                    break;
                case 0x9A:
                    v16 = IPFW();//offset
                    v16b = IPFW();//segment
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"CALLw %04Xh:%04Xh",v16b,v16);
#endif
                    break;
                case 0x9B:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"WAIT");
#endif
                    break;
                case 0x9C:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"PUSHFw");
#endif
                    break;
                case 0x9D:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"POPFw");
#endif
                    break;
                case 0x9E:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"SAHFb");
#endif
                    break;
                case 0x9F:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"LAHFb");
#endif
                    break;
                case 0xA0:
                    v16 = IPFW();
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"MOVw AL,[%04Xh]",v16);
#endif
                    break;
                case 0xA1:
                    v16 = IPFW();
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"MOVw AX,[%04Xh]",v16);
#endif
                    break;
                case 0xA2:
                    v16 = IPFW();
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"MOVw [%04Xh],AL",v16);
#endif
                    break;
                case 0xA3:
                    v16 = IPFW();
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"MOVw [%04Xh],AX",v16);
#endif
                    break;
                case 0xA4:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"MOVSB");
#endif
                    break;
                case 0xA5:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"MOVSW");
#endif
                    break;
                case 0xA6:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"CMPSB");
#endif
                    break;
                case 0xA7:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"CMPSW");
#endif
                    break;
                case 0xA8:
                    v8 = IPFB();
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"TEST AL,%02Xh",v8);
#endif
                    break;
                case 0xA9:
                    v16 = IPFW();
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"TEST AX,%04Xh",v16);
#endif
                    break;
                case 0xAA:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"STOSB");
#endif
                    break;
                case 0xAB:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"STOSW");
#endif
                    break;
                case 0xAC:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"LODSB");
#endif
                    break;
                case 0xAD:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"LODSW");
#endif
                    break;
                case 0xAE:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"SCASB");
#endif
                    break;
                case 0xAF:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"SCASW");
#endif
                    break;
                case_span_8(0xB0):
                    v8 = IPFB();
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"MOV %s,%02Xh",CPUregs8[op1&7],v8);
#endif
                    break;
                case_span_8(0xB8):
                    v16 = IPFW();
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"MOV %s,%04Xh",CPUregs16[op1&7],v16);
#endif
                    break;
                case 0xC2:
                case 0xC0: // ALIAS 8086
                    // TODO: Pull out the old IBM 5150 and check that 0xC0 is alias
                    v16 = IPFW();
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"RETw %04Xh",v16);
#endif
                    break;
                case 0xC3:
                case 0xC1: // ALIAS 8086
                    // TODO: Pull out the old IBM 5150 and check that 0xC1 is alias
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"RETw");
#endif
                    break;
                case 0xC4: // LES reg,r/m word size
                    mrm.set(IPFB());
                    disp = IPFmrmdisplace16(/*&*/mrm);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"LESw %s,%s",CPUregs16[mrm.reg()],IPDecPrint16(mrm,disp,2));
#endif
                    break;
                case 0xC5: // LDS reg,r/m word size
                    mrm.set(IPFB());
                    disp = IPFmrmdisplace16(/*&*/mrm);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"LDSw %s,%s",CPUregs16[mrm.reg()],IPDecPrint16(mrm,disp,2));
#endif
                    break;
                case 0xC6: // MOV r/m,imm8
                    mrm.set(IPFB());
                    disp = IPFmrmdisplace16(/*&*/mrm);
                    v8 = IPFB();
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"MOVb %s,%02Xh",IPDecPrint16(mrm,disp,1),v8);
#endif
                    break;
                case 0xC7: // MOV r/m,imm16
                    mrm.set(IPFB());
                    disp = IPFmrmdisplace16(/*&*/mrm);
                    v16 = IPFW();
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"MOVw %s,%04Xh",IPDecPrint16(mrm,disp,2),v16);
#endif
                    break;
                case 0xCA:
                case 0xC8: // ALIAS 8086
                    // TODO: Pull out the old IBM 5150 and check that 0xC8 is alias
                    v16 = IPFW();
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"RETFw %04Xh",v16);
#endif
                    break;
                case 0xCB:
                case 0xC9: // ALIAS 8086
                    // TODO: Pull out the old IBM 5150 and check that 0xC9 is alias
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"RETFw");
#endif
                    break;
                case 0xCC:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"INT3w");
#endif
                    break;
                case 0xCD:
                    v8 = IPFB();
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"INTw %02Xh",v8);
#endif
                    break;
                case 0xCE:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"INTOw");
#endif
                    break;
                case 0xCF:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"IRETw");
#endif
                    break;
                case 0xD0: // GRP2 r/m,1  NTS: undocumented reg==6 could be called "SAL", acts like "SHL"
                    mrm.set(IPFB());
                    disp = IPFmrmdisplace16(/*&*/mrm);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"%sb %s,1",CPUGRP2[mrm.reg()],IPDecPrint16(mrm,disp,1));
#endif
                    break;
                case 0xD1: // GRP2 r/m,1  NTS: undocumented reg==6 could be called "SAL", acts like "SHL"
                    mrm.set(IPFB());
                    disp = IPFmrmdisplace16(/*&*/mrm);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"%sw %s,1",CPUGRP2[mrm.reg()],IPDecPrint16(mrm,disp,2));
#endif
                    break;
                case 0xD2: // GRP2 r/m,CL  NTS: undocumented reg==6 could be called "SAL", acts like "SHL"
                    mrm.set(IPFB());
                    disp = IPFmrmdisplace16(/*&*/mrm);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"%sb %s,CL",CPUGRP2[mrm.reg()],IPDecPrint16(mrm,disp,1));
#endif
                    break;
                case 0xD3: // GRP2 r/m,CL  NTS: undocumented reg==6 could be called "SAL", acts like "SHL"
                    mrm.set(IPFB());
                    disp = IPFmrmdisplace16(/*&*/mrm);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"%sw %s,CL",CPUGRP2[mrm.reg()],IPDecPrint16(mrm,disp,2));
#endif
                    break;
                case 0xD4:
                    v8 = IPFB();
#ifdef DECOMPILEMODE
                    if (v8 == 0x0A)
                        w += snprintf(w,(size_t)(wf-w),"AAM");
                    else
                        w += snprintf(w,(size_t)(wf-w),"AAM %02Xh",v8);
#endif
                    break;
                case 0xD5:
                    v8 = IPFB();
#ifdef DECOMPILEMODE
                    if (v8 == 0x0A)
                        w += snprintf(w,(size_t)(wf-w),"AAD");
                    else
                        w += snprintf(w,(size_t)(wf-w),"AAD %02Xh",v8);
#endif
                    break;
                case 0xD6:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"SALCb");
#endif
                    break;
                case 0xD7:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"XLATw");
#endif
                    break;

                case 0xD9: // FPU ESCAPE + 0x1
                    mrm.set(IPFB());
                    disp = IPFmrmdisplace16(/*&*/mrm);
                    switch (mrm.byte) {
                        case 0xD0: // FNOP
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FNOP");
#endif
                            break;

                        case 0xE0: // FCHS
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FCHS");
#endif
                            break;
                        case 0xE1: // FABS
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FABS");
#endif
                            break;
                        case 0xE4: // FTST
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FTST");
#endif
                            break;
                        case 0xE5: // FXAM
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FXAM");
#endif
                            break;

                        case 0xE8: // FLD1
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FLD1");
#endif
                            break;
                        case 0xE9: // FLDL2T
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FLDL2T");
#endif
                            break;
                        case 0xEA: // FLDL2E
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FLDL2E");
#endif
                            break;
                        case 0xEB: // FLDPI
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FLDPI");
#endif
                            break;
                        case 0xEC: // FLDLG2
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FLDLG2");
#endif
                            break;
                        case 0xED: // FLDLN2
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FLDLN2");
#endif
                            break;
                        case 0xEE: // FLDZ
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FLDZ");
#endif
                            break;

                        case 0xF0: // F2XM1
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"F2XM1");
#endif
                            break;
                        case 0xF1: // FYL2X
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FYL2X");
#endif
                            break;
                        case 0xF2: // FPTAN
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FPTAN");
#endif
                            break;
                        case 0xF3: // FPATAN
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FPATAN");
#endif
                            break;
                        case 0xF4: // FXTRACT
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FXTRACT");
#endif
                            break;

                        case 0xF6: // FDECSTP
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FDECSTP");
#endif
                            break;
                        case 0xF7: // FINCSTP
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FINCSTP");
#endif
                            break;
                        case 0xF8: // FPREM
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FPREM");
#endif
                            break;
                        case 0xF9: // FYL2XP1
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FYL2XP1");
#endif
                            break;
                        case 0xFA: // FSQRT
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FSQRT");
#endif
                            break;

                        case 0xFC: // FRNDINT
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FRNDINT");
#endif
                            break;
                        case 0xFD: // FSCALE
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FSCALE");
#endif
                            break;
                        case_span_by_mod_reg(/*mod*/0,/*reg*/0): // FLD integer/real mem to ST(0)    MF == 0 32-bit real
                        case_span_by_mod_reg(/*mod*/1,/*reg*/0): // ESCAPE M F 1 | MOD 0 0 0 R/M     REG == 0 MOD == 0,1,2 RM == mem ref
                        case_span_by_mod_reg(/*mod*/2,/*reg*/0):
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FLDd %s ; MF=32-bit real",IPDecPrint16(mrm,disp,4,RC_FPUREG));
#endif
                            break;
                        case_span_by_mod_reg(/*mod*/3,/*reg*/0): // FLD ST(i) to ST(0)
                                                                 // ESCAPE 0 0 1 | 1 1 0 0 0 R/M     REG == 0 MOD == 3 RM == FPU register index
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FLD ST(%u)",mrm.rm());
#endif
                            break;

                        case_span_by_mod_reg(/*mod*/3,/*reg*/1): // FXCH ST(i) and ST(0)
                                                                 // ESCAPE 0 0 1 | 1 1 0 0 1 R/M     REG == 1 MOD == 3 RM == FPU register index
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FXCH ST(%u)",mrm.rm());
#endif
                            break;
                        case_span_by_mod_reg(/*mod*/0,/*reg*/2): // FST ST(0) to integer/real mem    MF == 0 32-bit real
                        case_span_by_mod_reg(/*mod*/1,/*reg*/2): // ESCAPE M F 1 | MOD 0 1 0 R/M     REG == 2 MOD == 0,1,2 RM == mem ref
                        case_span_by_mod_reg(/*mod*/2,/*reg*/2):
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FSTd %s ; MF=32-bit real",IPDecPrint16(mrm,disp,4,RC_FPUREG));
#endif
                            break;
                        case_span_by_mod_reg(/*mod*/0,/*reg*/3): // FSTP ST(0) to integer/real mem   MF == 0 32-bit real
                        case_span_by_mod_reg(/*mod*/1,/*reg*/3): // ESCAPE M F 1 | MOD 0 1 1 R/M     REG == 3 MOD == 0,1,2 RM == mem ref
                        case_span_by_mod_reg(/*mod*/2,/*reg*/3):
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FSTPd %s ; MF=32-bit real",IPDecPrint16(mrm,disp,4,RC_FPUREG));
#endif
                            break;
                    };
                    break;

                case 0xDB: // FPU ESCAPE + 0x3
                    mrm.set(IPFB());
                    disp = IPFmrmdisplace16(/*&*/mrm);
                    switch (mrm.byte) {
                        case_span_by_mod_reg(/*mod*/0,/*reg*/0): // FLD integer/real mem to ST(0)    MF == 1 32-bit integer
                        case_span_by_mod_reg(/*mod*/1,/*reg*/0): // ESCAPE M F 1 | MOD 0 0 0 R/M     REG == 0 MOD == 0,1,2 RM == mem ref
                        case_span_by_mod_reg(/*mod*/2,/*reg*/0):
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FLDd %s ; MF=32-bit integer",IPDecPrint16(mrm,disp,4,RC_FPUREG));
#endif
                            break;

                        case_span_by_mod_reg(/*mod*/0,/*reg*/2): // FST ST(0) to integer/real mem    MF == 1 32-bit integer
                        case_span_by_mod_reg(/*mod*/1,/*reg*/2): // ESCAPE M F 1 | MOD 0 1 0 R/M     REG == 2 MOD == 0,1,2 RM == mem ref
                        case_span_by_mod_reg(/*mod*/2,/*reg*/2):
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FSTd %s ; MF=32-bit integer",IPDecPrint16(mrm,disp,4,RC_FPUREG));
#endif
                            break;
                        case_span_by_mod_reg(/*mod*/0,/*reg*/3): // FSTP ST(0) to integer/real mem   MF == 1 32-bit integer
                        case_span_by_mod_reg(/*mod*/1,/*reg*/3): // ESCAPE M F 1 | MOD 0 1 1 R/M     REG == 2 MOD == 0,1,2 RM == mem ref
                        case_span_by_mod_reg(/*mod*/2,/*reg*/3):
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FSTPd %s ; MF=32-bit integer",IPDecPrint16(mrm,disp,4,RC_FPUREG));
#endif
                            break;

                        case_span_by_mod_reg(/*mod*/0,/*reg*/5): // FLD temporary real (80) mem to ST(0)
                        case_span_by_mod_reg(/*mod*/1,/*reg*/5): // ESCAPE 0 1 1 | MOD 1 0 1 R/M     REG == 5 MOD == 0,1,2 RM == mem ref
                        case_span_by_mod_reg(/*mod*/2,/*reg*/5):
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FLDt %s ; MF=80-bit temporary real",IPDecPrint16(mrm,disp,10,RC_FPUREG));
#endif
                            break;

                        case_span_by_mod_reg(/*mod*/0,/*reg*/7): // FSTP ST(0) to temporary real (80)
                        case_span_by_mod_reg(/*mod*/1,/*reg*/7): // ESCAPE 0 1 1 | MOD 1 1 1 R/M     REG == 7 MOD == 0,1,2 RM == mem ref
                        case_span_by_mod_reg(/*mod*/2,/*reg*/7):
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FSTPt %s ; MF=80-bit temporary real",IPDecPrint16(mrm,disp,10,RC_FPUREG));
#endif
                            break;
                    };
                    break;

                case 0xDD: // FPU ESCAPE + 0x5
                    mrm.set(IPFB());
                    disp = IPFmrmdisplace16(/*&*/mrm);
                    switch (mrm.byte) {
                        case_span_by_mod_reg(/*mod*/0,/*reg*/0): // FLD integer/real mem to ST(0)    MF == 2 64-bit real
                        case_span_by_mod_reg(/*mod*/1,/*reg*/0): // ESCAPE M F 1 | MOD 0 0 0 R/M     REG == 0 MOD == 0,1,2 RM == mem ref
                        case_span_by_mod_reg(/*mod*/2,/*reg*/0):
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FLDq %s ; MF=64-bit real",IPDecPrint16(mrm,disp,8,RC_FPUREG));
#endif
                            break;

                        case_span_by_mod_reg(/*mod*/0,/*reg*/2): // FST ST(0) to integer/real mem    MF == 2 64-bit real
                        case_span_by_mod_reg(/*mod*/1,/*reg*/2): // ESCAPE M F 1 | MOD 0 1 0 R/M     REG == 2 MOD == 0,1,2 RM == mem ref
                        case_span_by_mod_reg(/*mod*/2,/*reg*/2):
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FSTq %s ; MF=64-bit real",IPDecPrint16(mrm,disp,8,RC_FPUREG));
#endif
                            break;
                        case_span_by_mod_reg(/*mod*/3,/*reg*/2): // FST ST(0) to ST(i)
                                                                 // ESCAPE 1 0 1 | 1 1 0 1 0 R/M     REG == 2 MOD == 3 RM == FPU register index
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FST ST(%u)",mrm.rm());
#endif
                            break;
                        case_span_by_mod_reg(/*mod*/0,/*reg*/3): // FSTP ST(0) to integer/real mem   MF == 2 64-bit real
                        case_span_by_mod_reg(/*mod*/1,/*reg*/3): // ESCAPE M F 1 | MOD 0 1 1 R/M     REG == 2 MOD == 0,1,2 RM == mem ref
                        case_span_by_mod_reg(/*mod*/2,/*reg*/3):
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FSTPq %s ; MF=64-bit real",IPDecPrint16(mrm,disp,8,RC_FPUREG));
#endif
                            break;
                        case_span_by_mod_reg(/*mod*/3,/*reg*/3): // FSTP ST(0) to ST(i)
                                                                 // ESCAPE 1 0 1 | 1 1 0 1 1 R/M     REG == 3 MOD == 3 RM == FPU register index
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FSTP ST(%u)",mrm.rm());
#endif
                            break;
                    };
                    break;

                case 0xDF: // FPU ESCAPE + 0x7
                    mrm.set(IPFB());
                    disp = IPFmrmdisplace16(/*&*/mrm);
                    switch (mrm.byte) {
                        case_span_by_mod_reg(/*mod*/0,/*reg*/0): // FLD integer/real mem to ST(0)    MF == 3 16-bit integer
                        case_span_by_mod_reg(/*mod*/1,/*reg*/0): // ESCAPE M F 1 | MOD 0 0 0 R/M     REG == 0 MOD == 0,1,2 RM == mem ref
                        case_span_by_mod_reg(/*mod*/2,/*reg*/0):
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FLDw %s ; MF=16-bit integer",IPDecPrint16(mrm,disp,2,RC_FPUREG));
#endif
                            break;

                        case_span_by_mod_reg(/*mod*/0,/*reg*/2): // FST ST(0) to integer/real mem    MF == 3 16-bit integer
                        case_span_by_mod_reg(/*mod*/1,/*reg*/2): // ESCAPE M F 1 | MOD 0 1 0 R/M     REG == 2 MOD == 0,1,2 RM == mem ref
                        case_span_by_mod_reg(/*mod*/2,/*reg*/2):
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FSTw %s ; MF=16-bit integer",IPDecPrint16(mrm,disp,2,RC_FPUREG));
#endif
                            break;
                        case_span_by_mod_reg(/*mod*/0,/*reg*/3): // FSTP ST(0) to integer/real mem   MF == 3 16-bit integer
                        case_span_by_mod_reg(/*mod*/1,/*reg*/3): // ESCAPE M F 1 | MOD 0 1 0 R/M     REG == 2 MOD == 0,1,2 RM == mem ref
                        case_span_by_mod_reg(/*mod*/2,/*reg*/3):
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FSTPw %s ; MF=16-bit integer",IPDecPrint16(mrm,disp,2,RC_FPUREG));
#endif
                            break;
                        case_span_by_mod_reg(/*mod*/0,/*reg*/4): // FLD packed BCD (80) mem to ST(0)
                        case_span_by_mod_reg(/*mod*/1,/*reg*/4): // ESCAPE 1 1 1 | MOD 1 0 0 R/M     REG == 4 MOD == 0,1,2 RM == mem ref
                        case_span_by_mod_reg(/*mod*/2,/*reg*/4):
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FLDt %s ; MF=80-bit packed BCD",IPDecPrint16(mrm,disp,10,RC_FPUREG));
#endif
                            break;
                        case_span_by_mod_reg(/*mod*/0,/*reg*/5): // FLD long integer (64) mem to ST(0)
                        case_span_by_mod_reg(/*mod*/1,/*reg*/5): // ESCAPE 1 1 1 | MOD 1 0 1 R/M     REG == 5 MOD == 0,1,2 RM == mem ref
                        case_span_by_mod_reg(/*mod*/2,/*reg*/5):
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FLDq %s ; MF=64-bit integer",IPDecPrint16(mrm,disp,8,RC_FPUREG));
#endif
                            break;
                        case_span_by_mod_reg(/*mod*/0,/*reg*/6): // FSTP ST(0) to packed BCD (80)
                        case_span_by_mod_reg(/*mod*/1,/*reg*/6): // ESCAPE 1 1 1 | MOD 1 1 0 R/M     REG == 6 MOD == 0,1,2 RM == mem ref
                        case_span_by_mod_reg(/*mod*/2,/*reg*/6):
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FSTPt %s ; MF=80-bit packed BCD",IPDecPrint16(mrm,disp,10,RC_FPUREG));
#endif
                            break;
                        case_span_by_mod_reg(/*mod*/0,/*reg*/7): // FSTP ST(0) to long integer (64) mem
                        case_span_by_mod_reg(/*mod*/1,/*reg*/7): // ESCAPE 1 1 1 | MOD 1 1 1 R/M     REG == 7 MOD == 0,1,2 RM == mem ref
                        case_span_by_mod_reg(/*mod*/2,/*reg*/7):
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FSTPq %s ; MF=64-bit integer",IPDecPrint16(mrm,disp,8,RC_FPUREG));
#endif
                            break;
                    };
                    break;

                case 0xE0:
                    v16 = (uint16_t)IPFBsigned();
                    v16 = (v16 + IPval()) & 0xFFFFU;
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"LOOPNZw %04Xh",v16);
#endif
                    break;
                case 0xE1:
                    v16 = (uint16_t)IPFBsigned();
                    v16 = (v16 + IPval()) & 0xFFFFU;
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"LOOPZw %04Xh",v16);
#endif
                    break;
                case 0xE2:
                    v16 = (uint16_t)IPFBsigned();
                    v16 = (v16 + IPval()) & 0xFFFFU;
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"LOOPw %04Xh",v16);
#endif
                    break;
                case 0xE3:
                    v16 = (uint16_t)IPFBsigned();
                    v16 = (v16 + IPval()) & 0xFFFFU;
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"JCXZw %04Xh",v16);
#endif
                    break;
                case 0xE4:
                    v8 = IPFB(); // immediate port number
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"INb AL,%02Xh",v8);
#endif
                    break;
                case 0xE5:
                    v8 = IPFB(); // immediate port number
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"INw AX,%02Xh",v8);
#endif
                    break;
                case 0xE6:
                    v8 = IPFB(); // immediate port number
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"OUTb %02Xh,AL",v8);
#endif
                    break;
                case 0xE7:
                    v8 = IPFB(); // immediate port number
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"OUTw %02Xh,AX",v8);
#endif
                    break;
                case 0xE8:
                    v16 = (uint16_t)IPFWsigned();
                    v16 = (v16 + IPval()) & 0xFFFFU;
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"CALLw %04Xh",v16);
#endif
                    break;
                case 0xE9:
                    v16 = (uint16_t)IPFWsigned();
                    v16 = (v16 + IPval()) & 0xFFFFU;
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"JMPw %04Xh",v16);
#endif
                    break;
                case 0xEA:
                    v16 = IPFW();//offset
                    v16b = IPFW();//segment
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"JMPw %04Xh:%04Xh",v16b,v16);
#endif
                    break;
                case 0xEB:
                    v16 = (uint16_t)IPFBsigned();
                    v16 = (v16 + IPval()) & 0xFFFFU;
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"JMPw %04Xh",v16);
#endif
                    break;
                case 0xEC:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"INb AL,DX");
#endif
                    break;
                case 0xED:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"INw AX,DX");
#endif
                    break;
                case 0xEE:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"OUTb DX,AL");
#endif
                    break;
                case 0xEF:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"OUTw DX,AX");
#endif
                    break;
                case 0xF0:
                case 0xF1: // 8086 ALIAS [http://www.os2museum.com/wp/undocumented-8086-opcodes/]
                    // TODO: Pull out the old IBM 5150 and check that 0xF1 is alias
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"LOCK ");
#endif
                    goto after_prefix;
                case 0xF2:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"REPNZ ");
#endif
                    goto after_prefix;
                case 0xF3:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"REPZ ");
#endif
                    goto after_prefix;
                case 0xF4:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"HLT");
#endif
                    break;
                case 0xF5:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"CMC");
#endif
                    break;
                case 0xF6: // GRP3 r/m reg > 1  or GRP3 r/m,imm8 reg <= 1  reg == 1 is not defined, but x86 opcode map geek edition lists it as alias of reg == 0
                    mrm.set(IPFB());
                    disp = IPFmrmdisplace16(/*&*/mrm);
#ifdef DECOMPILEMODE
                    if (mrm.reg() <= 1) {
                        v8 = IPFB();
                        w += snprintf(w,(size_t)(wf-w),"%sb %s,%02Xh",CPUGRP3[mrm.reg()],IPDecPrint16(mrm,disp,1),v8);
                    }
                    else {
                        w += snprintf(w,(size_t)(wf-w),"%sb %s",CPUGRP3[mrm.reg()],IPDecPrint16(mrm,disp,1));
                    }
#endif
                    break;
                case 0xF7: // GRP3 r/m reg > 1  or GRP3 r/m,imm16 reg <= 1  reg == 1 is not defined, but x86 opcode map geek edition lists it as alias of reg == 0
                    mrm.set(IPFB());
                    disp = IPFmrmdisplace16(/*&*/mrm);
#ifdef DECOMPILEMODE
                    if (mrm.reg() <= 1) {
                        v16 = IPFW();
                        w += snprintf(w,(size_t)(wf-w),"%sb %s,%04Xh",CPUGRP3[mrm.reg()],IPDecPrint16(mrm,disp,2),v16);
                    }
                    else {
                        w += snprintf(w,(size_t)(wf-w),"%sb %s",CPUGRP3[mrm.reg()],IPDecPrint16(mrm,disp,2));
                    }
#endif
                    break;
                case 0xF8:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"CLC");
#endif
                    break;
                case 0xF9:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"STC");
#endif
                    break;
                case 0xFA:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"CLI");
#endif
                    break;
                case 0xFB:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"STI");
#endif
                    break;
                case 0xFC:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"CLD");
#endif
                    break;
                case 0xFD:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"STD");
#endif
                    break;
                case 0xFE:
                    // Question: If the 8086 encounters illegal encoding (reg >= 2) does it either:
                    //             a) read MRM, displacement field, then ignore instruction?
                    //             b) read MRM, then abort and ignore instruction (leaving IP at displacement field bytes)?
                    //             c) read MRM, displacement field, then execute some unknown alias of valid instructions?
                    //           Since 286 and higher have #INVD exception, this question applies only to 8086.
                    mrm.set(IPFB());
                    disp = IPFmrmdisplace16(/*&*/mrm);
#ifdef DECOMPILEMODE
                    if (mrm.reg() <= 1)
                        w += snprintf(w,(size_t)(wf-w),"%sb %s",CPUGRP4[mrm.reg()],IPDecPrint16(mrm,disp,1));
                    else // NTS: DOSBox 0.74 uses this opcode reg==7 for it's callback instruction which breaks the mod/reg/rm pattern
                        w += snprintf(w,(size_t)(wf-w),"%sb %s","(illegal)",IPDecPrint16(mrm,disp,1));
#endif
                    break;
                case 0xFF:
                    // Question: If the 8086 encounters illegal encoding (reg == 7) does it either:
                    //             a) read MRM, displacement field, then ignore instruction?
                    //             b) read MRM, then abort and ignore instruction (leaving IP at displacement field bytes)?
                    //             c) read MRM, displacement field, then execute some unknown alias of valid instructions?
                    //           Since 286 and higher have #INVD exception, this question applies only to 8086.
                    mrm.set(IPFB());
                    disp = IPFmrmdisplace16(/*&*/mrm);
#ifdef DECOMPILEMODE
                    if (mrm.reg() != 7)
                        w += snprintf(w,(size_t)(wf-w),"%sw %s",CPUGRP4[mrm.reg()],IPDecPrint16(mrm,disp,2));
                    else
                        w += snprintf(w,(size_t)(wf-w),"%sw %s","(illegal)",IPDecPrint16(mrm,disp,2));
#endif
                    break;
                default:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"(invalid opcode %02Xh)",op1);
                    break;
#else
                    goto invalidopcode;
#endif
            }
        }
        goto done;
invalidopcode:
done:
        { }
    }
}
//// END CORE

int main(int argc,char **argv) {
    const char *src_file = NULL,*arg;
    off_t file_size;
    int fd;
    int i;

    for (i=1;i < argc;) {
        arg = argv[i++];

        if (*arg == '-') {
            do { arg++; } while (*arg == '-');

            if (!strcmp(arg,"i")) {
                src_file = argv[i++];
            }
            else {
                fprintf(stderr,"Unknown sw %s\n",arg);
                return 1;
            }
        }
        else {
            fprintf(stderr,"Unexpected arg %s\n",arg);
            return 1;
        }
    }

    if (src_file == NULL) {
        fprintf(stderr,"Must specify file\n");
        return 1;
    }

    fd = open(src_file,O_RDONLY);
    if (fd < 0) {
        fprintf(stderr,"Cannot open input file\n");
        return 1;
    }
    file_size = lseek(fd,0,SEEK_END);
    if (lseek(fd,0,SEEK_SET) != (off_t)0 || file_size <= (off_t)0 || file_size > (off_t)0xFFFFU) {
        fprintf(stderr,"Bad file size\n");
        return 1;
    }

    exe_image = (unsigned char*)malloc((size_t)file_size + 32);
    if (exe_image == NULL) {
        fprintf(stderr,"Cannot alloc EXE image\n");
        return 1;
    }
    if (read(fd,exe_image,file_size) != (int)file_size) {
        fprintf(stderr,"Cannot read EXE image\n");
        return 1;
    }
    exe_image_fence = exe_image + (size_t)file_size;

    // decompile
    exe_ip = 0x100;
    exe_ip_ptr = exe_image;
    while (exe_ip_ptr < exe_image_fence) {
        unsigned char *i_ptr = exe_ip_ptr;
        unsigned int bip;

        IPDec(exe_ip);
        printf("%04X: ",IPDecIP);

        for (bip=0;bip < (exe_ip-IPDecIP);bip++)
            printf("%02X ",i_ptr[bip]);
        while (bip < 8) {
            printf("   ");
            bip++;
        }

        printf("%s\n",IPDecStr);
    }

    free(exe_image);
    exe_image = NULL;
    exe_image_fence = NULL;
    close(fd);
    return 0;
}

