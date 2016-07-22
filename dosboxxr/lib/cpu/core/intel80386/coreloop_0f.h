/* sub-sub fragment for 286 0F prefix */
                case 0x0F: // 0x0F prefix
                    switch (op0F=IPFB()) {
                        case 0x00: // SLDT,STR,LLDT,LTR,VERR,VERW,#UD,#UD
                            mrm.set(IPFB());
                            disp = IPFmrmdisplace16(/*&*/mrm);
#ifdef DECOMPILEMODE
                            if (mrm.mod() != 3 && mrm.reg() <= 5)
                                w += snprintf(w,(size_t)(wf-w),"%sw %s",CPU0F00ops[mrm.reg()],IPDecPrint16(mrm,disp,2));
                            else
                                goto invalidopcode;
#endif
                            break;
                        case 0x01: // LGDT,SGDT,LIDT,SGDT,SMSW,#UD,LMSW,#UD
                            mrm.set(IPFB());
                            disp = IPFmrmdisplace16(/*&*/mrm);
#ifdef DECOMPILEMODE
                            if (mrm.mod() != 3 && !(mrm.reg() == 5 || mrm.reg() == 7))
                                w += snprintf(w,(size_t)(wf-w),"%sw %s",CPU0F01ops[mrm.reg()],IPDecPrint16(mrm,disp,2));
                            else
                                goto invalidopcode;
#endif
                            break;
                        case 0x02: // LAR
                            mrm.set(IPFB());
                            disp = IPFmrmdisplace16(/*&*/mrm);
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"LARw %s,%s",CPUregs16[mrm.reg()],IPDecPrint16(mrm,disp,2));
#endif
                            break;
                        case 0x03: // LSL
                            mrm.set(IPFB());
                            disp = IPFmrmdisplace16(/*&*/mrm);
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"LSLw %s,%s",CPUregs16[mrm.reg()],IPDecPrint16(mrm,disp,2));
#endif
                            break;

                        case 0x06: // CLTS (NTS: Intel 80286 datasheet calls this CTS for some reason)
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"CLTS");
#endif
                            break;
                        case 0x07: // LOADALL (386)
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"LOADALL386");
#endif
                            break;

                        case 0x20: // MOV from control registers (i.e. MOV EAX,CR0). Only the register form is legal, not from/to memory. Register is ALWAYS 32-bit.
                            mrm.set(IPFB());
                            if (mrm.mod() == 3)
                                w += snprintf(w,(size_t)(wf-w),"MOVd %s,CR%u",CPUregs32[mrm.rm()],mrm.reg());
                            else
                                goto invalidopcode;
                            break;
                        case 0x21: // MOV from debug registers (i.e. MOV EAX,DR0). Only the register form is legal, not from/to memory. Register is ALWAYS 32-bit.
                            mrm.set(IPFB());
                            if (mrm.mod() == 3)
                                w += snprintf(w,(size_t)(wf-w),"MOVd %s,DR%u",CPUregs32[mrm.rm()],mrm.reg());
                            else
                                goto invalidopcode;
                            break;
                        case 0x22: // MOV to control registers (i.e. MOV CR0,EAX). Only the register form is legal, not from/to memory. Register is ALWAYS 32-bit.
                            mrm.set(IPFB());
                            if (mrm.mod() == 3)
                                w += snprintf(w,(size_t)(wf-w),"MOVd CR%u,%s",mrm.reg(),CPUregs32[mrm.rm()]);
                            else
                                goto invalidopcode;
                            break;
                        case 0x23: // MOV to control registers (i.e. MOV DR0,EAX). Only the register form is legal, not from/to memory. Register is ALWAYS 32-bit.
                            mrm.set(IPFB());
                            if (mrm.mod() == 3)
                                w += snprintf(w,(size_t)(wf-w),"MOVd DR%u,%s",mrm.reg(),CPUregs32[mrm.rm()]);
                            else
                                goto invalidopcode;
                            break;
                        case 0x24: // MOV from test registers (i.e. MOV EAX,TR0). Only the register form is legal, not from/to memory. Register is ALWAYS 32-bit.
                            mrm.set(IPFB());
                            if (mrm.mod() == 3)
                                w += snprintf(w,(size_t)(wf-w),"MOVd %s,TR%u",CPUregs32[mrm.rm()],mrm.reg());
                            else
                                goto invalidopcode;
                            break;

                        case 0x26: // MOV to control registers (i.e. MOV TR0,EAX). Only the register form is legal, not from/to memory. Register is ALWAYS 32-bit.
                            mrm.set(IPFB());
                            if (mrm.mod() == 3)
                                w += snprintf(w,(size_t)(wf-w),"MOVd TR%u,%s",mrm.reg(),CPUregs32[mrm.rm()]);
                            else
                                goto invalidopcode;
                            break;

                        case_span_16(0x80): // 0x80-0x8F
                            v16 = (uint16_t)IPFWsigned();
                            v16 = (v16 + IPval()) & 0xFFFFU;
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"%sw %04Xh",CPUjcc0F8x[op0F&15],v16);
#endif
                            break;
                        case_span_16(0x90): // 0x90-0x9F
                            mrm.set(IPFB());
                            disp = IPFmrmdisplace16(/*&*/mrm);
                            w += snprintf(w,(size_t)(wf-w),"%sb %s",CPUsetcc0F9x[op0F&15],IPDecPrint16(mrm,disp,1));
                            break;
                        case 0xA0:
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"PUSHw FS");
#endif
                            break;
                        case 0xA1:
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"POPw FS");
#endif
                            break;

                        case 0xA3: // BT r/m,reg
                            mrm.set(IPFB());
                            disp = IPFmrmdisplace16(/*&*/mrm);
                            w += snprintf(w,(size_t)(wf-w),"BTw %s,%s",IPDecPrint16(mrm,disp,2),CPUregs16[mrm.reg()]);
                            break;
                        case 0xA4: // SHLD r/m,reg,imm8
                            mrm.set(IPFB());
                            disp = IPFmrmdisplace16(/*&*/mrm);
                            v8 = IPFB();
                            w += snprintf(w,(size_t)(wf-w),"SHLDw %s,%s,%02Xh",IPDecPrint16(mrm,disp,2),CPUregs16[mrm.reg()],v8);
                            break;
                        case 0xA5: // SHLD r/m,reg,CL
                            mrm.set(IPFB());
                            disp = IPFmrmdisplace16(/*&*/mrm);
                            w += snprintf(w,(size_t)(wf-w),"SHLDw %s,%s,CL",IPDecPrint16(mrm,disp,2),CPUregs16[mrm.reg()]);
                            break;

                        case 0xA8:
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"PUSHw GS");
#endif
                            break;
                        case 0xA9:
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"POPw GS");
#endif
                            break;

                        case 0xAB: // BTS r/m,reg
                            mrm.set(IPFB());
                            disp = IPFmrmdisplace16(/*&*/mrm);
                            w += snprintf(w,(size_t)(wf-w),"BTSw %s,%s",IPDecPrint16(mrm,disp,2),CPUregs16[mrm.reg()]);
                            break;
                        case 0xAC: // SHRD r/m,reg,imm8
                            mrm.set(IPFB());
                            disp = IPFmrmdisplace16(/*&*/mrm);
                            v8 = IPFB();
                            w += snprintf(w,(size_t)(wf-w),"SHRDw %s,%s,%02Xh",IPDecPrint16(mrm,disp,2),CPUregs16[mrm.reg()],v8);
                            break;
                        case 0xAD: // SHRD r/m,reg,CL
                            mrm.set(IPFB());
                            disp = IPFmrmdisplace16(/*&*/mrm);
                            w += snprintf(w,(size_t)(wf-w),"SHRDw %s,%s,CL",IPDecPrint16(mrm,disp,2),CPUregs16[mrm.reg()]);
                            break;

                        case 0xAF: // IMUL reg,r/m
                            mrm.set(IPFB());
                            disp = IPFmrmdisplace16(/*&*/mrm);
                            w += snprintf(w,(size_t)(wf-w),"IMULw %s,%s",CPUregs16[mrm.reg()],IPDecPrint16(mrm,disp,2));
                            break;

                        case 0xB2: // LSS reg,r/m word size
                            mrm.set(IPFB());
                            disp = IPFmrmdisplace16(/*&*/mrm);
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"LSSw %s,%s",CPUregs16[mrm.reg()],IPDecPrint16(mrm,disp,2));
#endif
                            break;
                        case 0xB3: // BTR
                            mrm.set(IPFB());
                            disp = IPFmrmdisplace16(/*&*/mrm);
                            w += snprintf(w,(size_t)(wf-w),"BTRw %s,%s",CPUregs16[mrm.reg()],IPDecPrint16(mrm,disp,2));
                            break;
                        case 0xB4: // LFS reg,r/m word size
                            mrm.set(IPFB());
                            disp = IPFmrmdisplace16(/*&*/mrm);
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"LFSw %s,%s",CPUregs16[mrm.reg()],IPDecPrint16(mrm,disp,2));
#endif
                            break;
                        case 0xB5: // LGS reg,r/m word size
                            mrm.set(IPFB());
                            disp = IPFmrmdisplace16(/*&*/mrm);
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"LGSw %s,%s",CPUregs16[mrm.reg()],IPDecPrint16(mrm,disp,2));
#endif
                            break;
                        case 0xB6: // MOVZX reg,r/m    byte -> word
                            IPDec386Load_MRM_SIB(/*&*/mrm,/*&*/sib,/*&*/disp,addr32);
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"MOVZX%s %s,%s ; byte -> %s",sizesuffix[COREWORDSIZE],
                                CPUregsN[COREWORDSIZE][mrm.reg()],IPDecPrint386(mrm,sib,disp,1,addr32),sizespec[COREWORDSIZE]);
#endif
                            break;
                        case 0xB7: // MOVZX reg,r/m    word -> word
                            // FIXME: What does the Intel 80386 do when this instruction is used in the 16-bit mode? The obvious intent
                            //        of this instruction is for use from 32-bit code (or 16-bit code with 32-bit override) to expand
                            //        16-bit to 32-bit i.e. MOVZX EAX,AX.
                            IPDec386Load_MRM_SIB(/*&*/mrm,/*&*/sib,/*&*/disp,addr32);
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"MOVZX%s %s,%s ; word -> %s",sizesuffix[COREWORDSIZE],
                                CPUregsN[COREWORDSIZE][mrm.reg()],IPDecPrint386(mrm,sib,disp,2,addr32),sizespec[COREWORDSIZE]);
#endif
                            break;

                        case 0xBA: // 0x0F 0xBA group
                            mrm.set(IPFB());
                            disp = IPFmrmdisplace16(/*&*/mrm);
                            switch (mrm.reg()) {
                                case 4: // BT r/m,imm8
                                    v8 = IPFB();
                                    w += snprintf(w,(size_t)(wf-w),"BTw %s,%02Xh",IPDecPrint16(mrm,disp,2),v8);
                                    break;
                                case 5: // BTS r/m,imm8
                                    v8 = IPFB();
                                    w += snprintf(w,(size_t)(wf-w),"BTSw %s,%02Xh",IPDecPrint16(mrm,disp,2),v8);
                                    break;
                                case 6: // BTR r/m,imm8
                                    v8 = IPFB();
                                    w += snprintf(w,(size_t)(wf-w),"BTRw %s,%02Xh",IPDecPrint16(mrm,disp,2),v8);
                                    break;
                                case 7: // BTC r/m,imm8
                                    v8 = IPFB();
                                    w += snprintf(w,(size_t)(wf-w),"BTCw %s,%02Xh",IPDecPrint16(mrm,disp,2),v8);
                                    break;
                                default:
                                    goto invalidopcode;
                            };
                            break;
                        case 0xBB: // BTC
                            mrm.set(IPFB());
                            disp = IPFmrmdisplace16(/*&*/mrm);
                            w += snprintf(w,(size_t)(wf-w),"BTCw %s,%s",CPUregs16[mrm.reg()],IPDecPrint16(mrm,disp,2));
                            break;
                        case 0xBC: // BSF
                            mrm.set(IPFB());
                            disp = IPFmrmdisplace16(/*&*/mrm);
                            w += snprintf(w,(size_t)(wf-w),"BSFw %s,%s",CPUregs16[mrm.reg()],IPDecPrint16(mrm,disp,2));
                            break;
                        case 0xBD: // BSR
                            mrm.set(IPFB());
                            disp = IPFmrmdisplace16(/*&*/mrm);
                            w += snprintf(w,(size_t)(wf-w),"BSRw %s,%s",CPUregs16[mrm.reg()],IPDecPrint16(mrm,disp,2));
                            break;
                        case 0xBE: // MOVSX reg,r/m    byte -> word
                            IPDec386Load_MRM_SIB(/*&*/mrm,/*&*/sib,/*&*/disp,addr32);
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"MOVSX%s %s,%s ; byte -> %s",sizesuffix[COREWORDSIZE],
                                CPUregsN[COREWORDSIZE][mrm.reg()],IPDecPrint386(mrm,sib,disp,1,addr32),sizespec[COREWORDSIZE]);
#endif
                            break;
                        case 0xBF: // MOVSX reg,r/m    word -> word
                            // FIXME: What does the Intel 80386 do when this instruction is used in the 16-bit mode? The obvious intent
                            //        of this instruction is for use from 32-bit code (or 16-bit code with 32-bit override) to expand
                            //        16-bit to 32-bit i.e. MOVZX EAX,AX.
                            IPDec386Load_MRM_SIB(/*&*/mrm,/*&*/sib,/*&*/disp,addr32);
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"MOVSX%s %s,%s ; word -> %s",sizesuffix[COREWORDSIZE],
                                CPUregsN[COREWORDSIZE][mrm.reg()],IPDecPrint386(mrm,sib,disp,2,addr32),sizespec[COREWORDSIZE]);
#endif
                            break;

                        default:
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"(invalid opcode %02Xh %02Xh)",op1,op0F);
                            break;
#else
                            goto invalidopcode;
#endif
                    };
                    break;

