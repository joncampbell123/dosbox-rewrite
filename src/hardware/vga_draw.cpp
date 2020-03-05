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


#include <string.h>
#include <math.h>
#include <stdio.h>
#include "dosbox.h"
#if defined (WIN32)
#include <d3d9.h>
#endif
#include "timer.h"
#include "setup.h"
#include "support.h"
#include "video.h"
#include "render.h"
#include "../gui/render_scalers.h"
#include "vga.h"
#include "pic.h"
#include "menu.h"
#include "timer.h"
#include "config.h"
#include "control.h"

bool mcga_double_scan = false;

const char* const mode_texts[M_MAX] = {
    "M_CGA2",           // 0
    "M_CGA4",
    "M_EGA",
    "M_VGA",
    "M_LIN4",
    "M_LIN8",           // 5
    "M_LIN15",
    "M_LIN16",
    "M_LIN24",
    "M_LIN32",
    "M_TEXT",           // 10
    "M_HERC_GFX",
    "M_HERC_TEXT",
    "M_CGA16",
    "M_TANDY2",
    "M_TANDY4",         // 15
    "M_TANDY16",
    "M_TANDY_TEXT",
    "M_AMSTRAD",
    "M_PC98",
    "M_FM_TOWNS",       // 20 STUB
    "M_PACKED4",
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

extern bool vga_3da_polled;
extern bool vga_page_flip_occurred;
extern bool vga_enable_hpel_effects;
extern unsigned int vga_display_start_hretrace;
extern bool ignore_vblank_wraparound;
extern bool vga_double_buffered_line_compare;

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

void VGA_MarkCaptureAcquired(void);
void VGA_MarkCaptureInProgress(bool en);
bool VGA_CaptureValidateCurrentFrame(void);
void VGA_CaptureStartNextFrame(void);
void VGA_MarkCaptureRetrace(void);
void VGA_CaptureMarkError(void);
bool VGA_IsCaptureEnabled(void);
bool VGA_IsCapturePending(void);
void VGA_CaptureWriteScanline(const uint8_t *raw);
void VGA_ProcessScanline(const uint8_t *raw);
bool VGA_IsCaptureInProgress(void);

struct vsync_state vsync;

float uservsyncjolt=0.0f;

VGA_Vsync vsyncmode_current = VS_Off;

void VGA_VsyncUpdateMode(VGA_Vsync vsyncmode) {
    vsyncmode_current = vsyncmode;

    mainMenu.get_item("vsync_off").check(vsyncmode_current == VS_Off).refresh_item(mainMenu);
    mainMenu.get_item("vsync_on").check(vsyncmode_current == VS_On).refresh_item(mainMenu);
    mainMenu.get_item("vsync_force").check(vsyncmode_current == VS_Force).refresh_item(mainMenu);
    mainMenu.get_item("vsync_host").check(vsyncmode_current == VS_Host).refresh_item(mainMenu);

    switch(vsyncmode) {
    case VS_Off:
        vsync.manual    = false;
        vsync.persistent= false;
        vsync.faithful  = false;
        break;
    case VS_On:
        vsync.manual    = true;
        vsync.persistent= true;
        vsync.faithful  = true;
        break;
    case VS_Force:
    case VS_Host:
        vsync.manual    = true;
        vsync.persistent= true;
        vsync.faithful  = false;
        break;
    default:
        LOG_MSG("VGA_VsyncUpdateMode: Invalid mode, using defaults.");
        vsync.manual    = false;
        vsync.persistent= false;
        vsync.faithful  = false;
        break;
    }
}

void VGA_TweakUserVsyncOffset(float val) { uservsyncjolt = val; }

void VGA_Draw2_Recompute_CRTC_MaskAdd(void) {
    if (IS_EGAVGA_ARCH) {
        /* mem masking can be generalized for ALL VGA/SVGA modes */
        size_t new_mask = vga.mem.memmask >> (2ul + vga.config.addr_shift);
        size_t new_add = 0;

        if (vga.config.compatible_chain4 || svgaCard == SVGA_None)
            new_mask &= 0xFFFFul >> vga.config.addr_shift; /* 64KB planar (256KB linear when byte mode) */

        /* CGA/Hercules compatible interlacing, unless SVGA graphics mode.
         * Note that ET4000 and ET3000 emulation will NOT set compatible_chain4 */
        if (vga.config.compatible_chain4 || svgaCard == SVGA_None) {
            /* MAP13: If zero, bit 13 is taken from bit 0 of row scan counter (CGA compatible) */
            /* MAP14: If zero, bit 14 is taken from bit 1 of row scan counter (Hercules compatible) */
            if ((vga.crtc.mode_control & 3u) != 3u) {
                const unsigned int shift = 13u - vga.config.addr_shift;
                const unsigned char mask = (vga.crtc.mode_control & 3u) ^ 3u;

                new_mask &= (size_t)(~(size_t(mask) << shift));
                new_add  += (size_t)(vga.draw_2[0].vert.current_char_pixel & mask) << shift;
            }
        }

        /* 4 bitplanes are represented in emulation as 32 bits per planar byte */
        vga.draw_2[0].draw_base = vga.mem.linear;
        vga.draw_2[0].crtc_mask = (unsigned int)new_mask;  // 8KB character clocks (16KB bytes)
        vga.draw_2[0].crtc_add = (unsigned int)new_add;
    }
    else {
        /* TODO: PCjr/Tandy 16-color extended modes */

        /* CGA/MCGA/PCJr/Tandy is emulated as 16 bits per character clock */
        /* PCJr uses system memory < 128KB for video memory.
         * Tandy has an alternate location as well. */
        vga.draw_2[0].draw_base = vga.mem.linear;

        if (vga.tandy.mode_control & 0x2) { /*graphics*/
            vga.draw_2[0].crtc_mask = 0xFFFu;  // 4KB character clocks (8KB bytes)
            vga.draw_2[0].crtc_add = (vga.draw_2[0].vert.current_char_pixel & 1u) << 12u;
        }
        else { /*text*/
            vga.draw_2[0].crtc_mask = 0x1FFFu;  // 8KB character clocks (16KB bytes)
            vga.draw_2[0].crtc_add = 0;
        }
    }
}

#if SDL_BYTEORDER == SDL_LIL_ENDIAN && defined(MACOSX) /* Mac OS X Intel builds use a weird RGBA order (alpha in the low 8 bits) */
static inline Bit32u guest_bgr_to_macosx_rgba(const Bit32u x) {
    /* guest: XRGB      X   R   G   B
     * host:  RGBX      B   G   R   X */
    return      ((x & 0x000000FFU) << 24U) +      /* BBxxxxxx */
                ((x & 0x0000FF00U) <<  8U) +      /* xxGGxxxx */
                ((x & 0x00FF0000U) >>  8U);       /* xxxxRRxx */
}
#endif

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
        vidmem += (uintptr_t)1U << (uintptr_t)vga.config.addr_shift;

        Bitu chr = pixels.b[0];
        Bitu attr = pixels.b[1];
        // the font pattern
        Bitu font = vga.draw.font_tables[(attr >> 3)&1][(chr<<5)+line];
        
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
            if ((font&0x2) && (vga.attr.mode_control&0x04) &&
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
        Bits attr_addr = ((Bits)vga.draw.cursor.address - (Bits)vidstart) >> (Bits)vga.config.addr_shift; /* <- FIXME: This right? */
        if (attr_addr >= 0 && attr_addr < (Bits)vga.draw.blocks) {
            Bitu index = (Bitu)attr_addr * (vga.draw.char9dot ? 9u : 8u);
            draw = (((templine_type_t*)TempLine) + index) + 16 - vga.draw.panning;
            
            Bitu foreground = vga.tandy.draw_base[(vga.draw.cursor.address<<2ul)+1] & 0xf;
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

template <const unsigned int card,typename templine_type_t,const unsigned int pixels> static inline void Alt_EGAVGA_TEXT_Combined_Draw_Line_RenderBMP(templine_type_t* &draw,unsigned int font,const unsigned char foreground,const unsigned char background) {
    const unsigned int fontmask = 1u << (pixels - 1u);

    for (unsigned int n = 0; n < pixels; n++) {
        if (card == MCH_VGA)
            *draw++ = vga.dac.xlat32[(font&fontmask)? foreground:background];

        font <<= 1;
    }
}

template <const unsigned int card,const unsigned int pixelsperchar> inline unsigned int Alt_VGA_Alpha8to9Expand(unsigned int font,const unsigned char chr) {
    if (pixelsperchar == 9) {
        font <<= 1; // 9 pixels

        // extend to the 9th pixel if needed
        if ((font&0x2) && (vga.attr.mode_control&0x04) && (chr>=0xc0) && (chr<=0xdf)) font |= 1;
    }

    return font;
}
 
template <const unsigned int card> static inline unsigned int Alt_EGAVGA_TEXT_Load_Font_Bitmap(const unsigned char chr,const unsigned char attr,const unsigned int line) {
    return vga.draw.font_tables[(attr >> 3)&1][(chr<<5)+line];
}

template <const unsigned int card> static inline void Alt_EGAVGA_TEXT_GetFGBG(unsigned char &foreground,unsigned char &background,const unsigned char attr,const unsigned char line,const bool in_cursor_row,const unsigned int addr) {
    // if blinking is enabled bit7 is not mapped to attributes
    background = attr >> 4u;
    if (vga.draw.blinking) background &= ~0x8u;

    // choose foreground color if blinking not set for this cell or blink on
    foreground = (vga.draw.blink || (!(attr&0x80))) ? (attr&0xf) : background;

    // underline: all foreground [freevga: 0x77, previous 0x7]
    if (GCC_UNLIKELY(((attr&0x77) == 0x01) && (vga.crtc.underline_location&0x1f)==line))
        background = foreground;

    // text cursor
    if (GCC_UNLIKELY(in_cursor_row) && addr == vga.config.cursor_start)
        background = foreground;
}

template <const unsigned int card> static inline bool Alt_EGAVGA_TEXT_In_Cursor_Row(const unsigned int line) {
    return
        ((vga.draw.cursor.count&0x8) && (line >= vga.draw.cursor.sline) &&
        (line <= vga.draw.cursor.eline) && vga.draw.cursor.enabled);
}

template <const unsigned int card,typename templine_type_t,const unsigned int pixelsperchar> static inline Bit8u* Alt_EGAVGA_TEXT_Combined_Draw_Line(void) {
    // keep it aligned:
    templine_type_t* draw = ((templine_type_t*)TempLine) + 16 - vga.draw.panning;
    Bitu blocks = vga.draw.blocks;
    if (vga.draw.panning) blocks++; // if the text is panned part of an 
                                    // additional character becomes visible

    const unsigned int line = vga.draw_2[0].vert.current_char_pixel & vga.draw_2[0].vert.char_pixel_mask;
    const bool in_cursor_row = Alt_EGAVGA_TEXT_In_Cursor_Row<card>(line);

    unsigned char foreground,background;

    while (blocks--) { // for each character in the line
        const unsigned int addr = vga.draw_2[0].crtc_addr_fetch_and_advance();
        VGA_Latch pixels(*vga.draw_2[0].drawptr<Bit32u>(addr << vga.config.addr_shift));

        const unsigned char chr = pixels.b[0];
        const unsigned char attr = pixels.b[1];

        // the font pattern
        unsigned int font = Alt_EGAVGA_TEXT_Load_Font_Bitmap<card>(chr,attr,line);
        Alt_EGAVGA_TEXT_GetFGBG<card>(foreground,background,attr,line,in_cursor_row,addr);

        // Draw it
        Alt_EGAVGA_TEXT_Combined_Draw_Line_RenderBMP<card,templine_type_t,pixelsperchar>
            (draw,Alt_VGA_Alpha8to9Expand<card,pixelsperchar>(font,chr),foreground,background);
    }

    return TempLine+(16*sizeof(templine_type_t));
}

template <const unsigned int card,typename templine_type_t> static inline Bit8u* Alt_EGAVGA_TEXT_Combined_Draw_Line(void) {
    if (vga.draw.char9dot)
        return Alt_EGAVGA_TEXT_Combined_Draw_Line<card,templine_type_t,9>();
    else
        return Alt_EGAVGA_TEXT_Combined_Draw_Line<card,templine_type_t,8>();
}

// combined 8/9-dot wide text mode 16bpp line drawing function
static Bit8u* Alt_VGA_TEXT_Xlat32_Draw_Line(Bitu /*vidstart*/, Bitu /*line*/) {
    return Alt_EGAVGA_TEXT_Combined_Draw_Line<MCH_VGA,Bit32u>();
}

static void VGA_ProcessSplit() {
    vga.draw.has_split = true;
    if (vga.attr.mode_control&0x20) {
        vga.draw.address=0;
        // reset panning to 0 here so we don't have to check for 
        // it in the character draw functions. It will be set back
        // to its proper value in v-retrace
        vga.draw.panning=0; 
    } else {
        // In text mode only the characters are shifted by panning, not the address;
        // this is done in the text line draw function. EGA/VGA planar is handled the same way.
        vga.draw.address = vga.draw.byte_panning_shift*vga.draw.bytes_skip;
        {
            switch (vga.mode) {
                case M_TEXT:
                    /* ignore */
                    break;
                default:
                    vga.draw.address += vga.draw.panning;
                    break;
            }
        }
    }
    vga.draw.address_line=0;
}

static Bit8u bg_color_index = 0; // screen-off black index
static Bit8u VGA_GetBlankedIndex() {
    if (vga.dac.xlat16[bg_color_index] != 0) {
        for(Bitu i = 0; i < 256; i++)
            if (vga.dac.xlat16[i] == 0) {
                bg_color_index = i;
                break;
            }
    }
    return bg_color_index;
}

/* this is now called PER LINE because most VGA cards do not double-buffer the value.
 * a few demos rely on line compare schenanigans to play with the raster, as does my own VGA test program --J.C. */
void VGA_Update_SplitLineCompare() {
    if (vga_alt_new_mode)
        vga.draw.split_line = vga.config.line_compare + 1;
    else
        vga.draw.split_line = (vga.config.line_compare + 1) / vga.draw.render_max;

    if (svgaCard==SVGA_S3Trio) {
        /* FIXME: Is this really necessary? Is this what S3 chipsets do?
         *        What is supposed to happen is that line_compare == 0 on normal VGA will cause the first
         *        scanline to repeat twice. Do S3 chipsets NOT reproduce that quirk?
         *
         *        The other theory I have about whoever wrote this code is that they wanted to multiply
         *        the scan line by two but had to compensate for the line_compare+1 assignment above.
         *        Rather than end up with a splitscreen too far down, they did that.
         *
         *        I think the proper code to put here would be:
         *
         *        if (vga.s3.reg_42 & 0x20) {
         *            vga.draw.split_line--;
         *            vga.draw.split_line *= 2;
         *            vga.draw.split_line++;
         *        }
         *
         *        Is that right?
         *
         *        This behavior is the reason for Issue #40 "Flash productions "monstra" extra white line at top of screen"
         *        because it causes line compare to miss and the first scanline of the white bar appears on scanline 2,
         *        when the demo coder obviously intended for line_compare == 0 to repeat the first scanline twice so that
         *        on line 3, it can begin updating line compare to continue repeating the first scanline.
         *
         * TODO: Pull out some S3 graphics cards and check split line behavior when line_compare == 0 */
        if (vga.config.line_compare==0) vga.draw.split_line=0;
        if (vga.s3.reg_42 & 0x20) { // interlaced mode
            vga.draw.split_line *= 2;
        }
    }
    vga.draw.split_line -= vga.draw.vblank_skip;
}

void VGA_Alt_CheckSplit(void) {
    if (vga.draw_2[0].raster_scanline == vga.draw.split_line) {
        /* VGA line compare. split line */
        vga.draw.has_split = true;
        if (vga.attr.mode_control&0x20) {
            vga.draw_2[0].vert.crtc_addr = 0;
            vga.draw.panning = 0;
        }
        else {
            vga.draw_2[0].vert.crtc_addr = 0 + vga.draw.bytes_skip;
        }
        vga.draw_2[0].vert.current_char_pixel = 0;
    }
}

void VGA_Alt_UpdateCRTCPixels(void) {
    if (IS_EGAVGA_ARCH) {
        vga.draw_2[0].horz.char_pixels = (vga.attr.mode_control & 4/*9 pixels/char*/) ? 9 : 8;
        vga.draw_2[0].vert.char_pixels = (vga.crtc.maximum_scan_line & vga.draw_2[0].vert.char_pixel_mask) + 1u;
    }
    else {
        vga.draw_2[0].horz.char_pixels = 8;
        vga.draw_2[0].vert.char_pixels = (vga.other.max_scanline & vga.draw_2[0].vert.char_pixel_mask) + 1u;
    }
}

void VGA_Alt_UpdateCRTCAdd(void) {
    if (IS_EGAVGA_ARCH) {
        vga.draw_2[0].horz.crtc_addr_add = 1;
        vga.draw_2[0].vert.crtc_addr_add = vga.crtc.offset * 2u;
    }
    else {
        vga.draw_2[0].horz.crtc_addr_add = 1;
        vga.draw_2[0].vert.crtc_addr_add = vga.other.hdend;
    }
}

void VGA_Alt_NextLogScanLine(void) {
    vga.draw_2[0].horz.current = 0;
    vga.draw_2[0].vert.current.pixels++;

    VGA_Alt_UpdateCRTCPixels();
    VGA_Alt_UpdateCRTCAdd();

    vga.draw_2[0].vert.current_char_pixel++;

    // TODO: DOSBox SVN and DOSBox-X main VGA emulation go to next line if row char line >= max.
    //       Real hardware suggests that it only happens when line == max, meaning if you reprogram
    //       the max scanline register in such a way the card misses it, it will count through all
    //       5 bits of the row counter before coming back around to match it again.
    //
    //       It might be a nice emulation option to select comparator function, whether >= or == .
    if ((vga.draw_2[0].vert.current_char_pixel & vga.draw_2[0].vert.char_pixel_mask) ==
        (vga.draw_2[0].vert.char_pixels        & vga.draw_2[0].vert.char_pixel_mask)) {
        vga.draw_2[0].vert.current_char_pixel = 0;
        vga.draw_2[0].vert.crtc_addr += vga.draw_2[0].vert.crtc_addr_add;
    }

    if (IS_EGAVGA_ARCH)
        VGA_Alt_CheckSplit();

    vga.draw_2[0].horz.crtc_addr = vga.draw_2[0].vert.crtc_addr;
    vga.draw_2[0].horz.current_char_pixel = 0;

    VGA_Draw2_Recompute_CRTC_MaskAdd();
}

void VGA_Alt_NextScanLine(void) {
    /* track actual raster line to output */
    vga.draw_2[0].raster_scanline++;

    /* do not advance the vertical count nor carry out new scanline functions
     * if doublescan is set and this is the EVEN scan line */
    if (vga.draw_2[0].doublescan_count >= vga.draw_2[0].doublescan_max) {
        vga.draw_2[0].doublescan_count = 0;
        VGA_Alt_NextLogScanLine();
    }
    else {
        vga.draw_2[0].doublescan_count++;

        if (IS_EGAVGA_ARCH)
            VGA_Alt_CheckSplit();

        vga.draw_2[0].horz.crtc_addr = vga.draw_2[0].vert.crtc_addr;
        vga.draw_2[0].horz.current_char_pixel = 0;

        VGA_Draw2_Recompute_CRTC_MaskAdd();
    }
}

static void VGA_DrawSingleLine(Bitu /*blah*/) {
    unsigned int lines = 0;
    bool skiprender;

again:
    if (vga.draw.render_step == 0)
        skiprender = false;
    else
        skiprender = true;

    if ((++vga.draw.render_step) >= vga.draw.render_max)
        vga.draw.render_step = 0;

    if (!skiprender) {
        if (GCC_UNLIKELY(vga.attr.disabled)) {
            switch(machine) {
                case MCH_VGA:
                    // DoWhackaDo, Alien Carnage, TV sports Football
                    // when disabled by attribute index bit 5:
                    //  ET3000, ET4000, Paradise display the border color
                    //  S3 displays the content of the currently selected attribute register
                    // when disabled by sequencer the screen is black "257th color"

                    // the DAC table may not match the bits of the overscan register
                    // so use black for this case too...
                    //if (vga.attr.disabled& 2) {
                    VGA_GetBlankedIndex();
                    //} else 
                    //    bg_color_index = vga.attr.overscan_color;
                    break;
                default:
                    bg_color_index = 0;
                    break;
            }
            if (vga.draw.bpp==8) {
                memset(TempLine, bg_color_index, sizeof(TempLine));
            } else if (vga.draw.bpp==16) {
                Bit16u* wptr = (Bit16u*) TempLine;
                Bit16u value = vga.dac.xlat16[bg_color_index];
                for (Bitu i = 0; i < sizeof(TempLine)/2; i++) {
                    wptr[i] = value;
                }
            } else if (vga.draw.bpp==32) {
                Bit32u* wptr = (Bit32u*) TempLine;
                Bit32u value = vga.dac.xlat32[bg_color_index];
                for (Bitu i = 0; i < sizeof(TempLine)/4; i++) {
                    wptr[i] = value;
                }
            }

            if (vga_page_flip_occurred) {
                memxor(TempLine,0xFF,vga.draw.width*(vga.draw.bpp>>3));
                vga_page_flip_occurred = false;
            }
            if (vga_3da_polled) {
                if (vga.draw.bpp==32)
                    memxor_greendotted_32bpp((uint32_t*)TempLine,(vga.draw.width>>1)*(vga.draw.bpp>>3),vga.draw.lines_done);
                else
                    memxor_greendotted_16bpp((uint16_t*)TempLine,(vga.draw.width>>1)*(vga.draw.bpp>>3),vga.draw.lines_done);
                vga_3da_polled = false;
            }
            RENDER_DrawLine(TempLine);
        } else {
            Bit8u * data=VGA_DrawLine( vga.draw.address, vga.draw.address_line );
            if (vga_page_flip_occurred) {
                memxor(data,0xFF,vga.draw.width*(vga.draw.bpp>>3));
                vga_page_flip_occurred = false;
            }
            if (vga_3da_polled) {
                if (vga.draw.bpp==32)
                    memxor_greendotted_32bpp((uint32_t*)data,(vga.draw.width>>1)*(vga.draw.bpp>>3),vga.draw.lines_done);
                else
                    memxor_greendotted_16bpp((uint16_t*)data,(vga.draw.width>>1)*(vga.draw.bpp>>3),vga.draw.lines_done);
                vga_3da_polled = false;
            }

            if (VGA_IsCaptureEnabled())
                VGA_ProcessScanline(data);

            RENDER_DrawLine(data);
        }
    }

    /* parallel system */
    if (vga_alt_new_mode)
        VGA_Alt_NextScanLine();

    vga.draw.address_line++;
    if (vga.draw.address_line>=vga.draw.address_line_total) {
        vga.draw.address_line=0;
        vga.draw.address+=vga.draw.address_add;
    }

    if (!skiprender) {
        vga.draw.lines_done++;
        if (vga.draw.split_line==vga.draw.lines_done && !vga_alt_new_mode) VGA_ProcessSplit();
    }

    if (mcga_double_scan) {
        if (vga.draw.lines_done < vga.draw.lines_total) {
            if (++lines < 2)
                goto again;
        }
    }

    if (vga.draw.lines_done < vga.draw.lines_total) {
        PIC_AddEvent(VGA_DrawSingleLine,(float)vga.draw.delay.singleline_delay);
    } else {
        vga_mode_frames_since_time_base++;

        if (VGA_IsCaptureEnabled())
            VGA_ProcessScanline(NULL);

        RENDER_EndUpdate(false);
    }

    /* some VGA cards (ATI chipsets especially) do not double-buffer the
     * horizontal panning register. some DOS demos take advantage of that
     * to make the picture "waver".
     *
     * We stop doing this though if the attribute controller is setup to set hpel=0 at splitscreen. */
    if (IS_VGA_ARCH && vga_enable_hpel_effects) {
        /* Attribute Mode Controller: If bit 5 (Pixel Panning Mode) is set, then upon line compare the bottom portion is displayed as if Pixel Shift Count and Byte Panning are set to 0.
         * This ensures some demos like Future Crew "Yo" display correctly instead of the bottom non-scroller half jittering because the top half is scrolling. */
        if (vga.draw.has_split && (vga.attr.mode_control&0x20))
            vga.draw.panning = 0;
        else
            vga.draw.panning = vga.config.pel_panning;
    }

    if (IS_EGAVGA_ARCH && !vga_double_buffered_line_compare) VGA_Update_SplitLineCompare();
}

static void VGA_DrawEGASingleLine(Bitu /*blah*/) {
    bool skiprender;

    if (vga.draw.render_step == 0)
        skiprender = false;
    else
        skiprender = true;

    if ((++vga.draw.render_step) >= vga.draw.render_max)
        vga.draw.render_step = 0;

    if (!skiprender) {
        if (GCC_UNLIKELY(vga.attr.disabled)) {
            memset(TempLine, 0, sizeof(TempLine));
            RENDER_DrawLine(TempLine);
        } else {
            Bitu address = vga.draw.address;
            {
                switch (vga.mode) {
                    case M_TEXT:
                        /* ignore */
                        break;
                    default:
                        vga.draw.address += vga.draw.panning;
                        break;
                }
            }
            Bit8u * data=VGA_DrawLine(address, vga.draw.address_line ); 

            if (VGA_IsCaptureEnabled())
                VGA_ProcessScanline(data);

            RENDER_DrawLine(data);
        }
    }

    /* parallel system */
    if (vga_alt_new_mode)
        VGA_Alt_NextScanLine();

    vga.draw.address_line++;
    if (vga.draw.address_line>=vga.draw.address_line_total) {
        vga.draw.address_line=0;
        vga.draw.address+=vga.draw.address_add;
    }

    if (!skiprender) {
        vga.draw.lines_done++;
        if (vga.draw.split_line==vga.draw.lines_done && !vga_alt_new_mode) VGA_ProcessSplit();
    }

    if (vga.draw.lines_done < vga.draw.lines_total) {
        PIC_AddEvent(VGA_DrawEGASingleLine,(float)vga.draw.delay.singleline_delay);
    } else {
        vga_mode_frames_since_time_base++;

        if (VGA_IsCaptureEnabled())
            VGA_ProcessScanline(NULL);

        RENDER_EndUpdate(false);
    }
}

void VGA_SetBlinking(Bitu enabled) {
    Bitu b;
    LOG(LOG_VGA,LOG_NORMAL)("Blinking %d",(int)enabled);
    if (enabled) {
        b=0;vga.draw.blinking=1; //used to -1 but blinking is unsigned
        vga.attr.mode_control|=0x08;
        vga.tandy.mode_control|=0x20;
    } else {
        b=8;vga.draw.blinking=0;
        vga.attr.mode_control&=~0x08;
        vga.tandy.mode_control&=~0x20;
    }
    for (Bitu i=0;i<8;i++) TXT_BG_Table[i+8]=(b+i) | ((b+i) << 8)| ((b+i) <<16) | ((b+i) << 24);
}

static void VGA_VertInterrupt(Bitu /*val*/) {
    if ((!vga.draw.vret_triggered) && ((vga.crtc.vertical_retrace_end&0x30)==0x10)) {
        vga.draw.vret_triggered=true;
    }
}

static void VGA_Other_VertInterrupt(Bitu val) {
    if (val) PIC_ActivateIRQ(5);
    else PIC_DeActivateIRQ(5);
}

static void VGA_DisplayStartLatch(Bitu /*val*/) {
    /* hretrace fx support: store the hretrace value at start of picture so we have
     * a point of reference how far to displace the scanline when wavy effects are
     * made */
    vga_display_start_hretrace = vga.crtc.start_horizontal_retrace;
    vga.config.real_start=vga.config.display_start & vga.mem.memmask;
    vga.draw.bytes_skip = vga.config.bytes_skip;
}
 
static void VGA_PanningLatch(Bitu /*val*/) {
    vga.draw.panning = vga.config.pel_panning;
}

extern SDL_Rect                            vga_capture_current_rect;
extern uint32_t                            vga_capture_current_address;
extern uint32_t                            vga_capture_write_address;

void VGA_ProcessScanline(const uint8_t *raw) {
    if (raw == NULL) { // end of the frame
        if (VGA_IsCaptureInProgress()) {
            VGA_MarkCaptureInProgress(false);
            VGA_MarkCaptureAcquired();
        }

        return;
    }

    // assume VGA_IsCaptureEnabled()
    if (!VGA_IsCaptureInProgress()) {
        if (vga_capture_current_address != (uint32_t)0 && (unsigned int)vga.draw.lines_done == (unsigned int)vga_capture_current_rect.y) { // start
            VGA_MarkCaptureInProgress(true);
            VGA_CaptureWriteScanline(raw);
        }
    }
    else {
        if ((unsigned int)vga.draw.lines_done == ((unsigned int)vga_capture_current_rect.y+vga_capture_current_rect.h)) { // first line past end
            VGA_MarkCaptureInProgress(false);
            VGA_MarkCaptureAcquired();
        }
        else {
            VGA_CaptureWriteScanline(raw);
        }
    }
}

extern uint32_t GFX_Rmask;
extern unsigned char GFX_Rshift;
extern uint32_t GFX_Gmask;
extern unsigned char GFX_Gshift;
extern uint32_t GFX_Bmask;
extern unsigned char GFX_Bshift;
extern uint32_t GFX_Amask;
extern unsigned char GFX_Ashift;
extern unsigned char GFX_bpp;
extern uint32_t vga_capture_stride;

template <const unsigned int bpp,typename BPPT> uint32_t VGA_CaptureConvertPixel(const BPPT raw) {
    unsigned char r,g,b;

    /* FIXME: Someday this code will have to deal with 10:10:10 32-bit RGB.
     * Also the 32bpp case shows how hacky this codebase is with regard to 32bpp color order support */
    if (bpp == 32) {
        if (GFX_bpp >= 24) {
            r = ((uint32_t)raw & (uint32_t)GFX_Rmask) >> (uint32_t)GFX_Rshift;
            g = ((uint32_t)raw & (uint32_t)GFX_Gmask) >> (uint32_t)GFX_Gshift;
            b = ((uint32_t)raw & (uint32_t)GFX_Bmask) >> (uint32_t)GFX_Bshift;
        }
        else {
            // hack alt, see vga_dac.cpp
            return raw;
        }
    }
    else if (bpp == 16) {
        /* 5:5:5 or 5:6:5 */
        r = ((uint16_t)raw & (uint16_t)GFX_Rmask) >> (uint16_t)GFX_Rshift;
        g = ((uint16_t)raw & (uint16_t)GFX_Gmask) >> (uint16_t)GFX_Gshift;
        b = ((uint16_t)raw & (uint16_t)GFX_Bmask) >> (uint16_t)GFX_Bshift;

        r <<= 3;
        g <<= (GFX_Gmask == 0x3F ? 2/*5:6:5*/ : 3/*5:5:5*/);
        b <<= 3;
    }
    else if (bpp == 8) {
        r = render.pal.rgb[raw].red;
        g = render.pal.rgb[raw].green;
        b = render.pal.rgb[raw].blue;
    }
    else {
        r = g = b = 0;
    }

    /* XRGB */
    return  ((uint32_t)r << (uint32_t)16ul) +
            ((uint32_t)g << (uint32_t) 8ul) +
            ((uint32_t)b                  );
}

template <const unsigned int bpp,typename BPPT> void VGA_CaptureWriteScanlineChecked(const BPPT *raw) {
    raw += vga_capture_current_rect.x;

    /* output is ALWAYS 32-bit XRGB */
    for (unsigned int i=0;(int)i < vga_capture_current_rect.w;i++)
        phys_writed(vga_capture_write_address+(i*4),
            VGA_CaptureConvertPixel<bpp,BPPT>(raw[i]));

    vga_capture_write_address += vga_capture_stride;
}

void VGA_CaptureWriteScanline(const uint8_t *raw) {
    // NTS: phys_writew() will cause a segfault if the address is beyond the end of memory,
    //      because it computes MemBase+addr
    PhysPt MemMax = (PhysPt)MEM_TotalPages() * (PhysPt)4096ul;

    if (vga_capture_write_address != (uint32_t)0 &&
        vga_capture_write_address < 0xFFFF0000ul &&
        (vga_capture_write_address + (vga_capture_current_rect.w*4ul)) <= MemMax) {
        switch (vga.draw.bpp) {
            case 32:    VGA_CaptureWriteScanlineChecked<32>((uint32_t*)raw); break;
            case 16:    VGA_CaptureWriteScanlineChecked<16>((uint16_t*)raw); break;
            case 15:    VGA_CaptureWriteScanlineChecked<16>((uint16_t*)raw); break;
            case 8:     VGA_CaptureWriteScanlineChecked< 8>((uint8_t *)raw); break;
        }
    }
    else {
        VGA_CaptureMarkError();
    }
}

static void VGA_VerticalTimer(Bitu /*val*/) {
    double current_time = PIC_GetCurrentEventTime();

    if (VGA_IsCaptureEnabled()) {
        if (VGA_IsCaptureInProgress()) {
            VGA_MarkCaptureInProgress(false);
            VGA_MarkCaptureAcquired();
        }

        VGA_MarkCaptureRetrace();
        VGA_CaptureStartNextFrame();
        if (!VGA_CaptureValidateCurrentFrame())
            VGA_CaptureMarkError();
    }

    vga.draw.delay.framestart = current_time; /* FIXME: Anyone use this?? If not, remove it */
    vga_page_flip_occurred = false;
    vga.draw.has_split = false;
    vga_3da_polled = false;

    // FIXME: While this code is quite good at keeping time, I'm seeing drift "reset" back to
    //        14-30ms every video mode change. Is our INT 10h code that slow?
    /* compensate for floating point drift, make sure we're keeping the frame rate.
     * be very gentle about it. generally the drift is very small, and large adjustments can cause
     * DOS games dependent on vsync to fail/hang. */
    double shouldbe = (((double)vga_mode_frames_since_time_base * 1000.0) / vga_fps) + vga_mode_time_base;
    double vsync_err = shouldbe - current_time; /* < 0 too slow     > 0 too fast */
    double vsync_adj = vsync_err * 0.25;
    if (vsync_adj < -0.1) vsync_adj = -0.1;
    else if (vsync_adj > 0.1) vsync_adj = 0.1;

//  LOG_MSG("Vsync err %.6fms adj=%.6fms",vsync_err,vsync_adj);

    float vsynctimerval;
    float vdisplayendtimerval;
    if( vsync.manual ) {
        static float hack_memory = 0.0f;
        if( hack_memory > 0.0f ) {
            uservsyncjolt+=hack_memory;
            hack_memory = 0.0f;
        }

        float faithful_framerate_adjustment_delay = 0.0f;
        if( vsync.faithful ) {
            const float gfxmode_vsyncrate   = 1000.0f/vga.draw.delay.vtotal;
            const float user_vsyncrate      = 1000.0f/vsync.period;
            const float framerate_diff      = user_vsyncrate - gfxmode_vsyncrate;
            if( framerate_diff >= 0 ) {
                static float counter = 0.0f;
                // User vsync rate is greater than the target vsync rate
                const float adjustment_deadline = gfxmode_vsyncrate / framerate_diff;
                counter += 1.0f;
                if(counter >= adjustment_deadline) {
                    // double vsync duration this frame to resynchronize with the target vsync timing
                    faithful_framerate_adjustment_delay = vsync.period;
                    counter -= adjustment_deadline;
                }
            } else {
                // User vsync rate is less than the target vsync rate

                // I don't have a good solution for this right now.
                // I also don't have access to a 60Hz display for proper testing.
                // Making an instant vsync is more difficult than making a long vsync.. because
                // the emulated app's retrace loop must be able to detect that the retrace has both
                // begun and ended.  So it's not as easy as adjusting timer durations.
                // I think adding a hack to cause port 3da's code to temporarily force the
                // vertical retrace bit to cycle could work.. Even then, it's possible that
                // some shearing could be seen when the app draws two new frames during a single
                // host refresh.
                // Anyway, this could be worth dealing with for console ports since they'll be
                // running at either 60 or 50Hz (below 70Hz).
                /*
                const float adjustment_deadline = -(gfxmode_vsyncrate / framerate_diff);
                counter += 1.0f;
                if(counter >= adjustment_deadline) {
                    // nullify vsync duration this frame to resynchronize with the target vsync timing
                    // TODO(AUG): proper low user vsync rate synchronization
                    faithful_framerate_adjustment_delay = -uservsyncperiod + 1.2f;
                    vsync_hackval = 10;
                    hack_memory = -1.2f;
                    counter -= adjustment_deadline;
                }
                */
            }
        }

        const Bitu persistent_sync_update_interval = 100;
        static Bitu persistent_sync_counter = persistent_sync_update_interval;
        Bitu current_tick = GetTicks();
        static Bitu jolt_tick = 0;
        if( uservsyncjolt > 0.0f ) {
            jolt_tick = (Bitu)current_tick;

            // set the update counter to a low value so that the user will almost
            // immediately see the effects of an auto-correction.  This gives the
            // user a chance to compensate for it.
            persistent_sync_counter = 50;
        }

        float real_diff = 0.0f;
        if( vsync.persistent ) {
            if( persistent_sync_counter == 0 ) {
                float ticks_since_jolt = (signed long)current_tick - (signed long)jolt_tick;
                double num_host_syncs_in_those_ticks = floor(ticks_since_jolt / vsync.period);
                float diff_thing = ticks_since_jolt - (num_host_syncs_in_those_ticks * (double)vsync.period);

                if( diff_thing > (vsync.period / 2.0f) ) real_diff = diff_thing - vsync.period;
                else real_diff = diff_thing;

//              LOG_MSG("diff is %f",real_diff);

                if( ((real_diff > 0.0f) && (real_diff < 1.5f)) || ((real_diff < 0.0f) && (real_diff > -1.5f)) )
                    real_diff = 0.0f;

                persistent_sync_counter = persistent_sync_update_interval;
            } else --persistent_sync_counter;
        }

//      vsynctimerval       = uservsyncperiod + faithful_framerate_adjustment_delay + uservsyncjolt;
        vsynctimerval       = vsync.period - (real_diff/1.0f);  // formerly /2.0f
        vsynctimerval       += faithful_framerate_adjustment_delay + uservsyncjolt;

        // be sure to provide delay between end of one refresh, and start of the next
//      vdisplayendtimerval = vsynctimerval - 1.18f;

        // be sure to provide delay between end of one refresh, and start of the next
        vdisplayendtimerval = vsynctimerval - (vga.draw.delay.vtotal - vga.draw.delay.vrstart);

        // in case some calculations above cause this.  this really shouldn't happen though.
        if( vdisplayendtimerval < 0.0f ) vdisplayendtimerval = 0.0f;

        uservsyncjolt = 0.0f;
    } else {
        // Standard vsync behaviour
        vsynctimerval       = (float)vga.draw.delay.vtotal;
        vdisplayendtimerval = (float)vga.draw.delay.vrstart;
    }

    {
        double fv;

        fv = vsynctimerval + vsync_adj;
        if (fv < 1) fv = 1;
        PIC_AddEvent(VGA_VerticalTimer,fv);

        fv = vdisplayendtimerval + vsync_adj;
        if (fv < 1) fv = 1;
        PIC_AddEvent(VGA_DisplayStartLatch,fv);
    }
    
    switch(machine) {
    case MCH_VGA:
        PIC_AddEvent(VGA_DisplayStartLatch, (float)vga.draw.delay.vrstart);
        PIC_AddEvent(VGA_PanningLatch, (float)vga.draw.delay.vrend);
        // EGA: 82c435 datasheet: interrupt happens at display end
        // VGA: checked with scope; however disabled by default by jumper on VGA boards
        // add a little amount of time to make sure the last drawpart has already fired
        PIC_AddEvent(VGA_VertInterrupt,(float)(vga.draw.delay.vdend + 0.005));
        break;
    default:
        //E_Exit("This new machine needs implementation in VGA_VerticalTimer too.");
        PIC_AddEvent(VGA_DisplayStartLatch, (float)vga.draw.delay.vrstart);
        PIC_AddEvent(VGA_PanningLatch, (float)vga.draw.delay.vrend);
        PIC_AddEvent(VGA_VertInterrupt,(float)(vga.draw.delay.vdend + 0.005));
        break;
    }
    // for same blinking frequency with higher frameskip
    vga.draw.cursor.count++;

    //Check if we can actually render, else skip the rest
    if (vga.draw.vga_override || !RENDER_StartUpdate()) return;

    vga.draw.address_line = vga.config.hlines_skip;
    if (IS_EGAVGA_ARCH) VGA_Update_SplitLineCompare();
    vga.draw.address = vga.config.real_start;
    vga.draw.byte_panning_shift = 0;

    /* parallel system */
    if (vga_alt_new_mode) {
        /* the doublescan bit can be changed between frames, it can happen!
         *
         * "Show" by Majic 12: Two parts use 320x200 16-color planar mode, which by default
         *                     is programmed by INT 10h to use the doublescan bit and max scanline == 0.
         *                     These two parts then reprogram that register to turn off doublescan
         *                     and set max scanline == 1. This compensation is needed for those two
         *                     parts to show correctly. */
        if (IS_VGA_ARCH && (vga.crtc.maximum_scan_line & 0x80))
            vga.draw_2[0].doublescan_max = 1;
        else
            vga.draw_2[0].doublescan_max = 0;

        vga.draw_2[0].raster_scanline = 0;
        vga.draw_2[0].doublescan_count = 0;

        if (IS_EGAVGA_ARCH) {
            vga.draw_2[0].horz.current = 0;
            vga.draw_2[0].vert.current = 0;

            vga.draw_2[0].horz.current_char_pixel = 0;
            vga.draw_2[0].vert.current_char_pixel = vga.config.hlines_skip;

            VGA_Alt_UpdateCRTCPixels();
            VGA_Alt_UpdateCRTCAdd();

            vga.draw_2[0].vert.crtc_addr = vga.config.real_start + vga.draw.bytes_skip;
            vga.draw_2[0].horz.crtc_addr = vga.draw_2[0].vert.crtc_addr;

            VGA_Draw2_Recompute_CRTC_MaskAdd();
            VGA_Alt_CheckSplit();
        }
        else {
            vga.draw_2[0].horz.current = 0;
            vga.draw_2[0].vert.current = 0;

            vga.draw_2[0].horz.current_char_pixel = 0;
            vga.draw_2[0].vert.current_char_pixel = 0;

            VGA_Alt_UpdateCRTCPixels();
            VGA_Alt_UpdateCRTCAdd();

            vga.draw_2[0].vert.crtc_addr = vga.config.real_start;
            vga.draw_2[0].horz.crtc_addr = vga.draw_2[0].vert.crtc_addr;

            VGA_Draw2_Recompute_CRTC_MaskAdd();
        }
    }

    switch (vga.mode) {
    case M_TEXT:
        vga.draw.byte_panning_shift = 2;
        vga.draw.address += vga.draw.bytes_skip;
        if (IS_EGAVGA_ARCH) {
            if (vga.config.compatible_chain4 || svgaCard == SVGA_None)
                vga.draw.linear_mask = vga.mem.memmask & 0x3ffff;
            else
                vga.draw.linear_mask = vga.mem.memmask; // SVGA text mode
        }
        else vga.draw.linear_mask = 0x3fff; // CGA, Tandy 4 pages
        if (IS_EGAVGA_ARCH)
            vga.draw.cursor.address=vga.config.cursor_start<<vga.config.addr_shift;
        else
            vga.draw.cursor.address=vga.config.cursor_start*2;
        vga.draw.address *= 2;

        /* check for blinking and blinking change delay */
        FontMask[1]=(vga.draw.blinking & (unsigned int)(vga.draw.cursor.count >> 4u)) ?
            0 : 0xffffffff;
        /* if blinking is enabled, 'blink' will toggle between true
         * and false. Otherwise it's true */
        vga.draw.blink = ((vga.draw.blinking & (unsigned int)(vga.draw.cursor.count >> 4u))
            || !vga.draw.blinking) ? true:false;
        break;
    default:
        break;
    }

    if (IS_EGAVGA_ARCH)
        vga.draw.planar_mask = vga.draw.linear_mask >> 2;
    else
        vga.draw.planar_mask = vga.draw.linear_mask >> 1;

    // check if some lines at the top off the screen are blanked
    float draw_skip = 0.0;
    if (GCC_UNLIKELY(vga.draw.vblank_skip > 0)) {
        draw_skip = (float)(vga.draw.delay.htotal * vga.draw.vblank_skip);
        vga.draw.address_line += (vga.draw.vblank_skip % vga.draw.address_line_total);
        vga.draw.address += vga.draw.address_add * (vga.draw.vblank_skip / vga.draw.address_line_total);
    }

    /* do VGA split now if line compare <= 0. NTS: vga.draw.split_line is defined as Bitu (unsigned integer) so we need the typecast. */
    if (GCC_UNLIKELY((Bits)vga.draw.split_line <= 0) && !vga_alt_new_mode) {
        VGA_ProcessSplit();

        /* if vblank_skip != 0, line compare can become a negative value! Fixes "Warlock" 1992 demo by Warlock */
        if (GCC_UNLIKELY((Bits)vga.draw.split_line < 0)) {
            vga.draw.address_line += (Bitu)(-((Bits)vga.draw.split_line) % (Bits)vga.draw.address_line_total);
            vga.draw.address += vga.draw.address_add * (Bitu)(-((Bits)vga.draw.split_line) / (Bits)vga.draw.address_line_total);
        }
    }

    // add the draw event
    switch (vga.draw.mode) {
    case DRAWLINE:
    case EGALINE:
        if (GCC_UNLIKELY(vga.draw.lines_done < vga.draw.lines_total)) {
            LOG(LOG_VGAMISC,LOG_NORMAL)( "Lines left: %d", 
                (int)(vga.draw.lines_total-vga.draw.lines_done));
            if (vga.draw.mode==EGALINE) PIC_RemoveEvents(VGA_DrawEGASingleLine);
            else PIC_RemoveEvents(VGA_DrawSingleLine);
            vga_mode_frames_since_time_base++;

            if (VGA_IsCaptureEnabled())
                VGA_ProcessScanline(NULL);

            RENDER_EndUpdate(true);
        }
        vga.draw.lines_done = 0;
        if (vga.draw.mode==EGALINE)
            PIC_AddEvent(VGA_DrawEGASingleLine,(float)(vga.draw.delay.htotal/4.0 + draw_skip));
        else PIC_AddEvent(VGA_DrawSingleLine,(float)(vga.draw.delay.htotal/4.0 + draw_skip));
        break;
    }
}

void VGA_CheckScanLength(void) {
    switch (vga.mode) {
    case M_TEXT:
        if (IS_EGAVGA_ARCH)
            vga.draw.address_add=vga.config.scan_len*(unsigned int)(2u<<vga.config.addr_shift);
        else
            vga.draw.address_add=vga.draw.blocks;
        break;
    default:
        vga.draw.address_add=vga.draw.blocks*8;
        break;
    }
}

void VGA_ActivateHardwareCursor(void) {
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
    Bitu hbend_mask, vbend_mask;
    Bitu vblank_skip;

    if (IS_EGAVGA_ARCH) {
        htotal = vga.crtc.horizontal_total;
        hdend = vga.crtc.horizontal_display_end;
        hbend = vga.crtc.end_horizontal_blanking&0x1F;
        hbstart = vga.crtc.start_horizontal_blanking;
        hrstart = vga.crtc.start_horizontal_retrace;

        vtotal= vga.crtc.vertical_total | ((vga.crtc.overflow & 1u) << 8u);
        vdend = vga.crtc.vertical_display_end | ((vga.crtc.overflow & 2u) << 7u);
        vbstart = vga.crtc.start_vertical_blanking | ((vga.crtc.overflow & 0x08u) << 5u);
        vrstart = vga.crtc.vertical_retrace_start + ((vga.crtc.overflow & 0x04u) << 6u);
        
        if (IS_VGA_ARCH) {
            // additional bits only present on vga cards
            htotal |= (vga.s3.ex_hor_overflow & 0x1u) << 8u;
            htotal += 3u;
            hdend |= (vga.s3.ex_hor_overflow & 0x2u) << 7u;
            hbend |= (vga.crtc.end_horizontal_retrace&0x80u) >> 2u;
            hbstart |= (vga.s3.ex_hor_overflow & 0x4u) << 6u;
            hrstart |= (vga.s3.ex_hor_overflow & 0x10u) << 4u;
            hbend_mask = 0x3fu;
            
            vtotal |= (vga.crtc.overflow & 0x20u) << 4u;
            vtotal |= (vga.s3.ex_ver_overflow & 0x1u) << 10u;
            vdend |= (vga.crtc.overflow & 0x40u) << 3u; 
            vdend |= (vga.s3.ex_ver_overflow & 0x2u) << 9u;
            vbstart |= (vga.crtc.maximum_scan_line & 0x20u) << 4u;
            vbstart |= (vga.s3.ex_ver_overflow & 0x4u) << 8u;
            vrstart |= ((vga.crtc.overflow & 0x80u) << 2u);
            vrstart |= (vga.s3.ex_ver_overflow & 0x10u) << 6u;
            vbend_mask = 0xffu;
        } else { // EGA
            hbend_mask = 0x1fu;
            vbend_mask = 0x1fu;
        }
        htotal += 2;
        vtotal += 2;
        hdend += 1;
        vdend += 1;

        // horitzontal blanking
        if (hbend <= (hbstart & hbend_mask)) hbend += hbend_mask + 1;
        hbend += hbstart - (hbstart & hbend_mask);
        
        // horizontal retrace
        hrend = vga.crtc.end_horizontal_retrace & 0x1f;
        if (hrend <= (hrstart&0x1f)) hrend += 32;
        hrend += hrstart - (hrstart&0x1f);
        if (hrend > hbend) hrend = hbend; // S3 BIOS (???)
        
        // vertical retrace
        vrend = vga.crtc.vertical_retrace_end & 0xf;
        if (vrend <= (vrstart&0xf)) vrend += 16;
        vrend += vrstart - (vrstart&0xf);

        // vertical blanking
        vbend = vga.crtc.end_vertical_blanking & vbend_mask;
        if (vbstart != 0) {
        // Special case vbstart==0:
        // Most graphics cards agree that lines zero to vbend are
        // blanked. ET4000 doesn't blank at all if vbstart==vbend.
        // ET3000 blanks lines 1 to vbend (255/6 lines).
            vbstart += 1;
            if (vbend <= (vbstart & vbend_mask)) vbend += vbend_mask + 1;
            vbend += vbstart - (vbstart & vbend_mask);
        }
        vbend++;

        // TODO: Found a monitor document that lists two different scan rates for PC-98:
        //
        //       640x400  25.175MHz dot clock  70.15Hz refresh  31.5KHz horizontal refresh (basically, VGA)
        //       640x400  21.05MHz dot clock   56.42Hz refresh  24.83Hz horizontal refresh (original spec?)

        if (svga.get_clock) {
            oscclock = svga.get_clock();
        } else {
            switch ((vga.misc_output >> 2) & 3) {
            case 0: 
                oscclock = 25175000;
                break;
            case 1:
            default:
                oscclock = 28322000;
                break;
            }
        }

        /* Check for 8 or 9 character clock mode */
        if (vga.seq.clocking_mode & 1 ) clock = oscclock/8; else clock = oscclock/9;
        /* Check for pixel doubling, master clock/2 */
        /* NTS: VGA 256-color mode has a dot clock NOT divided by two, because real hardware reveals
         *      that internally the card processes pixels 4 bits per cycle through a 8-bit shift
         *      register and a bit is set to latch the 8-bit value out to the DAC every other cycle. */
        if (vga.seq.clocking_mode & 0x8) {
            clock /=2;
            oscclock /= 2;
        }

        if (svgaCard==SVGA_S3Trio) {
            // support for interlacing used by the S3 BIOS and possibly other drivers
            if (vga.s3.reg_42 & 0x20) {
                vtotal *= 2;    vdend *= 2;
                vbstart *= 2;   vbend *= 2;
                vrstart *= 2;   vrend *= 2;
                //clock /= 2;
        }
        }
    } else {
        // not EGAVGA_ARCH
        vga.draw.split_line = 0x10000;  // don't care

        htotal = vga.other.htotal + 1u;
        hdend = vga.other.hdend;

        hbstart = hdend;
        hbend = htotal;
        hrstart = vga.other.hsyncp;
        hrend = hrstart + (vga.other.hsyncw) ;

        vga.draw.address_line_total = vga.other.max_scanline + 1u;

        vtotal = vga.draw.address_line_total * (vga.other.vtotal+1u)+vga.other.vadjust;
        vdend = vga.draw.address_line_total * vga.other.vdend;
        vrstart = vga.draw.address_line_total * vga.other.vsyncp;
        vrend = vrstart + 16; // vsync width is fixed to 16 lines on the MC6845 TODO Tandy
        vbstart = vdend;
        vbend = vtotal;

        switch (machine) {
        default:
            clock = (PIT_TICK_RATE*12)/8;
            oscclock = clock * 8;
            break;
        }
        vga.draw.delay.hdend = hdend*1000.0/clock; //in milliseconds
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

    // Vertical blanking tricks
    vblank_skip = 0;
    if ((IS_VGA_ARCH) && !ignore_vblank_wraparound) { // others need more investigation
        if (vbstart < vtotal) { // There will be no blanking at all otherwise
            if (vbend > vtotal) {
                // blanking wraps to the start of the screen
                vblank_skip = vbend&0x7f;
                
                // on blanking wrap to 0, the first line is not blanked
                // this is used by the S3 BIOS and other S3 drivers in some SVGA modes
                if ((vbend&0x7f)==1) vblank_skip = 0;
                
                // it might also cut some lines off the bottom
                if (vbstart < vdend) {
                    vdend = vbstart;
                }
                LOG(LOG_VGA,LOG_WARN)("Blanking wrap to line %d", (int)vblank_skip);
            } else if (vbstart<=1) {
                // blanking is used to cut lines at the start of the screen
                vblank_skip = vbend;
                LOG(LOG_VGA,LOG_WARN)("Upper %d lines of the screen blanked", (int)vblank_skip);
            } else if (vbstart < vdend) {
                if (vbend < vdend) {
                    // the game wants a black bar somewhere on the screen
                    LOG(LOG_VGA,LOG_WARN)("Unsupported blanking: line %d-%d",(int)vbstart,(int)vbend);
                } else {
                    // blanking is used to cut off some lines from the bottom
                    vdend = vbstart;
                }
            }
            vdend -= vblank_skip;
        }
    }
    vga.draw.vblank_skip = vblank_skip;

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
    vga.draw.has_split=false;
    vga.draw.vret_triggered=false;

    if (vga_alt_new_mode) {
        vga.draw_2[0].doublescan_count = 0;
        vga.draw_2[0].doublescan_max = 0;
    }

    //Check to prevent useless black areas
    if (hbstart<hdend) hdend=hbstart;
    if ((!(IS_VGA_ARCH)) && (vbstart<vdend)) vdend=vbstart;

    Bitu width=hdend;
    Bitu height=vdend;

    if (IS_EGAVGA_ARCH) {
        vga.draw.address_line_total=(vga.crtc.maximum_scan_line&0x1fu)+1u;
        switch(vga.mode) {
        case M_TEXT:
            if (!vga_alt_new_mode) {
                // these use line_total internal
                // doublescanning needs to be emulated by renderer doubleheight
                // EGA has no doublescanning bit at 0x80
                if (vga.crtc.maximum_scan_line&0x80) {
                    // vga_draw only needs to draw every second line
                    height /= 2;
                }
                break;
            }
            /* fall through if vga_alt_new_mode */
        default:
            if (vga_alt_new_mode) {
                if (IS_VGA_ARCH && (vga.crtc.maximum_scan_line & 0x80))
                    vga.draw_2[0].doublescan_max = 1;
                else
                    vga.draw_2[0].doublescan_max = 0;
            }
            else {
                if (vga.crtc.maximum_scan_line & 0x80)
                    vga.draw.address_line_total *= 2;
            }

            break;
        }
    }

    //Set the bpp
    Bitu bpp;
    bpp = 8;
    vga.draw.linear_base = vga.mem.linear;
    vga.draw.linear_mask = vga.mem.memmask;

    /* parallel system */
    if (vga_alt_new_mode)
        VGA_Draw2_Recompute_CRTC_MaskAdd();

    /* Some games and plenty of demoscene productions like to rely on
     * the fact that the standard VGA modes wrap around at 256KB even
     * on SVGA hardware. Without this check, those demos will show
     * credits that scroll upward to blackness before "popping" back
     * onto the screen. */
    if (IS_VGA_ARCH) {
        /* NTS: S3 emulation ties "compatible chain4" to CRTC register 31 bit 3 which controls
         *      whether access to > 256KB of video RAM is enabled, which is why it's used here */
        if (vga.config.compatible_chain4 || svgaCard == SVGA_None)
            vga.draw.linear_mask &= 0x3FFFF;
    }

    if (IS_EGAVGA_ARCH)
        vga.draw.planar_mask = vga.draw.linear_mask >> 2;
    else
        vga.draw.planar_mask = vga.draw.linear_mask >> 1;

    Bitu pix_per_char = 8;
    switch (vga.mode) {
    case M_TEXT:
        vga.draw.blocks=width;
        // if char9_set is true, allow 9-pixel wide fonts
        if ((vga.seq.clocking_mode&0x01) || !vga.draw.char9_set) {
            // 8-pixel wide
            pix_per_char = 8;
            vga.draw.char9dot = false;
        } else {
            // 9-pixel wide
            pix_per_char = 9;
            vga.draw.char9dot = true;
        }

        {
            if (vga_alt_new_mode)
                VGA_DrawLine = Alt_VGA_TEXT_Xlat32_Draw_Line;
            else
                VGA_DrawLine = VGA_TEXT_Xlat32_Draw_Line;
            bpp = 32;
        }
        break;
    default:
        LOG(LOG_VGA,LOG_ERROR)("Unhandled VGA mode %d while checking for resolution",vga.mode);
        break;
    }
    width *= pix_per_char;
    VGA_CheckScanLength();

    /* for MCGA, need to "double scan" the screen in some cases */
    if (vga.other.mcga_mode_control & 2) { // 640x480 2-color
        height *= 2;
        mcga_double_scan = true;
    }
    else {
        mcga_double_scan = false;
    }
    
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
            PIC_RemoveEvents(VGA_Other_VertInterrupt);
            PIC_RemoveEvents(VGA_VerticalTimer);
            PIC_RemoveEvents(VGA_PanningLatch);
            PIC_RemoveEvents(VGA_DisplayStartLatch);
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
    PIC_RemoveEvents(VGA_DrawEGASingleLine);
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

uint32_t VGA_QuerySizeIG(void) {
    return  ((uint32_t)vga.draw.height << (uint32_t)16ul) |
             (uint32_t)vga.draw.width;
}

