
static inline int32_t IPFBsigned(void) {
    return (int32_t)((int8_t)IPFB());
}

static inline int32_t IPFWsigned(void) {
    return (int32_t)((int16_t)IPFW());
}

static inline int32_t IPFDWsigned(void) {
    return (int32_t)IPFDW();
}

static inline void IPFModRegRm(x86ModRegRm &m) {
    m.byte = IPFB();
}

// given mod/reg/rm fetch displacement (16-bit code)
static inline x86_offset_t IPFmrmdisplace16(x86ModRegRm &mrm) {
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

// given mod/reg/rm fetch displacement (32-bit code)
static inline x86_offset_t IPFmrmdisplace32(x86ModRegRm &mrm) {
    switch (mrm.mod()) {
        case 0:
            if (mrm.rm() == 5) return IPFDW();
            return 0;
        case 1:
            return IPFBsigned();
        case 2:
            return IPFDW();
        case 3:
            return 0;
    };
}

static inline void IPDec386Load_MRM_SIB(x86ModRegRm &mrm,x86ScaleIndexBase &sib,x86_offset_t &disp,const bool addr32) {
    IPFModRegRm(mrm);

    if (addr32) {
        if (mrm.has_a32_SIB()) sib.byte = IPFB();
        disp = IPFmrmdisplace32(mrm);
    }
    else {
        disp = IPFmrmdisplace16(mrm);
    }
}

