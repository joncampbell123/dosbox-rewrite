
#ifndef DOSBOXXR_LIB_UTIL_BITSCAN_H
#define DOSBOXXR_LIB_UTIL_BITSCAN_H

/* TODO: On i686 and x86_64 there are BSF, etc. instructions. Can we use those instead when inline ASM allowed? */

static inline unsigned int bitscan_forward(const uint32_t v,unsigned int bit) {
    while (bit < 32) {
        if (!(v & (1U << bit)))
            bit++;
        else
            return bit;
    }

    return bit;
}

static inline unsigned int bitscan_count(const uint32_t v,unsigned int bit) {
    while (bit < 32) {
        if (v & (1U << bit))
            bit++;
        else
            return bit;
    }

    return bit;
}

#endif //DOSBOXXR_LIB_UTIL_BITSCAN_H

