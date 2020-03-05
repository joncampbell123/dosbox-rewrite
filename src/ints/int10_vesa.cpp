/*
 *  Copyright (C) 2002-2019  The DOSBox Team
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


#include <string.h>
#include <stddef.h>

#include "dosbox.h"
#include "callback.h"
#include "regs.h"
#include "mem.h"
#include "inout.h"
#include "int10.h"
#include "render.h"
#include "dos_inc.h"

#define VESA_FAIL             0x01

void VESA_OnReset_Clear_Callbacks(void) {
}

Bit8u VESA_GetSVGAInformation(Bit16u seg,Bit16u off) {
	return VESA_FAIL;
}

Bit8u VESA_GetSVGAModeInformation(Bit16u mode,Bit16u seg,Bit16u off) {
	return VESA_FAIL;
}

Bit8u VESA_SetSVGAMode(Bit16u mode) {
	return VESA_FAIL;
}

Bit8u VESA_GetSVGAMode(Bit16u & mode) {
	return VESA_FAIL;
}

Bit8u VESA_SetCPUWindow(Bit8u window,Bit8u address) {
	return VESA_FAIL;
}

Bit8u VESA_GetCPUWindow(Bit8u window,Bit16u & address) {
	return VESA_FAIL;
}

Bit8u VESA_SetPalette(PhysPt data,Bitu index,Bitu count,bool wait) {
	return VESA_FAIL;
}

Bit8u VESA_GetPalette(PhysPt data,Bitu index,Bitu count) {
	return VESA_FAIL;
}

Bit8u VESA_ScanLineLength(Bit8u subcall,Bit16u val, Bit16u & bytes,Bit16u & pixels,Bit16u & lines) {
	return VESA_FAIL;
}

Bit8u VESA_SetDisplayStart(Bit16u x,Bit16u y,bool wait) {
	return VESA_FAIL;
}

Bit8u VESA_GetDisplayStart(Bit16u & x,Bit16u & y) {
	return VESA_FAIL;
}

Bitu INT10_WriteVESAModeList(Bitu max_modes) {
    return 0;
}

void INT10_SetupVESA(void) {
}

