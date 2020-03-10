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

#define SEQ_REGS 0x05
#define GFX_REGS 0x09
#define ATT_REGS 0x15

extern bool int10_vesa_map_as_128kb;
extern bool allow_vesa_lowres_modes;
extern bool allow_vesa_4bpp_packed;
extern bool allow_explicit_vesa_24bpp;
extern bool vesa12_modes_32bpp;
extern bool allow_vesa_32bpp;
extern bool allow_vesa_24bpp;
extern bool allow_vesa_16bpp;
extern bool allow_vesa_15bpp;
extern bool allow_vesa_8bpp;
extern bool allow_vesa_4bpp;
extern bool allow_vesa_tty;

/* This list includes non-explicitly 24bpp modes (in the 0x100-0x11F range) that are available
 * when the VBE1.2 setting indicates they should be 24bpp.
 *
 * Explicitly 24bpp modes (numbered 0x120 or higher) are available regardless of the VBE1.2
 * setting but only if enabled in dosbox.conf.
 *
 * Disabling the explicit 24bpp modes is intended to reflect actual SVGA hardware that tends
 * to support either 24bpp or 32bpp, but not both. */

VideoModeBlock ModeList_VGA[]={
/* mode  ,type     ,sw  ,sh  ,tw ,th ,cw,ch ,pt,pstart  ,plength,htot,vtot,hde,vde special flags */
{ 0x003  ,M_TEXT   ,720 ,400 ,80 ,25 ,9 ,16 ,8 ,0xB8000 ,0x1000 ,100 ,449 ,80 ,400 ,0	},

{0xFFFF  ,M_ERROR  ,0   ,0   ,0  ,0  ,0 ,0  ,0 ,0x00000 ,0x0000 ,0   ,0   ,0  ,0   ,0 	},
};

VideoModeBlock * CurMode = NULL;

static bool SetCurMode(VideoModeBlock modeblock[],Bit16u mode) {
	Bitu i=0;
	while (modeblock[i].mode!=0xffff) {
		if (modeblock[i].mode!=mode)
			i++;
        /* ignore deleted modes */
        else if (modeblock[i].type == M_ERROR) {
            /* ignore */
            i++;
        }
	    /* ignore disabled modes */
        else if (modeblock[i].special & _USER_DISABLED) {
            /* ignore */
            i++;
        }
        /* ignore modes beyond the render scaler architecture's limits... unless the user created it. We did warn the user! */
        else if (!(modeblock[i].special & _USER_MODIFIED) &&
                (modeblock[i].swidth > SCALER_MAXWIDTH || modeblock[i].sheight > SCALER_MAXHEIGHT)) {
            /* ignore */
            i++;
        }
		else {
			if ((!int10.vesa_oldvbe) || (ModeList_VGA[i].mode<0x120)) {
				CurMode=&modeblock[i];
				return true;
			}
			return false;
		}
	}
	return false;
}

static void FinishSetMode(bool clearmem) {
	/* Clear video memory if needs be */
	if (clearmem) {
        for (Bit16u ct=0;ct<(80*25);ct++) real_writew(0xB800,ct*2,0x0720);
    }

	/* Setup the BIOS */
	real_writew(BIOSMEM_SEG,BIOSMEM_NB_COLS,(Bit16u)CurMode->twidth);
	real_writeb(BIOSMEM_SEG,BIOSMEM_NB_ROWS,(Bit8u)(CurMode->theight-1));

	INT10_SetCursorPos(0,0,0);
}

bool INT10_SetVideoMode(Bit16u mode) {
    if (!SetCurMode(ModeList_VGA,mode)){
        LOG(LOG_INT10,LOG_ERROR)("VGA:Trying to set illegal mode %X",mode);
        return false;
    }
    FinishSetMode(true);
    return true;
}

