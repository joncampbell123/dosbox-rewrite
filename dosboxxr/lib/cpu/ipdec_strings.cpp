
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

