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

                        case 0xA3: // BT r/m,reg
                            mrm.set(IPFB());
                            disp = IPFmrmdisplace16(/*&*/mrm);
                            w += snprintf(w,(size_t)(wf-w),"BTw %s,%s",IPDecPrint16(mrm,disp,2),CPUregs16[mrm.reg()]);
                            break;

                        case 0xAB: // BTS r/m,reg
                            mrm.set(IPFB());
                            disp = IPFmrmdisplace16(/*&*/mrm);
                            w += snprintf(w,(size_t)(wf-w),"BTSw %s,%s",IPDecPrint16(mrm,disp,2),CPUregs16[mrm.reg()]);
                            break;

                        case 0xAF: // IMUL reg,r/m
                            mrm.set(IPFB());
                            disp = IPFmrmdisplace16(/*&*/mrm);
                            w += snprintf(w,(size_t)(wf-w),"IMULw %s,%s",CPUregs16[mrm.reg()],IPDecPrint16(mrm,disp,2));
                            break;

                        case 0xB3: // BTR
                            mrm.set(IPFB());
                            disp = IPFmrmdisplace16(/*&*/mrm);
                            w += snprintf(w,(size_t)(wf-w),"BTRw %s,%s",CPUregs16[mrm.reg()],IPDecPrint16(mrm,disp,2));
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

                        default:
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"(invalid opcode %02Xh %02Xh)",op1,op0F);
                            break;
#else
                            goto invalidopcode;
#endif
                    };
                    break;

