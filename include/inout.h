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


#ifndef DOSBOX_INOUT_H
#define DOSBOX_INOUT_H

#include <stdio.h>
#include <stdint.h>

#define IO_MAX (64*1024+3)

#define IO_MB	0x1
#define IO_MW	0x2
#define IO_MD	0x4
#define IO_MA	(IO_MB | IO_MW | IO_MD )

typedef Bitu IO_ReadHandler(Bitu port,Bitu iolen);
typedef void IO_WriteHandler(Bitu port,Bitu val,Bitu iolen);

extern IO_WriteHandler * io_writehandlers[3][IO_MAX];
extern IO_ReadHandler * io_readhandlers[3][IO_MAX];

void IO_RegisterReadHandler(Bitu port,IO_ReadHandler * handler,Bitu mask,Bitu range=1);
void IO_RegisterWriteHandler(Bitu port,IO_WriteHandler * handler,Bitu mask,Bitu range=1);

void IO_FreeReadHandler(Bitu port,Bitu mask,Bitu range=1);
void IO_FreeWriteHandler(Bitu port,Bitu mask,Bitu range=1);

void IO_InvalidateCachedHandler(Bitu port,Bitu range=1);

void IO_WriteB(Bitu port,Bit8u val);
void IO_WriteW(Bitu port,Bit16u val);
void IO_WriteD(Bitu port,Bit32u val);

Bit8u IO_ReadB(Bitu port);
Bit16u IO_ReadW(Bitu port);
Bit32u IO_ReadD(Bitu port);

static const Bitu IOMASK_ISA_10BIT = 0x3FFU; /* ISA 10-bit decode */
static const Bitu IOMASK_ISA_12BIT = 0xFFFU; /* ISA 12-bit decode */
static const Bitu IOMASK_FULL = 0xFFFFU; /* full 16-bit decode */

/* WARNING: Will only produce a correct result if 'x' is a nonzero power of two.
 * For use with IOMASK_Combine.
 *
 * A device with 16 I/O ports would produce a range mask of:
 *
 *       ~(16 - 1) = ~15 = ~0xF = 0xFFFFFFF0
 *
 *       or
 *
 *       ~(0x10 - 1) = ~0xF = 0xFFFFFFF0
 *
 */
static inline constexpr Bitu IOMASK_Range(const Bitu x) {
    return ~((Bitu)x - (Bitu)1U);
}

/* combine range mask with IOMASK value.
 *
 * Example: Sound Blaster 10-bit decode with 16 I/O ports:
 *
 *     IOMASK_Combine(IOMASK_ISA_10BIT,IOMASK_Range(16));
 *
 */
static inline constexpr Bitu IOMASK_Combine(const Bitu a,const Bitu b) {
    return a & b;
}

/* Classes to manage the IO objects created by the various devices.
 * The io objects will remove itself on destruction.*/
class IO_Base{
protected:
	bool installed;
	Bitu m_port, m_mask/*IO_MB, etc.*/, m_range/*number of ports*/;
public:
	IO_Base() : installed(false), m_port(0), m_mask(0), m_range(0) {};
};
class IO_ReadHandleObject: private IO_Base {
public:
    IO_ReadHandleObject() : IO_Base() {};
	void Install(Bitu port,IO_ReadHandler * handler,Bitu mask,Bitu range=1);
	void Uninstall();
	~IO_ReadHandleObject();
};
class IO_WriteHandleObject: private IO_Base{
public:
    IO_WriteHandleObject() : IO_Base() {};
	void Install(Bitu port,IO_WriteHandler * handler,Bitu mask,Bitu range=1);
	void Uninstall();
	~IO_WriteHandleObject();
};

static INLINE void IO_Write(Bitu port,Bit8u val) {
	IO_WriteB(port,val);
}
static INLINE Bit8u IO_Read(Bitu port){
	return IO_ReadB(port);
}

#endif
