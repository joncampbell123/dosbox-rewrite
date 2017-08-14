
enum opccSeg {
    OPSEG_NONE=-1,
    OPSEG_CS=0,
    OPSEG_DS,
    OPSEG_ES,
    OPSEG_FS,
    OPSEG_GS,
    OPSEG_SS
};

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
//        case 3:
//            return 0;
    };

    return 0;
}

// given mod/reg/rm fetch displacement (32-bit code)
static inline x86_offset_t IPFmrmdisplace32(x86ModRegRm &mrm,x86ScaleIndexBase &sib) {
    switch (mrm.mod()) {
        case 0:
            if (mrm.rm() == 5) return IPFDW();
            if (mrm.rm() == 4) {
                sib.byte = IPFB();
                if (sib.base() == 5) return IPFDW();
            }
            return 0;
        case 1:
            if (mrm.rm() == 4) sib.byte = IPFB();
            return IPFBsigned();
        case 2:
            if (mrm.rm() == 4) sib.byte = IPFB();
            return IPFDW();
//        case 3:
//            return 0;
    };

    return 0;
}

// 16-bit addressing
static inline void IPFB_mrm_sib_disp_a16_read(x86ModRegRm &mrm,x86ScaleIndexBase &sib,x86_offset_t &disp) {
    (void)sib; // unused

    IPFModRegRm(mrm);
    disp = IPFmrmdisplace16(mrm);
}

static inline void IPFB_mrm_sib_disp_a32_read(x86ModRegRm &mrm,x86ScaleIndexBase &sib,x86_offset_t &disp) {
    IPFModRegRm(mrm);
    disp = IPFmrmdisplace32(mrm,sib);
}

