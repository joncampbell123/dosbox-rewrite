
#ifdef HAVE_CONFIG_H
# include "config.h" // must be first
#endif

#include <stdio.h>
#include <assert.h>

#include "dosboxxr/lib/cpu/x86ModRegRm.h"

int main() {
    {
        x86ModRegRm a;
        a.set(0xFF);
        assert(a.byte == 0xFF);
        assert(a.mod() == 3);
        assert(a.reg() == 7);
        assert(a.rm() == 7);
    }

    {
        x86ModRegRm a;
        a.set(3,7,7);
        assert(a.byte == 0xFF);
        assert(a.mod() == 3);
        assert(a.reg() == 7);
        assert(a.rm() == 7);
    }

    {
        x86ModRegRm a;
        a.set(1,2,3);
        assert(a.byte == ((1<<6)+(2<<3)+3));
        assert(a.mod() == 1);
        assert(a.reg() == 2);
        assert(a.rm() == 3);
    }

    {
        x86ModRegRm a(0xFF);
        assert(a.byte == 0xFF);
        assert(a.mod() == 3);
        assert(a.reg() == 7);
        assert(a.rm() == 7);
    }

    {
        x86ModRegRm a(3,7,7);
        assert(a.byte == 0xFF);
        assert(a.mod() == 3);
        assert(a.reg() == 7);
        assert(a.rm() == 7);
    }

    {
        x86ModRegRm a(1,2,3);
        assert(a.byte == ((1<<6)+(2<<3)+3));
        assert(a.mod() == 1);
        assert(a.reg() == 2);
        assert(a.rm() == 3);
    }

    printf("All tests passed\n");
    return 0;
}

