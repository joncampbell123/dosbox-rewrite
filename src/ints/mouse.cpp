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

/* TODO: If biosps2=true and aux=false, also allow an option (default disabled)
 *       where if set, we don't bother to fire IRQ 12 at all but simply call the
 *       device callback directly. */

#include <string.h>
#include <math.h>


#include "dosbox.h"
#include "callback.h"
#include "mem.h"
#include "regs.h"
#include "cpu.h"
#include "mouse.h"
#include "pic.h"
#include "inout.h"
#include "int10.h"
#include "bios.h"
#include "dos_inc.h"
#include "support.h"
#include "setup.h"
#include "control.h"

#if defined(_MSC_VER)
# pragma warning(disable:4244) /* const fmath::local::uint64_t to double possible loss of data */
#endif

void DisableINT33() {
}

void MOUSE_Unsetup_DOS(void) {
}

void MOUSE_Unsetup_BIOS(void) {
}

#if !defined(C_SDL2)
bool GFX_IsFullscreen(void);
#else
static inline bool GFX_IsFullscreen(void) {
    return false;
}
#endif

void Mouse_CursorMoved(float xrel,float yrel,float x,float y,bool emulate) {
}

uint8_t Mouse_GetButtonState(void) {
    return 0;
}

void Mouse_ButtonPressed(Bit8u button) {
}

void Mouse_ButtonReleased(Bit8u button) {
}

void Mouse_BeforeNewVideoMode(bool setmode) {
}

void Mouse_AfterNewVideoMode(bool setmode) {
}

void MOUSE_OnReset(Section *sec) {
}

void MOUSE_ShutDown(Section *sec) {
}

void BIOS_PS2MOUSE_ShutDown(Section *sec) {
}

void BIOS_PS2Mouse_Startup(Section *sec) {
}

void MOUSE_Startup(Section *sec) {
}

void MOUSE_Init() {
}

bool MOUSE_IsHidden()
{
    return true;
}

bool MOUSE_IsBeingPolled()
{
    return false;
}

bool MOUSE_HasInterruptSub()
{
    return false;
}

