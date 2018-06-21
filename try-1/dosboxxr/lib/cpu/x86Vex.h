
#ifndef DOSBOXXR_LIB_CPUX86VEX_H
#define DOSBOXXR_LIB_CPUX86VEX_H

#include <stdint.h>

struct x86Vex { //32-bit only
    /* X86 VEX decoding 2-byte
     *
     *    7    6    5    4    3    2    1    0
     * +====+====+====+====+====+====+====+====+
     * |  1    1    0    0    0    1    0    1 |   LDS/LES opcode
     * +====+====+====+====+====+====+====+====+
     * | ~R   ~v3  ~v2  ~v1  ~v0   L   p1   p0 |
     * +====+====+====+====+====+====+====+====+
     *
     * X86 VEX decoding 3-byte
     *
     *    7    6    5    4    3    2    1    0
     * +====+====+====+====+====+====+====+====+
     * |  1    1    0    0    0    1    0    0 |   LDS/LES opcode
     * +====+====+====+====+====+====+====+====+
     * | ~R   ~X   ~B   m4   m3   m2   m1   m0 |
     * +====+====+====+====+====+====+====+====+
     * |  W   ~v3  ~v2  ~v1  ~v0   L   p1   p0 |
     * +====+====+====+====+====+====+====+====+
     */
public:
    bool                    W;      // W
    bool                    L;      // L
    unsigned char           V;      // v2-v0
    unsigned char           P;      // p1-p0
    unsigned char           M;      // m4-m0
public:
    x86Vex(void) { }
    void decode2(const uint8_t b2) {
        W = 0;
        L = (b2 >> 2) & 1;
        V = ((~b2) >> 3) & 7;
        P = b2 & 3;
        M = 1; // 0x0F prefix
    }
    void decode3(const uint8_t b2,const uint8_t b3) {
        W = (b3 >> 7) & 1;
        L = (b3 >> 2) & 1;
        V = ((~b3) >> 3) & 7;
        P = b3 & 3;
        M = b2 & 0x1F;
    }
};

#endif //DOSBOXXR_LIB_CPUX86VEX_H

