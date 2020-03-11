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

/* NTS: Hardware notes
 *
 * S3 Virge DX (PCI):
 *
 *   VGA 256-color chained mode appears to work differently than
 *   expected. Groups of 4 pixels are spread across the VGA planes
 *   as expected, but actual memory storage of those 32-bit quantities
 *   are 4 "bytes" apart (probably the inspiration for DOSBox's
 *   chain emulation using addr = ((addr & ~3) << 2) + (addr & 3) when
 *   emulating chained mode).
 *
 *   The attribute controller has a bug where if you attempt to read
 *   the palette indexes 0x00-0x0F with PAS=1 (see FreeVGA for more
 *   info) the returned value will be correct except for bit 5 which
 *   will be 1 i.e. if palette index 0x2 is read in this way and
 *   contains 0x02 you will get 0x22 instead. The trick is to write
 *   the index with PAS=0 and read the data, then issue the index with
 *   PAS=1. Related: the S3 acts as if there are different flip-flops
 *   associated with reading vs writing i.e. writing to 0x3C0, then
 *   reading port 0x3C1, then writing port 0x3C0, will treat the second
 *   write to 0x3C0 as data still, not as an index. Both flip flops are
 *   reset by reading 3xAh though.
 *
 *   VGA BIOS does not support INT 10h TTY functions in VESA BIOS modes.
 *
 *   Raw planar dumps of the VGA memory in alphanumeric modes suggest
 *   that, not only do the cards direct even writes to 0/2 and odd to 1/3,
 *   it does so without shifting down the address. So in a raw planar
 *   dump, you will see on plane 0 something like "C : \ > " where the
 *   spaces are hex 0x00, and in plane 1, something like 0x07 0x00 0x07 0x00 ...
 *   the card however (in even/odd mode) does mask out bit A0 which
 *   explains why the Plane 1 capture is 0x07 0x00 ... not 0x00 0x07.
 *
 * ATI Rage 128 (AGP):
 *
 *   VGA 256-color chained mode appears to work in the same weird way
 *   that S3 Virge DX does (see above).
 *
 *   VGA BIOS supports TTY INT 10h functions in 16 & 256-color modes
 *   only. There are apparently INT 10h functions for 15/16bpp modes
 *   as well, but they don't appear to render anything except shades
 *   of green.
 *
 *   The VESA BIOS interface seems to have INT 10h aliases for many
 *   VBE 1.2 modes i.e. mode 0x110 is also mode 0x42.
 *
 *   The Attribute Controller palette indexes work as expected in all
 *   VGA modes, however in SVGA VESA BIOS modes, the Attribute Controller
 *   palette has no effect on output EXCEPT in 16-color (4bpp) VESA BIOS
 *   modes.
 *
 *   Raw planar layout of alphanumeric text modes apply the same rules
 *   as mentioned above in the S3 Virge DX description.
 *
 * Compaq Elite LTE 4/50CX laptop:
 *
 *   No SVGA modes. All modes work as expected.
 *
 *   VGA 256-color chained mode acts the same weird way as described
 *   above, seems to act like addr = ((addr & ~3) << 2) + (addr & 3)
 *
 *   There seems to be undocumented INT 10h modes:
 *
 *        0x22:  640x480x?? INT 10h text is all green and garbled
 *        0x28:  320x200x?? INT 10h text is all green and garbled
 *        0x32:  640x480x?? INT 10h text is all yellow and garbled
 *        0x5E:  640x400x256-color with bank switching
 *        0x5F:  640x480x256-color with bank switching
 *        0x62:  640x480x?? INT 10h text is all dark gray
 *        0x68:  320x200x?? INT 10h text is all dark gray
 *        0x72:  640x480x?? INT 10h text is all dark gray
 *        0x78:  320x200x?? INT 10h text is all dark gray
 *
 *   And yet, the BIOS does not implement VESA BIOS extensions. Hm..
 *
 * Sharp PC-9030 with Cirrus SVGA (1996):
 *
 *   VGA 256-color chained mode acts the same weird way, as if:
 *   addr = ((addr & ~3) << 2) + (addr & 3)
 * 
 *   All VESA BIOS modes support INT 10h TTY output.
 *
 * Tseng ET4000AX:
 *
 *   The ET4000 cards appear to be the ONLY SVGA chipset out there
 *   that does NOT do the odd VGA 256-color chained mode that
 *   other cards do.
 *
 *   Chained 256-color on ET4000:
 *       addr = addr                             (addr >> 2) byte in planar space, plane select by (addr & 3)
 *
 *   Other VGA cards:
 *       addr = ((addr & ~3) << 2) + (addr & 3)  (addr & ~3) byte in planar space, plane select by (addr & 3)
 *
 *   I suspect that this difference may be the reason several 1992-1993-ish DOS demos have problems clearing
 *   VRAM. It's possible they noticed that zeroing RAM was faster in planar mode, and coded their routines
 *   around ET4000 cards, not knowing that Trident, Cirrus, and every VGA clone thereafter implemented the
 *   chained 256-color modes differently.
 */

#include "dosbox.h"
#include "setup.h"
#include "video.h"
#include "pic.h"
#include "vga.h"
#include "inout.h"
#include "programs.h"
#include "support.h"
#include "setup.h"
#include "timer.h"
#include "mem.h"
#include "cpu.h"
#include "util_units.h"
#include "control.h"
#include "mixer.h"
#include "menu.h"
#include "render.h"
#include "paging.h"
#include "config.h"
#include "../gui/render_scalers.h"

#include <string.h>
#include <stdlib.h>
#include <string>
#include <stdio.h>
#include <math.h>

#if defined(_MSC_VER)
# pragma warning(disable:4244) /* const fmath::local::uint64_t to double possible loss of data */
#endif

#define crtc(blah) vga.crtc.blah

#define VGA_PAGE_B8		(0xB8000/4096)

using namespace std;

bool VGA_IsCaptureEnabled(void);
void VGA_UpdateCapturePending(void);
bool VGA_CaptureHasNextFrame(void);
void VGA_CaptureStartNextFrame(void);
void VGA_CaptureMarkError(void);
bool VGA_CaptureValidateCurrentFrame(void);

SDL_Rect                            vga_capture_rect = {0,0,0,0};
SDL_Rect                            vga_capture_current_rect = {0,0,0,0};
uint32_t                            vga_capture_current_address = 0;
uint32_t                            vga_capture_write_address = 0; // literally the address written
uint32_t                            vga_capture_address = 0;
uint32_t                            vga_capture_stride = 0;
uint32_t                            vga_capture_state = 0;

SDL_Rect &VGA_CaptureRectCurrent(void) {
    return vga_capture_current_rect;
}

SDL_Rect &VGA_CaptureRectFromGuest(void) {
    return vga_capture_rect;
}

VGA_Type vga;

Bit32u TXT_Font_Table[16];
Bit32u TXT_FG_Table[16];
Bit32u TXT_BG_Table[16];
Bit32u ExpandTable[256];
Bit32u Expand16Table[4][16];
Bit32u FillTable[16];
Bit32u ColorTable[16];
double vga_force_refresh_rate = -1;

void VGA_SetModeNow(VGAModes mode) {
    if (vga.mode == mode) return;
    vga.mode=mode;
    VGA_SetupHandlers();
    VGA_StartResize(0);
}


void VGA_SetMode(VGAModes mode) {
    if (vga.mode == mode) return;
    vga.mode=mode;
    VGA_SetupHandlers();
    VGA_StartResize();
}

void VGA_DetermineMode(void) {
    VGA_SetMode(M_TEXT);
}

void VGA_StartResize(Bitu delay /*=50*/) {
    if (!vga.draw.resizing) {
        delay = 1;
        vga.draw.resizing=true;
        /* Start a resize after delay (default 50 ms) */
        if (delay==0) VGA_SetupDrawing(0);
        else PIC_AddEvent(VGA_SetupDrawing,(float)delay);
    }
}

Bit32u MEM_get_address_bits();
void vga_write_p3d4(Bitu port,Bitu val,Bitu iolen);
void vga_write_p3d5(Bitu port,Bitu val,Bitu iolen);

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

static void VGA_DAC_SendColor( Bitu index ) {
    const Bit8u red = vga.dac.rgb[index].red;
    const Bit8u green = vga.dac.rgb[index].green;
    const Bit8u blue = vga.dac.rgb[index].blue;

    /* FIXME: Can someone behind the GCC project explain how (unsigned int) OR (unsigned int) somehow becomes (signed int)?? */

    if (GFX_bpp >= 24) /* FIXME: Assumes 8:8:8. What happens when desktops start using the 10:10:10 format? */
        vga.dac.xlat32[index] =
            (uint32_t)((blue&0xffu) << (GFX_Bshift)) |
            (uint32_t)((green&0xffu) << (GFX_Gshift)) |
            (uint32_t)((red&0xffu) << (GFX_Rshift)) |
            (uint32_t)GFX_Amask;
    else {
        /* FIXME: Assumes 5:6:5. I need to test against 5:5:5 format sometime. Perhaps I could dig out some older VGA cards and XFree86 drivers that support that format? */
        vga.dac.xlat16[index] =
            (uint16_t)(((blue&0xffu)>>3u)<<GFX_Bshift) |
            (uint16_t)(((green&0xffu)>>2u)<<GFX_Gshift) |
            (uint16_t)(((red&0xffu)>>3u)<<GFX_Rshift) |
            (uint16_t)GFX_Amask;
    }

    RENDER_SetPal( (Bit8u)index, red, green, blue );
}

void VGA_DAC_UpdateColorPalette() {
    for ( Bitu i = 0;i<256;i++) 
        VGA_DAC_SendColor( i );
}

void VGASetPalette(Bit8u index,Bit8u r,Bit8u g,Bit8u b) {
    vga.dac.rgb[index].red=r;
    vga.dac.rgb[index].green=g;
    vga.dac.rgb[index].blue=b;
}

void VGA_Reset(Section*) {
    Section_prop * section=static_cast<Section_prop *>(control->GetSection("dosbox"));
    string str;

    LOG(LOG_MISC,LOG_DEBUG)("VGA_Reset() reinitializing VGA emulation");

    vga_force_refresh_rate = -1;
    str=section->Get_string("forcerate");
    if (str == "ntsc")
        vga_force_refresh_rate = 60000.0 / 1001;
    else if (str == "pal")
        vga_force_refresh_rate = 50;
    else if (str.find_first_of('/') != string::npos) {
        char *p = (char*)str.c_str();
        int num = 1,den = 1;
        num = strtol(p,&p,0);
        if (*p == '/') p++;
        den = strtol(p,&p,0);
        if (num < 1) num = 1;
        if (den < 1) den = 1;
        vga_force_refresh_rate = (double)num / den;
    }
    else {
        vga_force_refresh_rate = atof(str.c_str());
    }

    if (vga_force_refresh_rate > 0)
        LOG(LOG_VGA,LOG_NORMAL)("VGA forced refresh rate active = %.3f",vga_force_refresh_rate);

    vga.draw.resizing=false;
    vga.mode=M_TEXT;

    vga.mem.memsize = _KB_bytes(16);
    vga.mem.memmask = vga.mem.memsize - 1u;

    LOG(LOG_VGA,LOG_NORMAL)("Video RAM: %uKB",vga.mem.memsize>>10);

    VGA_SetupMemory();      // memory is allocated here

    IO_RegisterWriteHandler(0x3D4,vga_write_p3d4,IO_MB);
    IO_RegisterWriteHandler(0x3D5,vga_write_p3d5,IO_MB);

    VGA_StartResize();
}

void VGASetPalette(Bit8u index,Bit8u r,Bit8u g,Bit8u b);

static Bit8u text_palette[16][3]=
{
  {0x00,0x00,0x00},{0x00,0x00,0x2a},{0x00,0x2a,0x00},{0x00,0x2a,0x2a},{0x2a,0x00,0x00},{0x2a,0x00,0x2a},{0x2a,0x15,0x00},{0x2a,0x2a,0x2a},
  {0x15,0x15,0x15},{0x15,0x15,0x3f},{0x15,0x3f,0x15},{0x15,0x3f,0x3f},{0x3f,0x15,0x15},{0x3f,0x15,0x3f},{0x3f,0x3f,0x15},{0x3f,0x3f,0x3f}
};

void VGA_Init() {
    Bitu i,j;

	vga.config.chained = false;

    vga.draw.cursor.sline = 13;
    vga.draw.cursor.eline = 14;
    vga.draw.cursor.enabled = true;

    vga.draw.address_add = 80u*2u;

    vga.draw.render_step = 0;
    vga.draw.render_max = 1;

    LOG(LOG_MISC,LOG_DEBUG)("Initializing VGA");
    LOG(LOG_MISC,LOG_DEBUG)("Render scaler maximum resolution is %u x %u",SCALER_MAXWIDTH,SCALER_MAXHEIGHT);

    for (i=0;i<256;i++) {
        ExpandTable[i]=(Bitu)(i + (i << 8u) + (i << 16u) + (i << 24u));
    }
    for (i=0;i<16;i++) {
        TXT_FG_Table[i]=(Bitu)(i + (i << 8u) + (i << 16u) + (i << 24u));
        TXT_BG_Table[i]=(Bitu)(i + (i << 8u) + (i << 16u) + (i << 24u));
#ifdef WORDS_BIGENDIAN
        FillTable[i]=
            ((i & 1u) ? 0xff000000u : 0u) |
            ((i & 2u) ? 0x00ff0000u : 0u) |
            ((i & 4u) ? 0x0000ff00u : 0u) |
            ((i & 8u) ? 0x000000ffu : 0u) ;
        TXT_Font_Table[i]=
            ((i & 1u) ? 0x000000ffu : 0u) |
            ((i & 2u) ? 0x0000ff00u : 0u) |
            ((i & 4u) ? 0x00ff0000u : 0u) |
            ((i & 8u) ? 0xff000000u : 0u) ;
#else 
        FillTable[i]=
            ((i & 1u) ? 0x000000ffu : 0u) |
            ((i & 2u) ? 0x0000ff00u : 0u) |
            ((i & 4u) ? 0x00ff0000u : 0u) |
            ((i & 8u) ? 0xff000000u : 0u) ;
        TXT_Font_Table[i]=  
            ((i & 1u) ? 0xff000000u : 0u) |
            ((i & 2u) ? 0x00ff0000u : 0u) |
            ((i & 4u) ? 0x0000ff00u : 0u) |
            ((i & 8u) ? 0x000000ffu : 0u) ;
#endif
    }
    for (j=0;j<4;j++) {
        for (i=0;i<16;i++) {
#ifdef WORDS_BIGENDIAN
            Expand16Table[j][i] =
                ((i & 1u) ? 1u <<        j  : 0u) |
                ((i & 2u) ? 1u << (8u +  j) : 0u) |
                ((i & 4u) ? 1u << (16u + j) : 0u) |
                ((i & 8u) ? 1u << (24u + j) : 0u);
#else
            Expand16Table[j][i] =
                ((i & 1u) ? 1u << (24u + j) : 0u) |
                ((i & 2u) ? 1u << (16u + j) : 0u) |
                ((i & 4u) ? 1u << (8u  + j) : 0u) |
                ((i & 8u) ? 1u <<        j  : 0u);
#endif
        }
    }

    for (i=0;i < 16;i++)
        VGASetPalette(i,text_palette[i][0]<<2u,text_palette[i][1]<<2u,text_palette[i][2]<<2u);

    VGA_DAC_UpdateColorPalette();

    AddVMEventFunction(VM_EVENT_RESET,AddVMEventFunctionFuncPair(VGA_Reset));
}

static inline Bitu VGA_Generic_Read_Handler(PhysPt planeaddr,PhysPt rawaddr,unsigned char plane) {
    const unsigned char hobit_n = (vga.seq.memory_mode&2/*Extended Memory*/) ? 16u : 14u;

    /* Sequencer Memory Mode Register (04h)
     * bits[3:3] = Chain 4 enable
     * bits[2:2] = Odd/Even Host Memory Write Addressing Disable
     * bits[1:1] = Extended memory (when EGA cards have > 64KB of RAM)
     * 
     * NTS: Real hardware experience says that despite the name, the Odd/Even bit affects reading as well */
    plane = (plane & ~1u) + (rawaddr & 1u);

    /* Graphics Controller: Miscellaneous Graphics Register register (06h)
     * bits[3:2] = memory map select
     * bits[1:1] = Chain Odd/Even Enable
     * bits[0:0] = Alphanumeric Mode Disable
     *
     * http://www.osdever.net/FreeVGA/vga/graphreg.htm
     *
     * When enabled, address bit A0 (bit 0) becomes bit 0 of the plane index.
     * Then when addressing VRAM A0 is replaced by a "higher order bit", which is
     * probably A14 or A16 depending on Extended Memory bit 1 in Sequencer register 04h memory mode */
    {/* Odd/Even enable */
        const PhysPt mask = (vga.config.compatible_chain4 ? 0u : ~0xFFFFu) + (1u << hobit_n) - 2u;
        const PhysPt hobit = (planeaddr >> hobit_n) & 1u;
        /* 1 << 14 =     0x4000
         * 1 << 14 - 1 = 0x3FFF
         * 1 << 14 - 2 = 0x3FFE
         * The point is to mask upper bit AND the LSB */
        planeaddr = (planeaddr & mask & (vga.mem.memmask >> 2u)) + hobit;
    }

    vga.latch.d=((Bit32u*)vga.mem.linear)[planeaddr];
    return (vga.latch.b[plane]);
}

template <const bool chained> static inline void VGA_Generic_Write_Handler(PhysPt planeaddr,PhysPt rawaddr,Bit8u val) {
    const unsigned char hobit_n = (vga.seq.memory_mode&2/*Extended Memory*/) ? 16u : 14u;
    Bit32u mask = 0xFFFF;

    /* Sequencer Memory Mode Register (04h)
     * bits[3:3] = Chain 4 enable
     * bits[2:2] = Odd/Even Host Memory Write Addressing Disable
     * bits[1:1] = Extended memory (when EGA cards have > 64KB of RAM)
     * 
     * NTS: Real hardware experience says that despite the name, the Odd/Even bit affects reading as well */
    mask &= 0xFF00FFu << ((rawaddr & 1u) * 8u);

    /* Graphics Controller: Miscellaneous Graphics Register register (06h)
     * bits[3:2] = memory map select
     * bits[1:1] = Chain Odd/Even Enable
     * bits[0:0] = Alphanumeric Mode Disable
     *
     * http://www.osdever.net/FreeVGA/vga/graphreg.htm
     *
     * When enabled, address bit A0 (bit 0) becomes bit 0 of the plane index.
     * Then when addressing VRAM A0 is replaced by a "higher order bit", which is
     * probably A14 or A16 depending on Extended Memory bit 1 in Sequencer register 04h memory mode */
    {/* Odd/Even enable */
        const PhysPt mask = (vga.config.compatible_chain4 ? 0u : ~0xFFFFu) + (1u << hobit_n) - 2u;
        const PhysPt hobit = (planeaddr >> hobit_n) & 1u;
        /* 1 << 14 =     0x4000
         * 1 << 14 - 1 = 0x3FFF
         * 1 << 14 - 2 = 0x3FFE
         * The point is to mask upper bit AND the LSB */
        planeaddr = (planeaddr & mask & (vga.mem.memmask >> 2u)) + hobit;
    }

    VGA_Latch pixels;

    pixels.d =((Bit32u*)vga.mem.linear)[planeaddr];
    pixels.d&=~mask;
    pixels.d|=(ExpandTable[val] & mask);

    ((Bit32u*)vga.mem.linear)[planeaddr]=pixels.d;
}

class VGA_UnchainedVGA_Handler : public PageHandler {
public:
	Bitu readHandler(PhysPt start) {
        return VGA_Generic_Read_Handler(start, start, vga.config.read_map_select);
	}
public:
	Bit8u readb(PhysPt addr) {
		addr = PAGING_GetPhysicalAddress(addr) & vga.mem.memmask;
		return (Bit8u)readHandler(addr);
	}
	Bit16u readw(PhysPt addr) {
		addr = PAGING_GetPhysicalAddress(addr) & vga.mem.memmask;
		Bit16u ret = (Bit16u)(readHandler(addr+0) << 0);
		ret     |= (readHandler(addr+1) << 8);
		return  ret;
	}
	Bit32u readd(PhysPt addr) {
		addr = PAGING_GetPhysicalAddress(addr) & vga.mem.memmask;
		Bit32u ret = (Bit32u)(readHandler(addr+0) << 0);
		ret     |= (readHandler(addr+1) << 8);
		ret     |= (readHandler(addr+2) << 16);
		ret     |= (readHandler(addr+3) << 24);
		return ret;
	}
public:
	void writeHandler(PhysPt start, Bit8u val) {
        VGA_Generic_Write_Handler<false/*chained*/>(start, start, val);
	}
public:
	VGA_UnchainedVGA_Handler() : PageHandler(PFLAG_NOCODE) {}
	void writeb(PhysPt addr,Bit8u val) {
		addr = PAGING_GetPhysicalAddress(addr) & vga.mem.memmask;
		writeHandler(addr+0,(Bit8u)(val >> 0));
	}
	void writew(PhysPt addr,Bit16u val) {
		addr = PAGING_GetPhysicalAddress(addr) & vga.mem.memmask;
		writeHandler(addr+0,(Bit8u)(val >> 0));
		writeHandler(addr+1,(Bit8u)(val >> 8));
	}
	void writed(PhysPt addr,Bit32u val) {
		addr = PAGING_GetPhysicalAddress(addr) & vga.mem.memmask;
		writeHandler(addr+0,(Bit8u)(val >> 0));
		writeHandler(addr+1,(Bit8u)(val >> 8));
		writeHandler(addr+2,(Bit8u)(val >> 16));
		writeHandler(addr+3,(Bit8u)(val >> 24));
	}
};

static struct vg {
	VGA_UnchainedVGA_Handler	uvga;
} vgaph;

void VGA_SetupHandlers(void) {
    MEM_SetPageHandler( VGA_PAGE_B8, 8,  &vgaph.uvga  );
	PAGING_ClearTLB();
}

static bool VGA_Memory_ShutDown_init = false;

static void VGA_Memory_ShutDown(Section * /*sec*/) {
	PAGING_ClearTLB();

	if (vga.mem.linear_orgptr != NULL) {
		delete[] vga.mem.linear_orgptr;
		vga.mem.linear_orgptr = NULL;
		vga.mem.linear = NULL;
	}
}

void VGA_SetupMemory() {
    if (vga.mem.linear == NULL) {
        VGA_Memory_ShutDown(NULL);

        vga.mem.linear_orgptr = new Bit8u[vga.mem.memsize+32u];
        memset(vga.mem.linear_orgptr,0,vga.mem.memsize+32u);
        vga.mem.linear=(Bit8u*)(((uintptr_t)vga.mem.linear_orgptr + 16ull-1ull) & ~(16ull-1ull));

        /* HACK. try to avoid stale pointers */
	    vga.draw.linear_base = vga.mem.linear;

        /* may be related */
        VGA_SetupHandlers();
    }

	if (!VGA_Memory_ShutDown_init) {
		AddExitFunction(AddExitFunctionFuncPair(VGA_Memory_ShutDown));
		VGA_Memory_ShutDown_init = true;
	}
}

const char* const mode_texts[M_MAX] = {
    "M_TEXT",           // 0
    "M_ERROR"
};

#if defined(_MSC_VER)
# pragma warning(disable:4244) /* const fmath::local::uint64_t to double possible loss of data */
# pragma warning(disable:4305) /* truncation from double to float */
#endif

//#undef C_DEBUG
//#define C_DEBUG 1
//#define LOG(X,Y) LOG_MSG

double vga_fps = 70;
double vga_mode_time_base = -1;
int vga_mode_frames_since_time_base = 0;

void memxor(void *_d,unsigned int byte,size_t count) {
    unsigned char *d = (unsigned char*)_d;
    while (count-- > 0) *d++ ^= byte;
}

void memxor_greendotted_16bpp(uint16_t *d,unsigned int count,unsigned int line) {
    static const uint16_t greenptrn[2] = { (0x3F << 5), 0 };
    line &= 1;
    count >>= 1;
    while (count-- > 0) {
        *d++ ^= greenptrn[line];
        *d++ ^= greenptrn[line^1];
    }
}

void memxor_greendotted_32bpp(uint32_t *d,unsigned int count,unsigned int line) {
    static const uint32_t greenptrn[2] = { (0xFF << 8), 0 };
    line &= 1;
    count >>= 2;
    while (count-- > 0) {
        *d++ ^= greenptrn[line];
        *d++ ^= greenptrn[line^1];
    }
}

typedef Bit8u * (* VGA_Line_Handler)(Bitu vidstart, Bitu line);

static VGA_Line_Handler VGA_DrawLine;
static Bit8u TempLine[SCALER_MAXWIDTH * 4 + 256];

extern Bit8u int10_font_16[256 * 16];

extern Bit32u Expand16Table[4][16];

static const Bit32u* VGA_Planar_Memwrap(Bitu vidstart) {
    return (const Bit32u*)vga.mem.linear + (vidstart & vga.draw.planar_mask);
}

static Bit32u FontMask[2]={0xffffffff,0x0};

template <const unsigned int card,typename templine_type_t> static inline Bit8u* EGAVGA_TEXT_Combined_Draw_Line(Bitu vidstart,Bitu line) {
    // keep it aligned:
    templine_type_t* draw = ((templine_type_t*)TempLine) + 16 - vga.draw.panning;
    const Bit32u* vidmem = VGA_Planar_Memwrap(vidstart); // pointer to chars+attribs
    Bitu blocks = vga.draw.blocks;
    if (vga.draw.panning) blocks++; // if the text is panned part of an 
                                    // additional character becomes visible

    while (blocks--) { // for each character in the line
        VGA_Latch pixels;

        pixels.d = *vidmem;
        vidmem += (uintptr_t)1U << (uintptr_t)1;

        Bitu chr = pixels.b[0];
        Bitu attr = pixels.b[1];
        // the font pattern
        Bitu font = int10_font_16[(chr<<4)+line];
        
        Bitu background = attr >> 4u;
        // if blinking is enabled bit7 is not mapped to attributes
        if (vga.draw.blinking) background &= ~0x8u;
        // choose foreground color if blinking not set for this cell or blink on
        Bitu foreground = (vga.draw.blink || (!(attr&0x80)))?
            (attr&0xf):background;
        // underline: all foreground [freevga: 0x77, previous 0x7]
        if (GCC_UNLIKELY(((attr&0x77) == 0x01) &&
            (vga.crtc.underline_location&0x1f)==line))
                background = foreground;
        if (vga.draw.char9dot) {
            font <<=1; // 9 pixels
            // extend to the 9th pixel if needed
            if ((font&0x2) &&
                (chr>=0xc0) && (chr<=0xdf)) font |= 1;
            for (Bitu n = 0; n < 9; n++) {
                if (card == MCH_VGA)
                    *draw++ = vga.dac.xlat32[(font&0x100)? foreground:background];

                font <<= 1;
            }
        } else {
            for (Bitu n = 0; n < 8; n++) {
                if (card == MCH_VGA)
                    *draw++ = vga.dac.xlat32[(font&0x80)? foreground:background];

                font <<= 1;
            }
        }
    }
    // draw the text mode cursor if needed
    if ((vga.draw.cursor.count&0x8) && (line >= vga.draw.cursor.sline) &&
        (line <= vga.draw.cursor.eline) && vga.draw.cursor.enabled) {
        // the adress of the attribute that makes up the cell the cursor is in
        Bits attr_addr = ((Bits)vga.draw.cursor.address - (Bits)vidstart) >> (Bits)1; /* <- FIXME: This right? */
        if (attr_addr >= 0 && attr_addr < (Bits)vga.draw.blocks) {
            Bitu index = (Bitu)attr_addr * (vga.draw.char9dot ? 9u : 8u);
            draw = (((templine_type_t*)TempLine) + index) + 16 - vga.draw.panning;

            Bitu foreground = vga.draw.linear_base[(vga.draw.cursor.address<<2ul)+1] & 0xf;
            for (Bitu i = 0; i < 8; i++) {
                if (card == MCH_VGA)
                    *draw++ = vga.dac.xlat32[foreground];
            }
        }
    }

    return TempLine+(16*sizeof(templine_type_t));
}

// combined 8/9-dot wide text mode 16bpp line drawing function
static Bit8u* VGA_TEXT_Xlat32_Draw_Line(Bitu vidstart, Bitu line) {
    return EGAVGA_TEXT_Combined_Draw_Line<MCH_VGA,Bit32u>(vidstart,line);
}

static void VGA_DrawSingleLine(Bitu /*blah*/) {
    if ((++vga.draw.render_step) >= vga.draw.render_max)
        vga.draw.render_step = 0;

    {
        Bit8u * data=VGA_DrawLine( vga.draw.address, vga.draw.address_line );
        RENDER_DrawLine(data);
    }

    vga.draw.address_line++;
    if (vga.draw.address_line>=vga.draw.address_line_total) {
        vga.draw.address_line=0;
        vga.draw.address+=vga.draw.address_add;
    }

    vga.draw.lines_done++;

    if (vga.draw.lines_done < vga.draw.lines_total) {
        PIC_AddEvent(VGA_DrawSingleLine,(float)vga.draw.delay.singleline_delay);
    } else {
        vga_mode_frames_since_time_base++;
        RENDER_EndUpdate(false);
    }
}

static void VGA_VerticalTimer(Bitu /*val*/) {
    PIC_AddEvent(VGA_VerticalTimer,(float)vga.draw.delay.vtotal);

    // for same blinking frequency with higher frameskip
    vga.draw.cursor.count++;

    //Check if we can actually render, else skip the rest
    if (vga.draw.vga_override || !RENDER_StartUpdate()) return;

    vga.draw.cursor.address = vga.config.cursor_start * 2;
    vga.draw.address_line = 0;
    vga.draw.address = 0;

    /* check for blinking and blinking change delay */
    FontMask[1]=(vga.draw.blinking & (unsigned int)(vga.draw.cursor.count >> 4u)) ?
        0 : 0xffffffff;
    /* if blinking is enabled, 'blink' will toggle between true
     * and false. Otherwise it's true */
    vga.draw.blink = ((vga.draw.blinking & (unsigned int)(vga.draw.cursor.count >> 4u))
            || !vga.draw.blinking) ? true:false;

    vga.draw.linear_base = vga.mem.linear;
    vga.draw.planar_mask = vga.mem.memmask >> 2;

    // add the draw event
    switch (vga.draw.mode) {
    case DRAWLINE:
        if (GCC_UNLIKELY(vga.draw.lines_done < vga.draw.lines_total)) {
            LOG(LOG_VGAMISC,LOG_NORMAL)( "Lines left: %d", 
                (int)(vga.draw.lines_total-vga.draw.lines_done));
            PIC_RemoveEvents(VGA_DrawSingleLine);
            vga_mode_frames_since_time_base++;
            RENDER_EndUpdate(true);
        }
        vga.draw.lines_done = 0;
        PIC_AddEvent(VGA_DrawSingleLine,(float)(vga.draw.delay.htotal/4.0));
        break;
    }
}

void VGA_SetupDrawing(Bitu /*val*/) {
    // user choosable special trick support
    // multiscan -- zooming effects - only makes sense if linewise is enabled
    // linewise -- scan display line by line instead of 4 blocks
    // keep compatibility with other builds of DOSBox for vgaonly.
    vga.draw.render_step = 0;
    vga.draw.render_max = 1;

    // set the drawing mode
    switch (machine) {
        case MCH_VGA:
            if (svgaCard==SVGA_None) {
                vga.draw.mode = DRAWLINE;
                break;
            }
            // fall-through
        default:
            vga.draw.mode = DRAWLINE;
            break;
    }

    /* Calculate the FPS for this screen */
    Bitu oscclock = 0, clock;
    Bitu htotal, hdend, hbstart, hbend, hrstart, hrend;
    Bitu vtotal, vdend, vbstart, vbend, vrstart, vrend;

    {
        htotal = 100;
        hdend = 80;
        hbstart = 80 + 1;
        hbend = 100 - 1;
        hrstart = 110;
        hrend = 112;

        vtotal = 449;
        vdend = 400;
        vbstart = 400 + 8;
        vbend = 449 - 8;
        vrstart = 420;
        vrend = 422;

        oscclock = 28322000;
        clock = oscclock/9;
    }
#if C_DEBUG
    LOG(LOG_VGA,LOG_NORMAL)("h total %3d end %3d blank (%3d/%3d) retrace (%3d/%3d)",
        (int)htotal, (int)hdend, (int)hbstart, (int)hbend, (int)hrstart, (int)hrend );
    LOG(LOG_VGA,LOG_NORMAL)("v total %3d end %3d blank (%3d/%3d) retrace (%3d/%3d)",
        (int)vtotal, (int)vdend, (int)vbstart, (int)vbend, (int)vrstart, (int)vrend );
#endif
    if (!htotal) return;
    if (!vtotal) return;
    
    // The screen refresh frequency
    double fps;
    extern double vga_force_refresh_rate;
    if (vga_force_refresh_rate > 0) {
        /* force the VGA refresh rate by setting fps and bending the clock to our will */
        LOG(LOG_VGA,LOG_NORMAL)("VGA forced refresh rate in effect, %.3f",vga_force_refresh_rate);
        fps=vga_force_refresh_rate;
        clock=((double)(vtotal*htotal))*fps;
    }
    else {
        // The screen refresh frequency
        fps=(double)clock/(vtotal*htotal);
        LOG(LOG_VGA,LOG_NORMAL)("VGA refresh rate is now, %.3f",fps);
    }

    /* clip display end to stay within vtotal ("Monolith" demo part 4 320x570 mode fix) */
    if (vdend > vtotal) {
        LOG(LOG_VGA,LOG_WARN)("VGA display end greater than vtotal!");
        vdend = vtotal;
    }

    // Horizontal total (that's how long a line takes with whistles and bells)
    vga.draw.delay.htotal = htotal*1000.0/clock; //in milliseconds
    // Start and End of horizontal blanking
    vga.draw.delay.hblkstart = hbstart*1000.0/clock; //in milliseconds
    vga.draw.delay.hblkend = hbend*1000.0/clock; 
    // Start and End of horizontal retrace
    vga.draw.delay.hrstart = hrstart*1000.0/clock;
    vga.draw.delay.hrend = hrend*1000.0/clock;
    // Start and End of vertical blanking
    vga.draw.delay.vblkstart = vbstart * vga.draw.delay.htotal;
    vga.draw.delay.vblkend = vbend * vga.draw.delay.htotal;
    // Start and End of vertical retrace pulse
    vga.draw.delay.vrstart = vrstart * vga.draw.delay.htotal;
    vga.draw.delay.vrend = vrend * vga.draw.delay.htotal;
    vga.draw.vblank_skip = 0;

    // Display end
    vga.draw.delay.vdend = vdend * vga.draw.delay.htotal;

    /*
      6  Horizontal Sync Polarity. Negative if set
      7  Vertical Sync Polarity. Negative if set
         Bit 6-7 indicates the number of lines on the display:
            1:  400, 2: 350, 3: 480
    */
    //Try to determine the pixel size, aspect correct is based around square pixels

    //Base pixel width around 100 clocks horizontal
    //For 9 pixel text modes this should be changed, but we don't support that anyway :)
    //Seems regular vga only listens to the 9 char pixel mode with character mode enabled
    //Base pixel height around vertical totals of modes that have 100 clocks horizontal
    //Different sync values gives different scaling of the whole vertical range
    //VGA monitor just seems to thighten or widen the whole vertical range

    vga.draw.resizing=false;

    //Check to prevent useless black areas
    if (hbstart<hdend) hdend=hbstart;
    if ((!(IS_VGA_ARCH)) && (vbstart<vdend)) vdend=vbstart;

    Bitu width=hdend;
    Bitu height=vdend;

    vga.draw.address_line_total=16;

    //Set the bpp
    Bitu bpp;
    bpp = 8;

    Bitu pix_per_char = 8;
    switch (vga.mode) {
    case M_TEXT:
        vga.draw.blocks=width;
        // 9-pixel wide
        pix_per_char = 9;
        vga.draw.char9dot = true;
        VGA_DrawLine = VGA_TEXT_Xlat32_Draw_Line;
        bpp = 32;
        break;
    default:
        LOG(LOG_VGA,LOG_ERROR)("Unhandled VGA mode %d while checking for resolution",vga.mode);
        break;
    }
    width *= pix_per_char;

    vga.draw.lines_total=height;
    vga.draw.line_length = width * ((bpp + 1) / 8);
    vga.draw.oscclock = oscclock;
    vga.draw.clock = clock;

    double vratio = ((double)width)/(double)height; // ratio if pixels were square

    // the picture ratio factor
    double scanratio =  ((double)hdend/(double)(htotal-(hrend-hrstart)))/
                        ((double)vdend/(double)(vtotal-(vrend-vrstart)));
    double scanfield_ratio = 4.0/3.0;
    
    switch (vga.misc_output >> 6) {
        case 0: // VESA: "OTHER" scanline amount
            scanfield_ratio = (4.0/3.0) / scanratio;
            break;
        case 1: // 400 lines
            scanfield_ratio = 1.312;
            break;
        case 2: // 350 lines
            scanfield_ratio = 1.249;
            break;
        case 3: // 480 lines
            scanfield_ratio = 1.345;
            break;
    }

    // calculate screen ratio
    double screenratio = scanratio * scanfield_ratio;

    // override screenratio for certain cases:
    if (vratio == 1.6) screenratio = 4.0 / 3.0;
    else if (vratio == 0.8) screenratio = 4.0 / 3.0;
    else if (vratio == 3.2) screenratio = 4.0 / 3.0;
    else if (vratio == (4.0/3.0)) screenratio = 4.0 / 3.0;
    else if (vratio == (2.0/3.0)) screenratio = 4.0 / 3.0;
    else if ((width >= 800)&&(height>=600)) screenratio = 4.0 / 3.0;

#if C_DEBUG
            LOG(LOG_VGA,LOG_NORMAL)("screen: %1.3f, scanfield: %1.3f, scan: %1.3f, vratio: %1.3f",
                screenratio, scanfield_ratio, scanratio, vratio);
#endif

    bool fps_changed = false;

#if C_DEBUG
    LOG(LOG_VGA,LOG_NORMAL)("h total %2.5f (%3.2fkHz) blank(%02.5f/%02.5f) retrace(%02.5f/%02.5f)",
        vga.draw.delay.htotal,(1.0/vga.draw.delay.htotal),
        vga.draw.delay.hblkstart,vga.draw.delay.hblkend,
        vga.draw.delay.hrstart,vga.draw.delay.hrend);
    LOG(LOG_VGA,LOG_NORMAL)("v total %2.5f (%3.2fHz) blank(%02.5f/%02.5f) retrace(%02.5f/%02.5f)",
        vga.draw.delay.vtotal,(1000.0/vga.draw.delay.vtotal),
        vga.draw.delay.vblkstart,vga.draw.delay.vblkend,
        vga.draw.delay.vrstart,vga.draw.delay.vrend);

    LOG(LOG_VGA,LOG_NORMAL)("video clock: %3.2fMHz mode %s",
        oscclock/1000000.0, mode_texts[vga.mode]);
#endif

    // need to change the vertical timing?
    if (vga_mode_time_base < 0 || fabs(vga.draw.delay.vtotal - 1000.0 / fps) > 0.0001)
        fps_changed = true;

    // need to resize the output window?
    if ((width != vga.draw.width) ||
        (height != vga.draw.height) ||
        (fabs(screenratio - vga.draw.screen_ratio) > 0.0001) ||
        (vga.draw.bpp != bpp) || fps_changed) {

        VGA_KillDrawing();

        vga.draw.width = width;
        vga.draw.height = height;
        vga.draw.screen_ratio = screenratio;
        vga.draw.bpp = bpp;
#if C_DEBUG
        LOG(LOG_VGA,LOG_NORMAL)("%dx%d, %3.2fHz, %dbpp, screen %1.3f",(int)width,(int)height,fps,(int)bpp,screenratio);
#endif
        if (!vga.draw.vga_override)
            RENDER_SetSize(width,height,bpp,(float)fps,screenratio);

        if (fps_changed) {
            vga_mode_time_base = PIC_GetCurrentEventTime();
            vga_mode_frames_since_time_base = 0;
            PIC_RemoveEvents(VGA_VerticalTimer);
            vga.draw.delay.vtotal = 1000.0 / fps;
            vga.draw.lines_done = vga.draw.lines_total;
            vga_fps = fps;
            VGA_VerticalTimer(0);
        }
    }
    vga.draw.delay.singleline_delay = (float)vga.draw.delay.htotal;

    {
        /* FIXME: Why is this required to prevent VGA palette errors with Crystal Dream II?
         *        What is this code doing to change the palette prior to this point? */
        VGA_DAC_UpdateColorPalette();
    }
}

void VGA_KillDrawing(void) {
    PIC_RemoveEvents(VGA_DrawSingleLine);
}

void VGA_SetOverride(bool vga_override) {
    if (vga.draw.vga_override!=vga_override) {
        
        if (vga_override) {
            VGA_KillDrawing();
            vga.draw.vga_override=true;
        } else {
            vga.draw.vga_override=false;
            vga.draw.width=0; // change it so the output window gets updated
            VGA_SetupDrawing(0);
        }
    }
}

