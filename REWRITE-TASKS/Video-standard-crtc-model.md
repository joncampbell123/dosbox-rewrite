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

In the scaling cases the active display given by the guest is scaled up to fit the screen. Depending on the chipset, horizontal sync may be emulated either per unscaled scanline, or typical of older chipsets, per scanline up the upscaled output. In unscaled cases, older chipsets may report the actual horizontal sync (counting the extra scanlines) in 3DAh which can throw off raster effects (Chips & Tech).

In all laptop LCD cases, the external VGA connector provided by the laptop will match exactly the video data and sync timing of the LCD display if the laptop is instructed to emit simultaneously to LCD and external VGA. This is true even of laptops made in the 2000s and 2010s, especially Intel chipsets. If the laptop is instructed to send video only to external VGA, then the signal will have normal VGA timings and sync signals. The simultaneous mode rule may not apply if the guest OS is able to program the chipset to multi-monitor configuration, such as a desktop where the left half is on the LCD display and the right half is on a VGA monitor.

# Standard character clock model: character clock shift register output to DAC

Emulation renders one character clock to the shiftregister and the shift register is written to emulator display output.

pixeltype_t shiftregister[pixelsperclock];

unsigned int shiftregisterpos = 0;

When shiftregisterpos >= pixelclock, emulation will be called on to render the next character clock.
Note that graphics modes are also subject to the character clock and will render some number of pixels per character clock, be it 1, 2, 4, 8, 16, 32, etc.
This design ensures emulation that is close to how most hardware functions.

# Standard video timing model: Separation of VGA registers from timing tracking

To help keep the code clean, the rewrite will track timing from raster state that is updated from VGA registers, but will not directly count from VGA registers. This is to avoid the mess seen in current DOSBox SVN code and forks which uses the VGA register values directly all over the place, and to simplify video raster emulation.
