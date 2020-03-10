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
#include "inout.h"
#include "render.h"
#include "vga.h"

/*
3C6h (R/W):  PEL Mask
bit 0-7  This register is anded with the palette index sent for each dot.
         Should be set to FFh.

3C7h (R):  DAC State Register
bit 0-1  0 indicates the DAC is in Write Mode and 3 indicates Read mode.

3C7h (W):  PEL Address Read Mode
bit 0-7  The PEL data register (0..255) to be read from 3C9h.
Note: After reading the 3 bytes at 3C9h this register will increment,
      pointing to the next data register.

3C8h (R/W):  PEL Address Write Mode
bit 0-7  The PEL data register (0..255) to be written to 3C9h.
Note: After writing the 3 bytes at 3C9h this register will increment, pointing
      to the next data register.

3C9h (R/W):  PEL Data Register
bit 0-5  Color value
Note:  Each read or write of this register will cycle through first the
       registers for Red, Blue and Green, then increment the appropriate
       address register, thus the entire palette can be loaded by writing 0 to
       the PEL Address Write Mode register 3C8h and then writing all 768 bytes
       of the palette to this register.
*/

enum {DAC_READ,DAC_WRITE};

static void VGA_DAC_SendColor( Bitu index, Bitu src ) {
    const Bit8u dacshift = 2u;
    const Bit8u red = vga.dac.rgb[src].red << dacshift;
    const Bit8u green = vga.dac.rgb[src].green << dacshift;
    const Bit8u blue = vga.dac.rgb[src].blue << dacshift;

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

        /* PC-98 mode always renders 32bpp, therefore needs this fix */
        if (GFX_Bshift == 0)
            vga.dac.xlat32[index] = (uint32_t)(blue << 0U) | (uint32_t)(green << 8U) | (uint32_t)(red << 16U);
        else
            vga.dac.xlat32[index] = (uint32_t)(blue << 16U) | (uint32_t)(green << 8U) | (uint32_t)(red << 0U);
    }

    RENDER_SetPal( (Bit8u)index, red, green, blue );
}

void VGA_DAC_UpdateColor( Bitu index ) {
    VGA_DAC_SendColor( index, index );
}

void VGA_DAC_UpdateColorPalette() {
    for ( Bitu i = 0;i<256;i++) 
        VGA_DAC_UpdateColor( i );
}

void write_p3c6(Bitu port,Bitu val,Bitu iolen) {
    (void)iolen;//UNUSED
    (void)port;//UNUSED
    (void)val;
}

Bitu read_p3c6(Bitu port,Bitu iolen) {
    (void)iolen;//UNUSED
    (void)port;//UNUSED
    return 0xFF;
}

void write_p3c7(Bitu port,Bitu val,Bitu iolen) {
    (void)iolen;//UNUSED
    (void)port;//UNUSED
    vga.dac.pel_index=0;
    vga.dac.state=DAC_READ;
    vga.dac.read_index=(Bit8u)val;         /* NTS: Paradise SVGA behavior, read index = x, write index = x + 1 */
    vga.dac.write_index=(Bit8u)(val + 1);
}

Bitu read_p3c7(Bitu port,Bitu iolen) {
    (void)iolen;//UNUSED
    (void)port;//UNUSED
    if (vga.dac.state==DAC_READ) return 0x3;
    else return 0x0;
}

void write_p3c8(Bitu port,Bitu val,Bitu iolen) {
    (void)iolen;//UNUSED
    (void)port;//UNUSED
    vga.dac.pel_index=0;
    vga.dac.state=DAC_WRITE;
    vga.dac.write_index=(Bit8u)val;        /* NTS: Paradise SVGA behavior, this affects write index, but not read index */
    vga.dac.read_index = (Bit8u)(val - 1);
}

Bitu read_p3c8(Bitu port, Bitu iolen){
    (void)iolen;//UNUSED
    (void)port;//UNUSED
    return vga.dac.write_index;
}

static unsigned char tmp_dac[3] = {0,0,0};

void VGASetPalette(Bit8u index,Bit8u r,Bit8u g,Bit8u b) {
    vga.dac.rgb[index].red=r;
    vga.dac.rgb[index].green=g;
    vga.dac.rgb[index].blue=b;
}

void write_p3c9(Bitu port,Bitu val,Bitu iolen) {
}

Bitu read_p3c9(Bitu port,Bitu iolen) {
    return 0;
}

void VGA_DAC_CombineColor(Bit8u attr,Bit8u pal) {
    vga.dac.combine[attr] = pal;
    VGA_DAC_SendColor( attr, pal );
}

void VGA_DAC_SetEntry(Bitu entry,Bit8u red,Bit8u green,Bit8u blue) {
    //Should only be called in machine != vga
    vga.dac.rgb[entry].red=red;
    vga.dac.rgb[entry].green=green;
    vga.dac.rgb[entry].blue=blue;
    for (Bitu i=0;i<16;i++) 
        if (vga.dac.combine[i]==entry)
            VGA_DAC_SendColor( i, i );
}

void VGA_SetupDAC(void) {
    vga.dac.first_changed=256;
    vga.dac.bits=6;
    vga.dac.pel_index=0;
    vga.dac.state=DAC_READ;
    vga.dac.read_index=0;
    vga.dac.write_index=0;
    vga.dac.reg02=0;
    if (IS_VGA_ARCH) {
        /* Setup the DAC IO port Handlers */
        {
            IO_RegisterWriteHandler(0x3c6,write_p3c6,IO_MB);
            IO_RegisterReadHandler(0x3c6,read_p3c6,IO_MB);
            IO_RegisterWriteHandler(0x3c7,write_p3c7,IO_MB);
            IO_RegisterReadHandler(0x3c7,read_p3c7,IO_MB);
            IO_RegisterWriteHandler(0x3c8,write_p3c8,IO_MB);
            IO_RegisterReadHandler(0x3c8,read_p3c8,IO_MB);
            IO_RegisterWriteHandler(0x3c9,write_p3c9,IO_MB);
            IO_RegisterReadHandler(0x3c9,read_p3c9,IO_MB);
        }
    }
}

void VGA_UnsetupDAC(void) {
    IO_FreeWriteHandler(0x3c6,IO_MB);
    IO_FreeReadHandler(0x3c6,IO_MB);
    IO_FreeWriteHandler(0x3c7,IO_MB);
    IO_FreeReadHandler(0x3c7,IO_MB);
    IO_FreeWriteHandler(0x3c8,IO_MB);
    IO_FreeReadHandler(0x3c8,IO_MB);
    IO_FreeWriteHandler(0x3c9,IO_MB);
    IO_FreeReadHandler(0x3c9,IO_MB);
}

