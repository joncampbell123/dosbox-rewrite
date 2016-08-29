
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "dosboxxr/lib/cpu/ipdec.h"

x86_offset_t            IPDecIP;
char                    IPDecStr[256];

const char *CPUregs32[8] = {
    "EAX","ECX","EDX","EBX", "ESP","EBP","ESI","EDI"
};

const char *CPUregs16[8] = {
    "AX","CX","DX","BX", "SP","BP","SI","DI"
};

const char *CPUregs8[8] = {
    "AL","CL","DL","BL", "AH","CH","DH","BH"
};

const char *CPUregsZ[8] = {
    "","","","", "","","",""
};

const char *sizesuffix[12] = {
    "", "b","w","",         // 0-3
    "d","", "", "",         // 4-7
    "q","", "t",""          // 8-11
};

const char *sizespec[12] = {
    "",                     // 0
    "byte",                 // 1
    "word",                 // 2
    "",
    "dword",                // 4
    "",
    "",
    "",
    "qword",                // 8
    "",
    "tword",                // 10
    "",
};

const char **CPUregsN[5] = {
    CPUregsZ,                       // sz=0
    CPUregs8,                       // sz=1
    CPUregs16,                      // sz=2
    CPUregsZ,                       // sz=3
    CPUregs32                       // sz=4
};

const char *CPUsregs_8086[8] = {
    "ES","CS","SS","DS","","","",""
};

const char *CPUsregs_80386[8] = {
    "ES","CS","SS","DS","FS","GS","",""
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

const char *CPUjcc0F8x[16] = {
    "JO","JNO",
    "JB","JNB",
    "JZ","JNZ",
    "JBE","JA",
    "JS","JNS",
    "JPE","JPO",
    "JL","JGE",
    "JLE","JG"
};

const char *CPUsetcc0F9x[16] = {
    "SETO","SETNO",
    "SETB","SETNB",
    "SETZ","SETNZ",
    "SETBE","SETA",
    "SETS","SETNS",
    "SETPE","SETPO",
    "SETL","SETGE",
    "SETLE","SETG"
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

const char *CPU0F00ops[8] = {
    "SLDT","STR",
    "LLDT","LTR",
    "VERR","VERW",
    "",""
};

const char *CPU0F01ops[8] = {
    "SGDT","SIDT",
    "LGDT","LIDT",
    "SMSW","",
    "LMSW",""
};

// print 16-bit code form of mod/reg/rm with displacement
const char *IPDecPrint16(const x86ModRegRm &mrm,const x86_offset_t ofs,const unsigned int sz,const IPDecRegClass regclass,const char *suffix,const IPDecMRMMode mrm_mode) {
    static char tmp[64];
    char *w=tmp,*wf=tmp+sizeof(tmp)-1;

    (void)mrm_mode; // unused

    switch (mrm.mod()) {
        case 0: // [indirect] or [displacement]
            if (mrm.rm() == 6)
                w += snprintf(w,(size_t)(wf-w),"[%04lxh]%s",(unsigned long)ofs,suffix);
            else
                w += snprintf(w,(size_t)(wf-w),"[%s]%s",CPUmod0displacement16[mrm.rm()],suffix);
            break;
        case 1: // [indirect+disp8]
            w += snprintf(w,(size_t)(wf-w),"[%s%c%02Xh]%s",CPUmod0displacement16[mrm.rm()],ofs&0x80?'-':'+',IPDec8abs((uint8_t)ofs),suffix);
            break;
        case 2: // [indirect+disp16]
            w += snprintf(w,(size_t)(wf-w),"[%s+%04Xh]%s",CPUmod0displacement16[mrm.rm()],(uint16_t)ofs,suffix);
            break;
        case 3: // register
            switch (regclass) {
                case RC_REG:
                    w += snprintf(w,(size_t)(wf-w),"%s",CPUregsN[sz][mrm.rm()]);
                    break;
                case RC_FPUREG: // Floating point registers
                    w += snprintf(w,(size_t)(wf-w),"ST(%u)",mrm.rm());
                    break;
                case RC_MMXREG: // MMX registers
                    w += snprintf(w,(size_t)(wf-w),"MM%u",mrm.rm());
                    break;
                case RC_SSEREG: // SSE registers
                    w += snprintf(w,(size_t)(wf-w),"XMM%u",mrm.rm());
                    break;
                case RC_AVXREG: // AVX registers
                    w += snprintf(w,(size_t)(wf-w),"YMM%u",mrm.rm());
                    break;
            };
            break;
    }

    return tmp;
}

const char *IPDecPrint32(const x86ModRegRm &mrm,const x86ScaleIndexBase &sib,const x86_offset_t ofs,const unsigned int sz,const IPDecRegClass regclass,const char *suffix,const IPDecMRMMode mrm_mode) {
    bool ex=false;
    static char tmp[128];
    char *w=tmp,*wf=tmp+sizeof(tmp)-1;

    if (mrm.mod() == 3) {
        switch (regclass) {
            case RC_REG:
                w += snprintf(w,(size_t)(wf-w),"%s",CPUregsN[sz][mrm.rm()]);
                break;
            case RC_FPUREG: // Floating point registers
                w += snprintf(w,(size_t)(wf-w),"ST(%u)",mrm.rm());
                break;
            case RC_MMXREG: // MMX registers
                w += snprintf(w,(size_t)(wf-w),"MM%u",mrm.rm());
                break;
            case RC_SSEREG: // SSE registers
                w += snprintf(w,(size_t)(wf-w),"XMM%u",mrm.rm());
                break;
            case RC_AVXREG: // AVX registers
                w += snprintf(w,(size_t)(wf-w),"YMM%u",mrm.rm());
                break;
        };
    }
    else {
        *w++ = '[';

        if (mrm.rm() == 4) { // Decode from SIB byte
            if (sib.index() != 4) {
                if (ex) *w++ = '+';
                ex=true;

                if (mrm_mode == MRM_VSIB_X)
                    w += snprintf(w,(size_t)(wf-w),"XMM%u",sib.index());
                else if (mrm_mode == MRM_VSIB_Y)
                    w += snprintf(w,(size_t)(wf-w),"YMM%u",sib.index());
                else
                    w += snprintf(w,(size_t)(wf-w),"%s",CPUregs32[sib.index()]);

                if (sib.scale() != 0) w += snprintf(w,(size_t)(wf-w),"*%u",1U << sib.scale());
            }

            if (mrm.mod() == 0 && sib.base() == 5) {
                // no base
            }
            else {
                if (ex) *w++ = '+';
                ex=true;
                w += snprintf(w,(size_t)(wf-w),"%s",CPUregs32[sib.base()]);
            }
        }
        else if (mrm.rm() == 5 && mrm.mod() == 0) { // direct offset
            // do nothing, we have the offset already
        }
        else {
            if (ex) *w++ = '+';
            ex=true;
            w += snprintf(w,(size_t)(wf-w),"%s",CPUregs32[mrm.rm()]);
        }

        if (ofs != 0) {
            if (mrm.mod() != 1) {
                if (ex) *w++ = '+';
                ex=true;
                w += snprintf(w,(size_t)(wf-w),"%08lxh",(unsigned long)ofs);
            }
            else {
                // disp8 mode, ofs then is assumed to be 8-bit sign extended
                if (ex) *w++ = (ofs&0x80000000)?'-':'+';
                ex=true;
                w += snprintf(w,(size_t)(wf-w),"%02lxh",(unsigned long)IPDec8abs((uint8_t)ofs));
            }
        }

        *w++ = ']';
        w += sprintf(w,"%s",suffix);
    }

    *w = 0;
    return tmp;
}

const char *IPDecPrint1632(const bool addr32,const x86ModRegRm &mrm,const x86ScaleIndexBase &sib,const x86_offset_t ofs,const unsigned int sz,const IPDecRegClass regclass,const char *suffix,const IPDecMRMMode mrm_mode) {
    if (addr32)
        return IPDecPrint32(mrm,sib,ofs,sz,regclass,suffix,mrm_mode);
    else
        return IPDecPrint16(mrm,ofs,sz,regclass,suffix,mrm_mode);
}

