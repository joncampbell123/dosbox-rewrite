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
#include "control.h"
#include "mem.h"
#include "callback.h"
#include "regs.h"
#include "inout.h"
#include "int10.h"
#include "mouse.h"
#include "setup.h"

Int10Data int10;
static Bitu call_10 = 0;

extern bool enable_vga_8bit_dac;
extern bool vga_8bit_dac;

Bitu INT10_Handler(void) {
	switch (reg_ah) {
	case 0x02:								/* Set Cursor Pos */
		INT10_SetCursorPos(reg_dh,reg_dl,reg_bh);
		break;
	case 0x09:								/* Write Character & Attribute at cursor CX times */
		INT10_WriteChar(reg_al,reg_bl,reg_bh,reg_cx,true);
		break;
	case 0x0A:								/* Write Character at cursor CX times */
		INT10_WriteChar(reg_al,reg_bl,reg_bh,reg_cx,false);
		break;
	case 0x0E:								/* Teletype OutPut */
		INT10_TeletypeOutput(reg_al,reg_bl);
		break;
	default:
		LOG(LOG_INT10,LOG_ERROR)("Function %4X not supported",reg_ax);
//		reg_al=0x00;		//Successfull, breaks marriage
		break;
	}
	return CBRET_NONE;
}

static void INT10_Seg40Init(void) {
	// the default char height
	real_writeb(BIOSMEM_SEG,BIOSMEM_CHAR_HEIGHT,16);
	// Clear the screen 
	real_writeb(BIOSMEM_SEG,BIOSMEM_VIDEO_CTL,0x60);
	// Set the basic screen we have
	real_writeb(BIOSMEM_SEG,BIOSMEM_SWITCHES,0xF9);
	// Set the basic modeset options
	real_writeb(BIOSMEM_SEG,BIOSMEM_MODESET_CTL,0x51); // why is display switching enabled (bit 6) ?
	// Set the  default MSR
	real_writeb(BIOSMEM_SEG,BIOSMEM_CURRENT_MSR,0x09);
	// Set the pointer to video save pointer table
	real_writed(BIOSMEM_SEG, BIOSMEM_VS_POINTER, int10.rom.video_save_pointers);
}

static void INT10_InitVGA(void) {
	if (IS_EGAVGA_ARCH) {
		LOG(LOG_MISC,LOG_DEBUG)("INT 10: initializing EGA/VGA state");
	}
}

bool MEM_map_ROM_physmem(Bitu start,Bitu end);

extern bool VIDEO_BIOS_disable;
extern Bitu BIOS_VIDEO_TABLE_LOCATION;
extern Bitu BIOS_VIDEO_TABLE_SIZE;

bool ROMBIOS_FreeMemory(Bitu phys);

void BIOS_UnsetupKeyboard(void);
bool MEM_unmap_physmem(Bitu start,Bitu end);
void CALLBACK_DeAllocate(Bitu in);

void INT10_OnResetComplete() {
    if (call_10 != 0) {
        CALLBACK_DeAllocate(call_10);
        call_10 = 0;
    }

    BIOS_UnsetupKeyboard();
}

void INT10_Startup(Section *sec) {
    (void)sec;//UNUSED
	LOG(LOG_MISC,LOG_DEBUG)("INT 10h reinitializing");

    {
        INT10_InitVGA();
        /* Setup the INT 10 vector */
        call_10=CALLBACK_Allocate();	
        CALLBACK_Setup(call_10,&INT10_Handler,CB_IRET,"Int 10 video");
        RealSetVec(0x10,CALLBACK_RealPointer(call_10));
        //Init the 0x40 segment and init the datastructures in the the video rom area
        INT10_Seg40Init();
    }
}

void INT10_Init() {
}

