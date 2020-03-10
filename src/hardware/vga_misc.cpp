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


#include "dosbox.h"
#include "inout.h"
#include "pic.h"
#include "vga.h"
#include <math.h>

void vga_write_p3d4(Bitu port,Bitu val,Bitu iolen);
void vga_write_p3d5(Bitu port,Bitu val,Bitu iolen);

Bitu vga_read_p3da(Bitu port,Bitu iolen) {
    (void)port;//UNUSED
    (void)iolen;//UNUSED
	Bit8u retval = 0;
	double timeInFrame = PIC_FullIndex()-vga.draw.delay.framestart;

	// 3DAh (R):  Status Register
	// bit   0  Horizontal or Vertical blanking
	//       3  Vertical sync

	if (timeInFrame >= vga.draw.delay.vdend) {
		retval |= 1; // vertical blanking
	} else {
		double timeInLine=fmod(timeInFrame,vga.draw.delay.htotal);
		if (timeInLine >= vga.draw.delay.hblkstart && 
			timeInLine <= vga.draw.delay.hblkend) {
			retval |= 1; // horizontal blanking
		}
	}

    if (timeInFrame >= vga.draw.delay.vrstart &&
        timeInFrame <= vga.draw.delay.vrend) {
        retval |= 8; // vertical retrace
    }

	return retval;
}

void VGA_SetupMisc(void) {
}

void VGA_UnsetupMisc(void) {
    IO_FreeWriteHandler(0x3d4,IO_MB);
    IO_FreeReadHandler(0x3d4,IO_MB);
    IO_FreeWriteHandler(0x3d5,IO_MB);
    IO_FreeReadHandler(0x3d5,IO_MB);
    IO_FreeWriteHandler(0x3da,IO_MB);
    IO_FreeReadHandler(0x3da,IO_MB);
}

