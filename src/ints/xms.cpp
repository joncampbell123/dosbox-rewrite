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


#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "dosbox.h"
#include "callback.h"
#include "mem.h"
#include "regs.h"
#include "dos_inc.h"
#include "setup.h"
#include "inout.h"
#include "xms.h"
#include "bios.h"
#include "cpu.h"
#include "control.h"

bool XMS_IS_ACTIVE() {
	return false;
}

bool XMS_HMA_EXISTS() {
	return false;
}

bool XMS_Active(void) {
    return false;
}

void XMS_Init() {
}

