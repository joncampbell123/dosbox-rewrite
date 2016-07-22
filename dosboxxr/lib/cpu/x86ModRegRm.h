
#ifndef DOSBOXXR_LIB_CPU_X86MODREGRM_H
#define DOSBOXXR_LIB_CPU_X86MODREGRM_H

#include <stdint.h>

struct x86ModRegRm {
    /* X86 MOD/REG/RM byte
     *
     *    7    6    5    4    3    2    1    0
     * +====+====+====+====+====+====+====+====+
     * |   MOD   |      REG     |      RM      |
     * +====+====+====+====+====+====+====+====+ */
public:
    uint8_t         byte;
public:
    x86ModRegRm(void) { }
    x86ModRegRm(const uint8_t _byte) : byte(_byte) { }
    x86ModRegRm(const uint8_t mod,const uint8_t reg,const uint8_t rm) { set(mod,reg,rm); }

    static inline uint8_t encode(const uint8_t mod,const uint8_t reg,const uint8_t rm) {
        return (mod << 6) + (reg << 3) + rm; // WARNING: No protection against illegal values.
        // illegal values are:
        //    mod > 3
        //    reg > 7
        //    rm > 7
    }

    inline void set(const uint8_t mod,const uint8_t reg,const uint8_t rm) {
        byte = encode(mod,reg,rm);
    }
    inline void set(const uint8_t _byte) {
        byte = _byte;
    }

    inline uint8_t rm(void) const {
        return rm(byte);
    }
    static inline uint8_t rm(const uint8_t byte) {
        return byte & 7;
    }

    inline uint8_t reg(void) const {
        return reg(byte);
    }
    static inline uint8_t reg(const uint8_t byte) {
        return (byte >> 3) & 7;
    }

    inline uint8_t mod(void) const {
        return mod(byte);
    }
    static inline uint8_t mod(const uint8_t byte) {
        return byte >> 6;
    }

    // for 386 emulation: whether or not a SIB byte would follow when addr32 == true
    static inline bool has_a32_SIB(const uint8_t byte) {
        return (byte < 0xC0) && ((byte & 0x07) == 0x04); // mod != 3 and r/m == 4
    }
    inline bool has_a32_SIB(void) {
        return has_a32_SIB(byte);
    }
};

#endif //DOSBOXXR_LIB_CPU_X86MODREGRM_H

