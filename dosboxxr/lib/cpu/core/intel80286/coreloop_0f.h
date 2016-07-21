/* sub-sub fragment for 286 0F prefix */
                case 0x0F: // FPU ESCAPE + 0x0
                    switch (op0F=IPFB()) {
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

                        case 0x06: // CLTS (NTS: Intel 80286 datasheet calls this CTS for some reason)
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"CLTS");
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

