VGA raster system in DOSBox-X-Rewrite:

Output from VGA emulation will be line-by-line by default.

Provisions will be added to support skipping line-by-line rendering
if the VGA emulation knows that part of the screen has not changed.
Part of this system will include VGA memory read/write keeping track
of video memory "dirty pages" to determine what parts of the screen
need redrawing. Memory writes, if they are within range of the VGA
raster, will trigger raster rendering up to that point in time, then
apply memory write.

If software rendering, the VGA scanlines may be run through horizontal
and vertical interpolation routines on a line by line basis to a
surface on the display. If OpenGL, the VGA scanlines will be emitted
to a texture as-is and the GPU will scale the video to fit.

Provisions will be made to allow a user interface to render atop the
VGA output. If OpenGL, the UI will exist as its own texture atop the
VGA texture.

Software rendering is planned to allow an alternate interpolation mode
that deliberately creates a slight "scanline effect" for lower resolution
modes.

Future plans of VGA emulation also include SVGA chipsets that support
interlaced VGA modes. VGA scanline output will be written to support
field-wise rendering. A software renderer and OpenGL pixel shader
are planned to implement a bob filter if desired.

VGA monitor emulation will support by default what will be referred to
as the "naive" mode (supports anything) similar to what DOSBox/DOSBox-X
does now, but will also emulate:

- CRT displays, fixed frequency monitors that require specific sync rates
  or will distort/roll the image, and multisync monitors that prefer
  some specific sync rates.

- LCD flat panel displays, that either display VGA at what they *think*
  is the mode, or display "not support"

- Laptop flat panels, that emulate VGA but lock the refresh rate to
  one setting (usually 60Hz), and either stretch or center VGA output
  to fit the panel.

- CGA composite NTSC. I will match as closely as possible the gamma/colors
  that DOSBox has worked so hard to emulate, but will try to make more
  accurate NTSC emulation. Emulation will include monochrome display
  emulation, such as the amber display in old IBM PC "luggables" where
  color becomes a wavy pattern on the screen.

VGA scanline optimizations:

- If a doublescan mode is involved (i.e. 320x200) the code will emit the
  scanline once with instruction to repeat the scanline as directed by
  the max scanline register of VGA, UNLESS the VGA emulation knows that
  software is writing to the screen or changing the display in a way that
  affects the raster (i.e. color palette), where it will fall back to
  linewise raster.

