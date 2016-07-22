/* code fragment. include from within execute/decompile loop */
            switch (op1=IPFB()) {
                case 0x00: // ADD r/m,reg byte size
                    IPDec386Load_MRM_SIB(/*&*/mrm,/*&*/sib,/*&*/disp,addr32);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"ADD%s %s,%s",sizesuffix[1],
                        IPDecPrint386(mrm,sib,disp,1,addr32),CPUregsN[1][mrm.reg()]);
#endif
                    break;
                case 0x01: // ADD r/m,reg word size
                    IPDec386Load_MRM_SIB(/*&*/mrm,/*&*/sib,/*&*/disp,addr32);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"ADD%s %s,%s",sizesuffix[COREWORDSIZE],
                        IPDecPrint386(mrm,sib,disp,COREWORDSIZE,addr32),CPUregsN[COREWORDSIZE][mrm.reg()]);
#endif
                    break;
                case 0x02: // ADD reg,r/m byte size
                    IPDec386Load_MRM_SIB(/*&*/mrm,/*&*/sib,/*&*/disp,addr32);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"ADD%s %s,%s",sizesuffix[1],
                        CPUregsN[1][mrm.reg()],IPDecPrint386(mrm,sib,disp,1,addr32));
#endif
                    break;
                case 0x03: // ADD reg,r/m word size
                    IPDec386Load_MRM_SIB(/*&*/mrm,/*&*/sib,/*&*/disp,addr32);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"ADD%s %s,%s",sizesuffix[COREWORDSIZE],
                        CPUregsN[COREWORDSIZE][mrm.reg()],IPDecPrint386(mrm,sib,disp,COREWORDSIZE,addr32));
#endif
                    break;
                case 0x04: // ADD AL,imm8
                    v8 = IPFB();
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"ADDb AL,%02Xh",v8);
#endif
                    break;
                case 0x05: // ADD AX,immword
                    // Your C++ compiler is smart enough to optimize out the conditional here, right?
                    // COREWORDSIZE is a #define and therefore the ternary statements are predictable and can be reduced to one or the other for 16/32-bit modes.
                    v32 = COREWORDSIZE == 4 ? IPFDW() : IPFW();
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"ADD%c %s,%08lXh",COREWORDSIZE == 4 ? 'd' : 'w',CPUregsN[COREWORDSIZE][/*AX*/0],(unsigned long)v32);
#endif
                    break;
                case 0x06:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"PUSH%s ES",sizesuffix[COREWORDSIZE]);
#endif
                    break;
                case 0x07:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"POP%s ES",sizesuffix[COREWORDSIZE]);
#endif
                    break;
                case 0x08: // OR r/m,reg byte size
                    IPDec386Load_MRM_SIB(/*&*/mrm,/*&*/sib,/*&*/disp,addr32);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"OR%s %s,%s",sizesuffix[1],
                        IPDecPrint386(mrm,sib,disp,1,addr32),CPUregsN[1][mrm.reg()]);
#endif
                    break;
                case 0x09: // OR r/m,reg word size
                    IPDec386Load_MRM_SIB(/*&*/mrm,/*&*/sib,/*&*/disp,addr32);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"OR%s %s,%s",sizesuffix[COREWORDSIZE],
                        IPDecPrint386(mrm,sib,disp,COREWORDSIZE,addr32),CPUregsN[COREWORDSIZE][mrm.reg()]);
#endif
                    break;
                case 0x0A: // OR reg,r/m byte size
                    IPDec386Load_MRM_SIB(/*&*/mrm,/*&*/sib,/*&*/disp,addr32);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"OR%s %s,%s",sizesuffix[1],
                        CPUregsN[1][mrm.reg()],IPDecPrint386(mrm,sib,disp,1,addr32));
#endif
                    break;
                case 0x0B: // OR reg,r/m word size
                    IPDec386Load_MRM_SIB(/*&*/mrm,/*&*/sib,/*&*/disp,addr32);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"OR%s %s,%s",sizesuffix[COREWORDSIZE],
                        CPUregsN[COREWORDSIZE][mrm.reg()],IPDecPrint386(mrm,sib,disp,COREWORDSIZE,addr32));
#endif
                    break;
                case 0x0C: // OR AL,imm8
                    v8 = IPFB();
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"ORb AL,%02Xh",v8);
#endif
                    break;
                case 0x0D: // OR AX,immword
                    v32 = COREWORDSIZE == 4 ? IPFDW() : IPFW();
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"OR%c %s,%08lXh",COREWORDSIZE == 4 ? 'd' : 'w',CPUregsN[COREWORDSIZE][/*AX*/0],(unsigned long)v32);
#endif
                    break;
                case 0x0E:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"PUSH%s CS",sizesuffix[COREWORDSIZE]);
#endif
                    break;
#include "dosboxxr/lib/cpu/core/intel80386/coreloop_0f.h"
                case 0x10: // ADC r/m,reg byte size
                    IPDec386Load_MRM_SIB(/*&*/mrm,/*&*/sib,/*&*/disp,addr32);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"ADC%s %s,%s",sizesuffix[1],
                        IPDecPrint386(mrm,sib,disp,1,addr32),CPUregsN[1][mrm.reg()]);
#endif
                    break;
                case 0x11: // ADC r/m,reg word size
                    IPDec386Load_MRM_SIB(/*&*/mrm,/*&*/sib,/*&*/disp,addr32);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"ADC%s %s,%s",sizesuffix[COREWORDSIZE],
                        IPDecPrint386(mrm,sib,disp,COREWORDSIZE,addr32),CPUregsN[COREWORDSIZE][mrm.reg()]);
#endif
                    break;
                case 0x12: // ADC reg,r/m byte size
                    IPDec386Load_MRM_SIB(/*&*/mrm,/*&*/sib,/*&*/disp,addr32);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"ADC%s %s,%s",sizesuffix[1],
                        CPUregsN[1][mrm.reg()],IPDecPrint386(mrm,sib,disp,1,addr32));
#endif
                    break;
                case 0x13: // ADC reg,r/m word size
                    IPDec386Load_MRM_SIB(/*&*/mrm,/*&*/sib,/*&*/disp,addr32);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"ADC%s %s,%s",sizesuffix[COREWORDSIZE],
                        CPUregsN[COREWORDSIZE][mrm.reg()],IPDecPrint386(mrm,sib,disp,COREWORDSIZE,addr32));
#endif
                    break;
                case 0x14: // ADC AL,imm8
                    v8 = IPFB();
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"ADCb AL,%02Xh",v8);
#endif
                    break;
                case 0x15: // ADC AX,immword
                    // Your C++ compiler is smart enough to optimize out the conditional here, right?
                    // COREWORDSIZE is a #define and therefore the ternary statements are predictable and can be reduced to one or the other for 16/32-bit modes.
                    v32 = COREWORDSIZE == 4 ? IPFDW() : IPFW();
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"ADC%c %s,%08lXh",COREWORDSIZE == 4 ? 'd' : 'w',CPUregsN[COREWORDSIZE][/*AX*/0],(unsigned long)v32);
#endif
                    break;
                case 0x16:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"PUSH%s SS",sizesuffix[COREWORDSIZE]);
#endif
                    break;
                case 0x17:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"POP%s SS",sizesuffix[COREWORDSIZE]);
#endif
                    break;
                case 0x18: // SBB r/m,reg byte size
                    IPDec386Load_MRM_SIB(/*&*/mrm,/*&*/sib,/*&*/disp,addr32);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"SBB%s %s,%s",sizesuffix[1],
                        IPDecPrint386(mrm,sib,disp,1,addr32),CPUregsN[1][mrm.reg()]);
#endif
                    break;
                case 0x19: // SBB r/m,reg word size
                    IPDec386Load_MRM_SIB(/*&*/mrm,/*&*/sib,/*&*/disp,addr32);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"SBB%s %s,%s",sizesuffix[COREWORDSIZE],
                        IPDecPrint386(mrm,sib,disp,COREWORDSIZE,addr32),CPUregsN[COREWORDSIZE][mrm.reg()]);
#endif
                    break;
                case 0x1A: // SBB reg,r/m byte size
                    IPDec386Load_MRM_SIB(/*&*/mrm,/*&*/sib,/*&*/disp,addr32);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"SBB%s %s,%s",sizesuffix[1],
                        CPUregsN[1][mrm.reg()],IPDecPrint386(mrm,sib,disp,1,addr32));
#endif
                    break;
                case 0x1B: // SBB reg,r/m word size
                    IPDec386Load_MRM_SIB(/*&*/mrm,/*&*/sib,/*&*/disp,addr32);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"SBB%s %s,%s",sizesuffix[COREWORDSIZE],
                        CPUregsN[COREWORDSIZE][mrm.reg()],IPDecPrint386(mrm,sib,disp,COREWORDSIZE,addr32));
#endif
                    break;
                case 0x1C: // SBB AL,imm8
                    v8 = IPFB();
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"SBBb AL,%02Xh",v8);
#endif
                    break;
                case 0x1D: // SBB AX,immword
                    v32 = COREWORDSIZE == 4 ? IPFDW() : IPFW();
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"SBB%c %s,%08lXh",COREWORDSIZE == 4 ? 'd' : 'w',CPUregsN[COREWORDSIZE][/*AX*/0],(unsigned long)v32);
#endif
                    break;
                case 0x1E:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"PUSH%s DS",sizesuffix[COREWORDSIZE]);
#endif
                    break;
                case 0x1F:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"POP%s DS",sizesuffix[COREWORDSIZE]);
#endif
                    break;
                case 0x20: // AND r/m,reg byte size
                    IPDec386Load_MRM_SIB(/*&*/mrm,/*&*/sib,/*&*/disp,addr32);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"AND%s %s,%s",sizesuffix[1],
                        IPDecPrint386(mrm,sib,disp,1,addr32),CPUregsN[1][mrm.reg()]);
#endif
                    break;
                case 0x21: // AND r/m,reg word size
                    IPDec386Load_MRM_SIB(/*&*/mrm,/*&*/sib,/*&*/disp,addr32);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"AND%s %s,%s",sizesuffix[COREWORDSIZE],
                        IPDecPrint386(mrm,sib,disp,COREWORDSIZE,addr32),CPUregsN[COREWORDSIZE][mrm.reg()]);
#endif
                    break;
                case 0x22: // AND reg,r/m byte size
                    IPDec386Load_MRM_SIB(/*&*/mrm,/*&*/sib,/*&*/disp,addr32);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"AND%s %s,%s",sizesuffix[1],
                        CPUregsN[1][mrm.reg()],IPDecPrint386(mrm,sib,disp,1,addr32));
#endif
                    break;
                case 0x23: // AND reg,r/m word size
                    IPDec386Load_MRM_SIB(/*&*/mrm,/*&*/sib,/*&*/disp,addr32);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"AND%s %s,%s",sizesuffix[COREWORDSIZE],
                        CPUregsN[COREWORDSIZE][mrm.reg()],IPDecPrint386(mrm,sib,disp,COREWORDSIZE,addr32));
#endif
                    break;
                case 0x24: // AND AL,imm8
                    v8 = IPFB();
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"ANDb AL,%02Xh",v8);
#endif
                    break;
                case 0x25: // AND AX,immword
                    // Your C++ compiler is smart enough to optimize out the conditional here, right?
                    // COREWORDSIZE is a #define and therefore the ternary statements are predictable and can be reduced to one or the other for 16/32-bit modes.
                    v32 = COREWORDSIZE == 4 ? IPFDW() : IPFW();
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"AND%c %s,%08lXh",COREWORDSIZE == 4 ? 'd' : 'w',CPUregsN[COREWORDSIZE][/*AX*/0],(unsigned long)v32);
#endif
                    break;
                case 0x26:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"ES: ");
#endif
                    goto after_prefix_COREMODE;
                case 0x27:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"DAA");
#endif
                    break;
                case 0x28: // SUB r/m,reg byte size
                    IPDec386Load_MRM_SIB(/*&*/mrm,/*&*/sib,/*&*/disp,addr32);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"SUB%s %s,%s",sizesuffix[1],
                        IPDecPrint386(mrm,sib,disp,1,addr32),CPUregsN[1][mrm.reg()]);
#endif
                    break;
                case 0x29: // SUB r/m,reg word size
                    IPDec386Load_MRM_SIB(/*&*/mrm,/*&*/sib,/*&*/disp,addr32);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"SUB%s %s,%s",sizesuffix[COREWORDSIZE],
                        IPDecPrint386(mrm,sib,disp,COREWORDSIZE,addr32),CPUregsN[COREWORDSIZE][mrm.reg()]);
#endif
                    break;
                case 0x2A: // SUB reg,r/m byte size
                    IPDec386Load_MRM_SIB(/*&*/mrm,/*&*/sib,/*&*/disp,addr32);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"SUB%s %s,%s",sizesuffix[1],
                        CPUregsN[1][mrm.reg()],IPDecPrint386(mrm,sib,disp,1,addr32));
#endif
                    break;
                case 0x2B: // SUB reg,r/m word size
                    IPDec386Load_MRM_SIB(/*&*/mrm,/*&*/sib,/*&*/disp,addr32);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"SUB%s %s,%s",sizesuffix[COREWORDSIZE],
                        CPUregsN[COREWORDSIZE][mrm.reg()],IPDecPrint386(mrm,sib,disp,COREWORDSIZE,addr32));
#endif
                    break;
                case 0x2C: // SUB AL,imm8
                    v8 = IPFB();
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"SUBb AL,%02Xh",v8);
#endif
                    break;
                case 0x2D: // SUB AX,immword
                    // Your C++ compiler is smart enough to optimize out the conditional here, right?
                    // COREWORDSIZE is a #define and therefore the ternary statements are predictable and can be reduced to one or the other for 16/32-bit modes.
                    v32 = COREWORDSIZE == 4 ? IPFDW() : IPFW();
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"SUB%c %s,%08lXh",COREWORDSIZE == 4 ? 'd' : 'w',CPUregsN[COREWORDSIZE][/*AX*/0],(unsigned long)v32);
#endif
                    break;
                case 0x2E:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"CS: ");
#endif
                    goto after_prefix_COREMODE;
                case 0x2F:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"DAS");
#endif
                    break;
                case 0x30: // XOR r/m,reg byte size
                    IPDec386Load_MRM_SIB(/*&*/mrm,/*&*/sib,/*&*/disp,addr32);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"XOR%s %s,%s",sizesuffix[1],
                        IPDecPrint386(mrm,sib,disp,1,addr32),CPUregsN[1][mrm.reg()]);
#endif
                    break;
                case 0x31: // XOR r/m,reg word size
                    IPDec386Load_MRM_SIB(/*&*/mrm,/*&*/sib,/*&*/disp,addr32);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"XOR%s %s,%s",sizesuffix[COREWORDSIZE],
                        IPDecPrint386(mrm,sib,disp,COREWORDSIZE,addr32),CPUregsN[COREWORDSIZE][mrm.reg()]);
#endif
                    break;
                case 0x32: // XOR reg,r/m byte size
                    IPDec386Load_MRM_SIB(/*&*/mrm,/*&*/sib,/*&*/disp,addr32);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"XOR%s %s,%s",sizesuffix[1],
                        CPUregsN[1][mrm.reg()],IPDecPrint386(mrm,sib,disp,1,addr32));
#endif
                    break;
                case 0x33: // XOR reg,r/m word size
                    IPDec386Load_MRM_SIB(/*&*/mrm,/*&*/sib,/*&*/disp,addr32);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"XOR%s %s,%s",sizesuffix[COREWORDSIZE],
                        CPUregsN[COREWORDSIZE][mrm.reg()],IPDecPrint386(mrm,sib,disp,COREWORDSIZE,addr32));
#endif
                    break;
                case 0x34: // XOR AL,imm8
                    v8 = IPFB();
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"XORb AL,%02Xh",v8);
#endif
                    break;
                case 0x35: // XOR AX,immword
                    // Your C++ compiler is smart enough to optimize out the conditional here, right?
                    // COREWORDSIZE is a #define and therefore the ternary statements are predictable and can be reduced to one or the other for 16/32-bit modes.
                    v32 = COREWORDSIZE == 4 ? IPFDW() : IPFW();
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"XOR%c %s,%08lXh",COREWORDSIZE == 4 ? 'd' : 'w',CPUregsN[COREWORDSIZE][/*AX*/0],(unsigned long)v32);
#endif
                    break;
                case 0x36:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"SS: ");
#endif
                    goto after_prefix_COREMODE;
                case 0x37:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"AAA");
#endif
                    break;
                case 0x38: // CMP r/m,reg byte size
                    IPDec386Load_MRM_SIB(/*&*/mrm,/*&*/sib,/*&*/disp,addr32);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"CMP%s %s,%s",sizesuffix[1],
                        IPDecPrint386(mrm,sib,disp,1,addr32),CPUregsN[1][mrm.reg()]);
#endif
                    break;
                case 0x39: // CMP r/m,reg word size
                    IPDec386Load_MRM_SIB(/*&*/mrm,/*&*/sib,/*&*/disp,addr32);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"CMP%s %s,%s",sizesuffix[COREWORDSIZE],
                        IPDecPrint386(mrm,sib,disp,COREWORDSIZE,addr32),CPUregsN[COREWORDSIZE][mrm.reg()]);
#endif
                    break;
                case 0x3A: // CMP reg,r/m byte size
                    IPDec386Load_MRM_SIB(/*&*/mrm,/*&*/sib,/*&*/disp,addr32);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"CMP%s %s,%s",sizesuffix[1],
                        CPUregsN[1][mrm.reg()],IPDecPrint386(mrm,sib,disp,1,addr32));
#endif
                    break;
                case 0x3B: // CMP reg,r/m word size
                    IPDec386Load_MRM_SIB(/*&*/mrm,/*&*/sib,/*&*/disp,addr32);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"CMP%s %s,%s",sizesuffix[COREWORDSIZE],
                        CPUregsN[COREWORDSIZE][mrm.reg()],IPDecPrint386(mrm,sib,disp,COREWORDSIZE,addr32));
#endif
                    break;
                case 0x3C: // CMP AL,imm8
                    v8 = IPFB();
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"CMPb AL,%02Xh",v8);
#endif
                    break;
                case 0x3D: // CMP AX,immword
                    // Your C++ compiler is smart enough to optimize out the conditional here, right?
                    // COREWORDSIZE is a #define and therefore the ternary statements are predictable and can be reduced to one or the other for 16/32-bit modes.
                    v32 = COREWORDSIZE == 4 ? IPFDW() : IPFW();
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"CMP%c %s,%08lXh",COREWORDSIZE == 4 ? 'd' : 'w',CPUregsN[COREWORDSIZE][/*AX*/0],(unsigned long)v32);
#endif
                    break;
                case 0x3E:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"DS: ");
#endif
                    goto after_prefix_COREMODE;
                case 0x3F:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"AAS");
#endif
                    break;
                case_span_8(0x40): // 0x40-0x47
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"INC%s %s",sizesuffix[COREWORDSIZE],CPUregsN[COREWORDSIZE][op1&7]);
#endif
                    break;
                case_span_8(0x48): // 0x48-0x4F
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"DEC%s %s",sizesuffix[COREWORDSIZE],CPUregsN[COREWORDSIZE][op1&7]);
#endif
                    break;
                case_span_8(0x50): // 0x50-0x57
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"PUSH%s %s",sizesuffix[COREWORDSIZE],CPUregsN[COREWORDSIZE][op1&7]);
#endif
                    break;
                case_span_8(0x58): // 0x58-0x5F
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"POP%s %s",sizesuffix[COREWORDSIZE],CPUregsN[COREWORDSIZE][op1&7]);
#endif
                    break;
                case 0x60: // PUSHA
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"PUSHA%s",sizesuffix[COREWORDSIZE]);
#endif
                    break;
                case 0x61: // POPA
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"POPA%s",sizesuffix[COREWORDSIZE]);
#endif
                    break;
                case 0x62: // BOUND reg,r/m
                    IPDec386Load_MRM_SIB(/*&*/mrm,/*&*/sib,/*&*/disp,addr32);
                    if (mrm.mod() != 3) {
#ifdef DECOMPILEMODE
                        w += snprintf(w,(size_t)(wf-w),"BOUND%s %s,%s",sizesuffix[COREWORDSIZE],
                            IPDecPrint386(mrm,sib,disp,COREWORDSIZE,addr32),CPUregsN[COREWORDSIZE][mrm.reg()]);
#endif
                    }
                    else {
                        goto invalidopcode;
                    }
                    break;
                case 0x63: // ARPL r/m,reg. reg and memory ref are 16-bit only.
                    IPDec386Load_MRM_SIB(/*&*/mrm,/*&*/sib,/*&*/disp,addr32);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"ARPL%s %s,%s",sizesuffix[COREWORDSIZE],
                        IPDecPrint16(mrm,disp,2),CPUregs16[mrm.reg()]);
#endif
                    break;

                case 0x66: // operand size prefix (16->32 or 32->16 toggle)
                    opcode32 = !opcode32;
                    pre66 = !pre66;
                    goto after_prefix_COREMODE_opsizechg;
                case 0x67: // address size prefix (16->32 or 32->16 toggle)
                    addr32 = !addr32;
                    pre67 = !pre67;
                    goto after_prefix_COREMODE;
                case 0x68: // PUSH imm<word>
                    v32 = COREWORDSIZE == 4 ? IPFDW() : IPFW();
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"PUSH%s %08Xh",sizesuffix[COREWORDSIZE],v32);
#endif
                    break;
                case 0x69: // IMUL reg,r/m,imm16
                    IPDec386Load_MRM_SIB(/*&*/mrm,/*&*/sib,/*&*/disp,addr32);
                    v32 = COREWORDSIZE == 4 ? IPFDW() : IPFW();
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"IMUL%s %s,%s,%08Xh ; 1st = 2nd * 3rd",sizesuffix[COREWORDSIZE],
                        CPUregsN[COREWORDSIZE][mrm.reg()],IPDecPrint386(mrm,sib,disp,COREWORDSIZE,addr32),v32);
#endif
                    break;
                case 0x6A: // PUSH imm8
                    v32 = IPFBsigned();
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"PUSH%s %c%02Xh",sizesuffix[COREWORDSIZE],v32&0x80?'-':'+',IPDec8abs((uint8_t)v32));
#endif
                    break;
                case 0x6B: // IMUL reg,r/m,imm8
                    IPDec386Load_MRM_SIB(/*&*/mrm,/*&*/sib,/*&*/disp,addr32);
                    v8 = IPFB();
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"IMUL%s %s,%s,%02Xh ; 1st = 2nd * 3rd",sizesuffix[COREWORDSIZE],
                        CPUregsN[COREWORDSIZE][mrm.reg()],IPDecPrint386(mrm,sib,disp,COREWORDSIZE,addr32),v8);
#endif
                    break;
                case 0x6C: // INSB
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"INS%s",sizesuffix[1]);
#endif
                    break;
                case 0x6D: // INSW
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"INS%s",sizesuffix[COREWORDSIZE]);
#endif
                    break;
                case 0x6E: // OUTSB
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"OUTS%s",sizesuffix[1]);
#endif
                    break;
                case 0x6F: // OUTSW
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"OUTS%s",sizesuffix[COREWORDSIZE]);
#endif
                    break;
                case_span_16(0x70): // 0x70-0x7F
                    v32 = ((uint32_t)IPFBsigned() + (uint32_t)IPval()) & (uint32_t)COREOPMASK;
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"%sd %08lXh",CPUjcc7x[op1&15],(unsigned long)v32);
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
                    IPDec386Load_MRM_SIB(/*&*/mrm,/*&*/sib,/*&*/disp,addr32);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"TEST%s %s,%s",sizesuffix[1],
                        IPDecPrint386(mrm,sib,disp,1,addr32),CPUregsN[1][mrm.reg()]);
#endif
                    break;
                case 0x85: // TEST r/m,reg word size
                    IPDec386Load_MRM_SIB(/*&*/mrm,/*&*/sib,/*&*/disp,addr32);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"TEST%s %s,%s",sizesuffix[COREWORDSIZE],
                        IPDecPrint386(mrm,sib,disp,COREWORDSIZE,addr32),CPUregsN[COREWORDSIZE][mrm.reg()]);
#endif
                    break;
                case 0x86: // XCHG r/m,reg byte size
                    IPDec386Load_MRM_SIB(/*&*/mrm,/*&*/sib,/*&*/disp,addr32);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"XCHG%s %s,%s",sizesuffix[1],
                        CPUregsN[1][mrm.reg()],IPDecPrint386(mrm,sib,disp,1,addr32));
#endif
                    break;
                case 0x87: // XCHG r/m,reg word size
                    IPDec386Load_MRM_SIB(/*&*/mrm,/*&*/sib,/*&*/disp,addr32);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"XCHG%s %s,%s",sizesuffix[COREWORDSIZE],
                        CPUregsN[COREWORDSIZE][mrm.reg()],IPDecPrint386(mrm,sib,disp,COREWORDSIZE,addr32));
#endif
                    break;
                case 0x88: // MOV r/m,reg byte size
                    IPDec386Load_MRM_SIB(/*&*/mrm,/*&*/sib,/*&*/disp,addr32);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"MOV%s %s,%s",sizesuffix[1],
                        IPDecPrint386(mrm,sib,disp,1,addr32),CPUregsN[1][mrm.reg()]);
#endif
                    break;
                case 0x89: // MOV r/m,reg word size
                    IPDec386Load_MRM_SIB(/*&*/mrm,/*&*/sib,/*&*/disp,addr32);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"MOV%s %s,%s",sizesuffix[COREWORDSIZE],
                        IPDecPrint386(mrm,sib,disp,COREWORDSIZE,addr32),CPUregsN[COREWORDSIZE][mrm.reg()]);
#endif
                    break;
                case 0x8A: // MOV reg,r/m byte size
                    IPDec386Load_MRM_SIB(/*&*/mrm,/*&*/sib,/*&*/disp,addr32);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"MOV%s %s,%s",sizesuffix[1],
                        CPUregsN[1][mrm.reg()],IPDecPrint386(mrm,sib,disp,1,addr32));
#endif
                    break;
                case 0x8B: // MOV reg,r/m word size
                    IPDec386Load_MRM_SIB(/*&*/mrm,/*&*/sib,/*&*/disp,addr32);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"MOV%s %s,%s",sizesuffix[COREWORDSIZE],
                        CPUregsN[COREWORDSIZE][mrm.reg()],IPDecPrint386(mrm,sib,disp,COREWORDSIZE,addr32));
#endif
                    break;
                case 0x8C: // MOV r/m,sreg word size
                    mrm.set(IPFB());
                    disp = IPFmrmdisplace16(/*&*/mrm);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"MOVw %s,%s",IPDecPrint16(mrm,disp,2),CPUsregs_8086[mrm.reg()]);
#endif
                    break;
                case 0x8D: // LEA reg,r/m word size
                    IPDec386Load_MRM_SIB(/*&*/mrm,/*&*/sib,/*&*/disp,addr32);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"LEA%s %s,%s",sizesuffix[COREWORDSIZE],
                        CPUregsN[COREWORDSIZE][mrm.reg()],IPDecPrint386(mrm,sib,disp,COREWORDSIZE,addr32));
#endif
                    break;
                case 0x8E: // MOV sreg,r/m word size
                    mrm.set(IPFB());
                    disp = IPFmrmdisplace16(/*&*/mrm);
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"MOVw %s,%s",CPUsregs_8086[mrm.reg()],IPDecPrint16(mrm,disp,2));
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
                    w += snprintf(w,(size_t)(wf-w),"XCHG%s %s,%s",sizesuffix[COREWORDSIZE],
                        CPUregsN[COREWORDSIZE][op1&7],CPUregsN[COREWORDSIZE][0/*AX*/]);
#endif
                    break;
                case 0x98:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"CBWb");
#endif
                    break;
                case 0x99:
#ifdef DECOMPILEMODE
                    if (COREWORDSIZE == 4)
                        w += snprintf(w,(size_t)(wf-w),"CDQd");
                    else
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
                    if (COREWORDSIZE == 4)
                        w += snprintf(w,(size_t)(wf-w),"PUSHFd");
                    else
                        w += snprintf(w,(size_t)(wf-w),"PUSHFw");
#endif
                    break;
                case 0x9D:
#ifdef DECOMPILEMODE
                    if (COREWORDSIZE == 4)
                        w += snprintf(w,(size_t)(wf-w),"POPFd");
                    else
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
                    w += snprintf(w,(size_t)(wf-w),"MOVSb");
#endif
                    break;
                case 0xA5:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"MOVS%s",sizesuffix[COREWORDSIZE]);
#endif
                    break;
                case 0xA6:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"CMPSb");
#endif
                    break;
                case 0xA7:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"CMPS%s",sizesuffix[COREWORDSIZE]);
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
                    w += snprintf(w,(size_t)(wf-w),"STOSb");
#endif
                    break;
                case 0xAB:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"STOS%s",sizesuffix[COREWORDSIZE]);
#endif
                    break;
                case 0xAC:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"LODSb");
#endif
                    break;
                case 0xAD:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"LODS%s",sizesuffix[COREWORDSIZE]);
#endif
                    break;
                case 0xAE:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"SCASb");
#endif
                    break;
                case 0xAF:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"SCAS%s",sizesuffix[COREWORDSIZE]);
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
                case 0xC0: // GRP2 r/m,imm8  NTS: undocumented reg==6 could be called "SAL", acts like "SHL"
                    mrm.set(IPFB());
                    disp = IPFmrmdisplace16(/*&*/mrm);
                    v8 = IPFB();
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"%sb %s,%02Xh",CPUGRP2[mrm.reg()],IPDecPrint16(mrm,disp,1),v8);
#endif
                    break;
                case 0xC1: // GRP2 r/m,imm8  NTS: undocumented reg==6 could be called "SAL", acts like "SHL"
                    mrm.set(IPFB());
                    disp = IPFmrmdisplace16(/*&*/mrm);
                    v8 = IPFB();
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"%sw %s,%02Xh",CPUGRP2[mrm.reg()],IPDecPrint16(mrm,disp,2),v8);
#endif
                    break;
                case 0xC2:
                    v16 = IPFW();
#ifdef DECOMPILEMODE
                    if (COREWORDSIZE == 4)
                        w += snprintf(w,(size_t)(wf-w),"RETd %04Xh",v16);
                    else
                        w += snprintf(w,(size_t)(wf-w),"RETw %04Xh",v16);
#endif
                    break;
                case 0xC3:
#ifdef DECOMPILEMODE
                    if (COREWORDSIZE == 4)
                        w += snprintf(w,(size_t)(wf-w),"RETd");
                    else
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
                case 0xC8:
                    v16 = IPFW();
                    v8 = IPFB();
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"ENTERw %04Xh,%02Xh",v16,v8);
#endif
                    break;
                case 0xC9:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"LEAVEw");
#endif
                    break;
                case 0xCA:
                    v16 = IPFW();
#ifdef DECOMPILEMODE
                    if (COREWORDSIZE == 4)
                        w += snprintf(w,(size_t)(wf-w),"RETFd %04Xh",v16);
                    else
                        w += snprintf(w,(size_t)(wf-w),"RETFw %04Xh",v16);
#endif
                    break;
                case 0xCB:
#ifdef DECOMPILEMODE
                    if (COREWORDSIZE == 4)
                        w += snprintf(w,(size_t)(wf-w),"RETFd");
                    else
                        w += snprintf(w,(size_t)(wf-w),"RETFw");
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
                    w += snprintf(w,(size_t)(wf-w),"INT %02Xh",v8);
#endif
                    break;
                case 0xCE:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"INTO");
#endif
                    break;
                case 0xCF:
#ifdef DECOMPILEMODE
                    if (COREWORDSIZE == 4)
                        w += snprintf(w,(size_t)(wf-w),"IRETd");
                    else
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
                    w += snprintf(w,(size_t)(wf-w),"XLATB%s",sizesuffix[addr32?4:2]);
#endif
                    break;
#include "dosboxxr/lib/cpu/core/intel80386/coreloop_fpu.h"
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
                    if (COREWORDSIZE == 4)
                        w += snprintf(w,(size_t)(wf-w),"INd EAX,%02Xh",v8);
                    else
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
                    if (COREWORDSIZE == 4)
                        w += snprintf(w,(size_t)(wf-w),"OUTd %02Xh,EAX",v8);
                    else
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
                    if (COREWORDSIZE == 4) {
                        v32 = (uint32_t)IPFBsigned();
                        v32 = (v32 + IPval()) & 0xFFFFFFFFU;
#ifdef DECOMPILEMODE
                        w += snprintf(w,(size_t)(wf-w),"JMPd %08lXh",(unsigned long)v32);
#endif
                    }
                    else {
                        v16 = (uint16_t)IPFBsigned();
                        v16 = (v16 + IPval()) & 0xFFFFU;
#ifdef DECOMPILEMODE
                        w += snprintf(w,(size_t)(wf-w),"JMPw %04Xh",v16);
#endif
                    }
                    break;
                case 0xEC:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"INb AL,DX");
#endif
                    break;
                case 0xED:
#ifdef DECOMPILEMODE
                    if (COREWORDSIZE == 4)
                        w += snprintf(w,(size_t)(wf-w),"INd EAX,DX");
                    else
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
                    if (COREWORDSIZE == 4)
                        w += snprintf(w,(size_t)(wf-w),"OUTd DX,EAX");
                    else
                        w += snprintf(w,(size_t)(wf-w),"OUTw DX,AX");
#endif
                    break;
                case 0xF0:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"LOCK ");
#endif
                    goto after_prefix_COREMODE;
                case 0xF1:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"INT1/ICEBP");
#endif
                    break;
                case 0xF2:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"REPNZ ");
#endif
                    goto after_prefix_COREMODE;
                case 0xF3:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"REPZ ");
#endif
                    goto after_prefix_COREMODE;
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
                    mrm.set(IPFB());
                    disp = IPFmrmdisplace16(/*&*/mrm);
#ifdef DECOMPILEMODE
                    if (mrm.reg() <= 1)
                        w += snprintf(w,(size_t)(wf-w),"%sb %s",CPUGRP4[mrm.reg()],IPDecPrint16(mrm,disp,1));
                    else
                        goto invalidopcode;
#endif
                    break;
                case 0xFF:
                    mrm.set(IPFB());
                    disp = IPFmrmdisplace16(/*&*/mrm);
#ifdef DECOMPILEMODE
                    if (mrm.reg() != 7)
                        w += snprintf(w,(size_t)(wf-w),"%sw %s",CPUGRP4[mrm.reg()],IPDecPrint16(mrm,disp,2));
                    else
                        goto invalidopcode;
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

