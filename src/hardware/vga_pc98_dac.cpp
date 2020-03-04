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
#include "mixer.h"

#include <string.h>
#include <stdlib.h>
#include <string>
#include <stdio.h>

using namespace std;

extern bool                 vga_8bit_dac;

uint32_t                    pc98_text_palette[8];
uint8_t                     pc98_16col_analog_rgb_palette_index = 0;

uint8_t                     pc98_pal_vga[256*3];    /* G R B    0x0..0xFF */
uint8_t                     pc98_pal_analog[256*3]; /* G R B    0x0..0xF */
uint8_t                     pc98_pal_digital[8];    /* G R B    0x0..0x7 */

void pc98_update_palette(void) {
}

void pc98_update_digpal(unsigned char ent) {
}

void pc98_set_digpal_entry(unsigned char ent,unsigned char grb) {
}

void pc98_set_digpal_pair(unsigned char start,unsigned char pair) {
}

unsigned char pc98_get_digpal_pair(unsigned char start) {
    return 0;
}

