DOSBox-x-rewrite display system will be designed to
talk to a general output system, rather than tied to
SDL or SDL 2.

The intent is the ability to compile DOSBox-X-rewrite
with or without a GUI, as an SDL application, or
something that can use more specialized code to talk
to the display.

Ideas:
- SDL 2
- SDL
- Linux framebuffer
- Linux DRM/KMS
- X11 XLIB
- X11 XCB
- Mac OS Cocoa
- Windows DirectX
- Windows GDI
- OpenGL / OpenGL ES

Mouse and keyboard input will follow the same pattern.

