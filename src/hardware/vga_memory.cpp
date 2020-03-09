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


#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dosbox.h"
#include "mem.h"
#include "vga.h"
#include "paging.h"
#include "pic.h"
#include "inout.h"
#include "setup.h"
#include "cpu.h"

#ifndef C_VGARAM_CHECKED
#define C_VGARAM_CHECKED 1
#endif

#if C_VGARAM_CHECKED
// Checked linear offset
#define CHECKED(v) ((v)&vga.mem.memmask)
// Checked planar offset (latched access)
#define CHECKED2(v) ((v)&(vga.mem.memmask>>2))
#else
#define CHECKED(v) (v)
#define CHECKED2(v) (v)
#endif

#define CHECKED3(v) ((v)&vga.mem.memmask)
#define CHECKED4(v) ((v)&(vga.mem.memmask>>2))

#define TANDY_VIDBASE(_X_)  &MemBase[ 0x80000 + (_X_)]

template <class Size>
static INLINE void hostWrite(HostPt off, Bitu val) {
	if ( sizeof( Size ) == 1)
		host_writeb( off, (Bit8u)val );
	else if ( sizeof( Size ) == 2)
		host_writew( off, (Bit16u)val );
	else if ( sizeof( Size ) == 4)
		host_writed( off, (Bit32u)val );
}

template <class Size>
static INLINE Bitu  hostRead(HostPt off ) {
	if ( sizeof( Size ) == 1)
		return host_readb( off );
	else if ( sizeof( Size ) == 2)
		return host_readw( off );
	else if ( sizeof( Size ) == 4)
		return host_readd( off );
	return 0;
}

/* Gonna assume that whoever maps vga memory, maps it on 32/64kb boundary */

#define VGA_PAGES		(128/4)
#define VGA_PAGE_A0		(0xA0000/4096)
#define VGA_PAGE_B0		(0xB0000/4096)
#define VGA_PAGE_B8		(0xB8000/4096)

static struct {
	Bitu base, mask;
} vgapages;

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
    Bit32u mask = vga.config.full_map_mask;

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
		addr = PAGING_GetPhysicalAddress(addr) & vgapages.mask;
		return (Bit8u)readHandler(addr);
	}
	Bit16u readw(PhysPt addr) {
		addr = PAGING_GetPhysicalAddress(addr) & vgapages.mask;
		Bit16u ret = (Bit16u)(readHandler(addr+0) << 0);
		ret     |= (readHandler(addr+1) << 8);
		return  ret;
	}
	Bit32u readd(PhysPt addr) {
		addr = PAGING_GetPhysicalAddress(addr) & vgapages.mask;
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
		addr = PAGING_GetPhysicalAddress(addr) & vgapages.mask;
		writeHandler(addr+0,(Bit8u)(val >> 0));
	}
	void writew(PhysPt addr,Bit16u val) {
		addr = PAGING_GetPhysicalAddress(addr) & vgapages.mask;
		writeHandler(addr+0,(Bit8u)(val >> 0));
		writeHandler(addr+1,(Bit8u)(val >> 8));
	}
	void writed(PhysPt addr,Bit32u val) {
		addr = PAGING_GetPhysicalAddress(addr) & vgapages.mask;
		writeHandler(addr+0,(Bit8u)(val >> 0));
		writeHandler(addr+1,(Bit8u)(val >> 8));
		writeHandler(addr+2,(Bit8u)(val >> 16));
		writeHandler(addr+3,(Bit8u)(val >> 24));
	}
};

#include <stdio.h>

class VGA_Empty_Handler : public PageHandler {
public:
	VGA_Empty_Handler() : PageHandler(PFLAG_NOCODE) {}
	Bit8u readb(PhysPt /*addr*/) {
//		LOG(LOG_VGA, LOG_NORMAL ) ( "Read from empty memory space at %x", addr );
		return 0xff;
	} 
	void writeb(PhysPt /*addr*/,Bit8u /*val*/) {
//		LOG(LOG_VGA, LOG_NORMAL ) ( "Write %x to empty memory space at %x", val, addr );
	}
};

static struct vg {
	VGA_UnchainedVGA_Handler	uvga;
	VGA_Empty_Handler			empty;
} vgaph;

void VGA_ChangedBank(void) {
	VGA_SetupHandlers();
}

void MEM_ResetPageHandler_Unmapped(Bitu phys_page, Bitu pages);
void MEM_ResetPageHandler_RAM(Bitu phys_page, Bitu pages);

void VGA_SetupHandlers(void) {
    vgapages.base = VGA_PAGE_B8;
    vgapages.mask = 0x7fff & vga.mem.memmask;
    MEM_SetPageHandler( VGA_PAGE_B8, 8,  &vgaph.uvga  );
    MEM_SetPageHandler( VGA_PAGE_A0, 16, &vgaph.empty );
    MEM_SetPageHandler( VGA_PAGE_B0, 8,  &vgaph.empty );

	PAGING_ClearTLB();
}

static bool VGA_Memory_ShutDown_init = false;

static void VGA_Memory_ShutDown(Section * /*sec*/) {
	MEM_SetPageHandler(VGA_PAGE_A0,32,&vgaph.empty);
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

