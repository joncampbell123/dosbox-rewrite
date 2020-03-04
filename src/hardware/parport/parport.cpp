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
/* FIXME: At some point I would like to roll the Disney Sound Source emulation into this code */

#include <string.h>
#include <ctype.h>

#include "dosbox.h"

#include "support.h"
#include "inout.h"
#include "pic.h"
#include "setup.h"
#include "timer.h"
#include "bios.h"					// SetLPTPort(..)
#include "hardware.h"				// OpenCaptureFile

#include "parport.h"
#include "directlpt_win32.h"
#include "directlpt_linux.h"
#include "printer_redir.h"
#include "filelpt.h"
#include "dos_inc.h"

bool device_LPT::Read(Bit8u * data,Bit16u * size) {
	return true;
}

bool device_LPT::Write(const Bit8u * data,Bit16u * size) {
	return true;
}

bool device_LPT::Seek(Bit32u * pos,Bit32u type) {
	return true;
}

bool device_LPT::Close() {
	return false;
}

Bit16u device_LPT::GetInformation(void) {
	return 0x80A0;
}

device_LPT::device_LPT(Bit8u num, class CParallel* pp) {
}

device_LPT::~device_LPT() {
}

static void Parallel_EventHandler(Bitu val) {
}

void CParallel::setEvent(Bit16u type, float duration) {
}

void CParallel::removeEvent(Bit16u type) {
}

void CParallel::handleEvent(Bit16u type) {
}

static Bitu PARALLEL_Read (Bitu port, Bitu iolen) {
	return 0xff;
}

static void PARALLEL_Write (Bitu port, Bitu val, Bitu) {
}

CParallel::CParallel(CommandLine* cmd, Bitu portnr, Bit8u initirq) {
}

void CParallel::registerDOSDevice() {
}

void CParallel::unregisterDOSDevice() {
}

CParallel::~CParallel(void) {
}

Bit8u CParallel::getPrinterStatus()
{
	return 0;
}

#include "callback.h"

void RunIdleTime(Bitu milliseconds)
{
}

void CParallel::initialize()
{
}

Bitu bios_post_parport_count() {
	return 0;
}

void BIOS_Post_register_parports() {
}

void PARALLEL_Destroy (Section * sec) {
}

void PARALLEL_OnPowerOn (Section * sec) {
}

void PARALLEL_OnDOSKernelInit (Section * sec) {
}

void PARALLEL_OnDOSKernelExit (Section * sec) {
}

void PARALLEL_OnReset (Section * sec) {
}

void PARALLEL_Init () {
}

