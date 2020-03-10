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

bool INT10_SetCurMode(void) {
	bool mode_changed=false;
	Bit16u bios_mode=(Bit16u)real_readb(BIOSMEM_SEG,BIOSMEM_CURRENT_MODE);
	if (CurMode == NULL || CurMode->mode != bios_mode) {
		switch (machine) {
		case VGA_ARCH_CASE:
			switch (svgaCard) {
			case SVGA_S3Trio:
				if (bios_mode>=0x68 && CurMode->mode==(bios_mode+0x98)) break;
			default:
				mode_changed=SetCurMode(ModeList_VGA,bios_mode);
				break;
			}
			break;
		default:
			break;
		}
	}
	return mode_changed;
}

static void FinishSetMode(bool clearmem) {
	/* Clear video memory if needs be */
	if (clearmem) {
        switch (CurMode->type) {
		case M_TEXT: {
			Bit16u max = (Bit16u)(CurMode->ptotal*CurMode->plength)>>1;
			if (CurMode->mode == 7) {
				for (Bit16u ct=0;ct<max;ct++) real_writew(0xB000,ct*2,0x0720);
			}
			else {
				for (Bit16u ct=0;ct<max;ct++) real_writew(0xB800,ct*2,0x0720);
			}
			break;
		}
		default:
			break;
		}
	}
	/* Setup the BIOS */
	if (CurMode->mode<128) real_writeb(BIOSMEM_SEG,BIOSMEM_CURRENT_MODE,(Bit8u)CurMode->mode);
	else real_writeb(BIOSMEM_SEG,BIOSMEM_CURRENT_MODE,(Bit8u)(CurMode->mode-0x98));	//Looks like the s3 bios
	real_writew(BIOSMEM_SEG,BIOSMEM_NB_COLS,(Bit16u)CurMode->twidth);
	real_writew(BIOSMEM_SEG,BIOSMEM_PAGE_SIZE,(Bit16u)CurMode->plength);
	real_writew(BIOSMEM_SEG,BIOSMEM_CRTC_ADDRESS,((CurMode->mode==7 )|| (CurMode->mode==0x0f)) ? 0x3b4 : 0x3d4);
	real_writeb(BIOSMEM_SEG,BIOSMEM_NB_ROWS,(Bit8u)(CurMode->theight-1));
	real_writew(BIOSMEM_SEG,BIOSMEM_CHAR_HEIGHT,(Bit16u)CurMode->cheight);
	real_writeb(BIOSMEM_SEG,BIOSMEM_VIDEO_CTL,(0x60|(clearmem?0:0x80)));
	real_writeb(BIOSMEM_SEG,BIOSMEM_SWITCHES,0x09);

	// this is an index into the dcc table:
	if (IS_VGA_ARCH) real_writeb(BIOSMEM_SEG,BIOSMEM_DCC_INDEX,0x0b);

	// Set cursor shape
	if (CurMode->type==M_TEXT) {
		INT10_SetCursorShape(CURSOR_SCAN_LINE_NORMAL, CURSOR_SCAN_LINE_END);
	}
	// Set cursor pos for page 0..7
	for (Bit8u ct=0;ct<8;ct++) INT10_SetCursorPos(0,0,ct);
	// Set active page 0
	INT10_SetActivePage(0);
	/* Set some interrupt vectors */
	if (CurMode->mode<=3 || CurMode->mode==7) {
		RealSetVec(0x43,int10.rom.font_8_first);
	} else {
		switch (CurMode->cheight) {
		case 8:RealSetVec(0x43,int10.rom.font_8_first);break;
		case 14:RealSetVec(0x43,int10.rom.font_14);break;
		case 16:RealSetVec(0x43,int10.rom.font_16);break;
		}
	}
	/* FIXME */
	VGA_DAC_UpdateColorPalette();
}

bool unmask_irq0_on_int10_setmode = true;

bool INT10_SetVideoMode(Bit16u mode) {
	//LOG_MSG("set mode %x",mode);
	bool clearmem=true;Bitu i;
	if (mode>=0x100) {
		if ((mode & 0x4000) && int10.vesa_nolfb) return false;
		if (mode & 0x8000) clearmem=false;
		mode&=0xfff;
	}
	if ((mode<0x100) && (mode & 0x80)) {
		clearmem=false;
		mode-=0x80;
	}

    if (unmask_irq0_on_int10_setmode) {
        /* setting the video mode unmasks certain IRQs as a matter of course */
        PIC_SetIRQMask(0,false); /* Enable system timer */
    }

	int10.vesa_setmode=0xffff;
	LOG(LOG_INT10,LOG_NORMAL)("Set Video Mode %X",mode);

	/* First read mode setup settings from bios area */
//	Bit8u video_ctl=real_readb(BIOSMEM_SEG,BIOSMEM_VIDEO_CTL);
//	Bit8u vga_switches=real_readb(BIOSMEM_SEG,BIOSMEM_SWITCHES);
	Bit8u modeset_ctl=real_readb(BIOSMEM_SEG,BIOSMEM_MODESET_CTL);

    if (!SetCurMode(ModeList_VGA,mode)){
        LOG(LOG_INT10,LOG_ERROR)("VGA:Trying to set illegal mode %X",mode);
        return false;
    }

    // INT 10h modeset will always clear 8-bit DAC mode (by VESA BIOS standards)
    VGA_DAC_UpdateColorPalette();

	/* Setup the VGA to the correct mode */
	// turn off video
	IO_Write(0x3c4,0); IO_Write(0x3c5,1); // reset
	IO_Write(0x3c4,1); IO_Write(0x3c5,0x20); // screen off

	Bit16u crtc_base;
	bool mono_mode=(mode == 7) || (mode==0xf);  
	if (mono_mode) crtc_base=0x3b4;
	else crtc_base=0x3d4;

	/* Setup MISC Output Register */
	Bit8u misc_output=0x2 | (mono_mode ? 0x0 : 0x1);

	{
		// 28MHz clock for 9-pixel wide chars
		if ((CurMode->type==M_TEXT) && (CurMode->cwidth==9)) misc_output|=0x4;
	}

	switch (CurMode->vdispend) {
	case 400: 
		misc_output|=0x60;
		break;
	case 480:
		misc_output|=0xe0;
		break;
	case 350:
		misc_output|=0xa0;
		break;
	case 200:
	default:
		misc_output|=0x20;
	}
	IO_Write(0x3c2,misc_output);		//Setup for 3b4 or 3d4
	
	if (IS_VGA_ARCH && (svgaCard == SVGA_S3Trio)) {
	// unlock the S3 registers
		IO_Write(crtc_base,0x38);IO_Write(crtc_base+1u,0x48);	//Register lock 1
		IO_Write(crtc_base,0x39);IO_Write(crtc_base+1u,0xa5);	//Register lock 2
		IO_Write(0x3c4,0x8);IO_Write(0x3c5,0x06);
		// Disable MMIO here so we can read / write memory
		IO_Write(crtc_base,0x53);IO_Write(crtc_base+1u,0x0);
	}

	/* NTS: S3 INT 10 modesetting code below sets this bit anyway when writing CRTC register 0x31.
	 *      It needs to be done as I/O port write so that Windows 95 can virtualize it properly when
	 *      we're called to set INT10 mode 3 (from within virtual 8086 mode) when opening a DOS box.
	 *
	 *      If we just set it directly, then the generic S3 driver in Windows 95 cannot trap the I/O
	 *      and prevent our own INT 10h handler from setting the VGA memory mapping into "compatible
	 *      chain 4" mode, and then any non accelerated drawing from the Windows driver becomes a
	 *      garbled mess spread out across the screen (due to the weird way that VGA planar memory
	 *      is "chained" on SVGA chipsets).
	 *
	 *      The S3 linear framebuffer isn't affected by VGA chained mode, which is why only the
	 *      generic S3 driver was affected by this bug, since the generic S3 driver is the one that
	 *      uses only VGA access (0xA0000-0xAFFFF) and SVGA bank switching while the more specific
	 *      "S3 Trio32 PCI" driver uses the linear framebuffer.
	 *
	 *      But to avoid breaking other SVGA emulation in DOSBox-X, we still set this manually for
	 *      other VGA/SVGA emulation cases, just not S3 Trio emulation. */
	if (svgaCard != SVGA_S3Trio)
		vga.config.compatible_chain4 = true; // this may be changed by SVGA chipset emulation

	/* Program CRTC */
	/* First disable write protection */
	IO_Write(crtc_base,0x11);
	IO_Write(crtc_base+1u,IO_Read(crtc_base+1u)&0x7f);
	/* Clear all the regs */
	for (Bit8u ct=0x0;ct<=0x18;ct++) {
		IO_Write(crtc_base,ct);IO_Write(crtc_base+1u,0);
	}
	Bit8u overflow=0;Bit8u max_scanline=0;
	Bit8u ver_overflow=0;Bit8u hor_overflow=0;
	/* Horizontal Total */
	IO_Write(crtc_base,0x00);IO_Write(crtc_base+1u,(Bit8u)(CurMode->htotal-5));
	hor_overflow|=((CurMode->htotal-5) & 0x100) >> 8;
	/* Horizontal Display End */
	IO_Write(crtc_base,0x01);IO_Write(crtc_base+1u,(Bit8u)(CurMode->hdispend-1));
	hor_overflow|=((CurMode->hdispend-1) & 0x100) >> 7;
	/* Start horizontal Blanking */
	IO_Write(crtc_base,0x02);IO_Write(crtc_base+1u,(Bit8u)CurMode->hdispend);
	hor_overflow|=((CurMode->hdispend) & 0x100) >> 6;
	/* End horizontal Blanking */
	Bitu blank_end=(CurMode->htotal-2) & 0x7f;
	IO_Write(crtc_base,0x03);IO_Write(crtc_base+1u,0x80|(blank_end & 0x1f));

	/* Start Horizontal Retrace */
	Bitu ret_start;
	if ((CurMode->special & _EGA_HALF_CLOCK)) ret_start = (CurMode->hdispend+3);
	else if (CurMode->type==M_TEXT) ret_start = (CurMode->hdispend+5);
	else ret_start = (CurMode->hdispend+4);
	IO_Write(crtc_base,0x04);IO_Write(crtc_base+1u,(Bit8u)ret_start);
	hor_overflow|=(ret_start & 0x100) >> 4;

	/* End Horizontal Retrace */
	Bitu ret_end;
	if (CurMode->special & _EGA_HALF_CLOCK) {
		if (CurMode->special & _DOUBLESCAN) ret_end = (CurMode->htotal-18) & 0x1f;
		else ret_end = ((CurMode->htotal-18) & 0x1f) | 0x20; // mode 0&1 have 1 char sync delay
	} else if (CurMode->type==M_TEXT) ret_end = (CurMode->htotal-3) & 0x1f;
	else ret_end = (CurMode->htotal-4) & 0x1f;
	
	IO_Write(crtc_base,0x05);IO_Write(crtc_base+1u,(Bit8u)(ret_end | (blank_end & 0x20) << 2));

	/* Vertical Total */
	IO_Write(crtc_base,0x06);IO_Write(crtc_base+1u,(Bit8u)(CurMode->vtotal-2));
	overflow|=((CurMode->vtotal-2) & 0x100) >> 8;
	overflow|=((CurMode->vtotal-2) & 0x200) >> 4;
	ver_overflow|=((CurMode->vtotal-2) & 0x400) >> 10;

	Bitu vretrace;
	if (IS_VGA_ARCH) {
		switch (CurMode->vdispend) {
		case 400: vretrace=CurMode->vdispend+12;
				break;
		case 480: vretrace=CurMode->vdispend+10;
				break;
		case 350: vretrace=CurMode->vdispend+37;
				break;
		default: vretrace=CurMode->vdispend+12;
		}
	} else {
		switch (CurMode->vdispend) {
		case 350: vretrace=CurMode->vdispend;
				break;
		default: vretrace=CurMode->vdispend+24;
		}
	}

	/* Vertical Retrace Start */
	IO_Write(crtc_base,0x10);IO_Write(crtc_base+1u,(Bit8u)vretrace);
	overflow|=(vretrace & 0x100) >> 6;
	overflow|=(vretrace & 0x200) >> 2;
	ver_overflow|=(vretrace & 0x400) >> 6;

	/* Vertical Retrace End */
	IO_Write(crtc_base,0x11);IO_Write(crtc_base+1u,(vretrace+2) & 0xF);

	/* Vertical Display End */
	IO_Write(crtc_base,0x12);IO_Write(crtc_base+1u,(Bit8u)(CurMode->vdispend-1));
	overflow|=((CurMode->vdispend-1) & 0x100) >> 7;
	overflow|=((CurMode->vdispend-1) & 0x200) >> 3;
	ver_overflow|=((CurMode->vdispend-1) & 0x400) >> 9;
	
	Bitu vblank_trim;
	if (IS_VGA_ARCH) {
		switch (CurMode->vdispend) {
		case 400: vblank_trim=6;
				break;
		case 480: vblank_trim=7;
				break;
		case 350: vblank_trim=5;
				break;
		default: vblank_trim=8;
		}
	} else {
		switch (CurMode->vdispend) {
		case 350: vblank_trim=0;
				break;
		default: vblank_trim=23;
		}
	}

	/* Vertical Blank Start */
	IO_Write(crtc_base,0x15);IO_Write(crtc_base+1u,(Bit8u)(CurMode->vdispend+vblank_trim));
	overflow|=((CurMode->vdispend+vblank_trim) & 0x100) >> 5;
	max_scanline|=((CurMode->vdispend+vblank_trim) & 0x200) >> 4;
	ver_overflow|=((CurMode->vdispend+vblank_trim) & 0x400) >> 8;

	/* Vertical Blank End */
	IO_Write(crtc_base,0x16);IO_Write(crtc_base+1u,(Bit8u)(CurMode->vtotal-vblank_trim-2));

	/* Line Compare */
	Bitu line_compare=(CurMode->vtotal < 1024) ? 1023 : 2047;
	IO_Write(crtc_base,0x18);IO_Write(crtc_base+1u,line_compare&0xff);
	overflow|=(line_compare & 0x100) >> 4;
	max_scanline|=(line_compare & 0x200) >> 3;
	ver_overflow|=(line_compare & 0x400) >> 4;
	Bit8u underline=0;
	/* Maximum scanline / Underline Location */
	if (CurMode->special & _DOUBLESCAN) max_scanline|=0x80;
	if (CurMode->special & _REPEAT1) max_scanline|=0x01;

	switch (CurMode->type) {
	case M_TEXT:
		if(IS_VGA_ARCH) {
			switch(modeset_ctl & 0x90) {
			case 0x0: // 350-lines mode: 8x14 font
				max_scanline |= (14-1);
				break;
			default: // reserved
			case 0x10: // 400 lines 8x16 font
		max_scanline|=CurMode->cheight-1;
				break;
			case 0x80: // 200 lines: 8x8 font and doublescan
				max_scanline |= (8-1);
				max_scanline |= 0x80;
				break;
			}
		} else max_scanline |= CurMode->cheight-1;
		underline=(Bit8u)(mono_mode ? CurMode->cheight-1 : 0x1f); // mode 7 uses underline position
		break;
	default:
	    /* do NOT apply this to VESA BIOS modes */
		if (CurMode->mode < 0x100 && CurMode->vdispend==350) underline=0x0f;
		break;
	}

	IO_Write(crtc_base,0x09);IO_Write(crtc_base+1u,max_scanline);
	IO_Write(crtc_base,0x14);IO_Write(crtc_base+1u,underline);

	/* OverFlow */
	IO_Write(crtc_base,0x07);IO_Write(crtc_base+1u,overflow);

	if (svgaCard == SVGA_S3Trio) {
		/* Extended Horizontal Overflow */
		IO_Write(crtc_base,0x5d);IO_Write(crtc_base+1u,hor_overflow);
		/* Extended Vertical Overflow */
		IO_Write(crtc_base,0x5e);IO_Write(crtc_base+1u,ver_overflow);
	}

	/* Offset Register */
	Bitu offset;
    offset = CurMode->hdispend/2;
	IO_Write(crtc_base,0x13);
	IO_Write(crtc_base + 1u,offset & 0xff);

	if (svgaCard == SVGA_S3Trio) {
		/* Extended System Control 2 Register  */
		/* This register actually has more bits but only use the extended offset ones */
		IO_Write(crtc_base,0x51);
		IO_Write(crtc_base + 1u,(Bit8u)((offset & 0x300) >> 4));
		/* Clear remaining bits of the display start */
		IO_Write(crtc_base,0x69);
		IO_Write(crtc_base + 1u,0);
		/* Extended Vertical Overflow */
		IO_Write(crtc_base,0x5e);IO_Write(crtc_base+1u,ver_overflow);
	}

	/* Renable write protection */
	IO_Write(crtc_base,0x11);
	IO_Write(crtc_base+1u,IO_Read(crtc_base+1u)|0x80);

	/* Write Misc Output */
	IO_Write(0x3c2,misc_output);
	/* Program Graphics controller */
	Bit8u gfx_data[GFX_REGS];
	memset(gfx_data,0,GFX_REGS);
	gfx_data[0x7]=0xf;				/* Color don't care */
	gfx_data[0x8]=0xff;				/* BitMask */
	switch (CurMode->type) {
	case M_TEXT:
		gfx_data[0x5]|=0x10;		//Odd-Even Mode
		gfx_data[0x6]|=mono_mode ? 0x0a : 0x0e;		//Either b800 or b000, chain odd/even enable
		break;
	default:
		break;
	}
	for (Bit8u ct=0;ct<GFX_REGS;ct++) {
		IO_Write(0x3ce,ct);
		IO_Write(0x3cf,gfx_data[ct]);
	}
	Bit8u att_data[ATT_REGS];
	memset(att_data,0,ATT_REGS);
	att_data[0x12]=0xf;				//Always have all color planes enabled
	/* Program Attribute Controller */
	switch (CurMode->type) {
	case M_TEXT:
		if (CurMode->cwidth==9) {
			att_data[0x13]=0x08;	//Pel panning on 8, although we don't have 9 dot text mode
			att_data[0x10]=0x0C;	//Color Text with blinking, 9 Bit characters
		} else {
			att_data[0x13]=0x00;
			att_data[0x10]=0x08;	//Color Text with blinking, 8 Bit characters
		}
		real_writeb(BIOSMEM_SEG,BIOSMEM_CURRENT_PAL,0x30);
		if (CurMode->mode==7) {
			att_data[0]=0x00;
			att_data[8]=0x10;
			for (i=1; i<8; i++) {
				att_data[i]=0x08;
				att_data[i+8]=0x18;
			}
		} else {
			for (Bit8u ct=0;ct<8;ct++) {
				att_data[ct]=ct;
				att_data[ct+8]=ct+0x38;
			}
			att_data[0x06]=0x14;		//Odd Color 6 yellow/brown.
		}
		break;
	default:
		break;
	}
	IO_Read(mono_mode ? 0x3ba : 0x3da);
	if ((modeset_ctl & 8)==0) {
		for (Bit8u ct=0;ct<ATT_REGS;ct++) {
			IO_Write(0x3c0,ct);
			IO_Write(0x3c0,att_data[ct]);
		}
		IO_Write(0x3c0,0x20); IO_Write(0x3c0,0x00); //Disable palette access
		IO_Write(0x3c6,0xff); //Reset Pelmask
		/* Setup the DAC */
		IO_Write(0x3c8,0);
        /* make sure the DAC index is reset on modeset */
		IO_Write(0x3c7,0); /* according to src/hardware/vga_dac.cpp this sets read_index=0 and write_index=1 */
		IO_Write(0x3c8,0); /* so set write_index=0 */
	} else {
		for (Bit8u ct=0x10;ct<ATT_REGS;ct++) {
			if (ct==0x11) continue;	// skip overscan register
			IO_Write(0x3c0,ct);
			IO_Write(0x3c0,att_data[ct]);
		}
	}
	/* Write palette register data to dynamic save area if pointer is non-zero */
	RealPt vsavept=real_readd(BIOSMEM_SEG,BIOSMEM_VS_POINTER);
	RealPt dsapt=real_readd(RealSeg(vsavept),RealOff(vsavept)+4);
	if (dsapt) {
		for (Bit8u ct=0;ct<0x10;ct++) {
			real_writeb(RealSeg(dsapt),RealOff(dsapt)+ct,att_data[ct]);
		}
		real_writeb(RealSeg(dsapt),RealOff(dsapt)+0x10,0); // overscan
	}
	/* Setup some special stuff for different modes */
	Bit8u feature=real_readb(BIOSMEM_SEG,BIOSMEM_INITIAL_MODE);
	switch (CurMode->type) {
	case M_TEXT:
		feature=(feature&~0x30)|0x20;
		switch (CurMode->mode) {
		case 0:real_writeb(BIOSMEM_SEG,BIOSMEM_CURRENT_MSR,0x2c);break;
		case 1:real_writeb(BIOSMEM_SEG,BIOSMEM_CURRENT_MSR,0x28);break;
		case 2:real_writeb(BIOSMEM_SEG,BIOSMEM_CURRENT_MSR,0x2d);break;
		case 3:
		case 7:real_writeb(BIOSMEM_SEG,BIOSMEM_CURRENT_MSR,0x29);break;
		}
		break;
	default:
		break;
	}
	// disabled, has to be set in bios.cpp exclusively
//	real_writeb(BIOSMEM_SEG,BIOSMEM_INITIAL_MODE,feature);

	if (svgaCard == SVGA_S3Trio) {
		/* Setup the CPU Window */
		IO_Write(crtc_base,0x6a);
		IO_Write(crtc_base+1u,0);
		
		/* Setup some remaining S3 registers */
		IO_Write(crtc_base,0x41); // BIOS scratchpad
		IO_Write(crtc_base+1u,0x88);
		IO_Write(crtc_base,0x52); // extended BIOS scratchpad
		IO_Write(crtc_base+1u,0x80);

		IO_Write(0x3c4,0x15);
		IO_Write(0x3c5,0x03);

		IO_Write(crtc_base,0x45);
		IO_Write(crtc_base+1u,0x00);

		Bit8u reg_31, reg_3a;
        reg_3a=5;

        unsigned char s3_mode = 0x00;

        reg_31 = 5;

        /* SVGA text modes need the 256k+ access bit */
        if (CurMode->mode >= 0x100 && !int10.vesa_nolfb) {
            reg_31 |= 8; /* enable 256k+ access */
        }

		IO_Write(crtc_base,0x3a);IO_Write(crtc_base+1u,reg_3a);
		IO_Write(crtc_base,0x31);IO_Write(crtc_base+1u,reg_31);	//Enable banked memory and 256k+ access

		IO_Write(crtc_base,0x58);
		if (vga.mem.memsize >= (4*1024*1024))
			IO_Write(crtc_base+1u,0x3 | s3_mode);		// 4+ MB window
		else if (vga.mem.memsize >= (2*1024*1024))
			IO_Write(crtc_base+1u,0x2 | s3_mode);		// 2 MB window
		else
			IO_Write(crtc_base+1u,0x1 | s3_mode);		// 1 MB window

		IO_Write(crtc_base,0x38);IO_Write(crtc_base+1u,0x48);	//Register lock 1
		IO_Write(crtc_base,0x39);IO_Write(crtc_base+1u,0xa5);	//Register lock 2
	}

	FinishSetMode(clearmem);

	/* Set vga attrib register into defined state */
	IO_Read(mono_mode ? 0x3ba : 0x3da);
	IO_Write(0x3c0,0x20);
	IO_Read(mono_mode ? 0x3ba : 0x3da);

	return true;
}

Bitu VideoModeMemSize(Bitu mode) {
	if (!IS_VGA_ARCH)
		return 0;

	VideoModeBlock* modelist = NULL;

    modelist = ModeList_VGA;

	VideoModeBlock* vmodeBlock = NULL;
	Bitu i=0;
	while (modelist[i].mode!=0xffff) {
		if (modelist[i].mode==mode) {
			/* Hack for VBE 1.2 modes and 24/32bpp ambiguity */
			{
				vmodeBlock = &modelist[i];
				break;
			}
		}
		i++;
	}

	if (!vmodeBlock)
	        return ~0ul;

    return vmodeBlock->twidth*vmodeBlock->theight*2;
}

