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
#include "vga.h"

#define attr(blah) vga.attr.blah

void VGA_ATTR_SetEGAMonitorPalette(EGAMonitorMode m) {
	// palette bit assignment:
	// bit | pin | EGA        | CGA       | monochrome
	// ----+-----+------------+-----------+------------
	// 0   | 5   | blue       | blue      | nc
	// 1   | 4   | green      | green*    | nc
	// 2   | 3   | red        | red*      | nc
	// 3   | 7   | blue sec.  | nc        | video
	// 4   | 6   | green sec. | intensity | intensity
	// 5   | 2   | red sec.   | nc        | nc
    // 6-7 | not used
	// * additive color brown instead of yellow
	switch(m) {
		case CGA:
			//LOG_MSG("Monitor CGA");
			for (Bitu i=0;i<64;i++) {
				vga.dac.rgb[i].red=((i & 0x4)? 0x2a:0) + ((i & 0x10)? 0x15:0);
				vga.dac.rgb[i].blue=((i & 0x1)? 0x2a:0) + ((i & 0x10)? 0x15:0);
				
				// replace yellow with brown
				if ((i & 0x17) == 0x6) vga.dac.rgb[i].green = 0x15;
				else vga.dac.rgb[i].green =
					((i & 0x2)? 0x2a:0) + ((i & 0x10)? 0x15:0);
			}
			break;
		case EGA:
			//LOG_MSG("Monitor EGA");
			for (Bitu i=0;i<64;i++) {
				vga.dac.rgb[i].red=((i & 0x4)? 0x2a:0) + ((i & 0x20)? 0x15:0);
				vga.dac.rgb[i].green=((i & 0x2)? 0x2a:0) + ((i & 0x10)? 0x15:0);
				vga.dac.rgb[i].blue=((i & 0x1)? 0x2a:0) + ((i & 0x8)? 0x15:0);
			}
			break;
		case MONO:
			//LOG_MSG("Monitor MONO");
			for (Bitu i=0;i<64;i++) {
				Bit8u value = ((i & 0x8)? 0x2a:0) + ((i & 0x10)? 0x15:0);
				vga.dac.rgb[i].red = vga.dac.rgb[i].green =
					vga.dac.rgb[i].blue = value;
			}
			break;
	}

	// update the mappings
	for (Bit8u i=0;i<0x10;i++)
		VGA_ATTR_SetPalette(i,vga.attr.palette[i]);
}

void VGA_ATTR_SetPalette(Bit8u index, Bit8u val) {
	// the attribute table stores only 6 bits
	val &= 63; 
	vga.attr.palette[index] = val;

    if (IS_VGA_ARCH) {
        // apply the plane mask
        val = vga.attr.palette[index & 0xF];

        // apply
        VGA_DAC_CombineColor(index,val);
    }
    else {
        VGA_DAC_CombineColor(index,index);
    }
}

Bitu read_p3c0(Bitu /*port*/,Bitu /*iolen*/) {
	// Wcharts, Win 3.11 & 95 SVGA
	Bitu retval = attr(index) & 0x1f;
	return retval;
}
 
void write_p3c0(Bitu /*port*/,Bitu val,Bitu /*iolen*/) {
	if (!vga.internal.attrindex) {
		attr(index)=val & 0x1F;
		vga.internal.attrindex=true;
		return;
	} else {
		vga.internal.attrindex=false;
		switch (attr(index)) {
			/* Palette */
		case 0x00:		case 0x01:		case 0x02:		case 0x03:
		case 0x04:		case 0x05:		case 0x06:		case 0x07:
		case 0x08:		case 0x09:		case 0x0a:		case 0x0b:
		case 0x0c:		case 0x0d:		case 0x0e:		case 0x0f:
            VGA_ATTR_SetPalette(attr(index),(Bit8u)val);
            break;
		default:
			LOG(LOG_VGAMISC,LOG_NORMAL)("VGA:ATTR:Write to unkown Index %2X",attr(index));
			break;
		}
	}
}

Bitu read_p3c1(Bitu /*port*/,Bitu /*iolen*/) {
//	vga.internal.attrindex=false;
	switch (attr(index)) {
			/* Palette */
	case 0x00:		case 0x01:		case 0x02:		case 0x03:
	case 0x04:		case 0x05:		case 0x06:		case 0x07:
	case 0x08:		case 0x09:		case 0x0a:		case 0x0b:
	case 0x0c:		case 0x0d:		case 0x0e:		case 0x0f:
		return attr(palette[attr(index)]);
	default:
		LOG(LOG_VGAMISC,LOG_NORMAL)("VGA:ATTR:Read from unkown Index %2X",attr(index));
	}
	return 0;
}


void VGA_SetupAttr(void) {
	if (IS_EGAVGA_ARCH) {
		IO_RegisterWriteHandler(0x3c0,write_p3c0,IO_MB);
		if (IS_VGA_ARCH) {
			IO_RegisterReadHandler(0x3c0,read_p3c0,IO_MB);
			IO_RegisterReadHandler(0x3c1,read_p3c1,IO_MB);
		}
	}
}

void VGA_UnsetupAttr(void) {
    IO_FreeWriteHandler(0x3c0,IO_MB);
    IO_FreeReadHandler(0x3c0,IO_MB);
    IO_FreeWriteHandler(0x3c1,IO_MB);
    IO_FreeReadHandler(0x3c1,IO_MB);
}

