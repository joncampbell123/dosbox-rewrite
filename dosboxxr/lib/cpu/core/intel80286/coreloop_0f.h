/* sub-sub fragment for 286 0F prefix */
                case 0x0F: // FPU ESCAPE + 0x0
                    switch (op0F=IPFB()) {
                        default:
#ifdef DECOMPILEMODE
                        w += snprintf(w,(size_t)(wf-w),"(invalid opcode %02Xh %02Xh)",op1,op0F);
                        break;
#else
                        goto invalidopcode;
#endif
                    };
                    break;

