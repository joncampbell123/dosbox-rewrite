/* sub-sub fragment for FPU 8087 instructions */
                case 0xD8: // FPU ESCAPE + 0x0
                    mrm.set(IPFB());
                    disp = IPFmrmdisplace16(/*&*/mrm);
                    switch (mrm.byte) {
                        case_span_by_mod_reg(/*mod*/0,/*reg*/0): // FADD integer/real mem to ST(0)   MF == 0 32-bit real
                        case_span_by_mod_reg(/*mod*/1,/*reg*/0): // ESCAPE M F 0 | MOD 0 0 0 R/M     REG == 0 MOD == 0,1,2 RM == mem ref
                        case_span_by_mod_reg(/*mod*/2,/*reg*/0):
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FADDd %s ; MF=32-bit real",IPDecPrint16(mrm,disp,4,RC_FPUREG));
#endif
                            break;
                        case_span_by_mod_reg(/*mod*/3,/*reg*/0): // FADD ST(i) to ST(0)
                                                                 // ESCAPE D P 0 | 1 1 0 0 0 R/M     REG == 0 MOD == 3 RM == FPU register index D == 0 P == 0
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FADD ST(0),ST(%u) ; ST(0) += ST(%u)",mrm.rm(),mrm.rm());
#endif
                            break;
                        case_span_by_mod_reg(/*mod*/0,/*reg*/1): // FMUL integer/real mem to ST(0)   MF == 0 32-bit real
                        case_span_by_mod_reg(/*mod*/1,/*reg*/1): // ESCAPE M F 0 | MOD 0 0 1 R/M     REG == 1 MOD == 0,1,2 RM == mem ref
                        case_span_by_mod_reg(/*mod*/2,/*reg*/1):
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FMULd %s ; MF=32-bit real",IPDecPrint16(mrm,disp,4,RC_FPUREG));
#endif
                            break;
                        case_span_by_mod_reg(/*mod*/3,/*reg*/1): // FMUL ST(i) to ST(0)
                                                                 // ESCAPE D P 0 | 1 1 0 0 1 R/M     REG == 1 MOD == 3 RM == FPU register index D == 0 P == 0
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FMUL ST(0),ST(%u) ; ST(0) *= ST(%u)",mrm.rm(),mrm.rm());
#endif
                            break;
                        case_span_by_mod_reg(/*mod*/0,/*reg*/2): // FCOM integer/real mem to ST(0)   MF == 0 32-bit real
                        case_span_by_mod_reg(/*mod*/1,/*reg*/2): // ESCAPE M F 0 | MOD 0 1 0 R/M     REG == 2 MOD == 0,1,2 RM == mem ref
                        case_span_by_mod_reg(/*mod*/2,/*reg*/2):
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FCOMd %s ; MF=32-bit real",IPDecPrint16(mrm,disp,4,RC_FPUREG));
#endif
                            break;
                        case_span_by_mod_reg(/*mod*/3,/*reg*/2): // FCOM ST(i) to ST(0)
                                                                 // ESCAPE 0 0 0 | 1 1 0 1 0 R/M     REG == 2 MOD == 3 RM == FPU register index
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FCOM ST(%u)",mrm.rm());
#endif
                            break;
                        case_span_by_mod_reg(/*mod*/0,/*reg*/3): // FCOMP integer/real mem to ST(0)  MF == 0 32-bit real
                        case_span_by_mod_reg(/*mod*/1,/*reg*/3): // ESCAPE M F 0 | MOD 0 1 1 R/M     REG == 3 MOD == 0,1,2 RM == mem ref
                        case_span_by_mod_reg(/*mod*/2,/*reg*/3):
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FCOMPd %s ; MF=32-bit real",IPDecPrint16(mrm,disp,4,RC_FPUREG));
#endif
                            break;
                        case_span_by_mod_reg(/*mod*/3,/*reg*/3): // FCOMP ST(i) to ST(0)
                                                                 // ESCAPE 0 0 0 | 1 1 0 1 1 R/M     REG == 3 MOD == 3 RM == FPU register index
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FCOMP ST(%u)",mrm.rm());
#endif
                            break;
                        case_span_by_mod_reg(/*mod*/0,/*reg*/4): // FSUB integer/real mem to ST(0)   MF == 0 32-bit real
                        case_span_by_mod_reg(/*mod*/1,/*reg*/4): // ESCAPE M F 0 | MOD 1 0 R R/M     REG == 4 MOD == 0,1,2 RM == mem ref R == 0
                        case_span_by_mod_reg(/*mod*/2,/*reg*/4):
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FSUBd %s ; MF=32-bit real",IPDecPrint16(mrm,disp,4,RC_FPUREG));
#endif
                            break;
                        case_span_by_mod_reg(/*mod*/3,/*reg*/4): // FSUB ST(i) to ST(0)
                                                                 // ESCAPE D P 0 | 1 1 1 0 R R/M     REG == 4 MOD == 3 RM == FPU register index D == 0 P == 0 R == 0
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FSUB ST(0),ST(%u) ; ST(0) -= ST(%u)",mrm.rm(),mrm.rm());
#endif
                            break;
                        case_span_by_mod_reg(/*mod*/0,/*reg*/5): // FSUBR integer/real mem to ST(0)  MF == 0 32-bit real
                        case_span_by_mod_reg(/*mod*/1,/*reg*/5): // ESCAPE M F 0 | MOD 1 0 R R/M     REG == 5 MOD == 0,1,2 RM == mem ref R == 1
                        case_span_by_mod_reg(/*mod*/2,/*reg*/5):
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FSUBRd %s ; MF=32-bit real",IPDecPrint16(mrm,disp,4,RC_FPUREG));
#endif
                            break;
                        case_span_by_mod_reg(/*mod*/3,/*reg*/5): // FSUBR ST(i) to ST(0)
                                                                 // ESCAPE D P 0 | 1 1 1 0 R R/M     REG == 5 MOD == 3 RM == FPU register index D == 0 P == 0 R == 1
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FSUBR ST(0),ST(%u) ; ST(0) -= ST(%u)",mrm.rm(),mrm.rm());
#endif
                            break;

                        case_span_by_mod_reg(/*mod*/0,/*reg*/6): // FDIV integer/real mem to ST(0)   MF == 0 32-bit real
                        case_span_by_mod_reg(/*mod*/1,/*reg*/6): // ESCAPE M F 0 | MOD 1 1 R R/M     REG == 6 MOD == 0,1,2 RM == mem ref R == 0
                        case_span_by_mod_reg(/*mod*/2,/*reg*/6):
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FDIVd %s ; MF=32-bit real",IPDecPrint16(mrm,disp,4,RC_FPUREG));
#endif
                            break;
                        case_span_by_mod_reg(/*mod*/3,/*reg*/6): // FDIV ST(i) to ST(0)
                                                                 // ESCAPE D P 0 | 1 1 1 1 R R/M     REG == 6 MOD == 3 RM == FPU register index D == 0 P == 0 R == 0
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FDIV ST(0),ST(%u) ; ST(0) /= ST(%u)",mrm.rm(),mrm.rm());
#endif
                            break;
                        case_span_by_mod_reg(/*mod*/0,/*reg*/7): // FDIVR integer/real mem to ST(0)  MF == 0 32-bit real
                        case_span_by_mod_reg(/*mod*/1,/*reg*/7): // ESCAPE M F 0 | MOD 1 1 R R/M     REG == 7 MOD == 0,1,2 RM == mem ref R == 1
                        case_span_by_mod_reg(/*mod*/2,/*reg*/7):
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FDIVRd %s ; MF=32-bit real",IPDecPrint16(mrm,disp,4,RC_FPUREG));
#endif
                            break;
                        case_span_by_mod_reg(/*mod*/3,/*reg*/7): // FDIVR ST(i) to ST(0)
                                                                 // ESCAPE D P 0 | 1 1 1 1 R R/M     REG == 7 MOD == 3 RM == FPU register index D == 0 P == 0 R == 1
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FDIVR ST(0),ST(%u) ; ST(0) /= ST(%u)",mrm.rm(),mrm.rm());
#endif
                            break;
                        default:
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"(invalid FPU opcode %02Xh %02Xh)",op1,mrm.byte);
                            break;
#else
                            goto invalidopcode;
#endif
                    };
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

                        case_span_by_mod_reg(/*mod*/0,/*reg*/4): // FLDENV
                        case_span_by_mod_reg(/*mod*/1,/*reg*/4): // ESCAPE 0 0 1 | MOD 1 0 0 R/M     REG == 4 MOD == 0,1,2 RM == mem ref
                        case_span_by_mod_reg(/*mod*/2,/*reg*/4):
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FLDENV %s",IPDecPrint16(mrm,disp,2,RC_REG));
#endif
                            break;

                        case_span_by_mod_reg(/*mod*/0,/*reg*/5): // FLDCW
                        case_span_by_mod_reg(/*mod*/1,/*reg*/5): // ESCAPE 0 0 1 | MOD 1 0 1 R/M     REG == 5 MOD == 0,1,2 RM == mem ref
                        case_span_by_mod_reg(/*mod*/2,/*reg*/5):
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FLDCWw %s",IPDecPrint16(mrm,disp,2,RC_REG));
#endif
                            break;

                        case_span_by_mod_reg(/*mod*/0,/*reg*/6): // FSTENV
                        case_span_by_mod_reg(/*mod*/1,/*reg*/6): // ESCAPE 0 0 1 | MOD 1 1 0 R/M     REG == 6 MOD == 0,1,2 RM == mem ref
                        case_span_by_mod_reg(/*mod*/2,/*reg*/6):
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FSTENV %s",IPDecPrint16(mrm,disp,2,RC_REG));
#endif
                            break;

                        case_span_by_mod_reg(/*mod*/0,/*reg*/7): // FSTCW
                        case_span_by_mod_reg(/*mod*/1,/*reg*/7): // ESCAPE 0 0 1 | MOD 1 1 1 R/M     REG == 7 MOD == 0,1,2 RM == mem ref
                        case_span_by_mod_reg(/*mod*/2,/*reg*/7):
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FSTCWw %s",IPDecPrint16(mrm,disp,2,RC_REG));
#endif
                            break;
                        default:
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"(invalid FPU opcode %02Xh %02Xh)",op1,mrm.byte);
                            break;
#else
                            goto invalidopcode;
#endif
                    };
                    break;

                case 0xDA: // FPU ESCAPE + 0x2
                    mrm.set(IPFB());
                    disp = IPFmrmdisplace16(/*&*/mrm);
                    switch (mrm.byte) {
                        case_span_by_mod_reg(/*mod*/0,/*reg*/0): // FADD integer/real mem to ST(0)   MF == 1 32-bit integer
                        case_span_by_mod_reg(/*mod*/1,/*reg*/0): // ESCAPE M F 0 | MOD 0 0 0 R/M     REG == 0 MOD == 0,1,2 RM == mem ref
                        case_span_by_mod_reg(/*mod*/2,/*reg*/0):
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FIADDd %s ; MF=32-bit integer",IPDecPrint16(mrm,disp,4,RC_FPUREG));
#endif
                            break;
                        // WARNING: This is how the 8087 would interpret this opcode, according to Intel's datasheet.
                        //          Note that nobody would use this instruction because it is nonsensical (add to ST(0) then pop ST(0)).
                        //          It's not defined or known yet how later processors (286, 386, 486, Pentium, etc.) handle this opcode.
                        //          Later processors starting with the Pentium II re-define this opcode as FCMOVxx instructions.
                        case_span_by_mod_reg(/*mod*/3,/*reg*/0): // FADD ST(i) to ST(0)
                                                                 // ESCAPE D P 0 | 1 1 0 0 0 R/M     REG == 0 MOD == 3 RM == FPU register index D == 0 P == 1
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FADDP ST(0),ST(%u) ; ST(0) += ST(%u), POP",mrm.rm(),mrm.rm());
#endif
                            break;

                        case_span_by_mod_reg(/*mod*/0,/*reg*/1): // FMUL integer/real mem to ST(0)   MF == 1 32-bit integer
                        case_span_by_mod_reg(/*mod*/1,/*reg*/1): // ESCAPE M F 0 | MOD 0 0 1 R/M     REG == 1 MOD == 0,1,2 RM == mem ref
                        case_span_by_mod_reg(/*mod*/2,/*reg*/1):
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FIMULd %s ; MF=32-bit integer",IPDecPrint16(mrm,disp,4,RC_FPUREG));
#endif
                            break;
                        // WARNING: This is how the 8087 would interpret this opcode, according to Intel's datasheet.
                        //          Note that nobody would use this instruction because it is nonsensical (add to ST(0) then pop ST(0)).
                        //          It's not defined or known yet how later processors (286, 386, 486, Pentium, etc.) handle this opcode.
                        //          Later processors starting with the Pentium II re-define this opcode as FCMOVxx instructions.
                        case_span_by_mod_reg(/*mod*/3,/*reg*/1): // FMUL ST(i) to ST(0)
                                                                 // ESCAPE D P 0 | 1 1 0 0 1 R/M     REG == 1 MOD == 3 RM == FPU register index D == 0 P == 1
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FMULP ST(0),ST(%u) ; ST(0) *= ST(%u), POP",mrm.rm(),mrm.rm());
#endif
                            break;

                        case_span_by_mod_reg(/*mod*/0,/*reg*/2): // FCOM integer/real mem to ST(0)   MF == 1 32-bit integer
                        case_span_by_mod_reg(/*mod*/1,/*reg*/2): // ESCAPE M F 0 | MOD 0 1 0 R/M     REG == 2 MOD == 0,1,2 RM == mem ref
                        case_span_by_mod_reg(/*mod*/2,/*reg*/2):
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FICOMd %s ; MF=32-bit integer",IPDecPrint16(mrm,disp,4,RC_FPUREG));
#endif
                            break;

                        case_span_by_mod_reg(/*mod*/0,/*reg*/3): // FCOMP integer/real mem to ST(0)  MF == 1 32-bit integer
                        case_span_by_mod_reg(/*mod*/1,/*reg*/3): // ESCAPE M F 0 | MOD 0 1 1 R/M     REG == 3 MOD == 0,1,2 RM == mem ref
                        case_span_by_mod_reg(/*mod*/2,/*reg*/3):
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FICOMPd %s ; MF=32-bit integer",IPDecPrint16(mrm,disp,4,RC_FPUREG));
#endif
                            break;

                        case_span_by_mod_reg(/*mod*/0,/*reg*/4): // FSUB integer/real mem to ST(0)   MF == 1 32-bit integer
                        case_span_by_mod_reg(/*mod*/1,/*reg*/4): // ESCAPE M F 0 | MOD 1 0 R R/M     REG == 4 MOD == 0,1,2 RM == mem ref R == 0
                        case_span_by_mod_reg(/*mod*/2,/*reg*/4):
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FISUBd %s ; MF=32-bit integer",IPDecPrint16(mrm,disp,4,RC_FPUREG));
#endif
                            break;
                        // WARNING: This is how the 8087 would interpret this opcode, according to Intel's datasheet.
                        //          Note that nobody would use this instruction because it is nonsensical (sub from ST(0) then pop ST(0)).
                        //          It's not defined or known yet how later processors (286, 386, 486, Pentium, etc.) handle this opcode.
                        //          Later processors starting with the Pentium II re-define this opcode as ??????(TODO) instructions.
                        case_span_by_mod_reg(/*mod*/3,/*reg*/4): // FSUB ST(i) to ST(0)
                                                                 // ESCAPE D P 0 | 1 1 1 0 R R/M     REG == 4 MOD == 3 RM == FPU register index D == 0 P == 1 R == 0
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FSUBP ST(0),ST(%u) ; ST(0) -= ST(%u), POP",mrm.rm(),mrm.rm());
#endif
                            break;

                        case_span_by_mod_reg(/*mod*/0,/*reg*/5): // FSUBR integer/real mem to ST(0)  MF == 1 32-bit integer
                        case_span_by_mod_reg(/*mod*/1,/*reg*/5): // ESCAPE M F 0 | MOD 1 0 R R/M     REG == 5 MOD == 0,1,2 RM == mem ref R == 1
                        case_span_by_mod_reg(/*mod*/2,/*reg*/5):
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FISUBRd %s ; MF=32-bit integer",IPDecPrint16(mrm,disp,4,RC_FPUREG));
#endif
                            break;
                        // WARNING: This is how the 8087 would interpret this opcode, according to Intel's datasheet.
                        //          Note that nobody would use this instruction because it is nonsensical (sub from ST(0) then pop ST(0)).
                        //          It's not defined or known yet how later processors (286, 386, 486, Pentium, etc.) handle this opcode.
                        //          Later processors starting with the Pentium II re-define this opcode as ??????(TODO) instructions.
                        case_span_by_mod_reg(/*mod*/3,/*reg*/5): // FSUBR ST(i) to ST(0)
                                                                 // ESCAPE D P 0 | 1 1 1 0 R R/M     REG == 5 MOD == 3 RM == FPU register index D == 0 P == 1 R == 1
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FSUBRP ST(0),ST(%u) ; ST(0) -= ST(%u), POP",mrm.rm(),mrm.rm());
#endif
                            break;

                        case_span_by_mod_reg(/*mod*/0,/*reg*/6): // FDIV integer/real mem to ST(0)   MF == 1 32-bit integer
                        case_span_by_mod_reg(/*mod*/1,/*reg*/6): // ESCAPE M F 0 | MOD 1 1 R R/M     REG == 6 MOD == 0,1,2 RM == mem ref R == 0
                        case_span_by_mod_reg(/*mod*/2,/*reg*/6):
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FIDIVd %s ; MF=32-bit integer",IPDecPrint16(mrm,disp,4,RC_FPUREG));
#endif
                            break;
                        // WARNING: This is how the 8087 would interpret this opcode, according to Intel's datasheet.
                        //          Note that nobody would use this instruction because it is nonsensical (sub from ST(0) then pop ST(0)).
                        //          It's not defined or known yet how later processors (286, 386, 486, Pentium, etc.) handle this opcode.
                        //          Later processors starting with the Pentium II re-define this opcode as ??????(TODO) instructions.
                        case_span_by_mod_reg(/*mod*/3,/*reg*/6): // FDIV ST(i) to ST(0)
                                                                 // ESCAPE D P 0 | 1 1 1 1 R R/M     REG == 6 MOD == 3 RM == FPU register index D == 0 P == 1 R == 0
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FDIVP ST(0),ST(%u) ; ST(0) /= ST(%u), POP",mrm.rm(),mrm.rm());
#endif
                            break;

                        case_span_by_mod_reg(/*mod*/0,/*reg*/7): // FDIVR integer/real mem to ST(0)  MF == 1 32-bit integer
                        case_span_by_mod_reg(/*mod*/1,/*reg*/7): // ESCAPE M F 0 | MOD 1 1 R R/M     REG == 7 MOD == 0,1,2 RM == mem ref R == 1
                        case_span_by_mod_reg(/*mod*/2,/*reg*/7):
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FIDIVRd %s ; MF=32-bit integer",IPDecPrint16(mrm,disp,4,RC_FPUREG));
#endif
                            break;
                        // WARNING: This is how the 8087 would interpret this opcode, according to Intel's datasheet.
                        //          Note that nobody would use this instruction because it is nonsensical (sub from ST(0) then pop ST(0)).
                        //          It's not defined or known yet how later processors (286, 386, 486, Pentium, etc.) handle this opcode.
                        //          Later processors starting with the Pentium II re-define this opcode as ??????(TODO) instructions.
                        case_span_by_mod_reg(/*mod*/3,/*reg*/7): // FDIVR ST(i) to ST(0)
                                                                 // ESCAPE D P 0 | 1 1 1 1 R R/M     REG == 7 MOD == 3 RM == FPU register index D == 0 P == 1 R == 1
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FDIVRP ST(0),ST(%u) ; ST(0) /= ST(%u), POP",mrm.rm(),mrm.rm());
#endif
                            break;
                        default:
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"(invalid FPU opcode %02Xh %02Xh)",op1,mrm.byte);
                            break;
#else
                            goto invalidopcode;
#endif
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
                            w += snprintf(w,(size_t)(wf-w),"FILDd %s ; MF=32-bit integer",IPDecPrint16(mrm,disp,4,RC_FPUREG));
#endif
                            break;

                        case_span_by_mod_reg(/*mod*/0,/*reg*/2): // FST ST(0) to integer/real mem    MF == 1 32-bit integer
                        case_span_by_mod_reg(/*mod*/1,/*reg*/2): // ESCAPE M F 1 | MOD 0 1 0 R/M     REG == 2 MOD == 0,1,2 RM == mem ref
                        case_span_by_mod_reg(/*mod*/2,/*reg*/2):
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FISTd %s ; MF=32-bit integer",IPDecPrint16(mrm,disp,4,RC_FPUREG));
#endif
                            break;
                        case_span_by_mod_reg(/*mod*/0,/*reg*/3): // FSTP ST(0) to integer/real mem   MF == 1 32-bit integer
                        case_span_by_mod_reg(/*mod*/1,/*reg*/3): // ESCAPE M F 1 | MOD 0 1 1 R/M     REG == 2 MOD == 0,1,2 RM == mem ref
                        case_span_by_mod_reg(/*mod*/2,/*reg*/3):
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FISTPd %s ; MF=32-bit integer",IPDecPrint16(mrm,disp,4,RC_FPUREG));
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

                        case 0xE0:
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FENI");
#endif
                            break;
                        case 0xE1:
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FDISI");
#endif
                            break;
                        case 0xE2:
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FCLEX");
#endif
                            break;
                        case 0xE3:
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FINIT");
#endif
                            break;
                        default:
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"(invalid FPU opcode %02Xh %02Xh)",op1,mrm.byte);
                            break;
#else
                            goto invalidopcode;
#endif
                    };
                    break;

                case 0xDC: // FPU ESCAPE + 0x4
                    mrm.set(IPFB());
                    disp = IPFmrmdisplace16(/*&*/mrm);
                    switch (mrm.byte) {
                        case_span_by_mod_reg(/*mod*/0,/*reg*/0): // FADD integer/real mem to ST(0)   MF == 2 64-bit real
                        case_span_by_mod_reg(/*mod*/1,/*reg*/0): // ESCAPE M F 0 | MOD 0 0 0 R/M     REG == 0 MOD == 0,1,2 RM == mem ref
                        case_span_by_mod_reg(/*mod*/2,/*reg*/0):
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FADDq %s ; MF=64-bit real",IPDecPrint16(mrm,disp,8,RC_FPUREG));
#endif
                            break;
                        case_span_by_mod_reg(/*mod*/3,/*reg*/0): // FADD ST(0) to ST(i)
                                                                 // ESCAPE D P 0 | 1 1 0 0 0 R/M     REG == 0 MOD == 3 RM == FPU register index D == 1 P == 0
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FADD ST(%u),ST(0) ; ST(%u) += ST(0)",mrm.rm(),mrm.rm());
#endif
                            break;

                        case_span_by_mod_reg(/*mod*/0,/*reg*/1): // FMUL integer/real mem to ST(0)   MF == 2 64-bit real
                        case_span_by_mod_reg(/*mod*/1,/*reg*/1): // ESCAPE M F 0 | MOD 0 0 1 R/M     REG == 1 MOD == 0,1,2 RM == mem ref
                        case_span_by_mod_reg(/*mod*/2,/*reg*/1):
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FMULq %s ; MF=64-bit real",IPDecPrint16(mrm,disp,8,RC_FPUREG));
#endif
                            break;
                        case_span_by_mod_reg(/*mod*/3,/*reg*/1): // FMUL ST(0) to ST(i)
                                                                 // ESCAPE D P 0 | 1 1 0 0 1 R/M     REG == 1 MOD == 3 RM == FPU register index D == 1 P == 0
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FMUL ST(%u),ST(0) ; ST(%u) *= ST(0)",mrm.rm(),mrm.rm());
#endif
                            break;

                        case_span_by_mod_reg(/*mod*/0,/*reg*/2): // FCOM integer/real mem to ST(0)   MF == 2 64-bit real
                        case_span_by_mod_reg(/*mod*/1,/*reg*/2): // ESCAPE M F 0 | MOD 0 1 0 R/M     REG == 2 MOD == 0,1,2 RM == mem ref
                        case_span_by_mod_reg(/*mod*/2,/*reg*/2):
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FCOMq %s ; MF=64-bit real",IPDecPrint16(mrm,disp,8,RC_FPUREG));
#endif
                            break;

                        case_span_by_mod_reg(/*mod*/0,/*reg*/3): // FCOMP integer/real mem to ST(0)  MF == 2 64-bit real
                        case_span_by_mod_reg(/*mod*/1,/*reg*/3): // ESCAPE M F 0 | MOD 0 1 1 R/M     REG == 3 MOD == 0,1,2 RM == mem ref
                        case_span_by_mod_reg(/*mod*/2,/*reg*/3):
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FCOMPq %s ; MF=64-bit real",IPDecPrint16(mrm,disp,8,RC_FPUREG));
#endif
                            break;

                        case_span_by_mod_reg(/*mod*/0,/*reg*/4): // FSUB integer/real mem to ST(0)   MF == 2 64-bit real
                        case_span_by_mod_reg(/*mod*/1,/*reg*/4): // ESCAPE M F 0 | MOD 1 0 R R/M     REG == 4 MOD == 0,1,2 RM == mem ref R == 0
                        case_span_by_mod_reg(/*mod*/2,/*reg*/4):
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FSUBq %s ; MF=64-bit real",IPDecPrint16(mrm,disp,8,RC_FPUREG));
#endif
                            break;
                        case_span_by_mod_reg(/*mod*/3,/*reg*/4): // FSUB ST(0) to ST(i)
                                                                 // ESCAPE D P 0 | 1 1 1 0 R R/M     REG == 4 MOD == 3 RM == FPU register index D == 1 P == 0 R == 0
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FSUB ST(%u),ST(0) ; ST(%u) -= ST(0)",mrm.rm(),mrm.rm());
#endif
                            break;

                        case_span_by_mod_reg(/*mod*/0,/*reg*/5): // FSUBR integer/real mem to ST(0)  MF == 2 64-bit real
                        case_span_by_mod_reg(/*mod*/1,/*reg*/5): // ESCAPE M F 0 | MOD 1 0 R R/M     REG == 5 MOD == 0,1,2 RM == mem ref R == 1
                        case_span_by_mod_reg(/*mod*/2,/*reg*/5):
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FSUBRq %s ; MF=64-bit real",IPDecPrint16(mrm,disp,8,RC_FPUREG));
#endif
                            break;
                        case_span_by_mod_reg(/*mod*/3,/*reg*/5): // FSUB ST(0) to ST(i)
                                                                 // ESCAPE D P 0 | 1 1 1 0 R R/M     REG == 5 MOD == 3 RM == FPU register index D == 1 P == 0 R == 1
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FSUBR ST(%u),ST(0) ; ST(%u) -= ST(0)",mrm.rm(),mrm.rm());
#endif
                            break;

                        case_span_by_mod_reg(/*mod*/0,/*reg*/6): // FDIV integer/real mem to ST(0)   MF == 2 64-bit real
                        case_span_by_mod_reg(/*mod*/1,/*reg*/6): // ESCAPE M F 0 | MOD 1 1 R R/M     REG == 6 MOD == 0,1,2 RM == mem ref R == 0
                        case_span_by_mod_reg(/*mod*/2,/*reg*/6):
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FDIVq %s ; MF=64-bit real",IPDecPrint16(mrm,disp,8,RC_FPUREG));
#endif
                            break;
                        case_span_by_mod_reg(/*mod*/3,/*reg*/6): // FSUB ST(0) to ST(i)
                                                                 // ESCAPE D P 0 | 1 1 1 1 R R/M     REG == 6 MOD == 3 RM == FPU register index D == 1 P == 0 R == 0
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FDIV ST(%u),ST(0) ; ST(%u) /= ST(0)",mrm.rm(),mrm.rm());
#endif
                            break;

                        case_span_by_mod_reg(/*mod*/0,/*reg*/7): // FDIVR integer/real mem to ST(0)  MF == 2 64-bit real
                        case_span_by_mod_reg(/*mod*/1,/*reg*/7): // ESCAPE M F 0 | MOD 1 1 R R/M     REG == 7 MOD == 0,1,2 RM == mem ref R == 1
                        case_span_by_mod_reg(/*mod*/2,/*reg*/7):
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FDIVRq %s ; MF=64-bit real",IPDecPrint16(mrm,disp,8,RC_FPUREG));
#endif
                            break;
                        case_span_by_mod_reg(/*mod*/3,/*reg*/7): // FDIV ST(0) to ST(i)
                                                                 // ESCAPE D P 0 | 1 1 1 1 R R/M     REG == 7 MOD == 3 RM == FPU register index D == 1 P == 0 R == 1
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FDIVR ST(%u),ST(0) ; ST(%u) /= ST(0)",mrm.rm(),mrm.rm());
#endif
                            break;
                        default:
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"(invalid FPU opcode %02Xh %02Xh)",op1,mrm.byte);
                            break;
#else
                            goto invalidopcode;
#endif
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
                        case_span_by_mod_reg(/*mod*/3,/*reg*/0): // FFREE
                                                                 // ESCAPE 1 0 1 | 1 1 0 0 0 R/M     REG == 0 MOD == 3 RM == mem ref
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FFREE ST(%u)",mrm.rm());
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

                        case_span_by_mod_reg(/*mod*/0,/*reg*/4): // FRSTOR
                        case_span_by_mod_reg(/*mod*/1,/*reg*/4): // ESCAPE 1 0 1 | MOD 1 0 0 R/M     REG == 4 MOD == 0,1,2 RM == mem ref
                        case_span_by_mod_reg(/*mod*/2,/*reg*/4):
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FRSTOR %s",IPDecPrint16(mrm,disp,2,RC_REG));
#endif
                            break;

                        case_span_by_mod_reg(/*mod*/0,/*reg*/6): // FSAVE
                        case_span_by_mod_reg(/*mod*/1,/*reg*/6): // ESCAPE 1 0 1 | MOD 1 1 0 R/M     REG == 6 MOD == 0,1,2 RM == mem ref
                        case_span_by_mod_reg(/*mod*/2,/*reg*/6):
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FSAVE %s",IPDecPrint16(mrm,disp,2,RC_REG));
#endif
                            break;

                        case_span_by_mod_reg(/*mod*/0,/*reg*/7): // FSTSW
                        case_span_by_mod_reg(/*mod*/1,/*reg*/7): // ESCAPE 1 0 1 | MOD 1 1 1 R/M     REG == 7 MOD == 0,1,2 RM == mem ref
                        case_span_by_mod_reg(/*mod*/2,/*reg*/7):
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FSTSWw %s ; MF=64-bit real",IPDecPrint16(mrm,disp,8,RC_FPUREG));
#endif
                            break;
                        default:
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"(invalid FPU opcode %02Xh %02Xh)",op1,mrm.byte);
                            break;
#else
                            goto invalidopcode;
#endif
                    };
                    break;

                case 0xDE: // FPU ESCAPE + 0x6
                    mrm.set(IPFB());
                    disp = IPFmrmdisplace16(/*&*/mrm);
                    switch (mrm.byte) {
                        case_span_by_mod_reg(/*mod*/0,/*reg*/0): // FADD integer/real mem to ST(0)   MF == 3 16-bit integer
                        case_span_by_mod_reg(/*mod*/1,/*reg*/0): // ESCAPE M F 0 | MOD 0 0 0 R/M     REG == 0 MOD == 0,1,2 RM == mem ref
                        case_span_by_mod_reg(/*mod*/2,/*reg*/0):
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FIADDw %s ; MF=16-bit integer",IPDecPrint16(mrm,disp,2,RC_FPUREG));
#endif
                            break;
                        case_span_by_mod_reg(/*mod*/3,/*reg*/0): // FADD ST(0) to ST(i), POP
                                                                 // ESCAPE D P 0 | 1 1 0 0 0 R/M     REG == 0 MOD == 3 RM == FPU register index D == 1 P == 1
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FADDP ST(%u),ST(0) ; ST(%u) += ST(0), POP",mrm.rm(),mrm.rm());
#endif
                            break;

                        case_span_by_mod_reg(/*mod*/0,/*reg*/1): // FMUL integer/real mem to ST(0)   MF == 3 16-bit integer
                        case_span_by_mod_reg(/*mod*/1,/*reg*/1): // ESCAPE M F 0 | MOD 0 0 1 R/M     REG == 1 MOD == 0,1,2 RM == mem ref
                        case_span_by_mod_reg(/*mod*/2,/*reg*/1):
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FIMULw %s ; MF=16-bit integer",IPDecPrint16(mrm,disp,2,RC_FPUREG));
#endif
                            break;
                        case_span_by_mod_reg(/*mod*/3,/*reg*/1): // FMUL ST(0) to ST(i), POP
                                                                 // ESCAPE D P 0 | 1 1 0 0 1 R/M     REG == 1 MOD == 3 RM == FPU register index D == 1 P == 1
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FMULP ST(%u),ST(0) ; ST(%u) *= ST(0), POP",mrm.rm(),mrm.rm());
#endif
                            break;

                        case_span_by_mod_reg(/*mod*/0,/*reg*/2): // FCOM integer/real mem to ST(0)   MF == 3 16-bit integer
                        case_span_by_mod_reg(/*mod*/1,/*reg*/2): // ESCAPE M F 0 | MOD 0 1 0 R/M     REG == 2 MOD == 0,1,2 RM == mem ref
                        case_span_by_mod_reg(/*mod*/2,/*reg*/2):
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FICOMw %s ; MF=16-bit integer",IPDecPrint16(mrm,disp,2,RC_FPUREG));
#endif
                            break;

                        case_span_by_mod_reg(/*mod*/0,/*reg*/3): // FCOMP integer/real mem to ST(0)  MF == 3 16-bit integer
                        case_span_by_mod_reg(/*mod*/1,/*reg*/3): // ESCAPE M F 0 | MOD 0 1 0 R/M     REG == 3 MOD == 0,1,2 RM == mem ref
                        case_span_by_mod_reg(/*mod*/2,/*reg*/3):
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FICOMPw %s ; MF=16-bit integer",IPDecPrint16(mrm,disp,2,RC_FPUREG));
#endif
                            break;

                        case_span_by_mod_reg(/*mod*/0,/*reg*/4): // FSUB integer/real mem to ST(0)   MF == 3 16-bit integer
                        case_span_by_mod_reg(/*mod*/1,/*reg*/4): // ESCAPE M F 0 | MOD 1 0 R R/M     REG == 4 MOD == 0,1,2 RM == mem ref R == 0
                        case_span_by_mod_reg(/*mod*/2,/*reg*/4):
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FISUBw %s ; MF=16-bit integer",IPDecPrint16(mrm,disp,2,RC_FPUREG));
#endif
                            break;
                        case_span_by_mod_reg(/*mod*/3,/*reg*/4): // FSUB ST(0) to ST(i), POP
                                                                 // ESCAPE D P 0 | 1 1 1 0 R R/M     REG == 4 MOD == 3 RM == FPU register index D == 1 P == 1 R == 0
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FSUBP ST(%u),ST(0) ; ST(%u) -= ST(0), POP",mrm.rm(),mrm.rm());
#endif
                            break;

                        case_span_by_mod_reg(/*mod*/0,/*reg*/5): // FSUB integer/real mem to ST(0)   MF == 3 16-bit integer
                        case_span_by_mod_reg(/*mod*/1,/*reg*/5): // ESCAPE M F 0 | MOD 1 0 R R/M     REG == 5 MOD == 0,1,2 RM == mem ref R == 1
                        case_span_by_mod_reg(/*mod*/2,/*reg*/5):
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FISUBRw %s ; MF=16-bit integer",IPDecPrint16(mrm,disp,2,RC_FPUREG));
#endif
                            break;
                        case_span_by_mod_reg(/*mod*/3,/*reg*/5): // FSUB ST(0) to ST(i), POP
                                                                 // ESCAPE D P 0 | 1 1 1 0 R R/M     REG == 5 MOD == 3 RM == FPU register index D == 1 P == 1 R == 1
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FSUBRP ST(%u),ST(0) ; ST(%u) -= ST(0), POP",mrm.rm(),mrm.rm());
#endif
                            break;

                        case_span_by_mod_reg(/*mod*/0,/*reg*/6): // FDIV integer/real mem to ST(0)   MF == 3 16-bit integer
                        case_span_by_mod_reg(/*mod*/1,/*reg*/6): // ESCAPE M F 0 | MOD 1 1 R R/M     REG == 6 MOD == 0,1,2 RM == mem ref R == 0
                        case_span_by_mod_reg(/*mod*/2,/*reg*/6):
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FIDIVw %s ; MF=16-bit integer",IPDecPrint16(mrm,disp,2,RC_FPUREG));
#endif
                            break;
                        case_span_by_mod_reg(/*mod*/3,/*reg*/6): // FDIV ST(0) to ST(i), POP
                                                                 // ESCAPE D P 0 | 1 1 1 1 R R/M     REG == 6 MOD == 3 RM == FPU register index D == 1 P == 1 R == 0
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FDIVP ST(%u),ST(0) ; ST(%u) -= ST(0), POP",mrm.rm(),mrm.rm());
#endif
                            break;

                        case_span_by_mod_reg(/*mod*/0,/*reg*/7): // FDIV integer/real mem to ST(0)   MF == 3 16-bit integer
                        case_span_by_mod_reg(/*mod*/1,/*reg*/7): // ESCAPE M F 0 | MOD 1 1 R R/M     REG == 7 MOD == 0,1,2 RM == mem ref R == 1
                        case_span_by_mod_reg(/*mod*/2,/*reg*/7):
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FIDIVRw %s ; MF=16-bit integer",IPDecPrint16(mrm,disp,2,RC_FPUREG));
#endif
                            break;
                        case_span_by_mod_reg(/*mod*/3,/*reg*/7): // FDIV ST(0) to ST(i), POP
                                                                 // ESCAPE D P 0 | 1 1 1 1 R R/M     REG == 7 MOD == 3 RM == FPU register index D == 1 P == 1 R == 1
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FDIVRP ST(%u),ST(0) ; ST(%u) -= ST(0), POP",mrm.rm(),mrm.rm());
#endif
                            break;

                        case 0xD9: // FCOMPP  MOD == 3 REG == 3 RM == 1
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FCOMPP");
#endif
                            break;
                        default:
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"(invalid FPU opcode %02Xh %02Xh)",op1,mrm.byte);
                            break;
#else
                            goto invalidopcode;
#endif
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
                            w += snprintf(w,(size_t)(wf-w),"FILDw %s ; MF=16-bit integer",IPDecPrint16(mrm,disp,2,RC_FPUREG));
#endif
                            break;

                        case_span_by_mod_reg(/*mod*/0,/*reg*/2): // FST ST(0) to integer/real mem    MF == 3 16-bit integer
                        case_span_by_mod_reg(/*mod*/1,/*reg*/2): // ESCAPE M F 1 | MOD 0 1 0 R/M     REG == 2 MOD == 0,1,2 RM == mem ref
                        case_span_by_mod_reg(/*mod*/2,/*reg*/2):
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FISTw %s ; MF=16-bit integer",IPDecPrint16(mrm,disp,2,RC_FPUREG));
#endif
                            break;
                        case_span_by_mod_reg(/*mod*/0,/*reg*/3): // FSTP ST(0) to integer/real mem   MF == 3 16-bit integer
                        case_span_by_mod_reg(/*mod*/1,/*reg*/3): // ESCAPE M F 1 | MOD 0 1 0 R/M     REG == 2 MOD == 0,1,2 RM == mem ref
                        case_span_by_mod_reg(/*mod*/2,/*reg*/3):
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"FISTPw %s ; MF=16-bit integer",IPDecPrint16(mrm,disp,2,RC_FPUREG));
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
                            w += snprintf(w,(size_t)(wf-w),"FILDq %s ; MF=64-bit integer",IPDecPrint16(mrm,disp,8,RC_FPUREG));
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
                            w += snprintf(w,(size_t)(wf-w),"FISTPq %s ; MF=64-bit integer",IPDecPrint16(mrm,disp,8,RC_FPUREG));
#endif
                            break;
                        default:
#ifdef DECOMPILEMODE
                            w += snprintf(w,(size_t)(wf-w),"(invalid FPU opcode %02Xh %02Xh)",op1,mrm.byte);
                            break;
#else
                            goto invalidopcode;
#endif
                    };
                    break;

