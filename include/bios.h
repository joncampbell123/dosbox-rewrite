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

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

#include "regionalloctracking.h"

#ifndef DOSBOX_BIOS_H
#define DOSBOX_BIOS_H

#define BIOS_BASE_ADDRESS_COM1          0x400
#define BIOS_BASE_ADDRESS_COM2          0x402
#define BIOS_BASE_ADDRESS_COM3          0x404
#define BIOS_BASE_ADDRESS_COM4          0x406
#define BIOS_ADDRESS_LPT1               0x408
#define BIOS_ADDRESS_LPT2               0x40a
#define BIOS_ADDRESS_LPT3               0x40c
/* 0x40e is reserved */
#define BIOS_CONFIGURATION              0x410
/* 0x412 is reserved */
#define BIOS_MEMORY_SIZE                0x413
#define BIOS_TRUE_MEMORY_SIZE           0x415
/* #define bios_expansion_memory_size      (*(unsigned int   *) 0x415) */
#define BIOS_KEYBOARD_STATE             0x417
#define BIOS_KEYBOARD_FLAGS1            BIOS_KEYBOARD_STATE
#define BIOS_KEYBOARD_FLAGS1_RSHIFT_PRESSED			(1 << 0)
#define BIOS_KEYBOARD_FLAGS1_LSHIFT_PRESSED			(1 << 1)
#define BIOS_KEYBOARD_FLAGS1_CTRL_PRESSED			(1 << 2)
#define BIOS_KEYBOARD_FLAGS1_ALT_PRESSED			(1 << 3)
#define BIOS_KEYBOARD_FLAGS1_SCROLL_LOCK_ACTIVE		(1 << 4)
#define BIOS_KEYBOARD_FLAGS1_NUMLOCK_ACTIVE			(1 << 5)
#define BIOS_KEYBOARD_FLAGS1_CAPS_LOCK_ACTIVE		(1 << 6)
#define BIOS_KEYBOARD_FLAGS1_INSERT_ACTIVE			(1 << 7)

#define BIOS_KEYBOARD_FLAGS2            0x418
#define BIOS_KEYBOARD_FLAGS2_LCTRL_PRESSED			(1 << 0)
#define BIOS_KEYBOARD_FLAGS2_LALT_PRESSED			(1 << 1)
#define BIOS_KEYBOARD_FLAGS2_SYSTEMKEY_HELD			(1 << 2)
#define BIOS_KEYBOARD_FLAGS2_SUSPENDKEY_TOGGLED		(1 << 3)
#define BIOS_KEYBOARD_FLAGS2_SCROLL_LOCK_PRESSED	(1 << 4)
#define BIOS_KEYBOARD_FLAGS2_NUM_LOCK_PRESSED		(1 << 5)
#define BIOS_KEYBOARD_FLAGS2_CAPS_LOCK_PRESSED		(1 << 6)
#define BIOS_KEYBOARD_FLAGS2_INSERT_PRESSED			(1 << 7)

#define BIOS_KEYBOARD_TOKEN             0x419
/* used for keyboard input with Alt-Number */
#define BIOS_KEYBOARD_BUFFER_HEAD       0x41a
#define BIOS_KEYBOARD_BUFFER_TAIL       0x41c
#define BIOS_KEYBOARD_BUFFER            0x41e
/* #define bios_keyboard_buffer            (*(unsigned int   *) 0x41e) */
#define BIOS_DRIVE_ACTIVE               0x43e
#define BIOS_DRIVE_RUNNING              0x43f
#define BIOS_DISK_MOTOR_TIMEOUT         0x440
#define BIOS_DISK_STATUS                0x441
/* #define bios_fdc_result_buffer          (*(unsigned short *) 0x442) */
#define BIOS_VIDEO_MODE                 0x449
#define BIOS_SCREEN_COLUMNS             0x44a
#define BIOS_VIDEO_MEMORY_USED          0x44c
#define BIOS_VIDEO_MEMORY_ADDRESS       0x44e
#define BIOS_VIDEO_CURSOR_POS	        0x450


#define BIOS_CURSOR_SHAPE               0x460
#define BIOS_CURSOR_LAST_LINE           0x460
#define BIOS_CURSOR_FIRST_LINE          0x461
#define BIOS_CURRENT_SCREEN_PAGE        0x462
#define BIOS_VIDEO_PORT                 0x463
#define BIOS_VDU_CONTROL                0x465
#define BIOS_VDU_COLOR_REGISTER         0x466
/* 0x467-0x468 is reserved */
#define BIOS_LAST_UNEXPECTED_IRQ        0x46b
#define BIOS_TIMER                      0x46c
#define BIOS_24_HOURS_FLAG              0x470
#define BIOS_CTRL_BREAK_FLAG            0x471
#define BIOS_CTRL_ALT_DEL_FLAG          0x472
#define BIOS_HARDDISK_COUNT		0x475
/* 0x474, 0x476, 0x477 is reserved */
#define BIOS_LPT1_TIMEOUT               0x478
#define BIOS_LPT2_TIMEOUT               0x479
#define BIOS_LPT3_TIMEOUT               0x47a
/* 0x47b is reserved */
#define BIOS_COM1_TIMEOUT               0x47c
#define BIOS_COM2_TIMEOUT               0x47d
#define BIOS_COM3_TIMEOUT               0x47e
#define BIOS_COM4_TIMEOUT               0x47f
/* 0x47e is reserved */ //<- why that?
/* 0x47f-0x4ff is unknow for me */
#define BIOS_KEYBOARD_BUFFER_START      0x480
#define BIOS_KEYBOARD_BUFFER_END        0x482

#define BIOS_ROWS_ON_SCREEN_MINUS_1     0x484
#define BIOS_FONT_HEIGHT                0x485

#define BIOS_VIDEO_INFO_0               0x487
#define BIOS_VIDEO_INFO_1               0x488
#define BIOS_VIDEO_INFO_2               0x489
#define BIOS_VIDEO_COMBO                0x48a

#define BIOS_KEYBOARD_FLAGS3            0x496
#define BIOS_KEYBOARD_FLAGS3_HIDDEN_E1			(1 << 0)
#define BIOS_KEYBOARD_FLAGS3_HIDDEN_E0			(1 << 1)
#define BIOS_KEYBOARD_FLAGS3_RCTRL_PRESSED		(1 << 2)
#define BIOS_KEYBOARD_FLAGS3_RALT_PRESSED		(1 << 3)
#define BIOS_KEYBOARD_FLAGS3_ENHANCED_KEYBOARD	(1 << 4)
#define BIOS_KEYBOARD_FLAGS3_NUM_LOCK_FORCED	(1 << 5)
#define BIOS_KEYBOARD_FLAGS3_ID_CHAR_WAS_LAST	(1 << 6)
#define BIOS_KEYBOARD_FLAGS3_ID_READ_IN_PROCESS	(1 << 7)

#define BIOS_KEYBOARD_LEDS              0x497
#define BIOS_KEYBOARD_LEDS_SCROLL_LOCK    (1 << 0)
#define BIOS_KEYBOARD_LEDS_NUM_LOCK       (1 << 1)
#define BIOS_KEYBOARD_LEDS_CAPS_LOCK      (1 << 2)
#define BIOS_KEYBOARD_LEDS_CIRCUS         (1 << 3)
#define BIOS_KEYBOARD_LEDS_ACK            (1 << 4)
#define BIOS_KEYBOARD_LEDS_RESEND         (1 << 5)
#define BIOS_KEYBOARD_LEDS_MODE           (1 << 6)
#define BIOS_KEYBOARD_LEDS_TRANSMIT_ERROR (1 << 7)

#define BIOS_WAIT_FLAG_POINTER          0x498
#define BIOS_WAIT_FLAG_COUNT	        0x49c		
#define BIOS_WAIT_FLAG_ACTIVE			0x4a0
#define BIOS_WAIT_FLAG_TEMP				0x4a1


#define BIOS_PRINT_SCREEN_FLAG          0x500

#define BIOS_VIDEO_SAVEPTR              0x4a8

#define CURSOR_SCAN_LINE_NORMAL			(0x6)
#define CURSOR_SCAN_LINE_INSERT			(0x4)
#define CURSOR_SCAN_LINE_END			(0x7)

//#define BIOS_DEFAULT_IRQ0_LOCATION		(RealMake(0xf000,0xfea5))
//#define BIOS_DEFAULT_IRQ1_LOCATION		(RealMake(0xf000,0xe987))
//#define BIOS_DEFAULT_IRQ2_LOCATION		(RealMake(0xf000,0xff55))
//#define BIOS_DEFAULT_HANDLER_LOCATION		(RealMake(0xf000,0xff53))
//#define BIOS_VIDEO_TABLE_LOCATION		(RealMake(0xf000,0xf0a4))
//#define BIOS_DEFAULT_RESET_LOCATION		(RealMake(0xf000,0xe05b))

extern Bitu BIOS_DEFAULT_IRQ0_LOCATION;		// (RealMake(0xf000,0xfea5))
extern Bitu BIOS_DEFAULT_IRQ1_LOCATION;		// (RealMake(0xf000,0xe987))
extern Bitu BIOS_DEFAULT_IRQ2_LOCATION;		// (RealMake(0xf000,0xff55))
extern Bitu BIOS_DEFAULT_HANDLER_LOCATION;	// (RealMake(0xf000,0xff53))
extern Bitu BIOS_DEFAULT_INT5_LOCATION;		// (RealMake(0xf000,0xff54))
extern Bitu BIOS_VIDEO_TABLE_LOCATION;		// (RealMake(0xf000,0xf0a4))
extern Bitu BIOS_DEFAULT_RESET_LOCATION;	// RealMake(0xf000,0xe05b)
extern Bitu BIOS_VIDEO_TABLE_SIZE;

Bitu ROMBIOS_GetMemory(Bitu bytes,const char *who=NULL,Bitu alignment=1,Bitu must_be_at=0);

extern RegionAllocTracking rombios_alloc;

/* maximum of scancodes handled by keyboard bios routines */
#define MAX_SCAN_CODE 0x58

/* The Section handling Bios Disk Access */
//#define BIOS_MAX_DISK 10

//#define MAX_SWAPPABLE_DISKS 20

void BIOS_ZeroExtendedSize(bool in);

bool BIOS_AddKeyToBuffer(Bit16u code);

void INT10_ReloadRomFonts();

void BIOS_SetLPTPort (Bitu port, Bit16u baseaddr);

// \brief Synchronizes emulator num lock state with host.
void BIOS_SynchronizeNumLock();

// \brief Synchronizes emulator caps lock state with host.
void BIOS_SynchronizeCapsLock();

// \brief Synchronizes emulator scroll lock state with host.
void BIOS_SynchronizeScrollLock();

enum {
    UNHANDLED_IRQ_SIMPLE=0,
    UNHANDLED_IRQ_COOPERATIVE_2ND       // PC-98 type IRQ 8-15 handling
};

extern int unhandled_irq_method;

#endif
