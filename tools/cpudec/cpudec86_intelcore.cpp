
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
#include "dosboxxr/lib/cpu/x86ModRegRm.h"
#include "dosboxxr/lib/cpu/x86ScaleIndexBase.h"
#include "dosboxxr/lib/cpu/memreftypes.h"
#include "dosboxxr/lib/util/case_groups.h"
#include "cpudec86common.h"

#include "dosboxxr/lib/cpu/ipdec_pre_core.h"

void IPDec_IntelCore(x86_offset_t ip) {
    bool code32=exe_code32;
    bool addr32=exe_code32;
    x86ScaleIndexBase sib;
    bool prefix66=false;
    bool prefix67=false;
    x86_offset_t disp;
    uint32_t imm,imm2;
    char *ipw,*ipwf;
    x86ModRegRm mrm;
    uint8_t op;

    /* one instruction only */
    ipwf = IPDecStr+sizeof(IPDecStr);
    ipw = IPDecStr;
    IPDecStr[0] = 0;
    IPDecIP = ip;

    /* decode */
    code32 = addr32 = exe_code32;
_x86decode_begin_code16_addr32:
_x86decode_begin_code32_addr16:
_x86decode_after_prefix_code16_addr16:
_x86decode_after_prefix_code16_addr32:
_x86decode_after_prefix_code32_addr16:
_x86decode_after_prefix_code32_addr32:
_x86decode_after_prefix_386override_code16_addr16:
_x86decode_after_prefix_386override_code16_addr32:
_x86decode_after_prefix_386override_code32_addr16:
_x86decode_after_prefix_386override_code32_addr32:
#include "dosboxxr/lib/cpu/core/intelCore.cxx.axx.decom.h"
    goto _x86done;
_x86decode_illegal_opcode:
_x86done:
    { } /* shut up GCC */
}
//// END CORE

