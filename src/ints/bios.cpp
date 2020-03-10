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

#include <assert.h>
#include "dosbox.h"
#include "mem.h"
#include "cpu.h"
#include "bios.h"
#include "regs.h"
#include "cpu.h"
#include "callback.h"
#include "inout.h"
#include "pic.h"
#include "hardware.h"
#include "pci_bus.h"
#include "joystick.h"
#include "mouse.h"
#include "callback.h"
#include "setup.h"
#include "serialport.h"
#include "mapper.h"
#include "vga.h"
#include "shiftjis.h"
#include "regionalloctracking.h"
#include "build_timestamp.h"
extern bool PS1AudioCard;
#include "parport.h"
#include <time.h>

#if defined(DB_HAVE_CLOCK_GETTIME) && ! defined(WIN32)
//time.h is already included
#else
#include <sys/timeb.h>
#endif

#if C_EMSCRIPTEN
# include <emscripten.h>
#endif

#if defined(_MSC_VER)
# pragma warning(disable:4244) /* const fmath::local::uint64_t to double possible loss of data */
# pragma warning(disable:4305) /* truncation from double to float */
#endif

#if defined(WIN32) && !defined(S_ISREG)
# define S_ISREG(x) ((x & S_IFREG) == S_IFREG)
#endif

/* mouse.cpp */
extern bool rom_bios_8x8_cga_font;

uint32_t Keyb_ig_status();
bool VM_Boot_DOSBox_Kernel();
Bit32u MEM_get_address_bits();
Bitu bios_post_parport_count();
Bitu bios_post_comport_count();
bool MEM_map_ROM_alias_physmem(Bitu start,Bitu end);

bool bochs_port_e9 = false;
bool isa_memory_hole_512kb = false;
bool int15_wait_force_unmask_irq = false;

int unhandled_irq_method = UNHANDLED_IRQ_SIMPLE;

unsigned int reset_post_delay = 0;

Bitu call_irq_default = 0;
Bit16u biosConfigSeg=0;

Bitu BIOS_DEFAULT_IRQ0_LOCATION = ~0u;       // (RealMake(0xf000,0xfea5))
Bitu BIOS_DEFAULT_IRQ1_LOCATION = ~0u;       // (RealMake(0xf000,0xe987))
Bitu BIOS_DEFAULT_IRQ07_DEF_LOCATION = ~0u;  // (RealMake(0xf000,0xff55))
Bitu BIOS_DEFAULT_IRQ815_DEF_LOCATION = ~0u; // (RealMake(0xf000,0xe880))

Bitu BIOS_DEFAULT_HANDLER_LOCATION = ~0u;    // (RealMake(0xf000,0xff53))
Bitu BIOS_DEFAULT_INT5_LOCATION = ~0u;       // (RealMake(0xf000,0xff54))

Bitu BIOS_VIDEO_TABLE_LOCATION = ~0u;        // RealMake(0xf000,0xf0a4)
Bitu BIOS_VIDEO_TABLE_SIZE = 0u;

Bitu BIOS_DEFAULT_RESET_LOCATION = ~0u;      // RealMake(0xf000,0xe05b)

bool allow_more_than_640kb = false;

/* default bios type/version/date strings */
const char* const bios_type_string = "IBM COMPATIBLE 486 BIOS COPYRIGHT The DOSBox Team.";
const char* const bios_version_string = "DOSBox FakeBIOS v1.0";
const char* const bios_date_string = "01/01/92";

RegionAllocTracking             rombios_alloc;

Bitu                        rombios_minimum_location = 0xF0000; /* minimum segment allowed */
Bitu                        rombios_minimum_size = 0x10000;

bool MEM_map_ROM_physmem(Bitu start,Bitu end);
bool MEM_unmap_physmem(Bitu start,Bitu end);

static std::string bochs_port_e9_line;

static void bochs_port_e9_flush() {
    if (!bochs_port_e9_line.empty()) {
        LOG_MSG("Bochs port E9h: %s",bochs_port_e9_line.c_str());
        bochs_port_e9_line.clear();
    }
}

void bochs_port_e9_write(Bitu port,Bitu val,Bitu /*iolen*/) {
    (void)port;//UNUSED
    if (val == '\n' || val == '\r') {
        bochs_port_e9_flush();
    }
    else {
        bochs_port_e9_line += (char)val;
        if (bochs_port_e9_line.length() >= 256)
            bochs_port_e9_flush();
    }
}

void ROMBIOS_DumpMemory() {
    rombios_alloc.logDump();
}

void ROMBIOS_SanityCheck() {
    rombios_alloc.sanityCheck();
}

Bitu ROMBIOS_MinAllocatedLoc() {
    Bitu r = rombios_alloc.getMinAddress();

    if (r > (0x100000u - rombios_minimum_size))
        r = (0x100000u - rombios_minimum_size);

    return r & ~0xFFFu;
}

void ROMBIOS_FreeUnusedMinToLoc(Bitu phys) {
    Bitu new_phys;

    if (rombios_minimum_location & 0xFFF) E_Exit("ROMBIOS: FreeUnusedMinToLoc minimum location not page aligned");

    phys &= ~0xFFFUL;
    new_phys = rombios_alloc.freeUnusedMinToLoc(phys) & (~0xFFFUL);
    assert(new_phys >= phys);
    if (phys < new_phys) MEM_unmap_physmem(phys,new_phys-1);
    rombios_minimum_location = new_phys;
    ROMBIOS_SanityCheck();
    ROMBIOS_DumpMemory();
}

bool ROMBIOS_FreeMemory(Bitu phys) {
    return rombios_alloc.freeMemory(phys);
}

Bitu ROMBIOS_GetMemory(Bitu bytes,const char *who,Bitu alignment,Bitu must_be_at) {
    return rombios_alloc.getMemory(bytes,who,alignment,must_be_at);
}

void ROMBIOS_InitForCustomBIOS(void) {
    rombios_alloc.initSetRange(0xD8000,0xE0000);
}


extern bool user_cursor_locked;
extern int user_cursor_x,user_cursor_y;
extern int user_cursor_sw,user_cursor_sh;
extern int master_cascade_irq;
extern bool enable_slave_pic;

void VGA_SetCaptureStride(uint32_t v);
void VGA_SetCaptureAddress(uint32_t v);
void VGA_SetCaptureState(uint32_t v);
SDL_Rect &VGA_CaptureRectCurrent(void);
SDL_Rect &VGA_CaptureRectFromGuest(void);
uint32_t VGA_QueryCaptureAddress(void);
uint32_t VGA_QueryCaptureState(void);

uint32_t Mixer_MIXQ(void);
uint32_t Mixer_MIXC(void);
void Mixer_MIXC_Write(uint32_t v);
PhysPt Mixer_MIXWritePos(void);
void Mixer_MIXWritePos_Write(PhysPt np);
void Mixer_MIXWriteBegin_Write(PhysPt np);
void Mixer_MIXWriteEnd_Write(PhysPt np);

/* if mem_systems 0 then size_extended is reported as the real size else 
 * zero is reported. ems and xms can increase or decrease the other_memsystems
 * counter using the BIOS_ZeroExtendedSize call */
static Bit16u size_extended;

static Bits other_memsystems=0;

static Bitu INT15_Handler(void);

static Bitu INT70_Handler(void) {
    /* Acknowledge irq with cmos */
    IO_Write(0x70,0xc);
    IO_Read(0x71);
    if (mem_readb(BIOS_WAIT_FLAG_ACTIVE)) {
        Bit32u count=mem_readd(BIOS_WAIT_FLAG_COUNT);
        if (count>997) {
            mem_writed(BIOS_WAIT_FLAG_COUNT,count-997);
        } else {
            mem_writed(BIOS_WAIT_FLAG_COUNT,0);
            PhysPt where=Real2Phys(mem_readd(BIOS_WAIT_FLAG_POINTER));
            mem_writeb(where,mem_readb(where)|0x80);
            mem_writeb(BIOS_WAIT_FLAG_ACTIVE,0);
            mem_writed(BIOS_WAIT_FLAG_POINTER,RealMake(0,BIOS_WAIT_FLAG_TEMP));
            IO_Write(0x70,0xb);
            IO_Write(0x71,IO_Read(0x71)&~0x40);
        }
    } 
    /* Signal EOI to both pics */
    IO_Write(0xa0,0x20);
    IO_Write(0x20,0x20);
    return 0;
}

static Bitu INT1A_Handler(void) {
    CALLBACK_SIF(true);
    switch (reg_ah) {
    case 0x00:  /* Get System time */
        {
            Bit32u ticks=mem_readd(BIOS_TIMER);
            reg_al=mem_readb(BIOS_24_HOURS_FLAG);
            mem_writeb(BIOS_24_HOURS_FLAG,0); // reset the "flag"
            reg_cx=(Bit16u)(ticks >> 16u);
            reg_dx=(Bit16u)(ticks & 0xffff);
            break;
        }
    case 0x01:  /* Set System time */
        mem_writed(BIOS_TIMER,((unsigned int)reg_cx<<16u)|reg_dx);
        break;
    case 0x02:  /* GET REAL-TIME CLOCK TIME (AT,XT286,PS) */
        break;
    case 0x03:  // set RTC time
        break;
    case 0x04:  /* GET REAL-TIME ClOCK DATE  (AT,XT286,PS) */
        break;
    case 0x05:  // set RTC date
        break;
    case 0x80:  /* Pcjr Setup Sound Multiplexer */
        LOG(LOG_BIOS,LOG_ERROR)("INT1A:80:Setup tandy sound multiplexer to %d",reg_al);
        break;
    default:
        LOG(LOG_BIOS,LOG_ERROR)("INT1A:Undefined call %2X",reg_ah);
    }
    return CBRET_NONE;
}   

bool INT16_get_key(Bit16u &code);
bool INT16_peek_key(Bit16u &code);

// FIXME: This is STUPID. Cleanup is needed in order to properly use std::min without causing grief.
#ifdef _MSC_VER
# define MIN(a,b) ((a) < (b) ? (a) : (b))
# define MAX(a,b) ((a) > (b) ? (a) : (b))
#else
# define MIN(a,b) std::min(a,b)
# define MAX(a,b) std::max(a,b)
#endif

#include "int10.h"

static Bitu INT11_Handler(void) {
    reg_ax=mem_readw(BIOS_CONFIGURATION);
    return CBRET_NONE;
}
/* 
 * Define the following define to 1 if you want dosbox to check 
 * the system time every 5 seconds and adjust 1/2 a second to sync them.
 */
#ifndef DOSBOX_CLOCKSYNC
#define DOSBOX_CLOCKSYNC 0
#endif

static void BIOS_HostTimeSync() {
    Bit32u milli = 0;
#if defined(DB_HAVE_CLOCK_GETTIME) && ! defined(WIN32)
    struct timespec tp;
    clock_gettime(CLOCK_REALTIME,&tp);
	
    struct tm *loctime;
    loctime = localtime(&tp.tv_sec);
    milli = (Bit32u) (tp.tv_nsec / 1000000);
#else
    /* Setup time and date */
    struct timeb timebuffer;
    ftime(&timebuffer);
    
    struct tm *loctime;
    loctime = localtime (&timebuffer.time);
    milli = (Bit32u) timebuffer.millitm;
#endif
    /*
    loctime->tm_hour = 23;
    loctime->tm_min = 59;
    loctime->tm_sec = 45;
    loctime->tm_mday = 28;
    loctime->tm_mon = 2-1;
    loctime->tm_year = 2007 - 1900;
    */

    dos.date.day=(Bit8u)loctime->tm_mday;
    dos.date.month=(Bit8u)loctime->tm_mon+1;
    dos.date.year=(Bit16u)loctime->tm_year+1900;

    Bit32u ticks=(Bit32u)(((double)(
        (unsigned int)loctime->tm_hour*3600u*1000u+
        (unsigned int)loctime->tm_min*60u*1000u+
        (unsigned int)loctime->tm_sec*1000u+
        milli))*(((double)PIT_TICK_RATE/65536.0)/1000.0));
    mem_writed(BIOS_TIMER,ticks);
}

void KEYBOARD_SetLEDs(Bit8u bits);

void BIOS_KEYBOARD_SetLEDs(Bitu state) {
    Bitu x = mem_readb(BIOS_KEYBOARD_LEDS);

    x &= ~7u;
    x |= (state & 7u);
    mem_writeb(BIOS_KEYBOARD_LEDS,x);
    KEYBOARD_SetLEDs(state);
}

static Bitu INT8_Handler(void) {
    /* Increase the bios tick counter */
    Bit32u value = mem_readd(BIOS_TIMER) + 1;
    if(value >= 0x1800B0) {
        // time wrap at midnight
        mem_writeb(BIOS_24_HOURS_FLAG,mem_readb(BIOS_24_HOURS_FLAG)+1);
        value=0;
    }

#if DOSBOX_CLOCKSYNC
    static bool check = false;
    if((value %50)==0) {
        if(((value %100)==0) && check) {
            check = false;
            time_t curtime;struct tm *loctime;
            curtime = time (NULL);loctime = localtime (&curtime);
            Bit32u ticksnu = (Bit32u)((loctime->tm_hour*3600+loctime->tm_min*60+loctime->tm_sec)*(float)PIT_TICK_RATE/65536.0);
            Bit32s bios = value;Bit32s tn = ticksnu;
            Bit32s diff = tn - bios;
            if(diff>0) {
                if(diff < 18) { diff  = 0; } else diff = 9;
            } else {
                if(diff > -18) { diff = 0; } else diff = -9;
            }
         
            value += diff;
        } else if((value%100)==50) check = true;
    }
#endif
    mem_writed(BIOS_TIMER,value);

    /* decrease floppy motor timer */
    Bit8u val = mem_readb(BIOS_DISK_MOTOR_TIMEOUT);
    if (val) mem_writeb(BIOS_DISK_MOTOR_TIMEOUT,val-1);
    /* and running drive */
    mem_writeb(BIOS_DRIVE_RUNNING,mem_readb(BIOS_DRIVE_RUNNING) & 0xF0);
    return CBRET_NONE;
}
#undef DOSBOX_CLOCKSYNC

static Bitu INT1C_Handler(void) {
    return CBRET_NONE;
}

static Bitu INT12_Handler(void) {
    reg_ax=mem_readw(BIOS_MEMORY_SIZE);
    return CBRET_NONE;
}

static Bitu INT17_Handler(void) {
    if (reg_ah > 0x2 || reg_dx > 0x2) { // 0-2 printer port functions
                                        // and no more than 3 parallel ports
        LOG_MSG("BIOS INT17: Unhandled call AH=%2X DX=%4x",reg_ah,reg_dx);
        return CBRET_NONE;
    }

    switch(reg_ah) {
    case 0x00:      // PRINTER: Write Character
        break;
    case 0x01:      // PRINTER: Initialize port
        break;
    case 0x02:      // PRINTER: Get Status
        break;
    }
    return CBRET_NONE;
}

static bool INT14_Wait(Bit16u port, Bit8u mask, Bit8u timeout, Bit8u* retval) {
    double starttime = PIC_FullIndex();
    double timeout_f = timeout * 1000.0;
    while (((*retval = IO_ReadB(port)) & mask) != mask) {
        if (starttime < (PIC_FullIndex() - timeout_f)) {
            return false;
        }
        CALLBACK_Idle();
    }
    return true;
}

static Bitu INT4B_Handler(void) {
    /* TODO: This is where the Virtual DMA specification is accessed on modern systems.
     *       When we implement that, move this to EMM386 emulation code. */

    if (reg_ax >= 0x8102 && reg_ax <= 0x810D) {
        LOG(LOG_MISC,LOG_DEBUG)("Guest OS attempted Virtual DMA specification call (INT 4Bh AX=%04x BX=%04x CX=%04x DX=%04x",
            reg_ax,reg_bx,reg_cx,reg_dx);
    }
    else if (reg_ah == 0x80) {
        LOG(LOG_MISC,LOG_DEBUG)("Guest OS attempted IBM SCSI interface call");
    }
    else if (reg_ah <= 0x02) {
        LOG(LOG_MISC,LOG_DEBUG)("Guest OS attempted TI Professional PC parallel port function AH=%02x",reg_ah);
    }
    else {
        LOG(LOG_MISC,LOG_DEBUG)("Guest OS attempted unknown INT 4Bh call AX=%04x",reg_ax);
    }
    
    /* Oh, I'm just a BIOS that doesn't know what the hell you're doing. CF=1 */
    CALLBACK_SCF(true);
    return CBRET_NONE;
}

static Bitu INT14_Handler(void) {
    if (reg_ah > 0x3 || reg_dx > 0x3) { // 0-3 serial port functions
                                        // and no more than 4 serial ports
        LOG_MSG("BIOS INT14: Unhandled call AH=%2X DX=%4x",reg_ah,reg_dx);
        return CBRET_NONE;
    }
    
    Bit16u port = real_readw(0x40,reg_dx * 2u); // DX is always port number
    Bit8u timeout = mem_readb((PhysPt)((unsigned int)BIOS_COM1_TIMEOUT + (unsigned int)reg_dx));
    if (port==0)    {
        LOG(LOG_BIOS,LOG_NORMAL)("BIOS INT14: port %d does not exist.",reg_dx);
        return CBRET_NONE;
    }
    switch (reg_ah) {
    case 0x00:  {
        // Initialize port
        // Parameters:              Return:
        // AL: port parameters      AL: modem status
        //                          AH: line status

        // set baud rate
        Bitu baudrate = 9600u;
        Bit16u baudresult;
        Bitu rawbaud=(Bitu)reg_al>>5u;
        
        if (rawbaud==0){ baudrate=110u;}
        else if (rawbaud==1){ baudrate=150u;}
        else if (rawbaud==2){ baudrate=300u;}
        else if (rawbaud==3){ baudrate=600u;}
        else if (rawbaud==4){ baudrate=1200u;}
        else if (rawbaud==5){ baudrate=2400u;}
        else if (rawbaud==6){ baudrate=4800u;}
        else if (rawbaud==7){ baudrate=9600u;}

        baudresult = (Bit16u)(115200u / baudrate);

        IO_WriteB(port+3u, 0x80u);    // enable divider access
        IO_WriteB(port, (Bit8u)baudresult&0xffu);
        IO_WriteB(port+1u, (Bit8u)(baudresult>>8u));

        // set line parameters, disable divider access
        IO_WriteB(port+3u, reg_al&0x1Fu); // LCR
        
        // disable interrupts
        IO_WriteB(port+1u, 0u); // IER

        // get result
        reg_ah=IO_ReadB(port+5u)&0xffu;
        reg_al=IO_ReadB(port+6u)&0xffu;
        CALLBACK_SCF(false);
        break;
    }
    case 0x01: // Transmit character
        // Parameters:              Return:
        // AL: character            AL: unchanged
        // AH: 0x01                 AH: line status from just before the char was sent
        //                              (0x80 | unpredicted) in case of timeout
        //                      [undoc] (0x80 | line status) in case of tx timeout
        //                      [undoc] (0x80 | modem status) in case of dsr/cts timeout

        // set DTR & RTS on
        IO_WriteB(port+4u,0x3u);
        // wait for DSR & CTS
        if (INT14_Wait(port+6u, 0x30u, timeout, &reg_ah)) {
            // wait for TX buffer empty
            if (INT14_Wait(port+5u, 0x20u, timeout, &reg_ah)) {
                // fianlly send the character
                IO_WriteB(port,reg_al);
            } else
                reg_ah |= 0x80u;
        } else // timed out
            reg_ah |= 0x80u;

        CALLBACK_SCF(false);
        break;
    case 0x02: // Read character
        // Parameters:              Return:
        // AH: 0x02                 AL: received character
        //                      [undoc] will be trashed in case of timeout
        //                          AH: (line status & 0x1E) in case of success
        //                              (0x80 | unpredicted) in case of timeout
        //                      [undoc] (0x80 | line status) in case of rx timeout
        //                      [undoc] (0x80 | modem status) in case of dsr timeout

        // set DTR on
        IO_WriteB(port+4u,0x1u);

        // wait for DSR
        if (INT14_Wait(port+6, 0x20, timeout, &reg_ah)) {
            // wait for character to arrive
            if (INT14_Wait(port+5, 0x01, timeout, &reg_ah)) {
                reg_ah &= 0x1E;
                reg_al = IO_ReadB(port);
            } else
                reg_ah |= 0x80;
        } else
            reg_ah |= 0x80;

        CALLBACK_SCF(false);
        break;
    case 0x03: // get status
        reg_ah=IO_ReadB(port+5u)&0xffu;
        reg_al=IO_ReadB(port+6u)&0xffu;
        CALLBACK_SCF(false);
        break;

    }
    return CBRET_NONE;
}

Bits HLT_Decode(void);
void KEYBOARD_AUX_Write(Bitu val);
unsigned char KEYBOARD_AUX_GetType();
unsigned char KEYBOARD_AUX_DevStatus();
unsigned char KEYBOARD_AUX_Resolution();
unsigned char KEYBOARD_AUX_SampleRate();
void KEYBOARD_ClrBuffer(void);

static Bitu INT15_Handler(void) {
    switch (reg_ah) {
    case 0x06:
        LOG(LOG_BIOS,LOG_NORMAL)("INT15 Unkown Function 6 (Amstrad?)");
        break;
    case 0xC0:  /* Get Configuration*/
        CPU_SetSegGeneral(es,biosConfigSeg);
        reg_bx = 0;
        reg_ah = 0;
        CALLBACK_SCF(false);
        break;
    case 0x4f:  /* BIOS - Keyboard intercept */
        /* Carry should be set but let's just set it just in case */
        CALLBACK_SCF(true);
        break;
    case 0x83:  /* BIOS - SET EVENT WAIT INTERVAL */
        {
            if(reg_al == 0x01) { /* Cancel it */
                mem_writeb(BIOS_WAIT_FLAG_ACTIVE,0);
                IO_Write(0x70,0xb);
                IO_Write(0x71,IO_Read(0x71)&~0x40);
                CALLBACK_SCF(false);
                break;
            }
            if (mem_readb(BIOS_WAIT_FLAG_ACTIVE)) {
                reg_ah=0x80;
                CALLBACK_SCF(true);
                break;
            }
            Bit32u count=((Bit32u)reg_cx<<16u)|reg_dx;
            mem_writed(BIOS_WAIT_FLAG_POINTER,RealMake(SegValue(es),reg_bx));
            mem_writed(BIOS_WAIT_FLAG_COUNT,count);
            mem_writeb(BIOS_WAIT_FLAG_ACTIVE,1);
            /* Reprogram RTC to start */
            IO_Write(0x70,0xb);
            IO_Write(0x71,IO_Read(0x71)|0x40);
            CALLBACK_SCF(false);
        }
        break;
    case 0x86:  /* BIOS - WAIT (AT,PS) */
        {
            if (mem_readb(BIOS_WAIT_FLAG_ACTIVE)) {
                reg_ah=0x83;
                CALLBACK_SCF(true);
                break;
            }
            Bit8u t;
            Bit32u count=((Bit32u)reg_cx<<16u)|reg_dx;
            mem_writed(BIOS_WAIT_FLAG_POINTER,RealMake(0,BIOS_WAIT_FLAG_TEMP));
            mem_writed(BIOS_WAIT_FLAG_COUNT,count);
            mem_writeb(BIOS_WAIT_FLAG_ACTIVE,1);

            /* if the user has not set the option, warn if IRQs are masked */
            if (!int15_wait_force_unmask_irq) {
                /* make sure our wait function works by unmasking IRQ 2, and IRQ 8.
                 * (bugfix for 1993 demo Yodel "Mayday" demo. this demo keeps masking IRQ 2 for some stupid reason.) */
                if ((t=IO_Read(0x21)) & (1 << 2)) {
                    LOG(LOG_BIOS,LOG_ERROR)("INT15:86:Wait: IRQ 2 masked during wait. "
                        "Consider adding 'int15 wait force unmask irq=true' to your dosbox.conf");
                }
                if ((t=IO_Read(0xA1)) & (1 << 0)) {
                    LOG(LOG_BIOS,LOG_ERROR)("INT15:86:Wait: IRQ 8 masked during wait. "
                        "Consider adding 'int15 wait force unmask irq=true' to your dosbox.conf");
                }
            }

            /* Reprogram RTC to start */
            IO_Write(0x70,0xb);
            IO_Write(0x71,IO_Read(0x71)|0x40);
            while (mem_readd(BIOS_WAIT_FLAG_COUNT)) {
                if (int15_wait_force_unmask_irq) {
                    /* make sure our wait function works by unmasking IRQ 2, and IRQ 8.
                     * (bugfix for 1993 demo Yodel "Mayday" demo. this demo keeps masking IRQ 2 for some stupid reason.) */
                    if ((t=IO_Read(0x21)) & (1 << 2)) {
                        LOG(LOG_BIOS,LOG_WARN)("INT15:86:Wait: IRQ 2 masked during wait. "
                            "This condition might result in an infinite wait on "
                            "some BIOSes. Unmasking IRQ to keep things moving along.");
                        IO_Write(0x21,t & ~(1 << 2));

                    }
                    if ((t=IO_Read(0xA1)) & (1 << 0)) {
                        LOG(LOG_BIOS,LOG_WARN)("INT15:86:Wait: IRQ 8 masked during wait. "
                            "This condition might result in an infinite wait on some "
                            "BIOSes. Unmasking IRQ to keep things moving along.");
                        IO_Write(0xA1,t & ~(1 << 0));
                    }
                }

                CALLBACK_Idle();
            }
            CALLBACK_SCF(false);
            break;
        }
    case 0x87:  /* Copy extended memory */
        {
            bool enabled = MEM_A20_Enabled();
            MEM_A20_Enable(true);
            Bitu   bytes    = reg_cx * 2u;
            PhysPt data     = SegPhys(es)+reg_si;
            PhysPt source   = (mem_readd(data+0x12u) & 0x00FFFFFFu) + ((unsigned int)mem_readb(data+0x17u)<<24u);
            PhysPt dest     = (mem_readd(data+0x1Au) & 0x00FFFFFFu) + ((unsigned int)mem_readb(data+0x1Fu)<<24u);
            MEM_BlockCopy(dest,source,bytes);
            reg_ax = 0x00;
            MEM_A20_Enable(enabled);
            Segs.limit[cs] = 0xFFFF;
            Segs.limit[ds] = 0xFFFF;
            Segs.limit[es] = 0xFFFF;
            Segs.limit[ss] = 0xFFFF;
            CALLBACK_SCF(false);
            break;
        }   
    case 0x88:  /* SYSTEM - GET EXTENDED MEMORY SIZE (286+) */
        /* This uses the 16-bit value read back from CMOS which is capped at 64MB */
        reg_ax=other_memsystems?0:size_extended;
        LOG(LOG_BIOS,LOG_NORMAL)("INT15:Function 0x88 Remaining %04X kb",reg_ax);
        CALLBACK_SCF(false);
        break;
    case 0x89:  /* SYSTEM - SWITCH TO PROTECTED MODE */
        {
            IO_Write(0x20,0x10);IO_Write(0x21,reg_bh);IO_Write(0x21,0);IO_Write(0x21,0xFF);
            IO_Write(0xA0,0x10);IO_Write(0xA1,reg_bl);IO_Write(0xA1,0);IO_Write(0xA1,0xFF);
            MEM_A20_Enable(true);
            PhysPt table=SegPhys(es)+reg_si;
            CPU_LGDT(mem_readw(table+0x8),mem_readd(table+0x8+0x2) & 0xFFFFFF);
            CPU_LIDT(mem_readw(table+0x10),mem_readd(table+0x10+0x2) & 0xFFFFFF);
            CPU_SET_CRX(0,CPU_GET_CRX(0)|1);
            CPU_SetSegGeneral(ds,0x18);
            CPU_SetSegGeneral(es,0x20);
            CPU_SetSegGeneral(ss,0x28);
            Bitu ret = mem_readw(SegPhys(ss)+reg_sp);
            reg_sp+=6;          //Clear stack of interrupt frame
            CPU_SetFlags(0,FMASK_ALL);
            reg_ax=0;
            CPU_JMP(false,0x30,ret,0);
        }
        break;
    case 0x8A:  /* EXTENDED MEMORY SIZE */
        {
            Bitu sz = MEM_TotalPages()*4;
            if (sz >= 1024) sz -= 1024;
            else sz = 0;
            reg_ax = sz & 0xFFFF;
            reg_dx = sz >> 16;
            CALLBACK_SCF(false);
        }
        break;
    case 0x90:  /* OS HOOK - DEVICE BUSY */
        CALLBACK_SCF(false);
        reg_ah=0;
        break;
    case 0x91:  /* OS HOOK - DEVICE POST */
        CALLBACK_SCF(false);
        reg_ah=0;
        break;
    case 0xc3:      /* set carry flag so BorlandRTM doesn't assume a VECTRA/PS2 */
        reg_ah=0x86;
        CALLBACK_SCF(true);
        break;
    case 0xc4:  /* BIOS POS Programm option Select */
        LOG(LOG_BIOS,LOG_NORMAL)("INT15:Function %X called, bios mouse not supported",reg_ah);
        CALLBACK_SCF(true);
        break;
    case 0xe8:
        switch (reg_al) {
            case 0x01: { /* E801: memory size */
                    Bitu sz = MEM_TotalPages()*4;
                    if (sz >= 1024) sz -= 1024;
                    else sz = 0;
                    reg_ax = reg_cx = (sz > 0x3C00) ? 0x3C00 : sz; /* extended memory between 1MB and 16MB in KBs */
                    sz -= reg_ax;
                    sz /= 64;   /* extended memory size from 16MB in 64KB blocks */
                    if (sz > 65535) sz = 65535;
                    reg_bx = reg_dx = sz;
                    CALLBACK_SCF(false);
                }
                break;
            case 0x20: { /* E820: MEMORY LISTING */
                    if (reg_edx == 0x534D4150 && reg_ecx >= 20 && (MEM_TotalPages()*4) >= 24000) {
                        /* return a minimalist list:
                         *
                         *    0) 0x000000-0x09EFFF       Free memory
                         *    1) 0x0C0000-0x0FFFFF       Reserved
                         *    2) 0x100000-...            Free memory (no ACPI tables) */
                        if (reg_ebx < 3) {
                            uint32_t base = 0,len = 0,type = 0;
                            Bitu seg = SegValue(es);

                            assert((MEM_TotalPages()*4096) >= 0x100000);

                            switch (reg_ebx) {
                                case 0: base=0x000000; len=0x09F000; type=1; break;
                                case 1: base=0x0C0000; len=0x040000; type=2; break;
                                case 2: base=0x100000; len=(MEM_TotalPages()*4096)-0x100000; type=1; break;
                                default: E_Exit("Despite checks EBX is wrong value"); /* BUG! */
                            }

                            /* write to ES:DI */
                            real_writed(seg,reg_di+0x00,base);
                            real_writed(seg,reg_di+0x04,0);
                            real_writed(seg,reg_di+0x08,len);
                            real_writed(seg,reg_di+0x0C,0);
                            real_writed(seg,reg_di+0x10,type);
                            reg_ecx = 20;

                            /* return EBX pointing to next entry. wrap around, as most BIOSes do.
                             * the program is supposed to stop on CF=1 or when we return EBX == 0 */
                            if (++reg_ebx >= 3) reg_ebx = 0;
                        }
                        else {
                            CALLBACK_SCF(true);
                        }

                        reg_eax = 0x534D4150;
                    }
                    else {
                        reg_eax = 0x8600;
                        CALLBACK_SCF(true);
                    }
                }
                break;
            default:
                LOG(LOG_BIOS,LOG_ERROR)("INT15:Unknown call ah=E8, al=%2X",reg_al);
                reg_ah=0x86;
                CALLBACK_SCF(true);
                if ((IS_EGAVGA_ARCH)) {
                    /* relict from comparisons, as int15 exits with a retf2 instead of an iret */
                    CALLBACK_SZF(false);
                }
        }
        break;
    default:
        LOG(LOG_BIOS,LOG_ERROR)("INT15:Unknown call ax=%4X",reg_ax);
        reg_ah=0x86;
        CALLBACK_SCF(true);
        if ((IS_EGAVGA_ARCH)) {
            /* relict from comparisons, as int15 exits with a retf2 instead of an iret */
            CALLBACK_SZF(false);
        }
    }
    return CBRET_NONE;
}

void BIOS_UnsetupKeyboard(void);
void BIOS_SetupKeyboard(void);
void CPU_Snap_Back_To_Real_Mode();
void CPU_Snap_Back_Restore();

static Bitu Default_IRQ_Handler(void) {
    IO_WriteB(0x20, 0x0b);
    Bit8u master_isr = IO_ReadB(0x20);
    if (master_isr) {
        IO_WriteB(0xa0, 0x0b);
        Bit8u slave_isr = IO_ReadB(0xa0);
        if (slave_isr) {
            IO_WriteB(0xa1, IO_ReadB(0xa1) | slave_isr);
            IO_WriteB(0xa0, 0x20);
        }
        else IO_WriteB(0x21, IO_ReadB(0x21) | (master_isr & ~4));
        IO_WriteB(0x20, 0x20);
#if C_DEBUG
        Bit16u irq = 0;
        Bit16u isr = master_isr;
        if (slave_isr) isr = slave_isr << 8;
        while (isr >>= 1) irq++;
        LOG(LOG_BIOS, LOG_WARN)("Unexpected IRQ %u", irq);
#endif 
    }
    else master_isr = 0xff;
    mem_writeb(BIOS_LAST_UNEXPECTED_IRQ, master_isr);
    return CBRET_NONE;
}

static Bitu IRQ14_Dummy(void) {
    /* FIXME: That's it? Don't I EOI the PIC? */
    return CBRET_NONE;
}

static Bitu IRQ15_Dummy(void) {
    /* FIXME: That's it? Don't I EOI the PIC? */
    return CBRET_NONE;
}

void On_Software_CPU_Reset();

static Bitu INT18_Handler(void) {
    LOG_MSG("Restart by INT 18h requested\n");
    On_Software_CPU_Reset();
    /* does not return */
    return CBRET_NONE;
}

static Bitu INT19_Handler(void) {
    LOG_MSG("Restart by INT 19h requested\n");
    /* FIXME: INT 19h is sort of a BIOS boot BIOS reset-ish thing, not really a CPU reset at all. */
    On_Software_CPU_Reset();
    /* does not return */
    return CBRET_NONE;
}

void bios_enable_ps2() {
    mem_writew(BIOS_CONFIGURATION,
        mem_readw(BIOS_CONFIGURATION)|0x04); /* PS/2 mouse */
}

void BIOS_ZeroExtendedSize(bool in) {
    if(in) other_memsystems++; 
    else other_memsystems--;
    if(other_memsystems < 0) other_memsystems=0;
}

void MEM_ResetPageHandler_Unmapped(Bitu phys_page, Bitu pages);

unsigned int dos_conventional_limit = 0;

bool AdapterROM_Read(Bitu address,unsigned long *size) {
    unsigned char c[3];
    unsigned int i;

    if ((address & 0x1FF) != 0) {
        LOG(LOG_MISC,LOG_DEBUG)("AdapterROM_Read: Caller attempted ROM scan not aligned to 512-byte boundary");
        return false;
    }

    for (i=0;i < 3;i++)
        c[i] = mem_readb(address+i);

    if (c[0] == 0x55 && c[1] == 0xAA) {
        unsigned char chksum=0;
        *size = (unsigned long)c[2] * 512UL;
        for (i=0;i < (unsigned int)(*size);i++) chksum += mem_readb(address+i);
        if (chksum != 0) {
            LOG(LOG_MISC,LOG_WARN)("AdapterROM_Read: Found ROM at 0x%lx but checksum failed\n",(unsigned long)address);
            return false;
        }

        return true;
    }

    return false;
}

#include "src/gui/dosbox.vga16.bmp.h"
#include "src/gui/dosbox.cga640.bmp.h"

void DrawDOSBoxLogoCGA6(unsigned int x,unsigned int y) {
    unsigned char *s = dosbox_cga640_bmp;
    unsigned char *sf = s + sizeof(dosbox_cga640_bmp);
    uint32_t width,height;
    unsigned int dx,dy;
    uint32_t off;
    uint32_t sz;

    if (memcmp(s,"BM",2)) return;
    sz = host_readd(s+2); // size of total bitmap
    off = host_readd(s+10); // offset of bitmap
    if ((s+sz) > sf) return;
    if ((s+14+40) > sf) return;

    sz = host_readd(s+34); // biSize
    if ((s+off+sz) > sf) return;
    if (host_readw(s+26) != 1) return; // biBitPlanes
    if (host_readw(s+28) != 1)  return; // biBitCount

    width = host_readd(s+18);
    height = host_readd(s+22);
    if (width > (640-x) || height > (200-y)) return;

    LOG(LOG_MISC,LOG_DEBUG)("Drawing CGA logo (%u x %u)",(int)width,(int)height);
    for (dy=0;dy < height;dy++) {
        uint32_t vram  = ((y+dy) >> 1) * 80;
        vram += ((y+dy) & 1) * 0x2000;
        vram += (x / 8);
        s = dosbox_cga640_bmp + off + ((height-(dy+1))*((width+7)/8));
        for (dx=0;dx < width;dx += 8) {
            mem_writeb(0xB8000+vram,*s);
            vram++;
            s++;
        }
    }
}

void DrawDOSBoxLogoVGA(unsigned int x,unsigned int y) {
    unsigned char *s = dosbox_vga16_bmp;
    unsigned char *sf = s + sizeof(dosbox_vga16_bmp);
    unsigned int bit,dx,dy;
    uint32_t width,height;
    uint32_t vram;
    uint32_t off;
    uint32_t sz;

    if (memcmp(s,"BM",2)) return;
    sz = host_readd(s+2); // size of total bitmap
    off = host_readd(s+10); // offset of bitmap
    if ((s+sz) > sf) return;
    if ((s+14+40) > sf) return;

    sz = host_readd(s+34); // biSize
    if ((s+off+sz) > sf) return;
    if (host_readw(s+26) != 1) return; // biBitPlanes
    if (host_readw(s+28) != 4)  return; // biBitCount

    width = host_readd(s+18);
    height = host_readd(s+22);
    if (width > (640-x) || height > (350-y)) return;

    // EGA/VGA Write Mode 2
    LOG(LOG_MISC,LOG_DEBUG)("Drawing VGA logo (%u x %u)",(int)width,(int)height);
    IO_Write(0x3CE,0x05); // graphics mode
    IO_Write(0x3CF,0x02); // read=0 write=2 odd/even=0 shift=0 shift256=0
    IO_Write(0x3CE,0x03); // data rotate
    IO_Write(0x3CE,0x00); // no rotate, no XOP
    for (bit=0;bit < 8;bit++) {
        const unsigned char shf = ((bit & 1) ^ 1) * 4;

        IO_Write(0x3CE,0x08); // bit mask
        IO_Write(0x3CF,0x80 >> bit);

        for (dy=0;dy < height;dy++) {
            vram = ((y+dy) * 80) + (x / 8);
            s = dosbox_vga16_bmp + off + (bit/2) + ((height-(dy+1))*((width+1)/2));
            for (dx=bit;dx < width;dx += 8) {
                mem_readb(0xA0000+vram); // load VGA latches
                mem_writeb(0xA0000+vram,(*s >> shf) & 0xF);
                vram++;
                s += 4;
            }
        }
    }
    // restore write mode 0
    IO_Write(0x3CE,0x05); // graphics mode
    IO_Write(0x3CF,0x00); // read=0 write=0 odd/even=0 shift=0 shift256=0
    IO_Write(0x3CE,0x08); // bit mask
    IO_Write(0x3CF,0xFF);
}

static void BIOS_Int10RightJustifiedPrint(const int x,int &y,const char *msg) {
    const char *s = msg;

        while (*s != 0) {
            if (*s == '\n') {
                y++;
                reg_eax = 0x0200u;   // set cursor pos
                reg_ebx = 0;        // page zero
                reg_dh = y;     // row 4
                reg_dl = x;     // column 20
                CALLBACK_RunRealInt(0x10);
                s++;
            }
            else {
                reg_eax = 0x0E00u | ((unsigned char)(*s++));
                reg_ebx = 0x07u;
                CALLBACK_RunRealInt(0x10);
            }
        }
}

static Bitu ulimit = 0;
static Bitu t_conv = 0;
static bool bios_first_init=true;
static bool bios_has_exec_vga_bios=false;
static Bitu adapter_scan_start;

/* FIXME: At global scope their destructors are called after the rest of DOSBox has shut down. Move back into BIOS scope. */
static CALLBACK_HandlerObject int4b_callback;
static CALLBACK_HandlerObject callback[20]; /* <- fixme: this is stupid. just declare one per interrupt. */
static CALLBACK_HandlerObject cb_bios_post;

Bitu call_pnp_r = ~0UL;
Bitu call_pnp_rp = 0;

Bitu call_pnp_p = ~0UL;
Bitu call_pnp_pp = 0;

Bitu BIOS_boot_code_offset = 0;
Bitu BIOS_bootfail_code_offset = 0;

bool bios_user_reset_vector_blob_run = false;
Bitu bios_user_reset_vector_blob = 0;

Bitu bios_user_boot_hook = 0;

void CALLBACK_DeAllocate(Bitu in);

void BIOS_OnResetComplete(Section *x);

Bitu call_irq0 = 0;
Bitu call_irq07default = 0;
Bitu call_irq815default = 0;

/* NTS: Remember the 8259 is non-sentient, and the term "slave" is used in a computer programming context */
static Bitu Default_IRQ_Handler_Cooperative_Slave_Pic(void) {
    /* PC-98 style IRQ 8-15 handling.
     *
     * This mimics the recommended procedure [https://www.webtech.co.jp/company/doc/undocumented_mem/io_pic.txt]
     *
     *  mov al,20h      ;Send EOI to SLAVE
     *  out 0008h,al
     *  jmp $+2         ;I/O WAIT
     *  mov al,0Bh      ;ISR read mode set(slave)
     *  out 0008h,al
     *  jmp $+2         ;I/O WAIT
     *  in  al,0008h    ;ISR read(slave)
     *  cmp al,00h      ;slave pic in-service ?
     *  jne EoiEnd
     *  mov al,20h      ;Send EOI to MASTER
     *  out 0000h,al
     */
    IO_WriteB(0xA0,0x20); // send EOI to slave
    IO_WriteB(0xA0,0x0B); // ISR read mode set
    if (IO_ReadB(0xA0) == 0) // if slave pic in service..
        IO_WriteB(0x20,0x20); // then EOI the master

    return CBRET_NONE;
}

static int bios_post_counter = 0;

class BIOS:public Module_base{
private:
    static Bitu cb_bios_post__func(void) {
        void TIMER_BIOS_INIT_Configure();
#if C_DEBUG
        void DEBUG_CheckCSIP();

# if C_HEAVY_DEBUG
        /* the game/app obviously crashed, which is way more important
         * to log than what we do here in the BIOS at POST */
        void DEBUG_StopLog(void);
        DEBUG_StopLog();
# endif
#endif

        {
            Section_prop * section=static_cast<Section_prop *>(control->GetSection("dosbox"));
            int val = section->Get_int("reboot delay");

            if (val < 0)
                val = 500;

            reset_post_delay = (unsigned int)val;
        }

        if (bios_post_counter != 0 && reset_post_delay != 0) {
            /* reboot delay, in case the guest OS/application had something to day before hitting the "reset" signal */
            Bit32u lasttick=GetTicks();
            while ((GetTicks()-lasttick) < reset_post_delay) {
                void CALLBACK_IdleNoInts(void);
                CALLBACK_IdleNoInts();
            }
        }

        if (bios_post_counter != 0) {
            /* turn off the PC speaker if the guest left it on at reset */
            {
                IO_Write(0x61,IO_Read(0x61) & (~3u));
            }
        }

        bios_post_counter++;

        if (bios_first_init) {
            /* clear the first 1KB-32KB */
            for (Bit16u i=0x400;i<0x8000;i++) real_writeb(0x0,i,0);
        }

        if (bios_user_reset_vector_blob != 0 && !bios_user_reset_vector_blob_run) {
            LOG_MSG("BIOS POST: Running user reset vector blob at 0x%lx",(unsigned long)bios_user_reset_vector_blob);
            bios_user_reset_vector_blob_run = true;

            assert((bios_user_reset_vector_blob&0xF) == 0); /* must be page aligned */

            SegSet16(cs,bios_user_reset_vector_blob>>4);
            reg_eip = 0;

#if C_DEBUG
            /* help the debugger reflect the new instruction pointer */
            DEBUG_CheckCSIP();
#endif

            return CBRET_NONE;
        }

        if (cpu.pmode) E_Exit("BIOS error: POST function called while in protected/vm86 mode");

        CPU_CLI();

        /* we need A20 enabled for BIOS boot-up */
        void A20Gate_OverrideOn(Section *sec);
        void MEM_A20_Enable(bool enabled);
        A20Gate_OverrideOn(NULL);
        MEM_A20_Enable(true);

        BIOS_OnResetComplete(NULL);

        adapter_scan_start = 0xC0000;
        bios_has_exec_vga_bios = false;
        LOG(LOG_MISC,LOG_DEBUG)("BIOS: executing POST routine");

        // TODO: Anything we can test in the CPU here?

        // initialize registers
        SegSet16(ds,0x0000);
        SegSet16(es,0x0000);
        SegSet16(fs,0x0000);
        SegSet16(gs,0x0000);
        SegSet16(ss,0x0000);

        {
            Bitu sz = MEM_TotalPages();

            /* The standard BIOS is said to put it's stack (at least at OS boot time) 512 bytes past the end of the boot sector
             * meaning that the boot sector loads to 0000:7C00 and the stack is set grow downward from 0000:8000 */

            if (sz > 8) sz = 8; /* 4KB * 8 = 32KB = 0x8000 */
            sz *= 4096;
            reg_esp = sz - 4;
            reg_ebp = 0;
            LOG(LOG_MISC,LOG_DEBUG)("BIOS: POST stack set to 0000:%04x",reg_esp);
        }

        extern Bitu call_default;

        {
            /* Clear the vector table */
            for (Bit16u i=0x70*4;i<0x400;i++) real_writeb(0x00,i,0);

            /* Only setup default handler for first part of interrupt table */
            for (Bit16u ct=0;ct<0x60;ct++) {
                real_writed(0,ct*4,CALLBACK_RealPointer(call_default));
            }
            for (Bit16u ct=0x68;ct<0x70;ct++) {
                real_writed(0,ct*4,CALLBACK_RealPointer(call_default));
            }

            // default handler for IRQ 2-7
            for (Bit16u ct=0x0A;ct <= 0x0F;ct++)
                RealSetVec(ct,BIOS_DEFAULT_IRQ07_DEF_LOCATION);
        }

        if (unhandled_irq_method == UNHANDLED_IRQ_COOPERATIVE_2ND) {
            // PC-98 style: Master PIC ack with 0x20 for IRQ 0-7.
            //              For the slave PIC, ack with 0x20 on the slave, then only ack the master (cascade interrupt)
            //              if the ISR register on the slave indicates none are in service.
            CALLBACK_Setup(call_irq07default,NULL,CB_IRET_EOI_PIC1,Real2Phys(BIOS_DEFAULT_IRQ07_DEF_LOCATION),"bios irq 0-7 default handler");
            CALLBACK_Setup(call_irq815default,Default_IRQ_Handler_Cooperative_Slave_Pic,CB_IRET,Real2Phys(BIOS_DEFAULT_IRQ815_DEF_LOCATION),"bios irq 8-15 default handler");
        }
        else {
            // IBM PC style: Master PIC ack with 0x20, slave PIC ack with 0x20, no checking
            CALLBACK_Setup(call_irq07default,NULL,CB_IRET_EOI_PIC1,Real2Phys(BIOS_DEFAULT_IRQ07_DEF_LOCATION),"bios irq 0-7 default handler");
            CALLBACK_Setup(call_irq815default,NULL,CB_IRET_EOI_PIC2,Real2Phys(BIOS_DEFAULT_IRQ815_DEF_LOCATION),"bios irq 8-15 default handler");
        }

        bool null_68h = false;

        {
            Section_prop * section=static_cast<Section_prop *>(control->GetSection("dos"));

            null_68h = section->Get_bool("zero unused int 68h");
        }

        /* Default IRQ handler */
        if (call_irq_default == 0)
            call_irq_default = CALLBACK_Allocate();
        CALLBACK_Setup(call_irq_default, &Default_IRQ_Handler, CB_IRET, "irq default");
        RealSetVec(0x0b, CALLBACK_RealPointer(call_irq_default)); // IRQ 3
        RealSetVec(0x0c, CALLBACK_RealPointer(call_irq_default)); // IRQ 4
        RealSetVec(0x0d, CALLBACK_RealPointer(call_irq_default)); // IRQ 5
        RealSetVec(0x0f, CALLBACK_RealPointer(call_irq_default)); // IRQ 7
        RealSetVec(0x72, CALLBACK_RealPointer(call_irq_default)); // IRQ 10
        RealSetVec(0x73, CALLBACK_RealPointer(call_irq_default)); // IRQ 11

        // setup a few interrupt handlers that point to bios IRETs by default
        real_writed(0,0x66*4,CALLBACK_RealPointer(call_default));   //war2d
        real_writed(0,0x67*4,CALLBACK_RealPointer(call_default));
        if (null_68h) real_writed(0,0x68*4,0);  //Popcorn
        real_writed(0,0x5c*4,CALLBACK_RealPointer(call_default));   //Network stuff
        //real_writed(0,0xf*4,0); some games don't like it

        bios_first_init = false;

        DispatchVMEvent(VM_EVENT_BIOS_INIT);

        TIMER_BIOS_INIT_Configure();

        void INT10_Startup(Section *sec);
        INT10_Startup(NULL);

        /* INT 16 Keyboard handled in another file */
        BIOS_SetupKeyboard();

        {
            int4b_callback.Set_RealVec(0x4B,/*reinstall*/true);
            callback[1].Set_RealVec(0x11,/*reinstall*/true);
            callback[2].Set_RealVec(0x12,/*reinstall*/true);
            callback[3].Set_RealVec(0x14,/*reinstall*/true);
            callback[4].Set_RealVec(0x15,/*reinstall*/true);
            callback[5].Set_RealVec(0x17,/*reinstall*/true);
            callback[6].Set_RealVec(0x1A,/*reinstall*/true);
            callback[7].Set_RealVec(0x1C,/*reinstall*/true);
            callback[8].Set_RealVec(0x70,/*reinstall*/true);
            callback[9].Set_RealVec(0x71,/*reinstall*/true);
            callback[10].Set_RealVec(0x19,/*reinstall*/true);
            callback[11].Set_RealVec(0x76,/*reinstall*/true);
            callback[12].Set_RealVec(0x77,/*reinstall*/true);
            callback[13].Set_RealVec(0x0E,/*reinstall*/true);
            callback[15].Set_RealVec(0x18,/*reinstall*/true);
        }

        // FIXME: We're using IBM PC memory size storage even in PC-98 mode.
        //        This cannot be removed, because the DOS kernel uses this variable even in PC-98 mode.
        mem_writew(BIOS_MEMORY_SIZE,t_conv);

        RealSetVec(0x08,BIOS_DEFAULT_IRQ0_LOCATION);
        // pseudocode for CB_IRQ0:
        //  sti
        //  callback INT8_Handler
        //  push ds,ax,dx
        //  int 0x1c
        //  cli
        //  mov al, 0x20
        //  out 0x20, al
        //  pop dx,ax,ds
        //  iret

        {
            mem_writed(BIOS_TIMER,0);           //Calculate the correct time

            // INT 05h: Print Screen
            // IRQ1 handler calls it when PrtSc key is pressed; does nothing unless hooked
            phys_writeb(Real2Phys(BIOS_DEFAULT_INT5_LOCATION), 0xcf);
            RealSetVec(0x05, BIOS_DEFAULT_INT5_LOCATION);

            phys_writew(Real2Phys(RealGetVec(0x12))+0x12,0x20); //Hack for Jurresic
        }

        phys_writeb(Real2Phys(BIOS_DEFAULT_HANDLER_LOCATION),0xcf); /* bios default interrupt vector location -> IRET */

        {
            /* Setup some stuff in 0x40 bios segment */

            // Disney workaround
            //      Bit16u disney_port = mem_readw(BIOS_ADDRESS_LPT1);
            // port timeouts
            // always 1 second even if the port does not exist
            //      BIOS_SetLPTPort(0, disney_port);
            for(Bitu i = 1; i < 3; i++) BIOS_SetLPTPort(i, 0);
            mem_writeb(BIOS_COM1_TIMEOUT,1);
            mem_writeb(BIOS_COM2_TIMEOUT,1);
            mem_writeb(BIOS_COM3_TIMEOUT,1);
            mem_writeb(BIOS_COM4_TIMEOUT,1);
        }

        {
            /* Setup equipment list */
            // look http://www.bioscentral.com/misc/bda.htm

            //Bit16u config=0x4400; //1 Floppy, 2 serial and 1 parallel 
            Bit16u config = 0x0;

#if (C_FPU)
            extern bool enable_fpu;

            //FPU
            if (enable_fpu)
                config|=0x2;
#endif
            switch (machine) {
                case EGAVGA_ARCH_CASE:
                    //Startup 80x25 color
                    config|=0x20;
                    break;
                default:
                    //EGA VGA
                    config|=0;
                    break;
            }

            // Gameport
            config |= 0x1000;
            mem_writew(BIOS_CONFIGURATION,config);
            if (IS_EGAVGA_ARCH) config &= ~0x30; //EGA/VGA startup display mode differs in CMOS
        }

        {
            /* Setup extended memory size */
            IO_Write(0x70,0x30);
            size_extended=IO_Read(0x71);
            IO_Write(0x70,0x31);
            size_extended|=(IO_Read(0x71) << 8);
            BIOS_HostTimeSync();
        }

        {
            /* this belongs HERE not on-demand from INT 15h! */
            biosConfigSeg = ROMBIOS_GetMemory(16/*one paragraph*/,"BIOS configuration (INT 15h AH=0xC0)",/*paragraph align*/16)>>4;
            if (biosConfigSeg != 0) {
                PhysPt data = PhysMake(biosConfigSeg,0);
                phys_writew(data,8);                        // 8 Bytes following
                {
                    if (PS1AudioCard) { /* FIXME: Won't work because BIOS_Init() comes before PS1SOUND_Init() */
                        phys_writeb(data+2,0xFC);                   // Model ID (PC)
                        phys_writeb(data+3,0x0B);                   // Submodel ID (PS/1).
                    } else {
                        phys_writeb(data+2,0xFC);                   // Model ID (PC)
                        phys_writeb(data+3,0x00);                   // Submodel ID
                    }
                    phys_writeb(data+4,0x01);                   // Bios Revision
                    phys_writeb(data+5,(1<<6)|(1<<5)|(1<<4));   // Feature Byte 1
                }
                phys_writeb(data+6,(1<<6));             // Feature Byte 2
                phys_writeb(data+7,0);                  // Feature Byte 3
                phys_writeb(data+8,0);                  // Feature Byte 4
                phys_writeb(data+9,0);                  // Feature Byte 5
            }
        }

        {
            Section_prop * section=static_cast<Section_prop *>(control->GetSection("speaker"));
            bool bit0en = section->Get_bool("pcspeaker clock gate enable at startup");

            if (bit0en) {
                Bit8u x = IO_Read(0x61);
                IO_Write(0x61,(x & (~3u)) | 1u); /* set bits[1:0] = 01  (clock gate enable but output gate disable) */
                LOG_MSG("xxxx");
            }
        }

        CPU_STI();

        return CBRET_NONE;
    }
    CALLBACK_HandlerObject cb_bios_scan_video_bios;
    static Bitu cb_bios_scan_video_bios__func(void) {
        unsigned long size;

        if (cpu.pmode) E_Exit("BIOS error: VIDEO BIOS SCAN function called while in protected/vm86 mode");

        if (!bios_has_exec_vga_bios) {
            bios_has_exec_vga_bios = true;
            if (IS_EGAVGA_ARCH) {
                /* make sure VGA BIOS is there at 0xC000:0x0000 */
                if (AdapterROM_Read(0xC0000,&size)) {
                    LOG(LOG_MISC,LOG_DEBUG)("BIOS VIDEO ROM SCAN found VGA BIOS (size=%lu)",size);
                    adapter_scan_start = 0xC0000 + size;

                    // step back into the callback instruction that triggered this call
                    reg_eip -= 4;

                    // FAR CALL into the VGA BIOS
                    CPU_CALL(false,0xC000,0x0003,reg_eip);
                    return CBRET_NONE;
                }
                else {
                    LOG(LOG_MISC,LOG_WARN)("BIOS VIDEO ROM SCAN did not find VGA BIOS");
                }
            }
            else {
                // CGA, MDA, Tandy, PCjr. No video BIOS to scan for
            }
        }

        return CBRET_NONE;
    }
    CALLBACK_HandlerObject cb_bios_adapter_rom_scan;
    static Bitu cb_bios_adapter_rom_scan__func(void) {
        unsigned long size;
        Bit32u c1;

        if (cpu.pmode) E_Exit("BIOS error: ADAPTER ROM function called while in protected/vm86 mode");

        while (adapter_scan_start < 0xF0000) {
            if (AdapterROM_Read(adapter_scan_start,&size)) {
                Bit16u segm = (Bit16u)(adapter_scan_start >> 4);

                LOG(LOG_MISC,LOG_DEBUG)("BIOS ADAPTER ROM scan found ROM at 0x%lx (size=%lu)",(unsigned long)adapter_scan_start,size);

                c1 = mem_readd(adapter_scan_start+3);
                adapter_scan_start += size;
                if (c1 != 0UL) {
                    LOG(LOG_MISC,LOG_DEBUG)("Running ADAPTER ROM entry point");

                    // step back into the callback instruction that triggered this call
                    reg_eip -= 4;

                    // FAR CALL into the VGA BIOS
                    CPU_CALL(false,segm,0x0003,reg_eip);
                    return CBRET_NONE;
                }
                else {
                    LOG(LOG_MISC,LOG_DEBUG)("FIXME: ADAPTER ROM entry point does not exist");
                }
            }
            else {
                if (IS_EGAVGA_ARCH) // supposedly newer systems only scan on 2KB boundaries by standard? right?
                    adapter_scan_start = (adapter_scan_start | 2047UL) + 1UL;
                else // while older PC/XT systems scanned on 512-byte boundaries? right?
                    adapter_scan_start = (adapter_scan_start | 511UL) + 1UL;
            }
        }

        LOG(LOG_MISC,LOG_DEBUG)("BIOS ADAPTER ROM scan complete");
        return CBRET_NONE;
    }
    CALLBACK_HandlerObject cb_bios_startup_screen;
    static Bitu cb_bios_startup_screen__func(void) {
        return CBRET_NONE;
    }
    CALLBACK_HandlerObject cb_bios_boot;
    CALLBACK_HandlerObject cb_bios_bootfail;
    static Bitu cb_bios_bootfail__func(void) {
        int x,y;

        x = y = 0;

        /* PC-98 MS-DOS boot sector may RETF back to the BIOS, and this is where execution ends up */
        BIOS_Int10RightJustifiedPrint(x,y,"Guest OS failed to boot, returned failure");

        /* and then after this call, there is a JMP $ to loop endlessly */
        return CBRET_NONE;
    }
    static Bitu cb_bios_boot__func(void) {
        /* Reset/power-on overrides the user's A20 gate preferences.
         * It's time to revert back to what the user wants. */
        void A20Gate_TakeUserSetting(Section *sec);
        void MEM_A20_Enable(bool enabled);
        A20Gate_TakeUserSetting(NULL);
        MEM_A20_Enable(false);

        if (cpu.pmode) E_Exit("BIOS error: BOOT function called while in protected/vm86 mode");
        DispatchVMEvent(VM_EVENT_BIOS_BOOT);

        // TODO: If instructed to, follow the INT 19h boot pattern, perhaps follow the BIOS Boot Specification, etc.

        // TODO: If instructed to boot a guest OS...

        /* wipe out the stack so it's not there to interfere with the system */
        reg_esp = 0;
        reg_eip = 0;
        CPU_SetSegGeneral(cs, 0x60);
        CPU_SetSegGeneral(ss, 0x60);

        for (Bitu i=0;i < 0x400;i++) mem_writeb(0x7C00+i,0);

        // Begin booting the DOSBox shell. NOTE: VM_Boot_DOSBox_Kernel will change CS:IP instruction pointer!
        if (!VM_Boot_DOSBox_Kernel()) E_Exit("BIOS error: BOOT function failed to boot DOSBox kernel");
        return CBRET_NONE;
    }
public:
    void write_FFFF_signature() {
        /* write the signature at 0xF000:0xFFF0 */

        // The farjump at the processor reset entry point (jumps to POST routine)
        phys_writeb(0xffff0,0xEA);                  // FARJMP
        phys_writew(0xffff1,RealOff(BIOS_DEFAULT_RESET_LOCATION));  // offset
        phys_writew(0xffff3,RealSeg(BIOS_DEFAULT_RESET_LOCATION));  // segment

        // write system BIOS date
        for(Bitu i = 0; i < strlen(bios_date_string); i++) phys_writeb(0xffff5+i,(Bit8u)bios_date_string[i]);

        /* model byte */
        phys_writeb(0xffffe,0xfc); /* PC (FIXME: This is listed as model byte PS/2 model 60) */

        // signature
        phys_writeb(0xfffff,0x55);
    }
    BIOS(Section* configuration):Module_base(configuration){
        { // TODO: Eventually, move this to BIOS POST or init phase
            Section_prop * section=static_cast<Section_prop *>(control->GetSection("dosbox"));

            // NTS: This setting is also valid in PC-98 mode. According to Undocumented PC-98 by Webtech,
            //      there's nothing at I/O port E9h. I will move the I/O port in PC-98 mode if there is in
            //      fact a conflict. --J.C.
            bochs_port_e9 = section->Get_bool("bochs debug port e9");

            // TODO: motherboard init, especially when we get around to full Intel Triton/i440FX chipset emulation
            isa_memory_hole_512kb = section->Get_bool("isa memory hole at 512kb");

            // FIXME: Erm, well this couldv'e been named better. It refers to the amount of conventional memory
            //        made available to the operating system below 1MB, which is usually DOS.
            dos_conventional_limit = (unsigned int)section->Get_int("dos mem limit");

            {
                std::string s = section->Get_string("unhandled irq handler");

                if (s == "simple")
                    unhandled_irq_method = UNHANDLED_IRQ_SIMPLE;
                else if (s == "cooperative_2nd")
                    unhandled_irq_method = UNHANDLED_IRQ_COOPERATIVE_2ND;
                else
                    unhandled_irq_method = UNHANDLED_IRQ_SIMPLE;
            }
        }

        /* pick locations */
        BIOS_DEFAULT_RESET_LOCATION = PhysToReal416(ROMBIOS_GetMemory(64/*several callbacks*/,"BIOS default reset location",/*align*/4));
        BIOS_DEFAULT_HANDLER_LOCATION = PhysToReal416(ROMBIOS_GetMemory(1/*IRET*/,"BIOS default handler location",/*align*/4));
        BIOS_DEFAULT_INT5_LOCATION = PhysToReal416(ROMBIOS_GetMemory(1/*IRET*/, "BIOS default INT5 location",/*align*/4));
        BIOS_DEFAULT_IRQ0_LOCATION = PhysToReal416(ROMBIOS_GetMemory(0x13/*see callback.cpp for IRQ0*/,"BIOS default IRQ0 location",/*align*/4));
        BIOS_DEFAULT_IRQ1_LOCATION = PhysToReal416(ROMBIOS_GetMemory(0x20/*see callback.cpp for IRQ1*/,"BIOS default IRQ1 location",/*align*/4));
        BIOS_DEFAULT_IRQ07_DEF_LOCATION = PhysToReal416(ROMBIOS_GetMemory(7/*see callback.cpp for EOI_PIC1*/,"BIOS default IRQ2-7 location",/*align*/4));
        BIOS_DEFAULT_IRQ815_DEF_LOCATION = PhysToReal416(ROMBIOS_GetMemory(9/*see callback.cpp for EOI_PIC1*/,"BIOS default IRQ8-15 location",/*align*/4));

        write_FFFF_signature();

        /* Setup all the interrupt handlers the bios controls */

        /* INT 8 Clock IRQ Handler */
        call_irq0=CALLBACK_Allocate();
        CALLBACK_Setup(call_irq0,INT8_Handler,CB_IRQ0,Real2Phys(BIOS_DEFAULT_IRQ0_LOCATION),"IRQ 0 Clock");

        /* INT 11 Get equipment list */
        callback[1].Install(&INT11_Handler,CB_IRET,"Int 11 Equipment");

        /* INT 12 Memory Size default at 640 kb */
        callback[2].Install(&INT12_Handler,CB_IRET,"Int 12 Memory");

        ulimit = 640;
        t_conv = MEM_TotalPages() << 2; /* convert 4096/byte pages -> 1024/byte KB units */
        if (allow_more_than_640kb) {
            if (t_conv > ulimit) t_conv = ulimit;
            if (t_conv > 640) { /* because the memory emulation has already set things up */
                bool MEM_map_RAM_physmem(Bitu start,Bitu end);
                MEM_map_RAM_physmem(0xA0000,(t_conv<<10)-1);
                memset(GetMemBase()+(640<<10),0,(t_conv-640)<<10);
            }
        }
        else {
            if (t_conv > 640) t_conv = 640;
        }

        /* allow user to further limit the available memory below 1MB */
        if (dos_conventional_limit != 0 && t_conv > dos_conventional_limit)
            t_conv = dos_conventional_limit;

        // TODO: Allow dosbox.conf to specify an option to add an EBDA (Extended BIOS Data Area)
        //       at the top of the DOS conventional limit, which we then reduce further to hold
        //       it. Most BIOSes past 1992 or so allocate an EBDA.

        /* if requested to emulate an ISA memory hole at 512KB, further limit the memory */
        if (isa_memory_hole_512kb && t_conv > 512) t_conv = 512;

        /* and then unmap RAM between t_conv and ulimit */
        if (t_conv < ulimit) {
            Bitu start = (t_conv+3)/4;  /* start = 1KB to page round up */
            Bitu end = ulimit/4;        /* end = 1KB to page round down */
            if (start < end) MEM_ResetPageHandler_Unmapped(start,end-start);
        }

        /* INT 4B. Now we can safely signal error instead of printing "Invalid interrupt 4B"
         * whenever we install Windows 95. Note that Windows 95 would call INT 4Bh because
         * that is where the Virtual DMA API lies in relation to EMM386.EXE */
        int4b_callback.Install(&INT4B_Handler,CB_IRET,"INT 4B");

        /* INT 14 Serial Ports */
        callback[3].Install(&INT14_Handler,CB_IRET_STI,"Int 14 COM-port");

        /* INT 15 Misc Calls */
        callback[4].Install(&INT15_Handler,CB_IRET,"Int 15 Bios");

        /* INT 17 Printer Routines */
        callback[5].Install(&INT17_Handler,CB_IRET_STI,"Int 17 Printer");

        /* INT 1A TIME and some other functions */
        callback[6].Install(&INT1A_Handler,CB_IRET_STI,"Int 1a Time");

        /* INT 1C System Timer tick called from INT 8 */
        callback[7].Install(&INT1C_Handler,CB_IRET,"Int 1c Timer");
        
        /* IRQ 8 RTC Handler */
        callback[8].Install(&INT70_Handler,CB_IRET,"Int 70 RTC");

        /* Irq 9 rerouted to irq 2 */
        callback[9].Install(NULL,CB_IRQ9,"irq 9 bios");

        // INT 19h: Boot function
        callback[10].Install(&INT19_Handler,CB_IRET,"int 19");

        // INT 76h: IDE IRQ 14
        // This is just a dummy IRQ handler to prevent crashes when
        // IDE emulation fires the IRQ and OS's like Win95 expect
        // the BIOS to handle the interrupt.
        // FIXME: Shouldn't the IRQ send an ACK to the PIC as well?!?
        callback[11].Install(&IRQ14_Dummy,CB_IRET_EOI_PIC2,"irq 14 ide");

        // INT 77h: IDE IRQ 15
        // This is just a dummy IRQ handler to prevent crashes when
        // IDE emulation fires the IRQ and OS's like Win95 expect
        // the BIOS to handle the interrupt.
        // FIXME: Shouldn't the IRQ send an ACK to the PIC as well?!?
        callback[12].Install(&IRQ15_Dummy,CB_IRET_EOI_PIC2,"irq 15 ide");

        // INT 0Eh: IDE IRQ 6
        callback[13].Install(&IRQ15_Dummy,CB_IRET_EOI_PIC1,"irq 6 floppy");

        // INT 18h: Enter BASIC
        // Non-IBM BIOS would display "NO ROM BASIC" here
        callback[15].Install(&INT18_Handler,CB_IRET,"int 18");

        init_vm86_fake_io();

        /* Irq 2-7 */
        call_irq07default=CALLBACK_Allocate();

        /* Irq 8-15 */
        call_irq815default=CALLBACK_Allocate();

        /* BIOS boot stages */
        cb_bios_post.Install(&cb_bios_post__func,CB_RETF,"BIOS POST");
        cb_bios_scan_video_bios.Install(&cb_bios_scan_video_bios__func,CB_RETF,"BIOS Scan Video BIOS");
        cb_bios_adapter_rom_scan.Install(&cb_bios_adapter_rom_scan__func,CB_RETF,"BIOS Adapter ROM scan");
        cb_bios_startup_screen.Install(&cb_bios_startup_screen__func,CB_RETF,"BIOS Startup screen");
        cb_bios_boot.Install(&cb_bios_boot__func,CB_RETF,"BIOS BOOT");
        cb_bios_bootfail.Install(&cb_bios_bootfail__func,CB_RETF,"BIOS BOOT FAIL");

        // Compatible POST routine location: jump to the callback
        {
            Bitu wo_fence;

            Bitu wo = Real2Phys(BIOS_DEFAULT_RESET_LOCATION);
            wo_fence = wo + 64;

            // POST
            phys_writeb(wo+0x00,(Bit8u)0xFE);                       //GRP 4
            phys_writeb(wo+0x01,(Bit8u)0x38);                       //Extra Callback instruction
            phys_writew(wo+0x02,(Bit16u)cb_bios_post.Get_callback());           //The immediate word
            wo += 4;

            // video bios scan
            phys_writeb(wo+0x00,(Bit8u)0xFE);                       //GRP 4
            phys_writeb(wo+0x01,(Bit8u)0x38);                       //Extra Callback instruction
            phys_writew(wo+0x02,(Bit16u)cb_bios_scan_video_bios.Get_callback());        //The immediate word
            wo += 4;

            // adapter ROM scan
            phys_writeb(wo+0x00,(Bit8u)0xFE);                       //GRP 4
            phys_writeb(wo+0x01,(Bit8u)0x38);                       //Extra Callback instruction
            phys_writew(wo+0x02,(Bit16u)cb_bios_adapter_rom_scan.Get_callback());       //The immediate word
            wo += 4;

            // startup screen
            phys_writeb(wo+0x00,(Bit8u)0xFE);                       //GRP 4
            phys_writeb(wo+0x01,(Bit8u)0x38);                       //Extra Callback instruction
            phys_writew(wo+0x02,(Bit16u)cb_bios_startup_screen.Get_callback());     //The immediate word
            wo += 4;

            // user boot hook
            if (bios_user_boot_hook != 0) {
                phys_writeb(wo+0x00,0x9C);                          //PUSHF
                phys_writeb(wo+0x01,0x9A);                          //CALL FAR
                phys_writew(wo+0x02,0x0000);                        //seg:0
                phys_writew(wo+0x04,bios_user_boot_hook>>4);
                wo += 6;
            }

            // boot
            BIOS_boot_code_offset = wo;
            phys_writeb(wo+0x00,(Bit8u)0xFE);                       //GRP 4
            phys_writeb(wo+0x01,(Bit8u)0x38);                       //Extra Callback instruction
            phys_writew(wo+0x02,(Bit16u)cb_bios_boot.Get_callback());           //The immediate word
            wo += 4;

            // boot fail
            BIOS_bootfail_code_offset = wo;
            phys_writeb(wo+0x00,(Bit8u)0xFE);                       //GRP 4
            phys_writeb(wo+0x01,(Bit8u)0x38);                       //Extra Callback instruction
            phys_writew(wo+0x02,(Bit16u)cb_bios_bootfail.Get_callback());           //The immediate word
            wo += 4;

            /* fence */
            phys_writeb(wo++,0xEB);                             // JMP $-2
            phys_writeb(wo++,0xFE);

            if (wo > wo_fence) E_Exit("BIOS boot callback overrun");
        }
    }
    ~BIOS(){
        /* snap the CPU back to real mode. this code thinks in terms of 16-bit real mode
         * and if allowed to do it's thing in a 32-bit guest OS like WinNT, will trigger
         * a page fault. */
        CPU_Snap_Back_To_Real_Mode();

        real_writeb(0x40,0xd4,0x00);

        /* encourage the callback objects to uninstall HERE while we're in real mode, NOT during the
         * destructor stage where we're back in protected mode */
        for (unsigned int i=0;i < 20;i++) callback[i].Uninstall();

        /* assume these were allocated */
        CALLBACK_DeAllocate(call_irq0);
        CALLBACK_DeAllocate(call_irq07default);
        CALLBACK_DeAllocate(call_irq815default);

        /* done */
        CPU_Snap_Back_Restore();
    }
};

void BIOS_Enter_Boot_Phase(void) {
    /* direct CS:IP right to the instruction that leads to the boot process */
    /* note that since it's a callback instruction it doesn't really matter
     * what CS:IP is as long as it points to the instruction */
    reg_eip = BIOS_boot_code_offset & 0xFUL;
    CPU_SetSegGeneral(cs, BIOS_boot_code_offset >> 4UL);
}

void BIOS_SetCOMPort(Bitu port, Bit16u baseaddr) {
    switch(port) {
    case 0:
        mem_writew(BIOS_BASE_ADDRESS_COM1,baseaddr);
        mem_writeb(BIOS_COM1_TIMEOUT, 10); // FIXME: Right??
        break;
    case 1:
        mem_writew(BIOS_BASE_ADDRESS_COM2,baseaddr);
        mem_writeb(BIOS_COM2_TIMEOUT, 10); // FIXME: Right??
        break;
    case 2:
        mem_writew(BIOS_BASE_ADDRESS_COM3,baseaddr);
        mem_writeb(BIOS_COM3_TIMEOUT, 10); // FIXME: Right??
        break;
    case 3:
        mem_writew(BIOS_BASE_ADDRESS_COM4,baseaddr);
        mem_writeb(BIOS_COM4_TIMEOUT, 10); // FIXME: Right??
        break;
    }
}

void BIOS_SetLPTPort(Bitu port, Bit16u baseaddr) {
    switch(port) {
    case 0:
        mem_writew(BIOS_ADDRESS_LPT1,baseaddr);
        mem_writeb(BIOS_LPT1_TIMEOUT, 10);
        break;
    case 1:
        mem_writew(BIOS_ADDRESS_LPT2,baseaddr);
        mem_writeb(BIOS_LPT2_TIMEOUT, 10);
        break;
    case 2:
        mem_writew(BIOS_ADDRESS_LPT3,baseaddr);
        mem_writeb(BIOS_LPT3_TIMEOUT, 10);
        break;
    }
}

static BIOS* test = NULL;

void BIOS_Destroy(Section* /*sec*/){
    ROMBIOS_DumpMemory();
    if (test != NULL) {
        delete test;
        test = NULL;
    }
}

void BIOS_OnPowerOn(Section* sec) {
    (void)sec;//UNUSED
    if (test) delete test;
    test = new BIOS(control->GetSection("joystick"));
}

void INT10_OnResetComplete();
void CALLBACK_DeAllocate(Bitu in);

void BIOS_OnResetComplete(Section *x) {
    (void)x;//UNUSED
    INT10_OnResetComplete();

    if (biosConfigSeg != 0u) {
        ROMBIOS_FreeMemory((Bitu)(biosConfigSeg << 4u)); /* remember it was alloc'd paragraph aligned, then saved >> 4 */
        biosConfigSeg = 0u;
    }

    call_pnp_rp = 0;
    if (call_pnp_r != ~0UL) {
        CALLBACK_DeAllocate(call_pnp_r);
        call_pnp_r = ~0UL;
    }

    call_pnp_pp = 0;
    if (call_pnp_p != ~0UL) {
        CALLBACK_DeAllocate(call_pnp_p);
        call_pnp_p = ~0UL;
    }
}

void BIOS_Init() {
    LOG(LOG_MISC,LOG_DEBUG)("Initializing BIOS");

    /* NTS: VM_EVENT_BIOS_INIT this callback must be first. */
    AddExitFunction(AddExitFunctionFuncPair(BIOS_Destroy),false);
    AddVMEventFunction(VM_EVENT_POWERON,AddVMEventFunctionFuncPair(BIOS_OnPowerOn));
    AddVMEventFunction(VM_EVENT_RESET_END,AddVMEventFunctionFuncPair(BIOS_OnResetComplete));
}

void write_ID_version_string() {
    Bitu str_id_at,str_ver_at;
    size_t str_id_len,str_ver_len;

    /* NTS: We can't move the version and ID strings, it causes programs like MSD.EXE to lose
     *      track of the "IBM compatible blahblahblah" string. Which means that apparently
     *      programs looking for this information have the address hardcoded ALTHOUGH
     *      experiments show you can move the version string around so long as it's
     *      +1 from a paragraph boundary */
    /* TODO: *DO* allow dynamic relocation however if the dosbox.conf indicates that the user
     *       is not interested in IBM BIOS compatability. Also, it would be really cool if
     *       dosbox.conf could override these strings and the user could enter custom BIOS
     *       version and ID strings. Heh heh heh.. :) */
    str_id_at = 0xFE00E;
    str_ver_at = 0xFE061;
    str_id_len = strlen(bios_type_string)+1;
    str_ver_len = strlen(bios_version_string)+1;
    {
        /* need to mark these strings off-limits so dynamic allocation does not overwrite them */
        ROMBIOS_GetMemory((Bitu)str_id_len+1,"BIOS ID string",1,str_id_at);
        ROMBIOS_GetMemory((Bitu)str_ver_len+1,"BIOS version string",1,str_ver_at);
    }
    if (str_id_at != 0) {
        for (size_t i=0;i < str_id_len;i++) phys_writeb(str_id_at+(PhysPt)i,(Bit8u)bios_type_string[i]);
    }
    if (str_ver_at != 0) {
        for (size_t i=0;i < str_ver_len;i++) phys_writeb(str_ver_at+(PhysPt)i,(Bit8u)bios_version_string[i]);
    }
}

/* NTS: Do not use callbacks! This function is called before CALLBACK_Init() */
void ROMBIOS_Init() {
    Section_prop * section=static_cast<Section_prop *>(control->GetSection("dosbox"));
    Bitu oi;

    // log
    LOG(LOG_MISC,LOG_DEBUG)("Initializing ROM BIOS");

    oi = (Bitu)section->Get_int("rom bios minimum size"); /* in KB */
    oi = (oi + 3u) & ~3u; /* round to 4KB page */
    if (oi > 128u) oi = 128u;
    if (oi == 0u) {
        oi = 64u;
    }
    if (oi < 8) oi = 8; /* because of some of DOSBox's fixed ROM structures we can only go down to 8KB */
    rombios_minimum_size = (oi << 10); /* convert to minimum, using size coming downward from 1MB */

    oi = (Bitu)section->Get_int("rom bios allocation max"); /* in KB */
    oi = (oi + 3u) & ~3u; /* round to 4KB page */
    if (oi > 128u) oi = 128u;
    if (oi == 0u) {
        oi = 64u;
    }
    if (oi < 8u) oi = 8u; /* because of some of DOSBox's fixed ROM structures we can only go down to 8KB */
    oi <<= 10u;
    if (oi < rombios_minimum_size) oi = rombios_minimum_size;
    rombios_minimum_location = 0x100000ul - oi; /* convert to minimum, using size coming downward from 1MB */

    LOG(LOG_BIOS,LOG_DEBUG)("ROM BIOS range: 0x%05X-0xFFFFF",(int)rombios_minimum_location);
    LOG(LOG_BIOS,LOG_DEBUG)("ROM BIOS range according to minimum size: 0x%05X-0xFFFFF",(int)(0x100000 - rombios_minimum_size));

    if (!MEM_map_ROM_physmem(rombios_minimum_location,0xFFFFF)) E_Exit("Unable to map ROM region as ROM");

    /* and the BIOS alias at the top of memory (TODO: what about 486/Pentium emulation where the BIOS at the 4GB top is different
     * from the BIOS at the legacy 1MB boundary because of shadowing and/or decompressing from ROM at boot? */
    {
        uint64_t top = (uint64_t)1UL << (uint64_t)MEM_get_address_bits();
        if (top >= ((uint64_t)1UL << (uint64_t)21UL)) { /* 2MB or more */
            unsigned long alias_base,alias_end;

            alias_base = (unsigned long)top + (unsigned long)rombios_minimum_location - (unsigned long)0x100000UL;
            alias_end = (unsigned long)top - (unsigned long)1UL;

            LOG(LOG_BIOS,LOG_DEBUG)("ROM BIOS also mapping alias to 0x%08lx-0x%08lx",alias_base,alias_end);
            if (!MEM_map_ROM_alias_physmem(alias_base,alias_end)) {
                void MEM_cut_RAM_up_to(Bitu addr);

                /* it's possible if memory aliasing is set that memsize is too large to make room.
                 * let memory emulation know where the ROM BIOS starts so it can unmap the RAM pages,
                 * reduce the memory reported to the OS, and try again... */
                LOG(LOG_BIOS,LOG_DEBUG)("No room for ROM BIOS alias, reducing reported memory and unmapping RAM pages to make room");
                MEM_cut_RAM_up_to(alias_base);

                if (!MEM_map_ROM_alias_physmem(alias_base,alias_end))
                    E_Exit("Unable to map ROM region as ROM alias");
            }
        }
    }

    /* set up allocation */
    rombios_alloc.name = "ROM BIOS";
    rombios_alloc.topDownAlloc = true;
    rombios_alloc.initSetRange(rombios_minimum_location,0xFFFF0 - 1);

    write_ID_version_string();

    /* we allow dosbox.conf to specify a binary blob to load into ROM BIOS and execute after reset.
     * we allow this for both hacker curiosity and automated CPU testing. */
    {
        std::string path = section->Get_string("call binary on reset");
        struct stat st;

        if (!path.empty() && stat(path.c_str(),&st) == 0 && S_ISREG(st.st_mode) && st.st_size <= (off_t)(128u*1024u)) {
            Bitu base = ROMBIOS_GetMemory((Bitu)st.st_size,"User reset vector binary",16u/*page align*/,0u);

            if (base != 0) {
                FILE *fp = fopen(path.c_str(),"rb");

                if (fp != NULL) {
                    /* NTS: Make sure memory base != NULL, and that it fits within 1MB.
                     *      memory code allocates a minimum 1MB of memory even if
                     *      guest memory is less than 1MB because ROM BIOS emulation
                     *      depends on it. */
                    assert(GetMemBase() != NULL);
                    assert((base+(Bitu)st.st_size) <= 0x100000ul);
                    size_t readResult = fread(GetMemBase()+base,(size_t)st.st_size,1u,fp);
                    fclose(fp);
                    if (readResult != 1) {
                        LOG(LOG_IO, LOG_ERROR) ("Reading error in ROMBIOS_Init\n");
                        return;
                    }

                    LOG_MSG("User reset vector binary '%s' loaded at 0x%lx",path.c_str(),(unsigned long)base);
                    bios_user_reset_vector_blob = base;
                }
                else {
                    LOG_MSG("WARNING: Unable to open file to load user reset vector binary '%s' into ROM BIOS memory",path.c_str());
                }
            }
            else {
                LOG_MSG("WARNING: Unable to load user reset vector binary '%s' into ROM BIOS memory",path.c_str());
            }
        }
    }

    /* we allow dosbox.conf to specify a binary blob to load into ROM BIOS and execute just before boot.
     * we allow this for both hacker curiosity and automated CPU testing. */
    {
        std::string path = section->Get_string("call binary on boot");
        struct stat st;

        if (!path.empty() && stat(path.c_str(),&st) == 0 && S_ISREG(st.st_mode) && st.st_size <= (off_t)(128u*1024u)) {
            Bitu base = ROMBIOS_GetMemory((Bitu)st.st_size,"User boot hook binary",16u/*page align*/,0u);

            if (base != 0) {
                FILE *fp = fopen(path.c_str(),"rb");

                if (fp != NULL) {
                    /* NTS: Make sure memory base != NULL, and that it fits within 1MB.
                     *      memory code allocates a minimum 1MB of memory even if
                     *      guest memory is less than 1MB because ROM BIOS emulation
                     *      depends on it. */
                    assert(GetMemBase() != NULL);
                    assert((base+(Bitu)st.st_size) <= 0x100000ul);
                    size_t readResult = fread(GetMemBase()+base,(size_t)st.st_size,1u,fp);
                    fclose(fp);
                    if (readResult != 1) {
                        LOG(LOG_IO, LOG_ERROR) ("Reading error in ROMBIOS_Init\n");
                        return;
                    }

                    LOG_MSG("User boot hook binary '%s' loaded at 0x%lx",path.c_str(),(unsigned long)base);
                    bios_user_boot_hook = base;
                }
                else {
                    LOG_MSG("WARNING: Unable to open file to load user boot hook binary '%s' into ROM BIOS memory",path.c_str());
                }
            }
            else {
                LOG_MSG("WARNING: Unable to load user boot hook binary '%s' into ROM BIOS memory",path.c_str());
            }
        }
    }
}

//! \brief Updates the state of a lockable key.
void UpdateKeyWithLed(int nVirtKey, int flagAct, int flagLed);

void BIOS_SynchronizeNumLock()
{
#if defined(WIN32)
	UpdateKeyWithLed(VK_NUMLOCK, BIOS_KEYBOARD_FLAGS1_NUMLOCK_ACTIVE, BIOS_KEYBOARD_LEDS_NUM_LOCK);
#endif
}

void BIOS_SynchronizeCapsLock()
{
#if defined(WIN32)
	UpdateKeyWithLed(VK_CAPITAL, BIOS_KEYBOARD_FLAGS1_CAPS_LOCK_ACTIVE, BIOS_KEYBOARD_LEDS_CAPS_LOCK);
#endif
}

void BIOS_SynchronizeScrollLock()
{
#if defined(WIN32)
	UpdateKeyWithLed(VK_SCROLL, BIOS_KEYBOARD_FLAGS1_SCROLL_LOCK_ACTIVE, BIOS_KEYBOARD_LEDS_SCROLL_LOCK);
#endif
}

void UpdateKeyWithLed(int nVirtKey, int flagAct, int flagLed)
{
#if defined(WIN32)

	const auto state = GetKeyState(nVirtKey);

	const auto flags1 = BIOS_KEYBOARD_FLAGS1;
	const auto flags2 = BIOS_KEYBOARD_LEDS;

	auto flag1 = mem_readb(flags1);
	auto flag2 = mem_readb(flags2);

	if (state & 1)
	{
		flag1 |= flagAct;
		flag2 |= flagLed;
	}
	else
	{
		flag1 &= ~flagAct;
		flag2 &= ~flagLed;
	}

	mem_writeb(flags1, flag1);
	mem_writeb(flags2, flag2);

#else

    (void)nVirtKey;
    (void)flagAct;
    (void)flagLed;

#endif
}
