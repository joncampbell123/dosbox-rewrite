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

#include <string.h>
#include <stdlib.h>
#include <string>
#include <stdio.h>

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

