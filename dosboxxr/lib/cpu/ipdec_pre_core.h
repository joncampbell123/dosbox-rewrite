
#include "dosboxxr/lib/cpu/ipdec.h"
#include "dosboxxr/lib/cpu/x86Vex.h"
#include "dosboxxr/lib/cpu/x86ModRegRm.h"
#include "dosboxxr/lib/cpu/x86ScaleIndexBase.h"
#include "dosboxxr/lib/cpu/memreftypes.h"
#include "dosboxxr/lib/util/case_groups.h"

enum opccRep {
    OPREP_NONE=-1,
    OPREP_REPNZ,
    OPREP_REPZ,
    OPREP_REPNC,
    OPREP_REPC
};

enum opccSeg {
    OPSEG_NONE=-1,
    OPSEG_CS=0,
    OPSEG_DS,
    OPSEG_ES,
    OPSEG_FS,
    OPSEG_GS,
    OPSEG_SS
};

/* yes, I shamelessly copied this from minx86dec. so sue me for borrowing code from my own OSS projects. */
struct opcc_decom_state {
    bool                code32;
	bool			    addr32;
    bool                prefix66;
    bool                prefix67;
    bool                prefix_WAIT;
    bool                prefix_LOCK;
    uint8_t             op;
    signed char         segoverride;
    signed char         repmode;
	uint32_t		    ip_value;	/* IP instruction pointer value */
    uint32_t            imm,imm2;
    unsigned int        opcode;
    x86_offset_t        disp;
    x86ModRegRm         mrm;
    x86Vex              vex;
    x86ScaleIndexBase   sib;

    opcc_decom_state() : code32(false), addr32(false), prefix66(false), prefix67(false), prefix_WAIT(false), prefix_LOCK(false), segoverride(OPSEG_NONE), repmode(OPREP_NONE) { }
    opcc_decom_state(const bool _m32) : code32(_m32), addr32(_m32), prefix66(false), prefix67(false), prefix_WAIT(false), prefix_LOCK(false), segoverride(OPSEG_NONE), repmode(OPREP_NONE) { }
    opcc_decom_state(const bool _c32,const bool _a32) : code32(_c32), addr32(_a32), prefix66(false), prefix67(false), prefix_WAIT(false), prefix_LOCK(false), segoverride(OPSEG_NONE), repmode(OPREP_NONE) { }
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

