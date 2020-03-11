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
#include <stdlib.h>
#include <stdio.h>
#include "dosbox.h"
#include "control.h"
#include "hardware.h"
#include "setup.h"
#include "support.h"
#include "mem.h"
#include "mapper.h"
#include "pic.h"
#include "mixer.h"
#include "render.h"
#include "cross.h"
#include "timer.h"

#include "riff_wav_writer.h"
#include "avi_writer.h"
#include "rawint.h"

#include <map>

unsigned long PIT_TICK_RATE = PIT_TICK_RATE_IBM;

std::string GetCaptureFilePath(const char * type,const char * ext) {
    return "";
}

FILE * OpenCaptureFile(const char * type,const char * ext) {
    return 0;
}

void CAPTURE_StartCapture(void) {
}

void CAPTURE_StopCapture(void) {
}

void CAPTURE_StartWave(void) {
}

void CAPTURE_StopWave(void) {
}

void CAPTURE_AddImage(Bitu width, Bitu height, Bitu bpp, Bitu pitch, Bitu flags, float fps, Bit8u * data, Bit8u * pal) {
}

void CAPTURE_ScreenShotEvent(bool pressed) {
}

void CAPTURE_MultiTrackAddWave(Bit32u freq, Bit32u len, Bit16s * data,const char *name) {
}

void CAPTURE_AddWave(Bit32u freq, Bit32u len, Bit16s * data) {
}

void CAPTURE_WaveEvent(bool pressed) {
}

static void RawMidiAdd(Bit8u data) {
}

static void RawMidiAddNumber(Bit32u val) {
}

void CAPTURE_AddMidi(bool sysex, Bitu len, Bit8u * data) {
}

void CAPTURE_Destroy(Section *sec) {
}

void CAPTURE_Init() {
}

void HARDWARE_Destroy(Section * sec) {
    (void)sec;//UNUSED
}

void HARDWARE_Init() {
	LOG(LOG_MISC,LOG_DEBUG)("HARDWARE_Init: initializing");

	/* TODO: Hardware init. We moved capture init to it's own function. */
	AddExitFunction(AddExitFunctionFuncPair(HARDWARE_Destroy),true);
}

