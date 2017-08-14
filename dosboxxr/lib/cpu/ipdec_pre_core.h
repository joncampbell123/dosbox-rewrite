
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
    OPSEG_IMM=-2,
    OPSEG_NONE=-1,
    OPSEG_CS=0,
    OPSEG_DS,
    OPSEG_ES,
    OPSEG_FS,
    OPSEG_GS,
    OPSEG_SS
};

/* 8-bit, CPU reg order */
enum {
    OPREG_REG_AL=0,     OPREG_REG_CL,       OPREG_REG_DL,       OPREG_REG_BL,
    OPREG_REG_AH,       OPREG_REG_CH,       OPREG_REG_DH,       OPREG_REG_BH,
    OPREG_REG_R8L,      OPREG_REG_R9L,      OPREG_REG_R10L,     OPREG_REG_R11L,
    OPREG_REG_R12L,     OPREG_REG_R13L,     OPREG_REG_R14L,     OPREG_REG_R15L,
    OPREG_REG_UNDEF1,   OPREG_REG_UNDEF2,   OPREG_REG_UNDEF3,   OPREG_REG_UNDEF4,
    OPREG_REG_SPL,      OPREG_REG_BPL,      OPREG_REG_SIL,      OPREG_REG_DIL
};

/* 16-bit, CPU reg order */
enum {
    OPREG_REG_AX=0, OPREG_REG_CX,   OPREG_REG_DX,   OPREG_REG_BX,
    OPREG_REG_SP,   OPREG_REG_BP,   OPREG_REG_SI,   OPREG_REG_DI
};

/* 32-bit, CPU reg order */
enum {
    OPREG_REG_EAX=0,    OPREG_REG_ECX,  OPREG_REG_EDX,  OPREG_REG_EBX,
    OPREG_REG_ESP,      OPREG_REG_EBP,  OPREG_REG_ESI,  OPREG_REG_EDI
};

/* 64-bit, CPU reg order */
enum {
    OPREG_REG_RAX=0,    OPREG_REG_RCX,  OPREG_REG_RDX,  OPREG_REG_RBX,
    OPREG_REG_RSP,      OPREG_REG_RBP,  OPREG_REG_RSI,  OPREG_REG_RDI,
    OPREG_REG_R8,       OPREG_REG_R9,   OPREG_REG_R10,  OPREG_REG_R11,
    OPREG_REG_R12,      OPREG_REG_R13,  OPREG_REG_R14,  OPREG_REG_R15,

    OPREG_REG_RIP /* AMD64 rip support */
};

/* MMX registers */
enum {
    OPREG_MMX0=0,   OPREG_MMX1,     OPREG_MMX2,     OPREG_MMX3,
    OPREG_MMX4,     OPREG_MMX5,     OPREG_MMX6,     OPREG_MMX7
};
/* or simply */
#define OPREG_MMX(x) (OPREG_MMX0+(x))

/* SSE registers */
enum {
    OPREG_XMM0=0,   OPREG_XMM1,     OPREG_XMM2,     OPREG_XMM3,
    OPREG_XMM4,     OPREG_XMM5,     OPREG_XMM6,     OPREG_XMM7,
    OPREG_XMM8,     OPREG_XMM9,     OPREG_XMM10,    OPREG_XMM11,
    OPREG_XMM12,    OPREG_XMM13,    OPREG_XMM14,    OPREG_XMM15
};
/* or simply */
#define OPREG_XMM(x) (OPREG_XMM0+(x))

/* FPU registers */
enum {
    OPREG_ST0,  OPREG_ST1,  OPREG_ST2,  OPREG_ST3,
    OPREG_ST4,  OPREG_ST5,  OPREG_ST6,  OPREG_ST7
};
#define OPREG_ST(x) (OPREG_ST0+(x))

/* register type */
enum {
    OPREG_RT_NONE=0,    /* not a register reference */
    OPREG_RT_IMM,       /* immediate value */
    OPREG_RT_REG,       /* general register */
    OPREG_RT_SREG,      /* segment register */
    OPREG_RT_ST,        /* FPU register */
    OPREG_RT_MMX,       /* MMX register */
    OPREG_RT_SSE,       /* SSE register */
    OPREG_RT_CR,        /* CR0...CR7 control registers */
    OPREG_RT_DR,        /* DR0...DR7 debug registers */
    OPREG_RT_TR         /* TR0...TR7 test registers */
};

/* decoded instruction operand (32-bit or 16-bit) */
struct opcc_argv {
    signed char         segment;    /* which segment */
    unsigned char       size;       /* size of the operand (1=byte 2=word 4=dword etc) */
    unsigned char       regtype;    /* type of register (0 if memory ref) */
    unsigned char       reg;
    unsigned char       memregsz;
    unsigned char       memregs;
    unsigned char       memreg[4];  /* memory register offsets to add together. note that [0] is scaled by SIB scalar part */
    unsigned char       scalar;
    uint16_t            segval;     /* segment value (if segment == -2) */
    uint32_t            memref_base;    /* base immediate offset */
    uint32_t            value;
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

