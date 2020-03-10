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


void VGA_MapMMIO(void);
void VGA_UnmapMMIO(void);
void page_flip_debug_notify();

void vga_write_p3d5(Bitu port,Bitu val,Bitu iolen);
Bitu DEBUG_EnableDebugger(void);

extern bool vga_ignore_hdispend_change_if_smaller;

void vga_write_p3d4(Bitu port,Bitu val,Bitu iolen) {
    (void)iolen;//UNUSED
    (void)port;//UNUSED
	crtc(index)=(Bit8u)val;
}

Bitu vga_read_p3d4(Bitu port,Bitu iolen) {
    (void)port;//UNUSED
    (void)iolen;//UNUSED

    /* NOTES: Paradise/Westdern Digital SVGA decodes only bits [5:0] inclusive and repeat every 0x40 */

	return crtc(index);
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


Bitu vga_read_p3d5x(Bitu port,Bitu iolen);
Bitu vga_read_p3d5(Bitu port,Bitu iolen) {
	Bitu retval = vga_read_p3d5x(port,iolen);
//	LOG_MSG("CRTC r #%2x val %2x",crtc(index),retval);
	return retval;
}

Bitu vga_read_p3d5x(Bitu port,Bitu iolen) {
    (void)iolen;//UNUSED
    (void)port;//UNUSED
	switch(crtc(index)) {
	case 0x0E:	/*Cursor Location High Register */
		return crtc(cursor_location_high);
	case 0x0F:	/* Cursor Location Low Register */
		return crtc(cursor_location_low);
	default:
        LOG(LOG_VGAMISC,LOG_NORMAL)("VGA:CRTC:Read from unknown index %X",crtc(index));
        return 0x0;
    }
}

