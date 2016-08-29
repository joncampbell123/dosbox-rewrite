
extern x86_offset_t            exe_ip;
extern unsigned char*          exe_ip_ptr;
extern unsigned char*          exe_image;
extern unsigned char*          exe_image_fence;
extern bool                    exe_code32;

// include header core requires this
static inline bool IPcontinue(void) {
    return (exe_ip_ptr < exe_image_fence);
}

// include header core requires this
static inline x86_offset_t IPval(void) {
    return exe_ip;
}

// include header core requires this
static inline uint8_t IPFB(void) {
    const uint8_t r = *((const uint8_t*)exe_ip_ptr);
    exe_ip_ptr += 1;
    exe_ip += 1;
    return r;
}

// include header core requires this
static inline uint16_t IPFW(void) {
    const uint16_t r = le16toh(*((const uint16_t*)exe_ip_ptr));
    exe_ip_ptr += 2;
    exe_ip += 2;
    return r;
}

// include header core requires this
static inline uint32_t IPFDW(void) {
    const uint32_t r = le32toh(*((const uint32_t*)exe_ip_ptr));
    exe_ip_ptr += 4;
    exe_ip += 4;
    return r;
}

#pragma pack(push,1)
union x86_cpu_regpack {
    uint32_t                d;
    struct { // fixme: switch order if big endian
        uint16_t            w;
        uint16_t            x;
    } w;
    struct { // fixme: switch order if big endian
        uint8_t             l;
        uint8_t             h;
        uint16_t            x;
    } b;
};
#pragma pack(pop)

enum x86_cpu_regorder_t {
    CPU_REGI_AX,
    CPU_REGI_CX,
    CPU_REGI_DX,
    CPU_REGI_BX,
    CPU_REGI_SP,
    CPU_REGI_BP,
    CPU_REGI_SI,
    CPU_REGI_DI
};

struct x86_cpu_state {
    x86_cpu_state() {
        size_t i;

        for (i=0;i < 8;i++) reg[i].d = 0;
    }
public: // state
    x86_cpu_regpack         reg[8];
public: // 8-bit regs
    inline uint8_t &al(void) { return reg[CPU_REGI_AX].b.l; }
    inline uint8_t &bl(void) { return reg[CPU_REGI_BX].b.l; }
    inline uint8_t &cl(void) { return reg[CPU_REGI_CX].b.l; }
    inline uint8_t &dl(void) { return reg[CPU_REGI_DX].b.l; }
    inline uint8_t &ah(void) { return reg[CPU_REGI_AX].b.h; }
    inline uint8_t &bh(void) { return reg[CPU_REGI_BX].b.h; }
    inline uint8_t &ch(void) { return reg[CPU_REGI_CX].b.h; }
    inline uint8_t &dh(void) { return reg[CPU_REGI_DX].b.h; }
public: // 16-bit regs
    inline uint16_t &ax(void) { return reg[CPU_REGI_AX].w.w; }
    inline uint16_t &bx(void) { return reg[CPU_REGI_BX].w.w; }
    inline uint16_t &cx(void) { return reg[CPU_REGI_CX].w.w; }
    inline uint16_t &dx(void) { return reg[CPU_REGI_DX].w.w; }
    inline uint16_t &sp(void) { return reg[CPU_REGI_SP].w.w; }
    inline uint16_t &bp(void) { return reg[CPU_REGI_BP].w.w; }
    inline uint16_t &si(void) { return reg[CPU_REGI_SI].w.w; }
    inline uint16_t &di(void) { return reg[CPU_REGI_DI].w.w; }
public: // 32-bit regs
    inline uint32_t &eax(void) { return reg[CPU_REGI_AX].d; }
    inline uint32_t &ebx(void) { return reg[CPU_REGI_BX].d; }
    inline uint32_t &ecx(void) { return reg[CPU_REGI_CX].d; }
    inline uint32_t &edx(void) { return reg[CPU_REGI_DX].d; }
    inline uint32_t &esp(void) { return reg[CPU_REGI_SP].d; }
    inline uint32_t &ebp(void) { return reg[CPU_REGI_BP].d; }
    inline uint32_t &esi(void) { return reg[CPU_REGI_SI].d; }
    inline uint32_t &edi(void) { return reg[CPU_REGI_DI].d; }
};

extern x86_cpu_state               cpu;

extern uint8_t * const cpuref_regs_8[8];
extern uint16_t * const cpuref_regs_16[8];
extern uint32_t * const cpuref_regs_32[8];

static inline void IPFB_LEA16(x86ModRegRm &mrm,x86ScaleIndexBase &sib,x86_offset_t &disp) {
    (void)sib; // unused

    if (mrm.mod() != 3) {
        if (mrm.mod() == 0 && mrm.rm() == 6) // direct offset, no register addition
            return;

        // MRM decoding has already decoded offset.
        // our job is to add the CPU registers to the offset.
        switch (mrm.rm()) {
            case 0: disp += cpu.bx() + cpu.si(); break;
            case 1: disp += cpu.bx() + cpu.di(); break;
            case 2: disp += cpu.bp() + cpu.si(); break;
            case 3: disp += cpu.bp() + cpu.di(); break;
            case 4: disp += cpu.si(); break;
            case 5: disp += cpu.di(); break;
            case 6: disp += cpu.bp(); break;
            case 7: disp += cpu.bx(); break;
            default: break;
        }

        disp &= (x86_offset_t)0xFFFFU;
    }
}

static inline void IPFB_LEA32(x86ModRegRm &mrm,x86ScaleIndexBase &sib,x86_offset_t &disp) {
    if (mrm.mod() != 3) {
        if (mrm.mod() == 0 && mrm.rm() == 5) // direct offset, no register addition
            return;

        if (mrm.rm() == 4) { // offset += [indexreg<<scale + basereg]
            if (sib.index() != 4) disp += (x86_offset_t)(*cpuref_regs_32[sib.index()]) << (x86_offset_t)sib.scale();
            disp += (x86_offset_t)(*cpuref_regs_32[sib.base()]);
        }
        else { // offset += [r/m reg]
            disp += (x86_offset_t)(*cpuref_regs_32[mrm.rm()]);
        }
    }
}

