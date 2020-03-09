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

extern bool non_cga_ignore_oddeven;
extern bool non_cga_ignore_oddeven_engage;

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

/* how much delay to add to VGA memory I/O in nanoseconds */
int vga_memio_delay_ns = 1000;

void VGAMEM_USEC_read_delay() {
	if (vga_memio_delay_ns > 0) {
		Bits delaycyc = (CPU_CycleMax * vga_memio_delay_ns) / 1000000;
//		if(GCC_UNLIKELY(CPU_Cycles < 3*delaycyc)) delaycyc = 0; //Else port acces will set cycles to 0. which might trigger problem with games which read 16 bit values
		CPU_Cycles -= delaycyc;
		CPU_IODelayRemoved += delaycyc;
	}
}

void VGAMEM_USEC_write_delay() {
	if (vga_memio_delay_ns > 0) {
		Bits delaycyc = (CPU_CycleMax * vga_memio_delay_ns * 3) / (1000000 * 4);
//		if(GCC_UNLIKELY(CPU_Cycles < 3*delaycyc)) delaycyc = 0; //Else port acces will set cycles to 0. which might trigger problem with games which read 16 bit values
		CPU_Cycles -= delaycyc;
		CPU_IODelayRemoved += delaycyc;
	}
}

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


void VGA_MapMMIO(void);
//Nice one from DosEmu
INLINE static Bit32u RasterOp(Bit32u input,Bit32u mask) {
	switch (vga.config.raster_op) {
	case 0x00:	/* None */
		return (input & mask) | (vga.latch.d & ~mask);
	case 0x01:	/* AND */
		return (input | ~mask) & vga.latch.d;
	case 0x02:	/* OR */
		return (input & mask) | vga.latch.d;
	case 0x03:	/* XOR */
		return (input & mask) ^ vga.latch.d;
	}
	return 0;
}

INLINE static Bit32u ModeOperation(Bit8u val) {
	Bit32u full;
	switch (vga.config.write_mode) {
	case 0x00:
		// Write Mode 0: In this mode, the host data is first rotated as per the Rotate Count field, then the Enable Set/Reset mechanism selects data from this or the Set/Reset field. Then the selected Logical Operation is performed on the resulting data and the data in the latch register. Then the Bit Mask field is used to select which bits come from the resulting data and which come from the latch register. Finally, only the bit planes enabled by the Memory Plane Write Enable field are written to memory. 
		val=((val >> vga.config.data_rotate) | (val << (8-vga.config.data_rotate)));
		full=ExpandTable[val];
		full=(full & vga.config.full_not_enable_set_reset) | vga.config.full_enable_and_set_reset; 
		full=RasterOp(full,vga.config.full_bit_mask);
		break;
	case 0x01:
		// Write Mode 1: In this mode, data is transferred directly from the 32 bit latch register to display memory, affected only by the Memory Plane Write Enable field. The host data is not used in this mode. 
		full=vga.latch.d;
		break;
	case 0x02:
		//Write Mode 2: In this mode, the bits 3-0 of the host data are replicated across all 8 bits of their respective planes. Then the selected Logical Operation is performed on the resulting data and the data in the latch register. Then the Bit Mask field is used to select which bits come from the resulting data and which come from the latch register. Finally, only the bit planes enabled by the Memory Plane Write Enable field are written to memory. 
		full=RasterOp(FillTable[val&0xF],vga.config.full_bit_mask);
		break;
	case 0x03:
		// Write Mode 3: In this mode, the data in the Set/Reset field is used as if the Enable Set/Reset field were set to 1111b. Then the host data is first rotated as per the Rotate Count field, then logical ANDed with the value of the Bit Mask field. The resulting value is used on the data obtained from the Set/Reset field in the same way that the Bit Mask field would ordinarily be used. to select which bits come from the expansion of the Set/Reset field and which come from the latch register. Finally, only the bit planes enabled by the Memory Plane Write Enable field are written to memory.
		val=((val >> vga.config.data_rotate) | (val << (8-vga.config.data_rotate)));
		full=RasterOp(vga.config.full_set_reset,ExpandTable[val] & vga.config.full_bit_mask);
		break;
	default:
		LOG(LOG_VGAMISC,LOG_NORMAL)("VGA:Unsupported write mode %d",vga.config.write_mode);
		full=0;
		break;
	}
	return full;
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
    if (!(vga.seq.memory_mode&4) && !non_cga_ignore_oddeven_engage)/* Odd Even Host Memory Write Addressing Disable (is not set) */
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
    if ((vga.gfx.miscellaneous&2) && !non_cga_ignore_oddeven_engage) {/* Odd/Even enable */
        const PhysPt mask = (vga.config.compatible_chain4 ? 0u : ~0xFFFFu) + (1u << hobit_n) - 2u;
        const PhysPt hobit = (planeaddr >> hobit_n) & 1u;
        /* 1 << 14 =     0x4000
         * 1 << 14 - 1 = 0x3FFF
         * 1 << 14 - 2 = 0x3FFE
         * The point is to mask upper bit AND the LSB */
        planeaddr = (planeaddr & mask & (vga.mem.memmask >> 2u)) + hobit;
    }
    else {
        const PhysPt mask = (vga.config.compatible_chain4 ? 0u : ~0xFFFFu) + (1u << hobit_n) - 1u;
        planeaddr &= mask & (vga.mem.memmask >> 2u);
    }

    vga.latch.d=((Bit32u*)vga.mem.linear)[planeaddr];
    switch (vga.config.read_mode) {
        case 0:
            return (vga.latch.b[plane]);
        case 1:
            VGA_Latch templatch;
            templatch.d=(vga.latch.d & FillTable[vga.config.color_dont_care]) ^ FillTable[vga.config.color_compare & vga.config.color_dont_care];
            return (Bit8u)~(templatch.b[0] | templatch.b[1] | templatch.b[2] | templatch.b[3]);
    }

    return 0;
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
    if (chained) {
        if (!(vga.seq.memory_mode&4) && !non_cga_ignore_oddeven_engage)/* Odd Even Host Memory Write Addressing Disable (is not set) */
            mask &= 0xFF00FFu << ((rawaddr & 1u) * 8u);
        else
            mask &= 0xFFu << ((rawaddr & 3u) * 8u);
    }
    else {
        if (!(vga.seq.memory_mode&4) && !non_cga_ignore_oddeven_engage)/* Odd Even Host Memory Write Addressing Disable (is not set) */
            mask &= 0xFF00FFu << ((rawaddr & 1u) * 8u);
    }

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
    if ((vga.gfx.miscellaneous&2) && !non_cga_ignore_oddeven_engage) {/* Odd/Even enable */
        const PhysPt mask = (vga.config.compatible_chain4 ? 0u : ~0xFFFFu) + (1u << hobit_n) - 2u;
        const PhysPt hobit = (planeaddr >> hobit_n) & 1u;
        /* 1 << 14 =     0x4000
         * 1 << 14 - 1 = 0x3FFF
         * 1 << 14 - 2 = 0x3FFE
         * The point is to mask upper bit AND the LSB */
        planeaddr = (planeaddr & mask & (vga.mem.memmask >> 2u)) + hobit;
    }
    else {
        const PhysPt mask = (vga.config.compatible_chain4 ? 0u : ~0xFFFFu) + (1u << hobit_n) - 1u;
        planeaddr &= mask & (vga.mem.memmask >> 2u);
    }

    Bit32u data=ModeOperation(val);
    VGA_Latch pixels;

    pixels.d =((Bit32u*)vga.mem.linear)[planeaddr];
    pixels.d&=~mask;
    pixels.d|=(data & mask);

    ((Bit32u*)vga.mem.linear)[planeaddr]=pixels.d;
}

// Slow accurate emulation.
// This version takes the Graphics Controller bitmask and ROPs into account.
// This is needed for demos that use the bitmask to do color combination or bitplane "page flipping" tricks.
// This code will kick in if running in a chained VGA mode and the graphics controller bitmask register is
// changed to anything other than 0xFF.
//
// Impact Studios "Legend"
//  - The rotating objects, rendered as dots, needs this hack because it uses a combination of masking off
//    bitplanes using the VGA DAC pel mask and drawing on the hidden bitplane using the Graphics Controller
//    bitmask. It also relies on loading the VGA latches with zeros as a form of "overdraw". Without this
//    version the effect will instead become a glowing ball of flickering yellow/red.
class VGA_ChainedVGA_Slow_Handler : public PageHandler {
public:
	VGA_ChainedVGA_Slow_Handler() : PageHandler(PFLAG_NOCODE) {}
	static INLINE Bitu readHandler8(PhysPt addr ) {
        // planar byte offset = addr & ~3u      (discard low 2 bits)
        // planer index = addr & 3u             (use low 2 bits as plane index)
        // FIXME: Does chained mode use the lower 2 bits of the CPU address or does it use the read mode select???
        return VGA_Generic_Read_Handler(addr&~3u, addr, (Bit8u)(addr&3u));
	}
	static INLINE void writeHandler8(PhysPt addr, Bitu val) {
        // planar byte offset = addr & ~3u      (discard low 2 bits)
        // planer index = addr & 3u             (use low 2 bits as plane index)
        return VGA_Generic_Write_Handler<true/*chained*/>(addr&~3u, addr, (Bit8u)val);
	}
	Bit8u readb(PhysPt addr ) {
		VGAMEM_USEC_read_delay();
		addr = PAGING_GetPhysicalAddress(addr) & vgapages.mask;
		return (Bit8u)readHandler8( addr );
	}
	Bit16u readw(PhysPt addr ) {
		VGAMEM_USEC_read_delay();
		addr = PAGING_GetPhysicalAddress(addr) & vgapages.mask;
		Bit16u ret = (Bit16u)(readHandler8( addr+0 ) << 0 );
		ret     |= (readHandler8( addr+1 ) << 8 );
		return ret;
	}
	Bit32u readd(PhysPt addr ) {
		VGAMEM_USEC_read_delay();
		addr = PAGING_GetPhysicalAddress(addr) & vgapages.mask;
		Bit32u ret = (Bit32u)(readHandler8( addr+0 ) << 0 );
		ret     |= (readHandler8( addr+1 ) << 8 );
		ret     |= (readHandler8( addr+2 ) << 16 );
		ret     |= (readHandler8( addr+3 ) << 24 );
		return ret;
	}
	void writeb(PhysPt addr, Bit8u val ) {
		VGAMEM_USEC_write_delay();
		addr = PAGING_GetPhysicalAddress(addr) & vgapages.mask;
		writeHandler8( addr, val );
	}
	void writew(PhysPt addr,Bit16u val) {
		VGAMEM_USEC_write_delay();
		addr = PAGING_GetPhysicalAddress(addr) & vgapages.mask;
		writeHandler8( addr+0, (Bit8u)(val >> 0u) );
		writeHandler8( addr+1, (Bit8u)(val >> 8u) );
	}
	void writed(PhysPt addr,Bit32u val) {
		VGAMEM_USEC_write_delay();
		addr = PAGING_GetPhysicalAddress(addr) & vgapages.mask;
		writeHandler8( addr+0, (Bit8u)(val >> 0u) );
		writeHandler8( addr+1, (Bit8u)(val >> 8u) );
		writeHandler8( addr+2, (Bit8u)(val >> 16u) );
		writeHandler8( addr+3, (Bit8u)(val >> 24u) );
	}
};

class VGA_UnchainedVGA_Handler : public PageHandler {
public:
	Bitu readHandler(PhysPt start) {
        return VGA_Generic_Read_Handler(start, start, vga.config.read_map_select);
	}
public:
	Bit8u readb(PhysPt addr) {
		VGAMEM_USEC_read_delay();
		addr = PAGING_GetPhysicalAddress(addr) & vgapages.mask;
		return (Bit8u)readHandler(addr);
	}
	Bit16u readw(PhysPt addr) {
		VGAMEM_USEC_read_delay();
		addr = PAGING_GetPhysicalAddress(addr) & vgapages.mask;
		Bit16u ret = (Bit16u)(readHandler(addr+0) << 0);
		ret     |= (readHandler(addr+1) << 8);
		return  ret;
	}
	Bit32u readd(PhysPt addr) {
		VGAMEM_USEC_read_delay();
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
		VGAMEM_USEC_write_delay();
		addr = PAGING_GetPhysicalAddress(addr) & vgapages.mask;
		writeHandler(addr+0,(Bit8u)(val >> 0));
	}
	void writew(PhysPt addr,Bit16u val) {
		VGAMEM_USEC_write_delay();
		addr = PAGING_GetPhysicalAddress(addr) & vgapages.mask;
		writeHandler(addr+0,(Bit8u)(val >> 0));
		writeHandler(addr+1,(Bit8u)(val >> 8));
	}
	void writed(PhysPt addr,Bit32u val) {
		VGAMEM_USEC_write_delay();
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
//	VGA_Map_Handler				map;
//	VGA_Slow_CGA_Handler		slow;
//	VGA_TEXT_PageHandler		text;
//	VGA_CGATEXT_PageHandler		cgatext;
//	VGA_MCGATEXT_PageHandler	mcgatext;
//	VGA_TANDY_PageHandler		tandy;
//	VGA_ChainedEGA_Handler		cega;
//	VGA_ChainedVGA_Handler		cvga;
//	VGA_ChainedVGA_Slow_Handler	cvga_slow;
//	VGA_ET4000_ChainedVGA_Handler		cvga_et4000;
//	VGA_ET4000_ChainedVGA_Slow_Handler	cvga_et4000_slow;
//	VGA_UnchainedEGA_Handler	uega;
	VGA_UnchainedVGA_Handler	uvga;
//	VGA_PCJR_Handler			pcjr;
//	VGA_HERC_Handler			herc;
//	VGA_LIN4_Handler			lin4;
//	VGA_LFB_Handler				lfb;
//	VGA_MMIO_Handler			mmio;
//	VGA_AMS_Handler				ams;
	VGA_Empty_Handler			empty;
} vgaph;

void VGA_ChangedBank(void) {
	VGA_SetupHandlers();
}

void MEM_ResetPageHandler_Unmapped(Bitu phys_page, Bitu pages);
void MEM_ResetPageHandler_RAM(Bitu phys_page, Bitu pages);

void VGA_SetupHandlers(void) {
	PageHandler *newHandler;
	switch (machine) {
	case EGAVGA_ARCH_CASE:
        break;
	default:
		LOG_MSG("Illegal machine type %d", machine );
		return;
	}

	/* This should be vga only */
	switch (vga.mode) {
	default:
		return;
	case M_TEXT:
        newHandler = &vgaph.uvga;
        break;
	}
	switch ((vga.gfx.miscellaneous >> 2) & 3) {
	case 0:
        vgapages.base = VGA_PAGE_A0;
        switch (svgaCard) {
            case SVGA_S3Trio:
            default:
                vgapages.mask = 0xffff & vga.mem.memmask;
                break;
		}
		MEM_SetPageHandler(VGA_PAGE_A0, 32, newHandler );
		break;
	case 1:
		vgapages.base = VGA_PAGE_A0;
		vgapages.mask = 0xffff & vga.mem.memmask;
		MEM_SetPageHandler( VGA_PAGE_A0, 16, newHandler );
		MEM_SetPageHandler( VGA_PAGE_B0, 16, &vgaph.empty );
		break;
	case 2:
		vgapages.base = VGA_PAGE_B0;
		vgapages.mask = 0x7fff & vga.mem.memmask;
		MEM_SetPageHandler( VGA_PAGE_B0, 8, newHandler );
		MEM_SetPageHandler( VGA_PAGE_A0, 16, &vgaph.empty );
		MEM_SetPageHandler( VGA_PAGE_B8, 8, &vgaph.empty );
        break;
	case 3:
		vgapages.base = VGA_PAGE_B8;
		vgapages.mask = 0x7fff & vga.mem.memmask;
		MEM_SetPageHandler( VGA_PAGE_B8, 8, newHandler );
		MEM_SetPageHandler( VGA_PAGE_A0, 16, &vgaph.empty );
		MEM_SetPageHandler( VGA_PAGE_B0, 8, &vgaph.empty );
        break;
	}

    non_cga_ignore_oddeven_engage = false;

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

