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
#include "util_units.h"
#include "control.h"
#include "mixer.h"
#include "menu.h"
#include "mem.h"
#include "render.h"

#include <string.h>
#include <stdlib.h>
#include <string>
#include <stdio.h>

#if defined(_MSC_VER)
# pragma warning(disable:4244) /* const fmath::local::uint64_t to double possible loss of data */
#endif

#include "zipfile.h"

extern ZIPFile savestate_zip;

using namespace std;

bool VGA_IsCaptureEnabled(void);
void VGA_UpdateCapturePending(void);
bool VGA_CaptureHasNextFrame(void);
void VGA_CaptureStartNextFrame(void);
void VGA_CaptureMarkError(void);
bool VGA_CaptureValidateCurrentFrame(void);

bool                                vga_8bit_dac = false;
bool                                vga_alt_new_mode = false;
bool                                enable_vga_8bit_dac = true;

extern int                          vga_memio_delay_ns;

uint32_t S3_LFB_BASE =              S3_LFB_BASE_DEFAULT;

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
SVGA_Driver svga;
int enableCGASnow;
int vesa_modelist_cap = 0;
int vesa_mode_width_cap = 0;
int vesa_mode_height_cap = 0;
bool vesa_bios_modelist_in_info = false;
bool vga_3da_polled = false;
bool vga_page_flip_occurred = false;
bool enable_page_flip_debugging_marker = false;
bool enable_vretrace_poll_debugging_marker = false;
bool vga_enable_hretrace_effects = false;
bool vga_enable_hpel_effects = false;
bool vga_enable_3C6_ramdac = false;
bool vga_sierra_lock_565 = false;
bool enable_vga_resize_delay = false;
bool vga_ignore_hdispend_change_if_smaller = false;
bool ignore_vblank_wraparound = false;
bool non_cga_ignore_oddeven = false;
bool non_cga_ignore_oddeven_engage = false;
bool vga_palette_update_on_full_load = true;
bool vga_double_buffered_line_compare = false;
bool int10_vesa_map_as_128kb = false;

unsigned char VGA_AC_remap = AC_4x4;

unsigned int vga_display_start_hretrace = 0;
float hretrace_fx_avg_weight = 3;

bool allow_vesa_4bpp_packed = true;
bool allow_vesa_lowres_modes = true;
bool allow_unusual_vesa_modes = true;
bool allow_explicit_vesa_24bpp = true;
bool allow_hd_vesa_modes = true;
bool vesa12_modes_32bpp = true;
bool allow_vesa_32bpp = true;
bool allow_vesa_24bpp = true;
bool allow_vesa_16bpp = true;
bool allow_vesa_15bpp = true;
bool allow_vesa_8bpp = true;
bool allow_vesa_4bpp = true;
bool allow_vesa_tty = true;

void page_flip_debug_notify() {
    if (enable_page_flip_debugging_marker)
        vga_page_flip_occurred = true;
}

void vsync_poll_debug_notify() {
    if (enable_vretrace_poll_debugging_marker)
        vga_3da_polled = true;
}

Bit32u CGA_2_Table[16];
Bit32u CGA_4_Table[256];
Bit32u CGA_4_HiRes_Table[256];
Bit32u CGA_16_Table[256];
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
    if (svga.determine_mode) {
        svga.determine_mode();
        return;
    }
    /* Test for VGA output active or direct color modes */
    switch (vga.s3.misc_control_2 >> 4) {
    case 0:
        if (vga.attr.mode_control & 1) { // graphics mode
            if (IS_VGA_ARCH && ((vga.gfx.mode & 0x40)||(vga.s3.reg_3a&0x10))) {
                // access above 256k?
                if (vga.s3.reg_31 & 0x8) VGA_SetMode(M_LIN8);
                else VGA_SetMode(M_VGA);
            }
            else if (vga.gfx.mode & 0x20) VGA_SetMode(M_CGA4);
            else if ((vga.gfx.miscellaneous & 0x0c)==0x0c) VGA_SetMode(M_CGA2);
            else {
                // access above 256k?
                if (vga.s3.reg_31 & 0x8) VGA_SetMode(M_LIN4);
                else VGA_SetMode(M_EGA);
            }
        } else {
            VGA_SetMode(M_TEXT);
        }
        break;
    case 1:VGA_SetMode(M_LIN8);break;
    case 3:VGA_SetMode(M_LIN15);break;
    case 5:VGA_SetMode(M_LIN16);break;
    case 7:VGA_SetMode(M_LIN24);break;
    case 13:VGA_SetMode(M_LIN32);break;
    case 15:VGA_SetMode(M_PACKED4);break;// hacked
    }
}

void VGA_StartResize(Bitu delay /*=50*/) {
    if (!vga.draw.resizing) {
        /* even if the user disables the delay, we can avoid a lot of window resizing by at least having 1ms of delay */
        if (!enable_vga_resize_delay && delay > 1) delay = 1;

        vga.draw.resizing=true;
        if (vga.mode==M_ERROR) delay = 5;
        /* Start a resize after delay (default 50 ms) */
        if (delay==0) VGA_SetupDrawing(0);
        else PIC_AddEvent(VGA_SetupDrawing,(float)delay);
    }
}

#define IS_RESET ((vga.seq.reset&0x3)!=0x3)
#define IS_SCREEN_ON ((vga.seq.clocking_mode&0x20)==0)
//static bool hadReset = false;

// disabled for later improvement
// Idea behind this: If the sequencer was reset and screen off we can
// Problem is some programs measure the refresh rate after switching mode,
// and get it wrong because of the 50ms guard time.
// On the other side, buggers like UniVBE switch the screen mode several
// times so the window is flickering.
// Also the demos that switch display end on screen (Dowhackado)
// would need some attention

void VGA_SequReset(bool reset) {
    (void)reset;//UNUSED
    //if(!reset && !IS_SCREEN_ON) hadReset=true;
}

void VGA_Screenstate(bool enabled) {
    (void)enabled;//UNUSED
    /*if(enabled && hadReset) {
        hadReset=false;
        PIC_RemoveEvents(VGA_SetupDrawing);
        VGA_SetupDrawing(0);
    }*/
}

void VGA_SetClock(Bitu which,Bitu target) {
    if (svga.set_clock) {
        svga.set_clock(which, target);
        return;
    }
    struct{
        Bitu n,m;
        Bits err;
    } best;
    best.err=(Bits)target;
    best.m=1u;
    best.n=1u;
    Bitu r;

    for (r = 0; r <= 3; r++) {
        Bitu f_vco = target * ((Bitu)1u << (Bitu)r);
        if (MIN_VCO <= f_vco && f_vco < MAX_VCO) break;
    }
    for (Bitu n=1;n<=31;n++) {
        Bits m=(Bits)((target * (n + 2u) * ((Bitu)1u << (Bitu)r) + (S3_CLOCK_REF / 2u)) / S3_CLOCK_REF) - 2;
        if (0 <= m && m <= 127) {
            Bitu temp_target = (Bitu)S3_CLOCK(m,n,r);
            Bits err = (Bits)(target - temp_target);
            if (err < 0) err = -err;
            if (err < best.err) {
                best.err = err;
                best.m = (Bitu)m;
                best.n = (Bitu)n;
            }
        }
    }
    /* Program the s3 clock chip */
    vga.s3.clk[which].m=best.m;
    vga.s3.clk[which].r=r;
    vga.s3.clk[which].n=best.n;
    VGA_StartResize();
}

void VGA_SetCGA2Table(Bit8u val0,Bit8u val1) {
    const Bit8u total[2] = {val0,val1};
    for (Bitu i=0;i<16u;i++) {
        CGA_2_Table[i]=
#ifdef WORDS_BIGENDIAN
            ((Bitu)total[(i >> 0u) & 1u] << 0u  ) | ((Bitu)total[(i >> 1u) & 1u] << 8u  ) |
            ((Bitu)total[(i >> 2u) & 1u] << 16u ) | ((Bitu)total[(i >> 3u) & 1u] << 24u );
#else 
            ((Bitu)total[(i >> 3u) & 1u] << 0u  ) | ((Bitu)total[(i >> 2u) & 1u] << 8u  ) |
            ((Bitu)total[(i >> 1u) & 1u] << 16u ) | ((Bitu)total[(i >> 0u) & 1u] << 24u );
#endif
    }
}

void VGA_SetCGA4Table(Bit8u val0,Bit8u val1,Bit8u val2,Bit8u val3) {
    const Bit8u total[4] = {val0,val1,val2,val3};
    for (Bitu i=0;i<256u;i++) {
        CGA_4_Table[i]=
#ifdef WORDS_BIGENDIAN
            ((Bitu)total[(i >> 0u) & 3u] << 0u  ) | ((Bitu)total[(i >> 2u) & 3u] << 8u  ) |
            ((Bitu)total[(i >> 4u) & 3u] << 16u ) | ((Bitu)total[(i >> 6u) & 3u] << 24u );
#else
            ((Bitu)total[(i >> 6u) & 3u] << 0u  ) | ((Bitu)total[(i >> 4u) & 3u] << 8u  ) |
            ((Bitu)total[(i >> 2u) & 3u] << 16u ) | ((Bitu)total[(i >> 0u) & 3u] << 24u );
#endif
        CGA_4_HiRes_Table[i]=
#ifdef WORDS_BIGENDIAN
            ((Bitu)total[((i >> 0u) & 1u) | ((i >> 3u) & 2u)] << 0u  ) | (Bitu)(total[((i >> 1u) & 1u) | ((i >> 4u) & 2u)] << 8u  ) |
            ((Bitu)total[((i >> 2u) & 1u) | ((i >> 5u) & 2u)] << 16u ) | (Bitu)(total[((i >> 3u) & 1u) | ((i >> 6u) & 2u)] << 24u );
#else
            ((Bitu)total[((i >> 3u) & 1u) | ((i >> 6u) & 2u)] << 0u  ) | (Bitu)(total[((i >> 2u) & 1u) | ((i >> 5u) & 2u)] << 8u  ) |
            ((Bitu)total[((i >> 1u) & 1u) | ((i >> 4u) & 2u)] << 16u ) | (Bitu)(total[((i >> 0u) & 1u) | ((i >> 3u) & 2u)] << 24u );
#endif
    }
}

class VFRCRATE : public Program {
public:
    void Run(void) {
        WriteOut("Video refresh rate.\n\n");
        if (cmd->FindExist("/?", false)) {
			WriteOut("VFRCRATE [SET [OFF|PAL|NTSC|rate]\n");
			WriteOut("  SET OFF   unlock\n");
			WriteOut("  SET PAL   lock to PAL frame rate\n");
			WriteOut("  SET NTSC  lock to NTSC frame rate\n");
			WriteOut("  SET rate  lock to integer frame rate, e.g. 15\n");
			WriteOut("  SET rate  lock to decimal frame rate, e.g. 29.97\n");
			WriteOut("  SET rate  lock to fractional frame rate, e.g. 60000/1001\n");
			return;
		}
        if (cmd->FindString("SET",temp_line,false)) {
            char *x = (char*)temp_line.c_str();

            if (!strncasecmp(x,"off",3))
                vga_force_refresh_rate = -1;
            else if (!strncasecmp(x,"ntsc",4))
                vga_force_refresh_rate = 60000.0/1001;
            else if (!strncasecmp(x,"pal",3))
                vga_force_refresh_rate = 50;
            else if (strchr(x,'.'))
                vga_force_refresh_rate = atof(x);
            else {
                /* fraction */
                int major = -1,minor = 0;
                major = strtol(x,&x,0);
                if (*x == '/' || *x == ':') {
                    x++; minor = strtol(x,NULL,0);
                }

                if (major > 0) {
                    vga_force_refresh_rate = (double)major;
                    if (minor > 1) vga_force_refresh_rate /= minor;
                }
            }

            VGA_SetupHandlers();
            VGA_StartResize();
        }

        if (vga_force_refresh_rate > 0)
            WriteOut("Locked to %.3f fps\n",vga_force_refresh_rate);
        else
            WriteOut("Unlocked\n");
    }
};

static void VFRCRATE_ProgramStart(Program * * make) {
    *make=new VFRCRATE;
}

/*! \brief          CGASNOW.COM utility to control CGA snow emulation
 *
 *  \description    Utility to enable, disable, or query CGA snow emulation.
 *                  This command is only available when machine=cga and
 *                  the video mode is 80x25 text mode.
 */
class CGASNOW : public Program {
public:
    /*! \brief      Program entry point, when the command is run
     */
    void Run(void) {
        if(cmd->FindExist("ON")) {
            WriteOut("CGA snow enabled\n");
            enableCGASnow = 1;
            if (vga.mode == M_TEXT || vga.mode == M_TANDY_TEXT) {
                VGA_SetupHandlers();
                VGA_StartResize();
            }
        }
        else if(cmd->FindExist("OFF")) {
            WriteOut("CGA snow disabled\n");
            enableCGASnow = 0;
            if (vga.mode == M_TEXT || vga.mode == M_TANDY_TEXT) {
                VGA_SetupHandlers();
                VGA_StartResize();
            }
        }
        else {
            WriteOut("CGA snow currently %s\n",
                enableCGASnow ? "enabled" : "disabled");
        }
    }
};

static void CGASNOW_ProgramStart(Program * * make) {
    *make=new CGASNOW;
}

/* TODO: move to general header */
static inline unsigned int int_log2(unsigned int val) {
    unsigned int log = 0;
    while ((val >>= 1u) != 0u) log++;
    return log;
}

extern int hack_lfb_yadjust;

void VGA_VsyncUpdateMode(VGA_Vsync vsyncmode);

VGA_Vsync VGA_Vsync_Decode(const char *vsyncmodestr) {
    if (!strcasecmp(vsyncmodestr,"off")) return VS_Off;
    else if (!strcasecmp(vsyncmodestr,"on")) return VS_On;
    else if (!strcasecmp(vsyncmodestr,"force")) return VS_Force;
    else if (!strcasecmp(vsyncmodestr,"host")) return VS_Host;
    else
        LOG_MSG("Illegal vsync type %s, falling back to off.",vsyncmodestr);

    return VS_Off;
}

Bit32u MEM_get_address_bits();

void VGA_Reset(Section*) {
    Section_prop * section=static_cast<Section_prop *>(control->GetSection("dosbox"));
    bool lfb_default = false;
    string str;
    int i;

    Bit32u cpu_addr_bits = MEM_get_address_bits();
//    Bit64u cpu_max_addr = (Bit64u)1 << (Bit64u)cpu_addr_bits;

    LOG(LOG_MISC,LOG_DEBUG)("VGA_Reset() reinitializing VGA emulation");

    S3_LFB_BASE = (uint32_t)section->Get_hex("svga lfb base");
    if (S3_LFB_BASE == 0) {
        if (cpu_addr_bits >= 32)
            S3_LFB_BASE = S3_LFB_BASE_DEFAULT;
        else if (cpu_addr_bits >= 26)
            S3_LFB_BASE = 0x03400000;
        else if (cpu_addr_bits >= 24)
            S3_LFB_BASE = 0x00C00000;
        else
            S3_LFB_BASE = S3_LFB_BASE_DEFAULT;

        lfb_default = true;
    }

    /* no farther than 32MB below the top */
    if (S3_LFB_BASE > 0xFE000000UL)
        S3_LFB_BASE = 0xFE000000UL;

    {
        /* must be 64KB aligned (ISA) */
        S3_LFB_BASE +=  0x7FFFUL;
        S3_LFB_BASE &= ~0xFFFFUL;
    }

    /* must not overlap system RAM */
    if (S3_LFB_BASE < (MEM_TotalPages()*4096))
        S3_LFB_BASE = (MEM_TotalPages()*4096);

    /* announce LFB framebuffer address only if actually emulating the S3 */
    if (IS_VGA_ARCH && svgaCard == SVGA_S3Trio)
        LOG(LOG_VGA,LOG_DEBUG)("S3 linear framebuffer at 0x%lx%s as %s",
            (unsigned long)S3_LFB_BASE,lfb_default?" by default":"",
            "(E)ISA");

    /* other applicable warnings: */
    /* Microsoft Windows 3.1 S3 driver:
     *   If the LFB is set to an address below 16MB, the driver will program the base to something
     *   odd like 0x73000000 and access MMIO through 0x74000000.
     *
     *   Because of this, if memalias < 31 and LFB is below 16MB mark, Windows won't use the
     *   accelerated features of the S3 emulation properly.
     *
     *   If memalias=24, the driver hangs and nothing appears on screen.
     *
     *   As far as I can tell, it's mapping for the LFB, not the MMIO. It uses the MMIO in the
     *   A0000-AFFFF range anyway. The failure to blit and draw seems to be caused by mapping the
     *   LFB out of range like that and then trying to draw on the LFB.
     *
     *   As far as I can tell from http://www.vgamuseum.info and the list of S3 cards, the S3 chipsets
     *   emulated by DOSBox-X and DOSBox SVN here are all EISA and PCI cards, so it's likely the driver
     *   is written around the assumption that memory addresses are the full 32 bits to the card, not
     *   just the low 24 seen on the ISA slot. So it is unlikely the driver could ever support the
     *   card on a 386SX nor could such a card work on a 386SX. It shouldn't even work on a 486SX
     *   (26-bit limit), but it could. */
    if (IS_VGA_ARCH && svgaCard == SVGA_S3Trio && cpu_addr_bits <= 24)
        LOG(LOG_VGA,LOG_WARN)("S3 linear framebuffer warning: memalias setting is known to cause the Windows 3.1 S3 driver to crash");
    if (IS_VGA_ARCH && svgaCard == SVGA_S3Trio && cpu_addr_bits < 31 && S3_LFB_BASE < 0x1000000ul) /* below 16MB and memalias == 31 bits */
        LOG(LOG_VGA,LOG_WARN)("S3 linear framebuffer warning: A linear framebuffer below the 16MB mark in physical memory when memalias < 31 is known to have problems with the Windows 3.1 S3 driver");

    str = section->Get_string("vga attribute controller mapping");
    if (str == "4x4")
        VGA_AC_remap = AC_4x4;
    else if (str == "4low")
        VGA_AC_remap = AC_low4;
    else {
        /* auto:
         *
         * 4x4 by default.
         * except for ET4000 which is 4low */
        VGA_AC_remap = AC_4x4;
        if (IS_VGA_ARCH) {
            if (svgaCard == SVGA_TsengET3K || svgaCard == SVGA_TsengET4K)
                VGA_AC_remap = AC_low4;
        }
    }

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

    enableCGASnow = section->Get_bool("cgasnow");
    vesa_modelist_cap = section->Get_int("vesa modelist cap");
    vesa_mode_width_cap = section->Get_int("vesa modelist width limit");
    vesa_mode_height_cap = section->Get_int("vesa modelist height limit");
    vga_enable_3C6_ramdac = section->Get_bool("sierra ramdac");
    vga_enable_hpel_effects = section->Get_bool("allow hpel effects");
    vga_sierra_lock_565 = section->Get_bool("sierra ramdac lock 565");
    hretrace_fx_avg_weight = section->Get_double("hretrace effect weight");
    ignore_vblank_wraparound = section->Get_bool("ignore vblank wraparound");
    int10_vesa_map_as_128kb = section->Get_bool("vesa map non-lfb modes to 128kb region");
    vga_enable_hretrace_effects = section->Get_bool("allow hretrace effects");
    enable_page_flip_debugging_marker = section->Get_bool("page flip debug line");
    vga_palette_update_on_full_load = section->Get_bool("vga palette update on full load");
    non_cga_ignore_oddeven = section->Get_bool("ignore odd-even mode in non-cga modes");
    enable_vretrace_poll_debugging_marker = section->Get_bool("vertical retrace poll debug line");
    vga_double_buffered_line_compare = section->Get_bool("double-buffered line compare");
    hack_lfb_yadjust = section->Get_int("vesa lfb base scanline adjust");
    allow_vesa_lowres_modes = section->Get_bool("allow low resolution vesa modes");
    vesa12_modes_32bpp = section->Get_bool("vesa vbe 1.2 modes are 32bpp");
    allow_vesa_4bpp_packed = section->Get_bool("allow 4bpp packed vesa modes");
    allow_explicit_vesa_24bpp = section->Get_bool("allow explicit 24bpp vesa modes");
    allow_hd_vesa_modes = section->Get_bool("allow high definition vesa modes");
    allow_unusual_vesa_modes = section->Get_bool("allow unusual vesa modes");
    allow_vesa_32bpp = section->Get_bool("allow 32bpp vesa modes");
    allow_vesa_24bpp = section->Get_bool("allow 24bpp vesa modes");
    allow_vesa_16bpp = section->Get_bool("allow 16bpp vesa modes");
    allow_vesa_15bpp = section->Get_bool("allow 15bpp vesa modes");
    allow_vesa_8bpp = section->Get_bool("allow 8bpp vesa modes");
    allow_vesa_4bpp = section->Get_bool("allow 4bpp vesa modes");
    allow_vesa_tty = section->Get_bool("allow tty vesa modes");
    enable_vga_resize_delay = section->Get_bool("enable vga resize delay");
    vga_ignore_hdispend_change_if_smaller = section->Get_bool("resize only on vga active display width increase");
    vesa_bios_modelist_in_info = section->Get_bool("vesa vbe put modelist in vesa information");

    /* sanity check: "VBE 1.2 modes 32bpp" doesn't make any sense if neither 24bpp or 32bpp is enabled */
    if (!allow_vesa_32bpp && !allow_vesa_24bpp)
        vesa12_modes_32bpp = 0;
    /* sanity check: "VBE 1.2 modes 32bpp=true" doesn't make sense if the user disabled 32bpp */
    else if (vesa12_modes_32bpp && !allow_vesa_32bpp)
        vesa12_modes_32bpp = 0;
    /* sanity check: "VBE 1.2 modes 32bpp=false" doesn't make sense if the user disabled 24bpp */
    else if (!vesa12_modes_32bpp && !allow_vesa_24bpp && allow_vesa_32bpp)
        vesa12_modes_32bpp = 1;

    if (vga_force_refresh_rate > 0)
        LOG(LOG_VGA,LOG_NORMAL)("VGA forced refresh rate active = %.3f",vga_force_refresh_rate);

    vga.draw.resizing=false;
    vga.mode=M_ERROR;           //For first init

    vga_8bit_dac = false;
    enable_vga_8bit_dac = section->Get_bool("enable 8-bit dac");

    vga_memio_delay_ns = section->Get_int("vmemdelay");
    if (vga_memio_delay_ns < 0) {
        if (IS_EGAVGA_ARCH) {
            {
                /* very optimistic setting, ISA bus cycles are longer than 2, but also the 386/486/Pentium pipeline
                 * instruction decoding. so it's not a matter of sucking up enough CPU cycle counts to match the
                 * duration of a memory I/O cycle, because real hardware probably has another instruction decode
                 * going while it does it.
                 *
                 * this is long enough to fix some demo's raster effects to work properly but not enough to
                 * significantly bring DOS games to a crawl. Apparently, this also fixes Future Crew "Panic!"
                 * by making the shadebob take long enough to allow the 3D rotating dot object to finish it's
                 * routine just in time to become the FC logo, instead of sitting there waiting awkwardly
                 * for 3-5 seconds. */
                double t = (1000000000.0 * clockdom_ISA_BCLK.freq_div * 3.75) / clockdom_ISA_BCLK.freq;
                vga_memio_delay_ns = (int)floor(t);
            }
        }
        else if (machine == MCH_CGA || machine == MCH_HERC || machine == MCH_MDA) {
            /* default IBM PC/XT 4.77MHz timing. this is carefully tuned so that Trixter's CGA test program
             * times our CGA emulation as having about 305KB/sec reading speed. */
            double t = (1000000000.0 * clockdom_ISA_OSC.freq_div * 143) / (clockdom_ISA_OSC.freq * 3);
            vga_memio_delay_ns = (int)floor(t);
        }
        else {
            /* dunno. pick something */
            double t = (1000000000.0 * clockdom_ISA_BCLK.freq_div * 6) / clockdom_ISA_BCLK.freq;
            vga_memio_delay_ns = (int)floor(t);
        }
    }

    LOG(LOG_VGA,LOG_DEBUG)("VGA memory I/O delay %uns",vga_memio_delay_ns);

    /* mainline compatible vmemsize (in MB)
     * plus vmemsizekb for KB-level control.
     * then we round up a page.
     *
     * FIXME: If PCjr/Tandy uses system memory as video memory,
     *        can we get away with pointing at system memory
     *        directly and not allocate a dedicated char[]
     *        array for VRAM? Likewise for VGA emulation of
     *        various motherboard chipsets known to "steal"
     *        off the top of system RAM, like Intel and
     *        Chips & Tech VGA implementations? */
    {
        int sz_m = section->Get_int("vmemsize");
        int sz_k = section->Get_int("vmemsizekb");

        if (sz_m >= 0 || sz_k > 0) {
            vga.mem.memsize  = _MB_bytes((unsigned int)sz_m);
            vga.mem.memsize += _KB_bytes((unsigned int)sz_k);
            vga.mem.memsize  = (vga.mem.memsize + 0xFFFu) & (~0xFFFu);
            /* mainline compatible: vmemsize == 0 means 512KB */
            if (vga.mem.memsize == 0) vga.mem.memsize = _KB_bytes(512);

            /* round up to the nearest power of 2 (TODO: Any video hardware that uses non-power-of-2 sizes?).
             * A lot of DOSBox's VGA emulation code assumes power-of-2 VRAM sizes especially when wrapping
             * memory addresses with (a & (vmemsize - 1)) type code. */
            if (!is_power_of_2(vga.mem.memsize)) {
                vga.mem.memsize = 1u << (int_log2(vga.mem.memsize) + 1u);
                LOG(LOG_VGA,LOG_WARN)("VGA RAM size requested is not a power of 2, rounding up to %uKB",vga.mem.memsize>>10);
            }
        }
        else {
            vga.mem.memsize = 0; /* machine-specific code will choose below */
        }
    }

    /* sanity check according to adapter type.
     * FIXME: Again it was foolish for DOSBox to standardize on machine=
     * for selecting machine type AND video card. */
    switch (machine) {
        case MCH_HERC:
            if (vga.mem.memsize < _KB_bytes(64)) vga.mem.memsize = _KB_bytes(64);
            break;
        case MCH_MDA:
            if (vga.mem.memsize < _KB_bytes(4)) vga.mem.memsize = _KB_bytes(4);
            break;
        case MCH_CGA:
            if (vga.mem.memsize < _KB_bytes(16)) vga.mem.memsize = _KB_bytes(16);
            break;
        case MCH_PCJR:
            if (vga.mem.memsize < _KB_bytes(128)) vga.mem.memsize = _KB_bytes(128); /* FIXME: Right? */
            break;
        case MCH_EGA:
                 // EGA cards supported either 64KB, 128KB or 256KB.
                 if (vga.mem.memsize == 0)              vga.mem.memsize = _KB_bytes(256);//default
            else if (vga.mem.memsize <= _KB_bytes(64))  vga.mem.memsize = _KB_bytes(64);
            else if (vga.mem.memsize <= _KB_bytes(128)) vga.mem.memsize = _KB_bytes(128);
            else                                        vga.mem.memsize = _KB_bytes(256);
            break;
        case MCH_VGA:
            // TODO: There are reports of VGA cards that have less than 256KB in the early days of VGA.
            //       How does that work exactly, especially when 640x480 requires about 37KB per plane?
            //       Did these cards have some means to chain two bitplanes odd/even in the same way
            //       tha EGA did it?
            if (vga.mem.memsize != 0 || svgaCard == SVGA_None) {
                if (vga.mem.memsize < _KB_bytes(256)) vga.mem.memsize = _KB_bytes(256);
            }
            break;
        default:
            E_Exit("Unexpected machine");
    }

    /* I'm sorry, emulating 640x350 4-color chained EGA graphics is
     * harder than I thought and would require revision of quite a
     * bit of VGA planar emulation to update only bitplane 0 and 2
     * in such a manner. --J.C. */
    if (IS_EGA_ARCH && vga.mem.memsize < _KB_bytes(128))
        LOG_MSG("WARNING: EGA 64KB emulation is very experimental and not well supported");

    if (!IS_PC98_ARCH)
        SVGA_Setup_Driver();        // svga video memory size is set here, possibly over-riding the user's selection

    // NTS: This is WHY the memory size must be a power of 2
    vga.mem.memmask = vga.mem.memsize - 1u;

    LOG(LOG_VGA,LOG_NORMAL)("Video RAM: %uKB",vga.mem.memsize>>10);

    // TODO: If S3 emulation, and linear framebuffer bumps up against the CPU memalias limits,
    //       trim Video RAM to fit (within reasonable limits) or else E_Exit() to let the user
    //       know of impossible constraints.

    mainMenu.get_item("debug_pageflip").check(enable_page_flip_debugging_marker).refresh_item(mainMenu);
    mainMenu.get_item("debug_retracepoll").check(enable_vretrace_poll_debugging_marker).refresh_item(mainMenu);

    VGA_SetupMemory();      // memory is allocated here
    if (!IS_PC98_ARCH) {
        VGA_SetupMisc();
        VGA_SetupDAC();
        VGA_SetupGFX();
        VGA_SetupSEQ();
        VGA_SetupAttr();
        VGA_SetupOther();
        VGA_SetupXGA();
        VGA_SetClock(0,CLK_25);
        VGA_SetClock(1,CLK_28);
        /* Generate tables */
        VGA_SetCGA2Table(0,1);
        VGA_SetCGA4Table(0,1,2,3);
    }

    Section_prop * section2=static_cast<Section_prop *>(control->GetSection("vsync"));

    const char * vsyncmodestr;
    vsyncmodestr=section2->Get_string("vsyncmode");
    void change_output(int output);
    change_output(8);
    VGA_VsyncUpdateMode(VGA_Vsync_Decode(vsyncmodestr));

    const char * vsyncratestr;
    vsyncratestr=section2->Get_string("vsyncrate");
    double vsyncrate=70;
    if (!strcasecmp(vsyncmodestr,"host")) {
#if defined (WIN32)
        DEVMODE devmode;

        if (EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devmode))
            vsyncrate=devmode.dmDisplayFrequency;
        else
            sscanf(vsyncratestr,"%lf",&vsyncrate);
#endif
    }
    else {
        sscanf(vsyncratestr,"%lf",&vsyncrate);
    }

    vsync.period = (1000.0F)/vsyncrate;

    // TODO: Code to remove programs added by PROGRAMS_MakeFile

    if (machine == MCH_CGA) PROGRAMS_MakeFile("CGASNOW.COM",CGASNOW_ProgramStart);
    PROGRAMS_MakeFile("VFRCRATE.COM",VFRCRATE_ProgramStart);
}

extern void VGA_TweakUserVsyncOffset(float val);
void INT10_PC98_CurMode_Relocate(void);
void VGA_UnsetupMisc(void);
void VGA_UnsetupAttr(void);
void VGA_UnsetupDAC(void);
void VGA_UnsetupGFX(void);
void VGA_UnsetupSEQ(void);

#define gfx(blah) vga.gfx.blah
#define seq(blah) vga.seq.blah
#define crtc(blah) vga.crtc.blah

void VGA_OnEnterPC98(Section *sec) {
}

void MEM_ResetPageHandler_Unmapped(Bitu phys_page, Bitu pages);

void updateGDCpartitions4(bool enable) {
}

void PC98_Set24KHz(void) {
}

void PC98_Set31KHz(void) {
}

void PC98_Set31KHz_480line(void) {
}

void VGA_OnEnterPC98_phase2(Section *sec) {
}

void VGA_Destroy(Section*) {
}

void UpdateCGAFromSaveState(void);

void VGA_LoadState(Section *sec) {
    (void)sec;//UNUSED

    {
        {
            ZIPFileEntry *ent = savestate_zip.get_entry("vga.ac.palette.bin");
            if (ent != NULL) {
                ent->rewind();
                ent->read(vga.attr.palette, 0x10);
            }
        }

        {
            unsigned char tmp[256 * 3];

            ZIPFileEntry *ent = savestate_zip.get_entry("vga.dac.palette.bin");
            if (ent != NULL) {
                ent->rewind();
                ent->read(tmp, 256 * 3);
                for (unsigned int c=0;c < 256;c++) {
                    vga.dac.rgb[c].red =   tmp[c*3 + 0];
                    vga.dac.rgb[c].green = tmp[c*3 + 1];
                    vga.dac.rgb[c].blue =  tmp[c*3 + 2];
                }
            }
        }

        {
            ZIPFileEntry *ent = savestate_zip.get_entry("cgareg.txt");
            if (ent != NULL) {
                zip_nv_pair_map nv(*ent);
                vga.tandy.mode_control =        (unsigned char)nv.get_ulong("cga.mode_control");
                vga.tandy.color_select =        (unsigned char)nv.get_ulong("cga.color_select");
            }
        }

        UpdateCGAFromSaveState();

        for (unsigned int i=0;i < 0x10;i++)
            VGA_ATTR_SetPalette(i,vga.attr.palette[i]);

        VGA_DAC_UpdateColorPalette();
    }
}

void VGA_SaveState(Section *sec) {
    (void)sec;//UNUSED

    {
        {
            ZIPFileEntry *ent = savestate_zip.new_entry("vga.ac.palette.bin");
            if (ent != NULL) {
                ent->write(vga.attr.palette, 0x10);
            }
        }

        {

            ZIPFileEntry *ent = savestate_zip.new_entry("vga.dac.palette.bin");
            if (ent != NULL) {
                unsigned char tmp[256 * 3];
                for (unsigned int c=0;c < 256;c++) {
                    tmp[c*3 + 0] = vga.dac.rgb[c].red;
                    tmp[c*3 + 1] = vga.dac.rgb[c].green;
                    tmp[c*3 + 2] = vga.dac.rgb[c].blue;
                }
                ent->write(tmp, 256 * 3);
            }
        }

        {
            char tmp[512],*w=tmp;

            ZIPFileEntry *ent = savestate_zip.new_entry("cgareg.txt");
            if (ent != NULL) {
                w += sprintf(w,"cga.mode_control=0x%x\n",(unsigned int)vga.tandy.mode_control);
                w += sprintf(w,"cga.color_select=0x%x\n",(unsigned int)vga.tandy.color_select);
                assert(w < (tmp + sizeof(tmp)));
                ent->write(tmp, (size_t)(w - tmp));
            }
        }
    }
}

bool debugpollvga_pf_menu_callback(DOSBoxMenu * const xmenu, DOSBoxMenu::item * const menuitem) {
    (void)xmenu;//UNUSED
    (void)menuitem;//UNUSED

    enable_page_flip_debugging_marker = !enable_page_flip_debugging_marker;
    mainMenu.get_item("debug_pageflip").check(enable_page_flip_debugging_marker).refresh_item(mainMenu);

    return true;
}

bool debugpollvga_rtp_menu_callback(DOSBoxMenu * const xmenu, DOSBoxMenu::item * const menuitem) {
    (void)xmenu;//UNUSED
    (void)menuitem;//UNUSED

    enable_vretrace_poll_debugging_marker = !enable_vretrace_poll_debugging_marker;
    mainMenu.get_item("debug_retracepoll").check(enable_vretrace_poll_debugging_marker).refresh_item(mainMenu);

    return true;
}

void VGA_Init() {
    Bitu i,j;

    vga.other.mcga_mode_control = 0;

	vga.config.chained = false;

    vga.draw.render_step = 0;
    vga.draw.render_max = 1;

    vga.tandy.draw_base = NULL;
    vga.tandy.mem_base = NULL;
    LOG(LOG_MISC,LOG_DEBUG)("Initializing VGA");
    LOG(LOG_MISC,LOG_DEBUG)("Render scaler maximum resolution is %u x %u",SCALER_MAXWIDTH,SCALER_MAXHEIGHT);

    VGA_TweakUserVsyncOffset(0.0f);

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

    mainMenu.alloc_item(DOSBoxMenu::item_type_id,"debug_pageflip").set_text("Page flip debug line").set_callback_function(debugpollvga_pf_menu_callback);
    mainMenu.alloc_item(DOSBoxMenu::item_type_id,"debug_retracepoll").set_text("Retrace poll debug line").set_callback_function(debugpollvga_rtp_menu_callback);

    AddExitFunction(AddExitFunctionFuncPair(VGA_Destroy));
    AddVMEventFunction(VM_EVENT_RESET,AddVMEventFunctionFuncPair(VGA_Reset));

    AddVMEventFunction(VM_EVENT_LOAD_STATE,AddVMEventFunctionFuncPair(VGA_LoadState));
    AddVMEventFunction(VM_EVENT_SAVE_STATE,AddVMEventFunctionFuncPair(VGA_SaveState));
}

void SVGA_Setup_Driver(void) {
    memset(&svga, 0, sizeof(SVGA_Driver));

    switch(svgaCard) {
    case SVGA_S3Trio:
        SVGA_Setup_S3Trio();
        break;
    default:
        break;
    }
}

void VGA_CaptureStartNextFrame(void) {
    vga_capture_current_rect = vga_capture_rect;
    vga_capture_current_address = vga_capture_address;
    vga_capture_write_address = vga_capture_address;

    vga_capture_address = 0;

    VGA_UpdateCapturePending();
}

bool VGA_CaptureValidateCurrentFrame(void) {
    if (VGA_IsCaptureEnabled()) {
        if (vga_capture_current_rect.x >= 0 && vga_capture_current_rect.y >= 0 &&       // crop rect is within frame
            (unsigned int)vga_capture_current_rect.y < vga.draw.height &&
            (unsigned int)vga_capture_current_rect.x < vga.draw.width &&
            vga_capture_current_rect.w > 0 && vga_capture_current_rect.h > 0 &&         // crop rect size is within frame
            (unsigned int)vga_capture_current_rect.h <= vga.draw.height &&
            (unsigned int)vga_capture_current_rect.w <= vga.draw.width &&
            ((unsigned int)vga_capture_current_rect.x+vga_capture_current_rect.w) <= vga.draw.width && // crop rect pos+size within frame
            ((unsigned int)vga_capture_current_rect.y+vga_capture_current_rect.h) <= vga.draw.height) {
            return true;
        }
    }

    return false;
}

bool VGA_CaptureHasNextFrame(void) {
    return !!(vga_capture_address != (uint32_t)0);
}

void VGA_MarkCaptureAcquired(void) {
    if (vga_capture_state & ((uint32_t)(1ul << 1ul))) // if already acquired and guest has not cleared the bit
        vga_capture_state |= (uint32_t)(1ul << 6ul); // mark overrun

    vga_capture_state |= (uint32_t)(1ul << 1ul); // mark acquired
}

void VGA_MarkCaptureRetrace(void) {
    vga_capture_state |=   (uint32_t)(1ul << 5ul); // mark retrace
}

void VGA_MarkCaptureInProgress(bool en) {
    const uint32_t f = (uint32_t)(1ul << 3ul);

    if (en)
        vga_capture_state |= f;
    else
        vga_capture_state &= ~f;
}

bool VGA_IsCapturePending(void) {
    return !!(vga_capture_state & ((uint32_t)(1ul << 0ul)));
}

bool VGA_IsCaptureEnabled(void) {
    return !!(vga_capture_state & ((uint32_t)(1ul << 4ul)));
}

bool VGA_IsCaptureInProgress(void) {
    return !!(vga_capture_state & ((uint32_t)(1ul << 3ul)));
}

void VGA_CaptureMarkError(void) {
    vga_capture_state |=   (uint32_t)(1ul << 2ul);  // set error
    vga_capture_state &= ~((uint32_t)(1ul << 4ul)); // clear enable
}

void VGA_UpdateCapturePending(void) {
    bool en = false;

    if (VGA_IsCaptureEnabled()) {
        if (vga_capture_address != (uint32_t)0)
            en = true;
    }

    if (en)
        vga_capture_state |=   (uint32_t)(1ul << 0ul); // set bit 0 capture pending
    else
        vga_capture_state &= ~((uint32_t)(1ul << 0ul)); // clear bit 0 capture pending
}

uint32_t VGA_QueryCaptureState(void) {
    /* bits[0:0] = if set, capture pending
     * bits[1:1] = if set, capture acquired
     * bits[2:2] = if set, capture state error (such as crop rectangle out of bounds)
     * bits[3:3] = if set, capture in progress
     * bits[4:4] = if set, capture enabled
     * bits[5:5] = if set, vertical retrace occurred. capture must be enabled for this to occur
     * bits[6:6] = if set, capture was acquired and acquired bit was already set (overrun)
     *
     * both bits 0 and 1 can be set if one capture has finished and the "next" capture address has been loaded.
     */
    return vga_capture_state;
}

void VGA_SetCaptureState(uint32_t v) {
    /* bits[1:1] = if set, clear capture acquired bit
     * bits[2:2] = if set, clear capture state error
       bits[4:4] = if set, enable capture
       bits[5:5] = if set, clear vertical retrace occurrence flag
       bits[6:6] = if set, clear overrun (acquired) bit */
    vga_capture_state ^= (vga_capture_state & v & 0x66/*x110 0110*/);

    vga_capture_state &=    ~0x10u;
    vga_capture_state |= v & 0x10u;

    if (!VGA_IsCaptureEnabled())
        vga_capture_state = 0;

    VGA_UpdateCapturePending();
}

uint32_t VGA_QueryCaptureAddress(void) {
    return vga_capture_current_address;
}

void VGA_SetCaptureAddress(uint32_t v) {
    vga_capture_address = v;
    VGA_UpdateCapturePending();
}

void VGA_SetCaptureStride(uint32_t v) {
    vga_capture_stride = v;
    VGA_UpdateCapturePending();
}

