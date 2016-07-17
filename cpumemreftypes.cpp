
#ifdef HAVE_CONFIG_H
# include "config.h" // must be first
#endif

#include <stdio.h>
#include <assert.h>

#include "dosboxxr/lib/cpu/memreftypes.h"

int main() {
    {
        x86_realptr_t rp;

        rp = x86_realptr(0xABCDU,0x1234U);
        assert(rp == 0xABCD1234UL);
        assert(x86_realseg(rp) == 0xABCDU);
        assert(x86_realoff(rp) == 0x1234U);
    }
    {
        x86_farptr16_t rp;

        rp = x86_farptr16(0xABCDU,0x1234U);
        assert(rp == 0xABCD1234UL);
        assert(x86_far16seg(rp) == 0xABCDU);
        assert(x86_far16off(rp) == 0x1234U);
    }

    printf("All tests passed\n");
    return 0;
}

