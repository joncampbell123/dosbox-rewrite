
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

static const char *CPUsregs[4] = {
    "ES","CS","SS","DS"
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

// print 16-bit code form of mod/reg/rm with displacement
const char *IPDecPrint16(const x86ModRegRm &mrm,const x86_offset_t ofs,const unsigned int sz) {
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
            w += snprintf(w,(size_t)(wf-w),"[%s%c%02xh]",CPUmod0displacement16[mrm.rm()],ofs&0x80?'-':'+',IPDec8abs((uint8_t)ofs));
            break;
        case 2: // [indirect+disp16]
            w += snprintf(w,(size_t)(wf-w),"[%s+%04xh]",CPUmod0displacement16[mrm.rm()],(uint16_t)ofs);
            break;
        case 3: // register
            w += snprintf(w,(size_t)(wf-w),"%s",CPUregsN[sz][mrm.rm()]);
            break;
    }

    return tmp;
}

void IPDec(x86_offset_t ip) {
    char *w = IPDecStr,*wf = IPDecStr+sizeof(IPDecStr)-1;
    x86_offset_t disp;
    x86ModRegRm mrm;
    uint8_t op1,v8;
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
                    w += snprintf(w,(size_t)(wf-w),"ADDb AL,%02xh",v8);
#endif
                    break;
                case 0x05: // ADD AX,imm16
                    v16 = IPFW();
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"ADDw AX,%04xh",v16);
#endif
                    break;
                case 0x06:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"PUSH ES");
#endif
                    break;
                case 0x07:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"POP ES");
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
                    w += snprintf(w,(size_t)(wf-w),"ORb AL,%02xh",v8);
#endif
                    break;
                case 0x0D: // OR AX,imm16
                    v16 = IPFW();
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"ORw AX,%04xh",v16);
#endif
                    break;
                case 0x0E:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"PUSH CS");
#endif
                    break;
                case 0x0F: // 8086 only!
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"POP CS");
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
                    w += snprintf(w,(size_t)(wf-w),"ADCb AL,%02xh",v8);
#endif
                    break;
                case 0x15: // ADC AX,imm16
                    v16 = IPFW();
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"ADCw AX,%04xh",v16);
#endif
                    break;
                case 0x16:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"PUSH SS");
#endif
                    break;
                case 0x17:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"POP SS");
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
                    w += snprintf(w,(size_t)(wf-w),"SBBb AL,%02xh",v8);
#endif
                    break;
                case 0x1D: // SBB AX,imm16
                    v16 = IPFW();
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"SBBw AX,%04xh",v16);
#endif
                    break;
                case 0x1E:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"PUSH DS");
#endif
                    break;
                case 0x1F:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"POP DS");
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
                    w += snprintf(w,(size_t)(wf-w),"ANDb AL,%02xh",v8);
#endif
                    break;
                case 0x25: // AND AX,imm16
                    v16 = IPFW();
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"ANDw AX,%04xh",v16);
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
                    w += snprintf(w,(size_t)(wf-w),"SUBb AL,%02xh",v8);
#endif
                    break;
                case 0x2D: // SUB AX,imm16
                    v16 = IPFW();
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"SUBw AX,%04xh",v16);
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
                    w += snprintf(w,(size_t)(wf-w),"XORb AL,%02xh",v8);
#endif
                    break;
                case 0x35: // XOR AX,imm16
                    v16 = IPFW();
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"XORw AX,%04xh",v16);
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
                    w += snprintf(w,(size_t)(wf-w),"CMPb AL,%02xh",v8);
#endif
                    break;
                case 0x3D: // CMP AX,imm16
                    v16 = IPFW();
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"CMPw AX,%04xh",v16);
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

                case 0x40: case 0x41: case 0x42: case 0x43:
                case 0x44: case 0x45: case 0x46: case 0x47:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"INC %s",CPUregs16[op1&7]);
#endif
                    break;

                case 0x48: case 0x49: case 0x4A: case 0x4B:
                case 0x4C: case 0x4D: case 0x4E: case 0x4F:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"DEC %s",CPUregs16[op1&7]);
#endif
                    break;

                case 0x50: case 0x51: case 0x52: case 0x53:
                case 0x54: case 0x55: case 0x56: case 0x57:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"PUSH %s",CPUregs16[op1&7]);
#endif
                    break;

                case 0x58: case 0x59: case 0x5A: case 0x5B:
                case 0x5C: case 0x5D: case 0x5E: case 0x5F:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"POP %s",CPUregs16[op1&7]);
#endif
                    break;

                case 0x70: case 0x71: case 0x72: case 0x73:
                case 0x74: case 0x75: case 0x76: case 0x77:
                case 0x78: case 0x79: case 0x7A: case 0x7B:
                case 0x7C: case 0x7D: case 0x7E: case 0x7F:
                    v16 = (uint16_t)IPFBsigned();
                    v16 = (v16 + IPval()) & 0xFFFFU;
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"%s %04xh",CPUjcc7x[op1&15],v16);
#endif
                    break;

                case 0x90:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"NOP");
#endif
                    break;

                case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"XCHG %s,AX",CPUregs16[op1&7]);
#endif
                    break;

                case 0x98:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"CBW");
#endif
                    break;

                case 0x99:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"CWD");
#endif
                    break;

                case 0x9B:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"WAIT");
#endif
                    break;

                case 0x9C:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"PUSHF");
#endif
                    break;

                case 0x9D:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"POPF");
#endif
                    break;

                case 0x9E:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"SAHF");
#endif
                    break;

                case 0x9F:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"LAHF");
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

                case 0xEB:
                    v8 = IPFB();
                    v16 = (IPval() + ((int8_t)v8)) & 0xFFFFU; /* need to sign-extend the byte. offset relative to first byte after Jcc instruction */
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"JMP %04xh",v16);
#endif
                    break;

                case 0xC2:
                    v16 = IPFW();
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"RET %04xh",v16);
#endif
                    break;

                case 0xC3:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"RET");
#endif
                    break;

                case 0xCA:
                    v16 = IPFW();
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"RETF %04xh",v16);
#endif
                    break;

                case 0xCB:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"RETF");
#endif
                    break;

                case 0xCC:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"INT3");
#endif
                    break;

                case 0xCD:
                    v8 = IPFB();
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"INT %02xh",v8);
#endif
                    break;

                case 0xCF:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"IRET");
#endif
                    break;

                case 0xD4:
                    v8 = IPFB();
#ifdef DECOMPILEMODE
                    if (v8 == 0x0A)
                        w += snprintf(w,(size_t)(wf-w),"AAM");
                    else
                        w += snprintf(w,(size_t)(wf-w),"AAM %02xh",v8);
#endif
                    break;

                case 0xD5:
                    v8 = IPFB();
#ifdef DECOMPILEMODE
                    if (v8 == 0x0A)
                        w += snprintf(w,(size_t)(wf-w),"AAD");
                    else
                        w += snprintf(w,(size_t)(wf-w),"AAD %02xh",v8);
#endif
                    break;

                case 0xD7:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"XLAT");
#endif
                    break;

                case 0xE4:
                    v8 = IPFB(); // immediate port number
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"IN AL,%02xh",v8);
#endif
                    break;

                case 0xE5:
                    v8 = IPFB(); // immediate port number
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"IN AX,%02xh",v8);
#endif
                    break;

                case 0xE6:
                    v8 = IPFB(); // immediate port number
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"OUT %02xh,AL",v8);
#endif
                    break;

                case 0xE7:
                    v8 = IPFB(); // immediate port number
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"OUT %02xh,AX",v8);
#endif
                    break;

                case 0xEC:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"IN AL,DX");
#endif
                    break;

                case 0xED:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"IN AX,DX");
#endif
                    break;

                case 0xEE:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"OUT DX,AL");
#endif
                    break;

                case 0xEF:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"OUT DX,AX");
#endif
                    break;

                case 0xF0:
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

                default:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"(invalid opcode %02xh)",op1);
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
        printf("%04x: ",IPDecIP);

        for (bip=0;bip < (exe_ip-IPDecIP);bip++)
            printf("%02x ",i_ptr[bip]);
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

