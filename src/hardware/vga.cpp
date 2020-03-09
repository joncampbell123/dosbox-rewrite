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

/* TODO: move to general header */
static inline unsigned int int_log2(unsigned int val) {
    unsigned int log = 0;
    while ((val >>= 1u) != 0u) log++;
    return log;
}

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

    // NTS: This is WHY the memory size must be a power of 2
    vga.mem.memmask = vga.mem.memsize - 1u;

    LOG(LOG_VGA,LOG_NORMAL)("Video RAM: %uKB",vga.mem.memsize>>10);

    VGA_SetupMemory();      // memory is allocated here
    {
        VGA_SetupMisc();
        VGA_SetupDAC();
        VGA_SetupGFX();
        VGA_SetupSEQ();
        VGA_SetupAttr();
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

void MEM_ResetPageHandler_Unmapped(Bitu phys_page, Bitu pages);

void PC98_Set24KHz(void) {
}

void PC98_Set31KHz(void) {
}

void PC98_Set31KHz_480line(void) {
}

void VGA_Destroy(Section*) {
}

bool debugpollvga_pf_menu_callback(DOSBoxMenu * const xmenu, DOSBoxMenu::item * const menuitem) {
    (void)xmenu;//UNUSED
    (void)menuitem;//UNUSED
    return true;
}

bool debugpollvga_rtp_menu_callback(DOSBoxMenu * const xmenu, DOSBoxMenu::item * const menuitem) {
    (void)xmenu;//UNUSED
    (void)menuitem;//UNUSED
    return true;
}

void VGA_Init() {
    Bitu i,j;

	vga.config.chained = false;

    vga.draw.render_step = 0;
    vga.draw.render_max = 1;

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

