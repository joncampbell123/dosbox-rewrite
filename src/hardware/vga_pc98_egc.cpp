/*
 *  Copyright (C) 2018  Jon Campbell
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1335, USA.
 */

#include "dosbox.h"
#include "setup.h"
#include "video.h"
#include "pic.h"
#include "vga.h"
#include "inout.h"
#include "programs.h"
#include "support.h"
#include "setup.h"
#include "timer.h"
#include "mem.h"
#include "util_units.h"
#include "control.h"
#include "pc98_cg.h"
#include "pc98_dac.h"
#include "pc98_gdc.h"
#include "pc98_gdc_const.h"
#include "mixer.h"

#include <string.h>
#include <stdlib.h>
#include <string>
#include <stdio.h>

#if defined(_MSC_VER)
#pragma warning(disable:4065) /* switch statements without case labels */
#endif

void pc98_egc_shift_reinit();

uint16_t                    pc98_egc_raw_values[8] = {0};

uint8_t                     pc98_egc_access=0;
uint8_t                     pc98_egc_srcmask[2]; /* host given (Neko: egc.srcmask) */
uint8_t                     pc98_egc_maskef[2]; /* effective (Neko: egc.mask2) */
uint8_t                     pc98_egc_mask[2]; /* host given (Neko: egc.mask) */

uint8_t                     pc98_egc_fgc = 0;
uint8_t                     pc98_egc_lead_plane = 0;
uint8_t                     pc98_egc_compare_lead = 0;
uint8_t                     pc98_egc_lightsource = 0;
uint8_t                     pc98_egc_shiftinput = 0;
uint8_t                     pc98_egc_regload = 0;
uint8_t                     pc98_egc_rop = 0xF0;
uint8_t                     pc98_egc_foreground_color = 0;
uint8_t                     pc98_egc_background_color = 0;

bool                        pc98_egc_shift_descend = false;
uint8_t                     pc98_egc_shift_destbit = 0;
uint8_t                     pc98_egc_shift_srcbit = 0;
uint16_t                    pc98_egc_shift_length = 0xF;

Bitu pc98_egc4a0_read(Bitu port,Bitu iolen) {
    return ~0ul;
}

void pc98_egc4a0_write(Bitu port,Bitu val,Bitu iolen) {
}

Bitu pc98_egc4a0_read_warning(Bitu port,Bitu iolen) {
    return ~0ul;
}

void pc98_egc4a0_write_warning(Bitu port,Bitu val,Bitu iolen) {
}

