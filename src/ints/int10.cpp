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
	// NTS: We do have to check the "current video mode" from the BIOS data area every call.
	//      Some OSes like Windows 95 rely on overwriting the "current video mode" byte in
	//      the BIOS data area to play tricks with the BIOS. If we don't call this, tricks
	//      like the Windows 95 boot logo or INT 10h virtualization in Windows 3.1/9x/ME
	//      within the DOS "box" will not work properly.
	INT10_SetCurMode();

	switch (reg_ah) {
	case 0x00:								/* Set VideoMode */
		INT10_SetVideoMode(reg_al);
		break;
	case 0x02:								/* Set Cursor Pos */
		INT10_SetCursorPos(reg_dh,reg_dl,reg_bh);
		break;
	case 0x09:								/* Write Character & Attribute at cursor CX times */
		if (real_readb(BIOSMEM_SEG,BIOSMEM_CURRENT_MODE)==0x11)
			INT10_WriteChar(reg_al,(reg_bl&0x80)|0x3f,reg_bh,reg_cx,true);
		else INT10_WriteChar(reg_al,reg_bl,reg_bh,reg_cx,true);
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

		/* switch to color mode and enable CPU access 480 lines */
		IO_Write(0x3c2,0xc3);
		/* More than 64k */
		IO_Write(0x3c4,0x04);
		IO_Write(0x3c5,0x02);
		if (IS_VGA_ARCH) {
			/* Initialize DAC */
			IO_Write(0x3c8,0);
			for (Bitu i=0;i<3*256;i++) IO_Write(0x3c9,0);
		}
	}
}

bool MEM_map_ROM_physmem(Bitu start,Bitu end);

extern Bitu VGA_BIOS_Size;
extern Bitu VGA_BIOS_SEG;
extern Bitu VGA_BIOS_SEG_END;
extern bool VIDEO_BIOS_disable;
extern Bitu BIOS_VIDEO_TABLE_LOCATION;
extern Bitu BIOS_VIDEO_TABLE_SIZE;

bool ROMBIOS_FreeMemory(Bitu phys);
Bitu RealToPhys(Bitu x);

void BIOS_UnsetupKeyboard(void);
bool MEM_unmap_physmem(Bitu start,Bitu end);
void CALLBACK_DeAllocate(Bitu in);

void INT10_OnResetComplete() {
    if (VGA_BIOS_Size > 0)
        MEM_unmap_physmem(0xC0000,0xC0000+VGA_BIOS_Size-1);

    /* free the table */
    BIOS_VIDEO_TABLE_SIZE = 0;
    if (BIOS_VIDEO_TABLE_LOCATION != (~0U) && BIOS_VIDEO_TABLE_LOCATION != 0) {
        LOG(LOG_MISC,LOG_DEBUG)("INT 10h freeing BIOS VIDEO TABLE LOCATION");
        ROMBIOS_FreeMemory(RealToPhys(BIOS_VIDEO_TABLE_LOCATION));
        BIOS_VIDEO_TABLE_LOCATION = ~0u;		// RealMake(0xf000,0xf0a4)
    }

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
        INT10_SetupRomMemory();
        INT10_Seg40Init();
        INT10_SetupBasicVideoParameterTable();

        LOG(LOG_MISC,LOG_DEBUG)("INT 10: VGA bios used %d / %d memory",(int)int10.rom.used,(int)VGA_BIOS_Size);
        if (int10.rom.used > VGA_BIOS_Size) /* <- this is fatal, it means the Setup() functions scrozzled over the adjacent ROM or RAM area */
            E_Exit("VGA BIOS size too small %u > %u",(unsigned int)int10.rom.used,(unsigned int)VGA_BIOS_Size);

        /* NTS: Uh, this does seem bass-ackwards... INT 10h making the VGA BIOS appear. Can we refactor this a bit? */
        if (VGA_BIOS_Size > 0) {
            LOG(LOG_MISC,LOG_DEBUG)("VGA BIOS occupies segment 0x%04x-0x%04x",(int)VGA_BIOS_SEG,(int)VGA_BIOS_SEG_END-1);
            if (!MEM_map_ROM_physmem(0xC0000,0xC0000+VGA_BIOS_Size-1))
                LOG(LOG_MISC,LOG_WARN)("INT 10 video: unable to map BIOS");
        }
        else {
            LOG(LOG_MISC,LOG_DEBUG)("Not mapping VGA BIOS");
        }

        INT10_SetVideoMode(0x3);
    }
}

void INT10_Init() {
}

