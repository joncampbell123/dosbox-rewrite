
#ifndef DOSBOXXR_LIB_CPU_X86SCALEINDEXBYTE_H
#define DOSBOXXR_LIB_CPU_X86SCALEINDEXBYTE_H

#include <stdint.h>

struct x86ScaleIndexBase {
    /* X86 MOD/REG/RM byte
     *
     *    7    6    5    4    3    2    1    0
     * +====+====+====+====+====+====+====+====+
     * |  SCALE  |    INDEX     |     BASE     |
     * +====+====+====+====+====+====+====+====+ */
public:
    uint8_t         byte;
public:
    x86ScaleIndexBase(void) { }
    x86ScaleIndexBase(const uint8_t _byte) : byte(_byte) { }
    x86ScaleIndexBase(const uint8_t scale,const uint8_t index,const uint8_t base) { set(scale,index,base); }

    static inline uint8_t encode(const uint8_t scale,const uint8_t index,const uint8_t base) {
        return (scale << 6) + (index << 3) + base; // WARNING: No protection against illegal values.
        // illegal values are:
        //    scale > 3
        //    index > 7
        //    base > 7
    }

    inline void set(const uint8_t scale,const uint8_t index,const uint8_t base) {
        byte = encode(scale,index,base);
    }
    inline void set(const uint8_t _byte) {
        byte = _byte;
    }

    inline uint8_t base(void) const {
        return base(byte);
    }
    static inline uint8_t base(const uint8_t byte) {
        return byte & 7;
    }

    inline uint8_t index(void) const {
        return index(byte);
    }
    static inline uint8_t index(const uint8_t byte) {
        return (byte >> 3) & 7;
    }

    inline uint8_t scale(void) const {
        return scale(byte);
    }
    static inline uint8_t scale(const uint8_t byte) {
        return byte >> 6;
    }
};

#endif //DOSBOXXR_LIB_CPU_X86SCALEINDEXBYTE_H

