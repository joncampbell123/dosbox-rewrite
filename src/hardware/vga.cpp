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

#define VGA_PAGE_B8		(0xB8000/4096)

using namespace std;

VGA_Type vga;

enum class VGAPixelEmit : unsigned char {
    x1=0,   // once (single)
    x2=1,   // twice (double)
    x4=2,   // 4 times
    x8=3    // 8 times
};

template <typename T> struct VGACRTCCount {
    T       pixels,chars;

    VGACRTCCount() : pixels(0), chars(0) {
    }
    VGACRTCCount(const T z) : pixels(z), chars(z) {
    }
    VGACRTCCount(const T p,const T c) : pixels(p), chars(c) {
    }
};

template <typename T> struct VGACRTCStartStop {
    T       start,stop;

    VGACRTCStartStop() : start(0), stop(0) {
    }
    VGACRTCStartStop(const T z) : start(z), stop(z) {
    }
    VGACRTCStartStop(const T a,const T b) : start(a), stop(b) {
    }
};

template <typename T> struct VGACRTCCountMax {
    T       count,maximum;

    VGACRTCCountMax() : count(0), maximum(1) {
    }
    VGACRTCCountMax(const T z) : count(0), maximum(z) {
    }
    VGACRTCCountMax(const T a,const T b) : count(a), maximum(b) {
    }
};

// common values for one dimension
struct VGACRTCDAC_Dim {
    /* pixel 0 starts active display, immediately follows pixel total-1 (CGA/EGA/VGA/etc model).
     * In VGA emulation, any region not blanked outside of active display is border color.
     * PC-98 emulation will ignore border color. */
    /* example: */
    /* 0........................................................................(total-1) */
    /* | active display ----------------------------- |                          | */
    /* |                                              | border ----------------- | */
    /* |                                                 | blank ------------ |  | */
    /* |                                                             | sync |    | */
    VGACRTCCount<unsigned int>                          total;                  // h/v-total. counted 0 <= x < total
    VGACRTCCount<unsigned int>                          active;                 // h/v-active. counted 0 <= x < active
    VGACRTCStartStop< VGACRTCCount<unsigned int> >      blank;                  // h/v-blanking. starts at first dot clock of start, stops at first dot clock of stop
    VGACRTCStartStop< VGACRTCCount<unsigned int> >      sync;                   // h/v-sync. starts at first dot clock of start, stops at first dot clock of stop
};

struct VGACRTCDAC_Dim_H : VGACRTCDAC_Dim {
    unsigned int                    dot_clock_per_char_clock = 0;   // width of a character clock in dot clock pixels. not necessarily pixels per cell (EGA/VGA)
};

struct VGACRTCDACStatus_Dim {
    union vsig_t {
        struct {
            unsigned int            active:1;
            unsigned int            border:1;
            unsigned int            blank:1;
            unsigned int            sync:1;
            unsigned int            display_disable:1;
            unsigned int            sync_disable:1;
        } f;
        unsigned int                raw = 0;
    } vsig;
    VGACRTCCount<unsigned int>      count;                          // h/v count
};

struct VGACRTCDACStatus_Dim_V : VGACRTCDACStatus_Dim {
    VGACRTCCountMax<unsigned int>   row_count;                      // row count/max, scanline
    VGACRTCCountMax<unsigned int>   scandouble;                     // scanline doubling in hardware (MCGA, VGA 200-line CRTC values doubled to 400-line)
};

// maximum width of DAC shift output (more than enough for anything)
#define MAX_CRTC_SHIFT_REGISTER     128

// NTS: video hardware emulation should always set bpp to the highest depth of it's outputs if possible.
//      CGA should use bpp = 8 to cover the 16 colors supported.
//      EGA should use bpp = 8 to cover all 64 colors supported.
//      VGA should use bpp = 32 to cover the 2^18 colors possible (6-bit R/G/B)
//      SVGA should use bpp = 32 to cover the 2^24 colors possible
//      PC-98 should use bpp = 32 to cover the 8 colors (8-color digital), 4096 colors (16-color analog), or 2^24 (256-color analog)
struct VGACRTCDAC_ShiftReg {
    union src_t { /* I hope your compiler aligns this for best performance, or at least to a multiple of 4 bytes (32 bits) */
        uint8_t                     r8[MAX_CRTC_SHIFT_REGISTER];        // when bpp = 8
        uint16_t                    r16[MAX_CRTC_SHIFT_REGISTER];       // when bpp = 16
        uint32_t                    r32[MAX_CRTC_SHIFT_REGISTER];       // whne bpp = 32
    } src;
    unsigned char                   bpp = 0;                            // bits per pixel of shift register
    unsigned int                    shiftpos = 0;                       // shift register position. count from 0 <= x < shift_register_pixels, then resets to 0
    unsigned int                    shift_register_pixels = 0;          // number of pixels to emit per shift register load
    enum VGAPixelEmit               output_pixel_emit = VGAPixelEmit::x1;// pixel duplication to output
};

// value is num/den
template <typename T> struct DRational {
    T                               num,den;

    DRational() : num(1), den(1) { }
    DRational(const T r) : num(r), den(1) { }
    DRational(const T n,const T d) : num(n), den(d) { }

    /* NTS: Unlike Boost rational we do NOT auto-reduce on assign! */
    void do_reduce(void) {
        _nd_reduce</*check*/true>();
    }

    DRational &ip_reduce(void) {
        do_reduce();
        return *this;
    }

    static inline constexpr T _nd_gcd(const T a,const T b) {
        return (b != 0L) ? _nd_gcd(b,a%b) : (a);
    }

    template <const bool check=false> void _nd_reduce(void) {
        const T gcd = _nd_gcd(num,den);
        if (gcd > 0L) {
            if (check && (num % gcd) != 0L && (den % gcd) != 0L)
                throw std::runtime_error("reduce gcd failed");

            num /= gcd;
            den /= gcd;

            if (check) {
                const T chk_gcd = _nd_gcd(num,den);
                if (chk_gcd != T(1)) // unsigned
//              if (chk_gcd < -1L || chk_gcd > 1L) // signed
                    throw std::runtime_error("reduce gcd ineffective");
            }
        }
    }
};

struct ClockTracking {
    // in case we need to use a different type later
    typedef uint64_t clock_rational_num_t;
    using clock_rational = DRational<clock_rational_num_t>;

    clock_rational                  clock_rate_ms;                      // clock rate in 1ms ticks
    Bitu                            pic_tick_start;
    double                          pic_start;                          // dot clock counts from this pic_index()
    double                          clock_to_tick;                      // pic_index() * clock_to_tick = ticks
    int64_t                         count_from = 0;                     // first tick count
    int64_t                         count = 0;                          // last computed count

    ClockTracking() : clock_rate_ms(1) { _clock_to_tick_update(); _pic_start_init(); }
    ClockTracking(const clock_rational_num_t rate) : clock_rate_ms(rate) { _clock_to_tick_update(); _pic_start_init(); }
    ClockTracking(const clock_rational_num_t rate_n,const clock_rational_num_t rate_d) : clock_rate_ms(rate_n,rate_d) { _clock_to_tick_update(); _pic_start_init(); }
    ClockTracking(const clock_rational rate) : clock_rate_ms(rate) { _clock_to_tick_update(); _pic_start_init(); }

    void _clock_to_tick_update(void) {
        clock_to_tick = double(clock_rate_ms.num) / double(clock_rate_ms.den);
    }

    void _pic_start_init(void) {
        pic_tick_start = PIC_Ticks;
        pic_start = PIC_TickIndex();
    }

    void pic_tick_complete(void) {
        /* call upon completion of a 1ms tick */
        if ((PIC_Ticks - pic_tick_start) >= clock_rate_ms.den) {
            pic_tick_start += clock_rate_ms.den;
            count_from += clock_rate_ms.num;
        }
    }

    void reset_counter(void) {
        for (unsigned int i=0;i < 4;i++)
            pic_tick_complete();

        /* reset counter to zero without missing a tick by setting count_from so that the next update_count() would return zero */
        count_from = -get_count_rel();
        count = 0;
    }

    void reset_counter_hard(void) {
        /* do a hard reset, resetting the counter and setting the time base to now.
         * this will lose some tick precision because the next tick will happen now no matter the amount of time
         * from what would have been the next tick. use reset_counter() if clock tick accuracy is required across counter resets. */
        _pic_start_init();
        count_from = count = 0;
    }

    void reset_tick_base(void) {
        /* maintain tick count, reset time base to now.
         * this will lose some tick precision because the next tick will happen now no matter the amount of time
         * from what would have been the next tick. */
        update_count();
        _pic_start_init();
        count_from = count;
    }

    void set_rate(const uint64_t rate) {
        reset_tick_base();
        clock_rate_ms = rate;
        _clock_to_tick_update();
    }

    void set_rate(const clock_rational rate) {
        reset_tick_base();
        clock_rate_ms = rate;
        _clock_to_tick_update();
    }

    inline double pic_index(void) const {
        /* NTS: PIC_FullIndex() is PIC_Ticks + PIC_TickIndex().
         * Keep the whole part small so floating point precision does not gradually decay with larger and larger numbers. */
        return double(PIC_Ticks - pic_tick_start) + PIC_TickIndex() - pic_start;
    }

    inline int64_t get_count_rel(void) const {
        return int64_t(pic_index() * clock_to_tick);
    }

    inline int64_t get_count(void) const {
        return get_count_rel() + count_from;
    }

    void update_count(void) {
        count = get_count();
    }

    inline double count_duration(int64_t t) const {
        return double(t) / clock_to_tick;
    }
};

// Ref: CGA 80x25
//      dot_clock_per_char_clock = 8
//      shift_register_pixels = 8
//      output_pixel_emit = VGAPixelEmit::x1
//
// Ref: CGA 40x25 text / 320x200x4 graphics
//      dot_clock_per_char_clock = 16
//      shift_register_pixels = 8
//      output_pixel_emit = VGAPixelEmit::x2
//
// Ref: CGA 640x200x2 graphics
//      dot_clock_per_char_clock = 16
//      shift_register_pixels = 16
//      output_pixel_emit = VGAPixelEmit::x1

class VGACRTCDAC {
public:
    VGACRTCDAC_Dim_H                hd;
    VGACRTCDAC_Dim                  vd;
    VGACRTCDACStatus_Dim            hs;
    VGACRTCDACStatus_Dim_V          vs;

    VGACRTCDAC_ShiftReg             shiftreg;

    double                          frame_raw_start = 0;
    double                          dot_clock_src_hz = 0;   // dot clock frequency crystal frequency
    unsigned int                    dot_clock_divide = 1;   // dot clock divider
    unsigned int                    frame_count = 0;        // frame count
    unsigned int                    frame_raw_pixels = 0;   // raw dot clock pixels since frame_raw_start
    unsigned int                    frame_raw_pixels_render = 0;// raw dot clock pixels since frame_raw_start, render position
    unsigned int                    frame_raw_pixels_total = 0;// raw dot clock pixels per frame
    signed char                     frame_field = -1;       // frame field (-1 if not interlaced)

    inline unsigned int compute_frame_raw_pixel_total(void) const {
        /* TODO: Half a frame if interlaced */
        return hd.total.pixels * vd.total.pixels;
    }

    inline double dot_clock_hz(void) const {
        return dot_clock_src_hz / dot_clock_divide;
    }
    inline double frame_duration_base(const unsigned int m) const {
        return (double(frame_raw_pixels_total) * double(m * dot_clock_divide)) / dot_clock_src_hz;
    }
    inline double frame_duration_ms(void) const {
        return frame_duration_base(1000);
    }
    inline double frame_duration_s(void) const {
        return frame_duration_base(1);
    }
    inline double frame_rate(void) const {
        return dot_clock_hz() / double(frame_raw_pixels_total);
    }
};

VGACRTCDAC                          vga_crtc;

Bit32u ExpandTable[256];
double vga_force_refresh_rate = -1;

void VGA_SetModeNow() {
    VGA_SetupHandlers();
    VGA_StartResize(0);
}

void VGA_SetMode() {
    VGA_SetupHandlers();
    VGA_StartResize();
}

void VGA_DetermineMode(void) {
    VGA_SetMode();
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

void VGA_SetCursorAddr(Bitu addr) {
    vga.config.cursor_start = (Bit16u)addr;
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

    vga.mem.memsize = _KB_bytes(16); // NTS: 4KB planar bytes x 4 planes
    vga.mem.memmask = vga.mem.memsize - 1u;

    LOG(LOG_VGA,LOG_NORMAL)("Video RAM: %uKB",vga.mem.memsize>>10);

    VGA_SetupMemory();      // memory is allocated here

    VGA_StartResize();
}

void VGASetPalette(Bit8u index,Bit8u r,Bit8u g,Bit8u b);

static Bit8u text_palette[16][3]=
{
  {0x00,0x00,0x00},{0x00,0x00,0x2a},{0x00,0x2a,0x00},{0x00,0x2a,0x2a},{0x2a,0x00,0x00},{0x2a,0x00,0x2a},{0x2a,0x15,0x00},{0x2a,0x2a,0x2a},
  {0x15,0x15,0x15},{0x15,0x15,0x3f},{0x15,0x3f,0x15},{0x15,0x3f,0x3f},{0x3f,0x15,0x15},{0x3f,0x15,0x3f},{0x3f,0x3f,0x15},{0x3f,0x3f,0x3f}
};

void VGA_Init() {
    Bitu i;

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

    for (i=0;i < 16;i++)
        VGASetPalette(i,text_palette[i][0]<<2u,text_palette[i][1]<<2u,text_palette[i][2]<<2u);

    VGA_DAC_UpdateColorPalette();

    AddVMEventFunction(VM_EVENT_RESET,AddVMEventFunctionFuncPair(VGA_Reset));
}

static inline Bitu VGA_Generic_Read_Handler(PhysPt planeaddr,PhysPt rawaddr,unsigned char plane) {
    const unsigned char hobit_n = 14u;

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

static inline void VGA_Generic_Write_Handler(PhysPt planeaddr,PhysPt rawaddr,Bit8u val) {
    const unsigned char hobit_n = 14u;
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
        VGA_Generic_Write_Handler(start, start, val);
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

#if defined(_MSC_VER)
# pragma warning(disable:4244) /* const fmath::local::uint64_t to double possible loss of data */
# pragma warning(disable:4305) /* truncation from double to float */
#endif

double vga_fps = 70;
double vga_mode_time_base = -1;
int vga_mode_frames_since_time_base = 0;

typedef Bit8u * (* VGA_Line_Handler)(Bitu vidstart, Bitu line);

static VGA_Line_Handler VGA_DrawLine;
static Bit8u TempLine[SCALER_MAXWIDTH * 4 + 256];

extern Bit8u int10_font_16[256 * 16];

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
        // choose foreground color if blinking not set for this cell or blink on
        Bitu foreground = (vga.draw.blink || (!(attr&0x80)))?
            (attr&0xf):background;
        // underline: all foreground [freevga: 0x77, previous 0x7]
        if (GCC_UNLIKELY(((attr&0x77) == 0x01) &&
            (vga.crtc.underline_location&0x1f)==line))
                background = foreground;
        {
            font <<=1; // 9 pixels
            // extend to the 9th pixel if needed
            if ((font&0x2) &&
                (chr>=0xc0) && (chr<=0xdf)) font |= 1;
            for (Bitu n = 0; n < 9; n++) {
                if (card == MCH_VGA)
                    *draw++ = vga.dac.xlat32[(font&0x100)? foreground:background];

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
            Bitu index = (Bitu)attr_addr * 9u;
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
    FontMask[1]=((unsigned int)(vga.draw.cursor.count >> 4u) & 1u) ?
        0 : 0xffffffff;
    /* if blinking is enabled, 'blink' will toggle between true
     * and false. Otherwise it's true */
    vga.draw.blink = (unsigned int)(vga.draw.cursor.count >> 4u);

    vga.draw.linear_base = vga.mem.linear;
    vga.draw.planar_mask = vga.mem.memmask >> 2;

    // add the draw event
    if (GCC_UNLIKELY(vga.draw.lines_done < vga.draw.lines_total)) {
        LOG(LOG_VGAMISC,LOG_NORMAL)( "Lines left: %d", 
                (int)(vga.draw.lines_total-vga.draw.lines_done));
        PIC_RemoveEvents(VGA_DrawSingleLine);
        vga_mode_frames_since_time_base++;
        RENDER_EndUpdate(true);
    }
    vga.draw.lines_done = 0;
    PIC_AddEvent(VGA_DrawSingleLine,(float)(vga.draw.delay.htotal/4.0));
}

void VGA_SetupDrawing(Bitu /*val*/) {
    // user choosable special trick support
    // multiscan -- zooming effects - only makes sense if linewise is enabled
    // linewise -- scan display line by line instead of 4 blocks
    // keep compatibility with other builds of DOSBox for vgaonly.
    vga.draw.render_step = 0;
    vga.draw.render_max = 1;

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
    {
        vga.draw.blocks=width;
        // 9-pixel wide
        pix_per_char = 9;
        VGA_DrawLine = VGA_TEXT_Xlat32_Draw_Line;
        bpp = 32;
    }
    width *= pix_per_char;

    vga.draw.lines_total=height;
    vga.draw.line_length = width * ((bpp + 1) / 8);
    vga.draw.clock = clock;

    double vratio = ((double)width)/(double)height; // ratio if pixels were square

    // the picture ratio factor
    double scanratio =  ((double)hdend/(double)(htotal-(hrend-hrstart)))/
                        ((double)vdend/(double)(vtotal-(vrend-vrstart)));
    double scanfield_ratio = (4.0/3.0) / scanratio;

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

