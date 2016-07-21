
static inline int32_t IPFBsigned(void) {
    return (int32_t)((int8_t)IPFB());
}

static inline int32_t IPFWsigned(void) {
    return (int32_t)((int16_t)IPFW());
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

