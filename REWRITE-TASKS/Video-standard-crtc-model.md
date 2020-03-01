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
EGA/VGA standard graphics modes, except 256-color mode, will use pixelsperclock == 8.
VGA 256-color mode will use pixelsperclock == 16 because of how 256-color mode is rendered in hardware, in which the output is latched to the DAC every other dot clock and shifted 4 bits at a time behind the scenes. No, really, check the Hackipedia VGA register dumps, that's how it works. 8 pixels will be doubled to 16, unless the 256-color bit is disabled, in which case the 4-bit shifting behavior is made apparent as a strange psuedo 256-color 640x200 mode on most VGA hardware.

_Additional study needs to be done on what happens if EGA/VGA graphics modes are programmed with 9 pixels per clock instead of 8._

SVGA chipsets seem to define the higher resolution modes, including 15/16bpp, 24bpp, and 32bpp by character clocks, and will use whatever pixelsperclock the hardware is wired to use for those modes.

MCGA is unknown at this time.

PC-98 will use pixelsperclock == 16 if 2.5MHz GDC clock, pixelsperclock == 8 if 5MHz GDC clock. The master and slave GDCs will be given independent pixelsperclock due to the way hardware connects the two independent video outputs together. Each GDC will be given it's own state regarding hsync/vsync and raster position to better emulate the scrambled display that occurs if the master and slave are not genlocked or misprogrammed. Based on real hardware, the slave GDC does not generate hsync/vsync pulses and therefore the master GDC's output is still stable when the two are de-synchronized. _TODO: confirm this._

# Standard character clock model: character clock shift register output to DAC

Emulation renders one character clock to the shiftregister and the shift register is written to emulator display output.

pixeltype_t shiftregister[pixelsperclock];

unsigned int shiftregisterpos = 0;

When shiftregisterpos >= pixelclock, emulation will be called on to render the next character clock.
Note that graphics modes are also subject to the character clock and will render some number of pixels per character clock, be it 1, 2, 4, 8, 16, 32, etc.
This design ensures emulation that is close to how most hardware functions.

