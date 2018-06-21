
#ifdef HAVE_CONFIG_H
# include "config.h" // must be first
#endif

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>

#if HAVE_ENDIAN_H
# include <endian.h>
#else
# include "dosboxxr/lib/util/endian.h"
#endif

#include "dosboxxr/lib/cpu/ipdec.h"
#include "dosboxxr/lib/cpu/x86Vex.h"
#include "dosboxxr/lib/cpu/x86ModRegRm.h"
#include "dosboxxr/lib/cpu/x86ScaleIndexBase.h"
#include "dosboxxr/lib/cpu/memreftypes.h"
#include "dosboxxr/lib/util/case_groups.h"
#include "cpustep86common.h"

#include "dosboxxr/lib/cpu/ipdec_pre_core.h"

static inline void CPU_CMC(void) {
    cpu.flags() ^= CPU_FLAG_CF;
}

static inline void CPU_CLC(void) {
    cpu.flags() &= ~CPU_FLAG_CF;
}

static inline void CPU_STC(void) {
    cpu.flags() |= CPU_FLAG_CF;
}

static inline void CPU_CLI(void) {
    cpu.flags() &= ~CPU_FLAG_IF;
}

static inline void CPU_STI(void) {
    cpu.flags() |= CPU_FLAG_IF;
}

static inline void CPU_CLD(void) {
    cpu.flags() &= ~CPU_FLAG_DF;
}

static inline void CPU_STD(void) {
    cpu.flags() |= CPU_FLAG_DF;
}

void IPExec_Everything(x86_offset_t ip) {
    struct opcc_decom_state opstate(exe_code32);
    char *ipw,*ipwf;

    (void)ipwf;
    (void)ipw;

    /* one instruction only */
    ipwf = IPDecStr+sizeof(IPDecStr);
    ipw = IPDecStr;
    IPDecStr[0] = 0;
    IPDecIP = ip;

    /* jump to 32-bit code entry if that's the CPU mode */
    if (exe_code32)
        goto _x86decode_begin_code32_addr32;

    /* decode */
// COMMON CASE
_x86decode_begin_code16_addr16:
    opstate.code32 = opstate.addr32 = 0;
_x86decode_after_prefix_code16_addr16:
_x86decode_after_prefix_386override_code16_addr16:
#include "dosboxxr/lib/cpu/core/everything.c16.a16.emu.h"
    goto _x86done;
// UNCOMMON CASES
_x86decode_begin_code16_addr32:
_x86decode_after_prefix_code16_addr32:
_x86decode_after_prefix_386override_code16_addr32:
_x86decode_begin_code32_addr16:
_x86decode_after_prefix_code32_addr16:
_x86decode_after_prefix_386override_code32_addr16:
#include "dosboxxr/lib/cpu/core/everything.cxx.axx.emu.h"
    goto _x86done;
// COMMON CASE
_x86decode_begin_code32_addr32:
    opstate.code32 = opstate.addr32 = 1;
_x86decode_after_prefix_code32_addr32:
_x86decode_after_prefix_386override_code32_addr32:
#include "dosboxxr/lib/cpu/core/everything.c32.a32.emu.h"
    goto _x86done;
_x86decode_illegal_opcode:
_x86done:
    { } /* shut up GCC */
}
//// END CORE

