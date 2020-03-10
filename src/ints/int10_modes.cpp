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


#include <stdlib.h>
#include <string.h>

#include "dosbox.h"
#include "mem.h"
#include "inout.h"
#include "int10.h"
#include "vga.h"
#include "bios.h"
#include "programs.h"
#include "render.h"

void InitVGAMode() {
    for (Bit16u ct=0;ct<(80*25);ct++) real_writew(0xB800,ct*2,0x0720);
    real_writew(BIOSMEM_SEG,BIOSMEM_NB_COLS,(Bit16u)80);
    real_writeb(BIOSMEM_SEG,BIOSMEM_NB_ROWS,(Bit8u)(25-1));
    INT10_SetCursorPos(0,0,0);
}

