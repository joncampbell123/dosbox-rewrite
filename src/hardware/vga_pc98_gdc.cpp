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

using namespace std;

extern bool                 gdc_5mhz_mode;
extern bool                 gdc_5mhz_mode_initial;
extern bool                 GDC_vsync_interrupt;
extern bool                 pc98_256kb_boundary;
extern uint8_t              GDC_display_plane;
extern uint8_t              GDC_display_plane_pending;
extern uint8_t              GDC_display_plane_wait_for_vsync;

double                      gdc_proc_delay = 0.001; /* time from FIFO to processing in GDC (1us) FIXME: Is this right? */
bool                        gdc_proc_delay_set = false;
struct PC98_GDC_state       pc98_gdc[2];

void pc98_update_display_page_ptr(void) {
}

void pc98_update_cpu_page_ptr(void) {
}

void pc98_update_page_ptrs(void) {
}

void gdc_proc_schedule_delay(void);
void gdc_proc_schedule_cancel(void);
void gdc_proc_schedule_done(void);
void GDC_ProcDelay(Bitu /*val*/);
void PC98_show_cursor(bool show);
void pc98_port6A_command_write(unsigned char b);
void pc98_port68_command_write(unsigned char b);

PC98_GDC_state::PC98_GDC_state() {
}

size_t PC98_GDC_state::fifo_can_read(void) {
}

void PC98_GDC_state::take_reset_sync_parameters(void) {
}

void PC98_GDC_state::cursor_advance(void) {
}

void PC98_GDC_state::take_cursor_pos(unsigned char bi) {
}

void PC98_GDC_state::take_cursor_char_setup(unsigned char bi) {
}

void PC98_GDC_state::idle_proc(void) {
}

bool PC98_GDC_state::fifo_empty(void) {
    return true;
}

Bit16u PC98_GDC_state::read_fifo(void) {
    return 0;
}

void PC98_GDC_state::next_line(void) {
}

void PC98_GDC_state::begin_frame(void) {
}

void PC98_GDC_state::load_display_partition(void) {
}

void PC98_GDC_state::force_fifo_complete(void) {
}

void PC98_GDC_state::next_display_partition(void) {
}

void PC98_GDC_state::reset_fifo(void) {
}

void PC98_GDC_state::reset_rfifo(void) {
}

void PC98_GDC_state::flush_fifo_old(void) {
}

bool PC98_GDC_state::write_rfifo(const uint8_t c) {
    return true;
}

bool PC98_GDC_state::write_fifo(const uint16_t c) {
    return true;
}

bool PC98_GDC_state::write_fifo_command(const unsigned char c) {
    return true;
}

bool PC98_GDC_state::write_fifo_param(const unsigned char c) {
    return true;
}

bool PC98_GDC_state::rfifo_has_content(void) {
    return false;
}

uint8_t PC98_GDC_state::read_status(void) {
    return 0;
}

uint8_t PC98_GDC_state::rfifo_read_data(void) {
    return 0;
}

void gdc_proc_schedule_delay(void) {
}

void gdc_proc_schedule_cancel(void) {
}

void gdc_proc_schedule_done(void) {
}

void PC98_show_cursor(bool show) {
}

void GDC_ProcDelay(Bitu /*val*/) {
}

bool gdc_5mhz_according_to_bios(void) {
    return false;
}

void gdc_5mhz_mode_update_vars(void) {
}

/*==================================================*/

void pc98_gdc_write(Bitu port,Bitu val,Bitu iolen) {
}

Bitu pc98_gdc_read(Bitu port,Bitu iolen) {
    return ~0ul;
}

