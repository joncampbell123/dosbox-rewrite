
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "dosboxxr/lib/cpu/ipdec.h"

x86_offset_t            IPDecIP;
char                    IPDecStr[256];

const char *CPUregs16[8] = {
    "AX","CX","DX","BX", "SP","BP","SI","DI"
};

const char *CPUregs8[8] = {
    "AL","CL","DL","BL", "AH","CH","DH","BH"
};

const char *CPUregsZ[8] = {
    "","","","", "","","",""
};

const char **CPUregsN[5] = {
    CPUregsZ,                       // sz=0
    CPUregs8,                       // sz=1
    CPUregs16,                      // sz=2
    CPUregsZ,                       // sz=3
    CPUregsZ                        // sz=4
};

const char *CPUsregs_8086[8] = {
    "ES","CS","SS","DS","","","",""
};

const char *CPUjcc7x[16] = {
    "JO","JNO",
    "JB","JNB",
    "JZ","JNZ",
    "JBE","JA",
    "JS","JNS",
    "JPE","JPO",
    "JL","JGE",
    "JLE","JG"
};

const char *CPUGRP1[8] = {
    "ADD","OR",
    "ADC","SBB",
    "AND","SUB",
    "XOR","CMP"
};

const char *CPUGRP2[8] = {
    "ROL","ROR",
    "RCL","RCR",
    "SHL","SHR",
    "SAL","SAR"
};

const char *CPUGRP3[8] = {
    "TEST","TEST", // according to x86 opcode map geek edition, reg==1 is undefined alias of reg==0
    "NOT","NEG",
    "MUL","IMUL",
    "DIV","IDIV"
};

const char *CPUGRP4[8] = {
    "INC","DEC",
    "CALL","CALLF", // reg==2 (CALL) or higher invalid unless opcode == 0xFF
    "JMP","JMPF",
    "PUSH","???"    // reg==7 illegal
};

const char *CPUmod0displacement16[8] = {
    "BX+SI","BX+DI","BP+SI","BP+DI",
    "SI",   "DI",   "BP",   "BX"
};

const char *CPU0F01ops[8] = {
    "SGDT","SIDT",
    "LGDT","LIDT",
    "SMSW","",
    "LMSW",""
};

// print 16-bit code form of mod/reg/rm with displacement
const char *IPDecPrint16(const x86ModRegRm &mrm,const x86_offset_t ofs,const unsigned int sz,const IPDecRegClass regclass) {
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

