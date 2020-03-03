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
#include "menu.h"
#include "util_units.h"
#include "control.h"
#include "pc98_cg.h"
#include "pc98_dac.h"
#include "pc98_gdc.h"
#include "pc98_gdc_const.h"
#include "mixer.h"

void pc98_update_page_ptrs(void);

extern bool                 pc98_40col_text;
extern bool                 pc98_31khz_mode;
extern bool                 pc98_attr4_graphic;
extern bool                 pc98_display_enable;
extern bool                 pc98_graphics_hide_odd_raster_200line;
extern bool                 pc98_crt_mode;      // see port 6Ah command 40h/41h.

// TODO: Other parts that change gdc_5mhz_mode should update these too!
bool                        gdc_clock_1 = false;
bool                        gdc_clock_2 = false;

bool                        gdc_5mhz_mode = false;
bool                        enable_pc98_egc = true;
bool                        enable_pc98_grcg = true;
bool                        enable_pc98_16color = true;
bool                        enable_pc98_256color = true;
bool                        enable_pc98_256color_planar = true;
bool                        enable_pc98_188usermod = true;
bool                        pc98_256kb_boundary = false;         /* port 6Ah command 68h/69h */
bool                        GDC_vsync_interrupt = false;
bool                        gdc_5mhz_mode_initial = false;
uint8_t                     GDC_display_plane_wait_for_vsync = false;
uint8_t                     GDC_display_plane_pending = false;
uint8_t                     GDC_display_plane = false;

uint8_t                     pc98_gdc_tile_counter=0;
uint8_t                     pc98_gdc_modereg=0;
uint8_t                     pc98_gdc_vramop=0;
egc_quad                    pc98_gdc_tiles;

extern unsigned char        pc98_text_first_row_scanline_start;  /* port 70h */
extern unsigned char        pc98_text_first_row_scanline_end;    /* port 72h */
extern unsigned char        pc98_text_row_scanline_blank_at;     /* port 74h */
extern unsigned char        pc98_text_row_scroll_lines;          /* port 76h */
extern unsigned char        pc98_text_row_scroll_count_start;    /* port 78h */
extern unsigned char        pc98_text_row_scroll_num_lines;      /* port 7Ah */

// the guest can change the GDC to 5MHz by setting both GDC clock bits.
// NTS: For whatever reason, Windows 3.1 will set the GDC to 5MHz when entering a DOS application fullscreen.
void gdc_clock_check(void) {
}

void pc98_crtc_write(Bitu port,Bitu val,Bitu iolen) {
}

Bitu pc98_crtc_read(Bitu port,Bitu iolen) {
    return ~0ul;
}

bool egc_enable_enable = false;

void update_gdc_analog(void) {
}

void pc98_port6A_command_write(unsigned char b) {
}

void pc98_port68_command_write(unsigned char b) {
}

unsigned char sel_9a0 = 0;

Bitu pc98_read_9a0(Bitu /*port*/,Bitu /*iolen*/) {
    return ~0ul;
}

void pc98_write_9a0(Bitu port,Bitu val,Bitu iolen) {
}

Bitu pc98_read_9a8(Bitu /*port*/,Bitu /*iolen*/) {
    return ~0ul;
}

void pc98_write_9a8(Bitu /*port*/,Bitu val,Bitu /*iolen*/) {
}

