
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
#include "dosboxxr/lib/cpu/core/all_symbols.h"
#include "dosboxxr/lib/util/case_groups.h"
#include "cpudec86common.h"

#include "dosboxxr/lib/cpu/ipdec_pre_core.h"

void IPDec_8086(x86_offset_t ip) {
    x86ScaleIndexBase sib;
    bool prefix_WAIT = false;
    bool prefix_LOCK = false;
    signed char repmode = -1;
    signed char segoverride = -1;
    unsigned int opcode = 0;
    x86_offset_t disp;
    uint16_t imm,imm2;
    char *ipw,*ipwf;
    x86ModRegRm mrm;
    uint8_t op;

    /* one instruction only */
    ipwf = IPDecStr+sizeof(IPDecStr);
    ipw = IPDecStr;
    IPDecStr[0] = 0;
    IPDecIP = ip;

    /* decode */
_x86decode_begin_code16_addr16:
_x86decode_after_prefix_code16_addr16:
#include "dosboxxr/lib/cpu/core/intel8088.cxx.axx.decom.h"
    goto _x86done;
_x86decode_illegal_opcode:
_x86done:
    { } /* shut up GCC */
}
//// END CORE

