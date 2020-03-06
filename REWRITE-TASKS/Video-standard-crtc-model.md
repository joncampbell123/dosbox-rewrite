# Standard character clock model: dot clock and character clock model

double dotclockfreq = dot clock frequency

unsigned int pixelsperclock = pixels per character

constexpr double charclockfreq = dotclockfreq / pixelsperclock

Pixel doubling will be handled in one of two ways depending on how the actual hardware behaves.

In the CGA case, dotclockfreq will always be the 14MHz rate coming from the ISA bus. For 80x25 mode, pixelsperclock == 8 and the shift register is 8 pixels wide (as hardware seems to behave). For all other modes, pixelsperclock == 16.

40x25 alphanumeric and 320x200 4-color mode will be pixelsperclock == 16 and 8 pixels will be doubled to shift register.
640x200 2-color mode will be pixelsperclock == 16 and 16 pixels will not be doubled to the shift register.

The reason for this logic for CGA should be apparent according to the horizontal timing values written to the CRTC for each mode.

For MDA and Hercules (80x25 text), pixelsperclock == 9 and pixels will not be doubled. _TODO: confirm this._
For Hercules graphics mode, pixelsperclock == 8 and pixels will not be doubled. _TODO: confirm this._

At this time PCjr is assumed to have the same constraints as CGA, but _additional analysis is needed_

Tandy is unknown at this time.

EGA/VGA standard alphanumeric modes will use pixelsperclock == 8 or pixelsperclock == 9 according to register settings.

EGA/VGA standard graphics modes will use pixelsperclock == 8. If a VGA register bit, normally reserved for 256-color mode, is set, then only every other pixel will sent and doubled to the shift register, as observed on real hardware.

EGA/VGA standard modes will use a 25MHz/28MHz dot clock, unless a bit is set to divide the dot clock by two which will not change the content sent to the shift register but will halve the rate the shift register is emptied to the DAC.

Note that 320x200 256-color VGA mode is the only 320-column graphics mode that does NOT divide the dot clock by two due to behavior behind the scenes in which 4 bits are shifted per dot clock and latched to output every other dot clock. This can be confirmed from the horizontal CRTC timing written to registers that match 80-column alphanumeric and 640-pixel wide graphics modes.

VGA 256-color mode will render 4 pixels doubled to 8, with 8 pixels per clock, to reflect how actual hardware behaves. If the VGA registers are programmed to leave on 256-color mode, but the bit is cleared that latches only every other dot clock, then 8 pixels will be rendered to reflect the 4-bit shifting behavior behind the scenes that makes 256-color mode possible as a strange 640x200 256-color mode.

_Additional study needs to be done on what happens if EGA/VGA graphics modes are programmed with 9 pixels per clock instead of 8._

SVGA chipsets seem to define the higher resolution modes, including 15/16bpp, 24bpp, and 32bpp by character clocks, and will use whatever pixelsperclock the hardware is wired to use for those modes.

More recent SVGA chipsets may allow granular (pixel level) controls of active display and hsync timing, and may separate standard VGA registers entirely from MMIO registers that provide finer control. VGA registers may disappear entirely or partially in SVGA modes. Some chipsets, such as those by Intel, provide legacy VGA graphics by enabling a "VGA engine" for one of the pipelines of the chipset. The VGA engine is disabled when setting up SVGA modes and therefore VGA registers do not exist or have no effect. This is why CRTC values will update a general raster engine instead of counting directly.

For some older SVGA chipsets that follow the "highcolor RAMDAC" model, the pixelsperclock will remain the same but pixels will be transformed (and the resolution halved) to convert VGA digital output into highcolor 15bpp/16bpp output, if the appropriate register values are set up (Tseng cards?). Emulation of other chipsets may require emulating the half-hearted "emulation" the SVGA chipset does of the highcolor RAMDAC register set (Chips & Technologies). A good example of a DOS program setting up highcolor directly on a specific chipset is MFX's "Transgression 2" demo, though it seems to do it slightly wrong.

MCGA is unknown at this time.

PC-98 will use pixelsperclock == 16 if 2.5MHz GDC clock, pixelsperclock == 8 if 5MHz GDC clock. The master and slave GDCs will be given independent pixelsperclock due to the way hardware connects the two independent video outputs together. In normal operation, they are genlocked such that the master GDC output, the text, overlays the slave GDC output, the graphics. Each GDC will be given it's own state regarding hsync/vsync and raster position to better emulate the scrambled display that occurs if the master and slave are not genlocked or misprogrammed. Based on real hardware, the slave GDC does not generate hsync/vsync pulses and therefore the master GDC's output is still stable when the two are de-synchronized. _TODO: confirm this._

It is intended that video emulation may simulate laptop SVGA chipsets that ignore CRTC timing except to map active display to a subregion of the LCD display, and lock the video timing to a fixed rate appropriate for the LCD (usually 60Hz). The usual effect is that the VGA screen, regardless of video mode or even custom modes programmed by DOS games, is centered in the LCD display with a black border. More recent chipsets support auto-scaling of the display to fill the screen. More recent SVGA chipsets permit bilinear interpolation while older late 1990s chipsets provide nearest neighbor upscaling.

In the scaling cases the active display given by the guest is scaled up to fit the screen. Depending on the chipset, horizontal sync may be emulated either per unscaled scanline (Intel), or typical of older chipsets, per scanline of the upscaled output (Chips & Tech).

In all laptop LCD cases, the external VGA connector provided by the laptop will match exactly the video data and sync timing of the LCD display if the laptop is instructed to emit simultaneously to LCD and external VGA. This is true even of laptops made in the 2000s and 2010s, especially Intel chipsets, even if the external connection is HDMI. If the laptop is instructed to send video only to external VGA, then the signal will have normal VGA timings and sync signals. The simultaneous mode rule may not apply if the guest OS is able to program the chipset to multi-monitor configuration, such as a desktop where the left half is on the LCD display and the right half is on a VGA monitor.

# Standard character clock model: character clock shift register output to DAC

Emulation renders one character clock to the shiftregister and the shift register is written to emulator display output.

pixeltype_t shiftregister[pixelsperclock];

unsigned int shiftregisterpos = 0;

When shiftregisterpos >= pixelclock, emulation will be called on to render the next character clock.
Note that graphics modes are also subject to the character clock and will render some number of pixels per character clock, be it 1, 2, 4, 8, 16, 32, etc.
This design ensures emulation that is close to how most hardware functions.

# Standard memory layout model: Planar memory where appropriate, packed otherwise

MDA/CGA memory will be formatted as an array of 16-bit little Endian WORDs, which depending on the mode, is either the combined character/attribute byte in alphanumeric mode or a bit string containing pixel data to be written to the shift register. 8 pixels per character clock (doubled to 16) for 320x200 and 16 pixels per character clock (not doubled) for 640x200.

EGA/VGA standard modes will be formatted as an array of 32-bit little Endian DWORDs, within which 4 8-bit bitplanes are contained. While the 4 bitplanes are plainly obvious in the 16-color modes (as programmers are forced to work with them), the bitplanar layout underpins every standard EGA/VGA mode, including alphanumeric modes, CGA modes, and 256-color mode.

EGA/VGA text mode uses bitplane 0 for character data and bitplane 1 for attribute data. The character generator reads font data from bitplane 2, and bitplane 3 is unused. The hardware maps CPU memory access with odd/even mode enabled so that the CPU sees a CGA/MDA compatible layout where even bytes are character data and odd bytes are attribute data. Odd/even mode maps even bytes to bitplane 0 and odd bytes to bitplane 1 to accmplish this.

CGA modes on EGA/VGA hardware also implements odd/even mode, which emulates the CGA memory layout across bitplanes 0 and 1. The CRTC is instructed to read across the same as well. For 320x200 4-color modes, the sequencer is programmed to shuffle the 16-bit WORD data to render it in the compatible manner needed for the 4-color mode to display properly. As far as planar layout is concerned, CGA modes are really just the 16-color planar modes with only 1 bitplane (if 640x200) or 2 bitplanes (if 320x200) enabled and the rest are disabled. The unused bitplanes are also masked off through the attribute controller. If desired, a programmer could set up 640x200 2-color mode, then re-enable and unmask the bitplanes to turn it into EGA 640x200 16-color mode.

The 8KB interleave of the original CGA hardware is simulated by a bit in the CRTC that remaps bit 0 of the scanline row counter to bit 13 of the CRTC address. The Maximum scanline register is set to 1, without which bit 0 would never be nonzero and the interleave would never happen. This causes the CRTC to display odd scanlines from the start of the display + 8KB as expected for CGA. This ALSO means a programmer could set up a non-interleaved CGA mode if desired by clearing that bit and then setting the Maximum scanline register to 0. Note that there is another bit in the CRTC to map bit 1 of the scanline row counter to bit 14, which if set could be used to configure the EGA/VGA card to emulate the 4-way memory and display interleave of the Hercules graphics mode given that EGA/VGA can be programmed to respond to 3B0-3BFh and B0000-B7FFFh like MDA compatible hardware.

Set in the CRTC is the doublescan bit (bit 7) of the Maximum scanline register which instructs the VGA to count each outgoing raster scanline twice going to the monitor when counting through the vertical timing programmed into the CRTC registers. To put it another way the CRTC registers regarding vertical timing are set as if a 200-line graphics mode, and then doubled in hardware to the monitor, which is also reflected in port 3DAh, for compatibility with older software. Note that the doublescan bit is NOT set for higher resolution EGA/VGA modes (640x350 or higher), nor is it set for VGA 320x200 256-color mode. If it was, classic DOS programming tricks like clearing the Maximum scanline register to get 320x400 256-color mode would not be possible.

VGA 256-color mode, despite the apparently linear appearance of the framebuffer to the CPU, is mapped across the 4 bitplanes sequentially. This is known as chained mode. On CPU access, bits 1 and 0 are used to select which bitplane to operate on and masked off the memory address sent to the memory chip. In the display circuitry, the CRTC reads all 4 bitplanes in one character clock and serializes each 8-bit value to the DAC in this order: bitplane 0, 1, 2, 3. Notice that bits 1 and 0 are masked off, the address is not shifted right 2 bits to compensate, which is why 256-color mode normally only provides access to 64KB of video RAM and 320x400 mode without extra programming effort will show the 320x200 screen twice. This combined effort maintains the illusion of a linear framebuffer of 8-bit pixels. However, game developers and democoders figured out that if "chained mode" is turned off, and memory were set up to operate like 16-color planer mode, it is possible to operate on up to 4 pixels at a time and access this way the full 256KB of video RAM on the VGA card. Several games in the 1990s, which otherwise would not have required the unchained mode, used it regardless since having more than 64KB permitted flicker-free methods of animation like page flipping on vsync (DOOM, Wolfenstein 3D, and other iD software for example).

The 32-bit DWORD format of the planar data also permits high performance emulation of EGA/VGA planar raster operations, as already implemented by DOSBox SVN and it's forks and refined in DOSBox-X.

PC-98 emulation will use an array of 16-bit WORDs for the master GDC and text display, since that is the format of the display memory. The 16-bit WORDs for character data vs the 16-bit WORDs for attribute data will be stored as separate parallel arrays since there is no case in the hardware for any kind of combined operation across both. Character data is an array of 16-bit WORDs at A000:0000 and attribute data is an array of 16-bit WORDs (with only the low 8 valid) at A200:0000. The last 32 bytes of attribute RAM are inaccesible to the CPU and are occupied by non-volatile RAM that is normally read-only unless writing enabled through another I/O port. The NVRAM contains BIOS settings and configuration on newer hardware that do not fit within the DIP switch settings readable through the 8255 at ports 31h-37h.

PC-98 graphics will be stored as an array of 64-bit QWORDs, 16-bits per plane. For the same reason as planar VGA, the storage will permit higher performance code to carry out raw, tile, and EGC binary and shift operations. The 64-bit format reflects the 16-bit nature of the hardware, and the way that planar modes appear to be actually stored. On actual hardware, switching from 8/16-color planar to 256-color packed reveals that the 16-bit WORDs of the bitplanes are stored such that bitplanes E G R B become B G R E sequentially in memory. However implementation of that detail is not high priority, I have yet to find a PC-98 game that relies on that quirk. If a game does use 256-color packed mode it usually does not rely on any particular behavior regarding old/new format and layout. This mapping behavior may be the PC-98 256-color equivalent of VGA 256-color chained mode, though with 16-bit planar data instead of 8-bit planar data. _This description will change if any of this is wrong_.

    E      G      R      B
    E0000h B8000h B0000h A8000h     8/16 planar memory address of a WORD of data
    A8006h A8002h A8004h A8000h     256-color packed memory address of the same WORD of data
    
    ------------------------------------------------
    
    A0000h B8000h B0000h A8000h     8/16 planar, (memory address & 0x18000) | 0xA0000
    
    ------------------------------------------------
    
    0      3      2      1          8/16 planar, (memory address >> 15) & 3
    3      2      1      0          8/16 planar, ((memory address >> 15) + 3) & 3
    3      1      2      0          8/16 planar, swap_two_bits(((memory address >> 15) + 3) & 3)
    
    8/16 planar E G R B (3 1 2 0) becomes 256-color packed B G R E (0 1 2 3)
    
    ------------------------------------------------
    
    A8006h A8002h A8004h A8000h     A8000h + (swap_two_bits(((memory address >> 15) + 3) & 3) * 2)
    
    ------------------------------------------------
    
    0      1      2      3          Decimal value
    00b    01b    10b    11b        Binary value
    00b    10b    01b    11b        Binary value, bits swapped
    0      2      1      3          Decimal value of binary value after bits swapped

# Standard video timing model: Separation of VGA registers from timing tracking

To help keep the code clean, the rewrite will track timing from raster state that is updated from VGA registers, but will not directly count from VGA registers. This is to avoid the mess seen in current DOSBox SVN code and forks which uses the VGA register values directly all over the place, and to simplify video raster emulation. Note that the mess makes it more difficult to implement additional SVGA cards and requires other SVGA hardware to patch into S3 registers, or the mode setting code to implement a case for each new SVGA card.

The standard model will count dot clock vs pixels horizontally, and scanlines vertically. It is up to VGA emulation to fill in the pixel counts properly, and track shift register vs pixel output properly when asked.

The standard model will allow an extra half a scanline to emulate interlaced video output, in which case the current field will be tracked as well.

horizontal refresh rate = dot clock / htotal
vertical refresh rate = dot clock / (htotal * vtotal)
vertical refresh rate interlaced = dot clock / ((htotal * vtotal) + htotal interlaced half a scan line)
vertical frame rate interlaced = dot clock / (((htotal * vtotal) + htotal interlaced half a scan line) * 2)

Standard simulation will signal hsync when hsync happens and vsync when vsync happens. Blanking will be emulated as well.
For EGA/VGA, non-blanking areas outside the active display area will be filled with the overscan color as set in the Attribute Controller.

VGA emulation can change htotal during active display, which is not recommended, but should be emulated to show the picture distortion that results when a DOS program fiddles with hsync/htotal during active display. Monitor emulation is expected to render a distorted display in the manner that a CRT would handle it.

# Standard video memory model: Video RAM wait states

CGA video memory uses wait state signals to prevent the CPU from writing while fetching from video RAM. However 80x25 mode requires twice the bandwidth, while the wait state logic is not changed to reflect that. Noted by a DOSLIB testing utility, is that the CGA snow will occupy every other column, which suggests the wait states are applied every character clock except in 80x25 where wait states only apply every other clock. [@Scalibq notes 20200306-1327-Twitter.txt]

_TODO: Write a DOSLIB utility to measure VRAM writing speed a) display enabled and then b) display disabled/blanked, several times, to try to measure the exact delay caused by the wait states. This test should be done in all CGA modes. Blanking the display should mean no VRAM access by the card and therefore no wait states... I think_

_TODO: Wait states on MDA? EGA? MCGA? VGA (stock IBM PS/2 model 30)?_

