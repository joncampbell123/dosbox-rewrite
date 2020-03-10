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
#include "dosbox.h"
#include "inout.h"
#include "vga.h"
#include "debug.h"
#include "cpu.h"
#include "video.h"
#include "pic.h"

#define crtc(blah) vga.crtc.blah

void vga_write_p3d4(Bitu port,Bitu val,Bitu iolen) {
    (void)iolen;//UNUSED
    (void)port;//UNUSED
	crtc(index)=(Bit8u)val;
}

void vga_write_p3d5(Bitu port,Bitu val,Bitu /*iolen*/) {
    (void)port;//UNUSED
//	if((crtc(index)!=0xe)&&(crtc(index)!=0xf)) 
//		LOG_MSG("CRTC w #%2x val %2x",crtc(index),val);
	switch(crtc(index)) {
	case 0x0E:	/*Cursor Location High Register */
		crtc(cursor_location_high)=(Bit8u)val;
		vga.config.cursor_start&=0xff00ff;
		vga.config.cursor_start|=val << 8;
		/*	0-7  Upper 8 bits of the address of the cursor */
		break;
	case 0x0F:	/* Cursor Location Low Register */
//TODO update cursor on screen
		crtc(cursor_location_low)=(Bit8u)val;
		vga.config.cursor_start&=0xffff00;
		vga.config.cursor_start|=val;
		/*	0-7  Lower 8 bits of the address of the cursor */
		break;
	default:
        LOG(LOG_VGAMISC,LOG_NORMAL)("VGA:CRTC:Write to unknown index %X",crtc(index));
        break;
	}
}

