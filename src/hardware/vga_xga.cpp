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
#include "dosbox.h"
#include "inout.h"
#include "vga.h"
#include <math.h>
#include <stdio.h>
#include "callback.h"
#include "cpu.h"		// for 0x3da delay

void XGA_Write_Multifunc(Bitu val, Bitu len) {
    (void)val;
    (void)len;
}

Bitu XGA_Read_Multifunc() {
    return 0;
}

void XGA_DrawPoint(Bitu x, Bitu y, Bitu c) {
    (void)x;
    (void)y;
    (void)c;
}

Bitu XGA_GetPoint(Bitu x, Bitu y) {
    (void)x;
    (void)y;
	return 0;
}

Bitu XGA_GetMixResult(Bitu mixmode, Bitu srcval, Bitu dstdata) {
    (void)mixmode;
    (void)srcval;
    (void)dstdata;
	return 0;
}

void XGA_DrawLineVector(Bitu val) {
    (void)val;
}

void XGA_DrawLineBresenham(Bitu val) {
    (void)val;
}

void XGA_DrawRectangle(Bitu val) {
    (void)val;
}

bool XGA_CheckX(void) {
	return false;
}

void XGA_DrawWaitSub(Bitu mixmode, Bitu srcval) {
    (void)mixmode;
    (void)srcval;
}

void XGA_DrawWait(Bitu val, Bitu len) {
    (void)val;
    (void)len;
}

void XGA_BlitRect(Bitu val) {
    (void)val;
}

void XGA_DrawPattern(Bitu val) {
    (void)val;
}

void XGA_DrawCmd(Bitu val, Bitu len) {
    (void)val;
    (void)len;
}

void XGA_SetDualReg(Bit32u& reg, Bitu val) {
    (void)reg;
    (void)val;
}

Bitu XGA_GetDualReg(Bit32u reg) {
    (void)reg;
	return 0;
}

void XGA_Write(Bitu port, Bitu val, Bitu len) {
    (void)port;
    (void)val;
    (void)len;
}

Bitu XGA_Read(Bitu port, Bitu len) {
    (void)port;
    (void)len;
	return 0xffffffff; 
}

void VGA_SetupXGA(void) {
}

