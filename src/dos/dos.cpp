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
#include <ctype.h>
#include "dosbox.h"
#include "dos_inc.h"
#include "bios.h"
#include "mem.h"
#include "paging.h"
#include "callback.h"
#include "regs.h"
#include "menu.h"
#include "mapper.h"
#include "drives.h"
#include "dos_inc.h"
#include "setup.h"
#include "support.h"
#include "parport.h"
#include "serialport.h"
#include "control.h"

Bitu INT29_HANDLER(void);

int ascii_toupper(int c) {
    if (c >= 'a' && c <= 'z')
        return c + 'A' - 'a';

    return c;
}

bool shiftjis_lead_byte(int c) {
    if ((((unsigned char)c & 0xE0) == 0x80) ||
        (((unsigned char)c & 0xE0) == 0xE0))
        return true;

    return false;
}

char * shiftjis_upcase(char * str) {
    for (char* idx = str; *idx ; ) {
        if (shiftjis_lead_byte(*idx)) {
            /* Shift-JIS is NOT ASCII and should not be converted to uppercase like ASCII.
             * The trailing byte can be mistaken for ASCII */
            idx++;
            if (*idx != 0) idx++;
        }
        else {
            *idx = ascii_toupper(*reinterpret_cast<unsigned char*>(idx));
            idx++;
        }
    }

    return str;
}

unsigned char cpm_compat_mode = CPM_COMPAT_MSDOS5;

bool dos_in_hma = true;
bool DOS_BreakFlag = false;
bool enable_dbcs_tables = true;
bool enable_filenamechar = true;
bool enable_share_exe_fake = true;
int dos_initial_hma_free = 34*1024;
int dos_sda_size = 0x560;

extern bool int15_wait_force_unmask_irq;

Bit32u dos_hma_allocator = 0; /* physical memory addr */

Bitu XMS_EnableA20(bool enable);
Bitu XMS_GetEnabledA20(void);
bool XMS_IS_ACTIVE();
bool XMS_HMA_EXISTS();

bool DOS_IS_IN_HMA() {
	if (dos_in_hma && XMS_IS_ACTIVE() && XMS_HMA_EXISTS())
		return true;

	return false;
}

Bit32u DOS_HMA_LIMIT() {
	if (dos.version.major < 5) return 0; /* MS-DOS 5.0+ only */
	if (!DOS_IS_IN_HMA()) return 0;
	return (0x110000 - 16); /* 1MB + 64KB - 16 bytes == (FFFF:FFFF + 1) == (0xFFFF0 + 0xFFFF + 1) == 0x10FFF0 */
}

Bit32u DOS_HMA_FREE_START() {
	if (dos.version.major < 5) return 0; /* MS-DOS 5.0+ only */
	if (!DOS_IS_IN_HMA()) return 0;

	if (dos_hma_allocator == 0) {
		dos_hma_allocator = 0x110000u - 16u - (unsigned int)dos_initial_hma_free;
		LOG(LOG_MISC,LOG_DEBUG)("Starting HMA allocation from physical address 0x%06x (FFFF:%04x)",
			dos_hma_allocator,(dos_hma_allocator+0x10u)&0xFFFFu);
	}

	return dos_hma_allocator;
}

Bit32u DOS_HMA_GET_FREE_SPACE() {
	Bit32u start;

	if (dos.version.major < 5) return 0; /* MS-DOS 5.0+ only */
	if (!DOS_IS_IN_HMA()) return 0;
	start = DOS_HMA_FREE_START();
	if (start == 0) return 0;
	return (DOS_HMA_LIMIT() - start);
}

void DOS_HMA_CLAIMED(Bit16u bytes) {
	Bit32u limit = DOS_HMA_LIMIT();

	if (limit == 0) E_Exit("HMA allocatiom bug: Claim function called when HMA allocation is not enabled");
	if (dos_hma_allocator == 0) E_Exit("HMA allocatiom bug: Claim function called without having determined start");
	dos_hma_allocator += bytes;
	if (dos_hma_allocator > limit) E_Exit("HMA allocation bug: Exceeded limit");
}

Bit16u DOS_INFOBLOCK_SEG=0x80;	// sysvars (list of lists)
Bit16u DOS_CONDRV_SEG=0xa0;
Bit16u DOS_CONSTRING_SEG=0xa8;
Bit16u DOS_SDA_SEG=0xb2;		// dos swappable area
Bit16u DOS_SDA_SEG_SIZE=0x560;  // WordPerfect 5.1 consideration (emendelson)
Bit16u DOS_SDA_OFS=0;
Bit16u DOS_CDS_SEG=0x108;
Bit16u DOS_MEM_START=0x158;	 // regression to r3437 fixes nascar 2 colors
Bit16u minimum_mcb_segment=0x70;
Bit16u minimum_mcb_free=0x70;
Bit16u minimum_dos_initial_private_segment=0x70;

Bit16u DOS_PRIVATE_SEGMENT=0;//0xc800;
Bit16u DOS_PRIVATE_SEGMENT_END=0;//0xd000;

Bitu DOS_PRIVATE_SEGMENT_Size=0x800;	// 32KB (0x800 pages), mainline DOSBox behavior

bool enable_dummy_device_mcb = true;

extern unsigned int MAXENV;// = 32768u;
extern unsigned int ENV_KEEPFREE;// = 83;

DOS_Block dos;
DOS_InfoBlock dos_infoblock;

extern bool dos_kernel_disabled;

Bit16u DOS_Block::psp() {
	if (dos_kernel_disabled) {
		LOG_MSG("BUG: DOS kernel is disabled (booting a guest OS), and yet somebody is still asking for DOS's current PSP segment\n");
		return 0x0000;
	}

	return DOS_SDA(DOS_SDA_SEG,DOS_SDA_OFS).GetPSP();
}

void DOS_Block::psp(Bit16u _seg) {
	if (dos_kernel_disabled) {
		LOG_MSG("BUG: DOS kernel is disabled (booting a guest OS), and yet somebody is still attempting to change DOS's current PSP segment\n");
		return;
	}

	DOS_SDA(DOS_SDA_SEG,DOS_SDA_OFS).SetPSP(_seg);
}

RealPt DOS_Block::dta() {
	if (dos_kernel_disabled) {
		LOG_MSG("BUG: DOS kernel is disabled (booting a guest OS), and yet somebody is still asking for DOS's DTA (disk transfer address)\n");
		return 0;
	}

	return DOS_SDA(DOS_SDA_SEG,DOS_SDA_OFS).GetDTA();
}

void DOS_Block::dta(RealPt _dta) {
	if (dos_kernel_disabled) {
		LOG_MSG("BUG: DOS kernel is disabled (booting a guest OS), and yet somebody is still attempting to change DOS's DTA (disk transfer address)\n");
		return;
	}

	DOS_SDA(DOS_SDA_SEG,DOS_SDA_OFS).SetDTA(_dta);
}

#define DOS_COPYBUFSIZE 0x10000
Bit8u dos_copybuf[DOS_COPYBUFSIZE];
#ifdef WIN32
Bit16u	NetworkHandleList[127];Bit8u dos_copybuf_second[DOS_COPYBUFSIZE];
#endif

void DOS_SetError(Bit16u code) {
	dos.errorcode=code;
}

const Bit8u DOS_DATE_months[] = {
	0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

#define DATA_TRANSFERS_TAKE_CYCLES 1
#define DOS_OVERHEAD 1

#ifndef DOSBOX_CPU_H
#include "cpu.h"
#endif

// TODO: Make this configurable.
//       Additionally, allow this to vary per-drive so that
//       Drive D: can be as slow as a 2X IDE CD-ROM drive in PIO mode
//       Drive C: can be as slow as a IDE drive in PIO mode and
//       Drive A: can be as slow as a 3.5" 1.44MB floppy disk
//
// This fixes MS-DOS games that crash or malfunction if the disk I/O is too fast.
// This also fixes "380 volt" and prevents the "city animation" from loading too fast for it's music timing (and getting stuck)
int disk_data_rate = 2100000;    // 2.1MBytes/sec mid 1990s IDE PIO hard drive without SMARTDRV

void diskio_delay(Bits value/*bytes*/) {
    if (disk_data_rate != 0) {
        double scalar = (double)value / disk_data_rate;
        double endtime = PIC_FullIndex() + (scalar * 1000);

        /* MS-DOS will most likely enable interrupts in the course of
         * performing disk I/O */
        CPU_STI();

        do {
            CALLBACK_Idle();
        } while (PIC_FullIndex() < endtime);
    }
}

static inline void overhead() {
	reg_ip += 2;
}

#define BCD2BIN(x)	((((unsigned int)(x) >> 4u) * 10u) + ((x) & 0x0fu))
#define BIN2BCD(x)	((((x) / 10u) << 4u) + (x) % 10u)

static Bitu DOS_21Handler(void);
Bitu DEBUG_EnableDebugger(void);
void CALLBACK_RunRealInt_retcsip(Bit8u intnum,Bitu &cs,Bitu &ip);

bool DOS_BreakINT23InProgress = false;

void DOS_PrintCBreak() {
	/* print ^C <newline> */
	Bit16u n = 4;
	const char *nl = "^C\r\n";
	DOS_WriteFile(STDOUT,(Bit8u*)nl,&n);
}

bool DOS_BreakTest() {
	if (DOS_BreakFlag) {
		bool terminate = true;
		bool terminint23 = false;
		Bitu segv,offv;

		/* print ^C on the console */
		DOS_PrintCBreak();

		DOS_BreakFlag = false;

		offv = mem_readw((0x23*4)+0);
		segv = mem_readw((0x23*4)+2);
		if (offv != 0 && segv != 0) { /* HACK: DOSBox's shell currently does not assign INT 23h */
			/* NTS: DOS calls are allowed within INT 23h! */
			Bitu save_sp = reg_sp;

			/* set carry flag */
			reg_flags |= 1;

			/* invoke INT 23h */
			/* NTS: Some DOS programs provide their own INT 23h which then calls INT 21h AH=0x4C
			 *      inside the handler! Set a flag so that if that happens, the termination
			 *      handler will throw us an exception to force our way back here after
			 *      termination completes!
			 *
			 *      This fixes: PC Mix compiler PCL.EXE
			 *
			 *      FIXME: This is an ugly hack! */
			try {
				DOS_BreakINT23InProgress = true;
				CALLBACK_RunRealInt(0x23);
				DOS_BreakINT23InProgress = false;
			}
			catch (int x) {
				if (x == 0) {
					DOS_BreakINT23InProgress = false;
					terminint23 = true;
				}
				else {
					LOG_MSG("Unexpected code in INT 23h termination exception\n");
					abort();
				}
			}

			/* if the INT 23h handler did not already terminate itself... */
			if (!terminint23) {
				/* if it returned with IRET, or with RETF and CF=0, don't terminate */
				if (reg_sp == save_sp || (reg_flags & 1) == 0) {
					terminate = false;
					LOG_MSG("Note: DOS handler does not wish to terminate\n");
				}
				else {
					/* program does not wish to continue. it used RETF. pop the remaining flags off */
					LOG_MSG("Note: DOS handler does wish to terminate\n");
				}

				if (reg_sp != save_sp) reg_sp += 2;
			}
		}

		if (terminate) {
			LOG_MSG("Note: DOS break terminating program\n");
			DOS_Terminate(dos.psp(),false,0);
			return false;
		}
		else if (terminint23) {
			LOG_MSG("Note: DOS break handler terminated program for us.\n");
			return false;
		}
	}

	return true;
}

void DOS_BreakAction() {
	DOS_BreakFlag = true;
}

/* unmask IRQ 0 automatically on disk I/O functions.
 * there exist old DOS games and demos that rely on very selective IRQ masking,
 * but, their code also assumes that calling into DOS or the BIOS will unmask the IRQ.
 *
 * This fixes "Rebel by Arkham" which masks IRQ 0-7 (PIC port 21h) in a VERY stingy manner!
 *
 *    Pseudocode (early in demo init):
 *
 *             in     al,21h
 *             or     al,3Bh        ; mask IRQ 0, 1, 3, 4, and 5
 *             out    21h,al
 *
 *    Later:
 *
 *             mov    ah,3Dh        ; open file
 *             ...
 *             int    21h
 *             ...                  ; demo apparently assumes that INT 21h will unmask IRQ 0 when reading, because ....
 *             in     al,21h
 *             or     al,3Ah        ; mask IRQ 1, 3, 4, and 5
 *             out    21h,al
 *
 * The demo runs fine anyway, but if we do not unmask IRQ 0 at the INT 21h call, the timer never ticks and the
 * demo does not play any music (goldplay style, of course).
 *
 * This means several things. One is that a disk cache (which may provide the file without using INT 13h) could
 * mysteriously prevent the demo from playing music. Future OS changes, where IRQ unmasking during INT 21h could
 * not occur, would also prevent it from working. I don't know what the programmer was thinking, but side
 * effects like that are not to be relied on!
 *
 * On the other hand, perhaps masking the keyboard (IRQ 1) was intended as an anti-debugger trick? You can't break
 * into the demo if you can't trigger the debugger, after all! The demo can still poll the keyboard controller
 * for ESC or whatever.
 *
 * --J.C. */
bool disk_io_unmask_irq0 = true;

//! \brief Is a DOS program running ? (set by INT21 4B/4C)
bool dos_program_running = false;

void XMS_DOS_LocalA20EnableIfNotEnabled(void);

#define DOSNAMEBUF 256
static Bitu DOS_21Handler(void) {
    bool unmask_irq0 = false;

    /* Real MS-DOS behavior:
     *   If HIMEM.SYS is loaded and CONFIG.SYS says DOS=HIGH, DOS will load itself into the HMA area.
     *   To prevent crashes, the INT 21h handler down below will enable the A20 gate before executing
     *   the DOS kernel. */
    if (DOS_IS_IN_HMA())
        XMS_DOS_LocalA20EnableIfNotEnabled();

    if (((reg_ah != 0x50) && (reg_ah != 0x51) && (reg_ah != 0x62) && (reg_ah != 0x64)) && (reg_ah<0x6c)) {
        DOS_PSP psp(dos.psp());
        psp.SetStack(RealMake(SegValue(ss),reg_sp-18));
    }

    if (((reg_ah >= 0x01 && reg_ah <= 0x0C) || (reg_ah != 0 && reg_ah != 0x4C && reg_ah != 0x31 && dos.breakcheck)) && !DOS_BreakTest()) return CBRET_NONE;

    char name1[DOSNAMEBUF+2+DOS_NAMELENGTH_ASCII];
    char name2[DOSNAMEBUF+2+DOS_NAMELENGTH_ASCII];
    
    switch (reg_ah) {
        case 0x00:      /* Terminate Program */
            /* HACK for demoscene prod parties/1995/wired95/surprisecode/w95spcod.zip/WINNERS/SURP-KLF
             *
             * This demo starts off by popping 3 words off the stack (the third into ES to get the top
             * of DOS memory which it then uses to draw into VGA memory). Since SP starts out at 0xFFFE,
             * that means SP wraps around to start popping values out of the PSP segment.
             *
             * Real MS-DOS will also start the demo with SP at 0xFFFE.
             *
             * The demo terminates with INT 20h.
             *
             * This code will fail since the stack pointer must wrap back around to read the segment,
             * unless we read by popping. */
            if (reg_sp > 0xFFFA) {
                LOG(LOG_DOSMISC,LOG_WARN)("DOS:INT 20h/INT 21h AH=00h WARNING, process terminated where stack pointer wrapped around 64K");

                uint16_t f_ip = CPU_Pop16();
                uint16_t f_cs = CPU_Pop16();
                uint16_t f_flags = CPU_Pop16();

                (void)f_flags;
                (void)f_ip;

                LOG(LOG_DOSMISC,LOG_DEBUG)("DOS:INT 20h/INT 21h AH=00h recovered CS segment %04x",f_cs);

                DOS_Terminate(f_cs,false,0);
            }
            else {
                DOS_Terminate(mem_readw(SegPhys(ss)+reg_sp+2),false,0);
            }

            if (DOS_BreakINT23InProgress) throw int(0); /* HACK: Ick */
            dos_program_running = false;
            break;
        case 0x01:      /* Read character from STDIN, with echo */
            {   
                Bit8u c;Bit16u n=1;
                dos.echo=true;
                DOS_ReadFile(STDIN,&c,&n);
                if (c == 3) {
                    DOS_BreakAction();
                    if (!DOS_BreakTest()) return CBRET_NONE;
                }
                reg_al=c;
                dos.echo=false;
            }
            break;
        case 0x02:      /* Write character to STDOUT */
            {
                Bit8u c=reg_dl;Bit16u n=1;
                DOS_WriteFile(STDOUT,&c,&n);
                //Not in the official specs, but happens nonetheless. (last written character)
                reg_al=(c==9)?0x20:c; //strangely, tab conversion to spaces is reflected here
            }
            break;
        case 0x03:      /* Read character from STDAUX */
            {
            }
            break;
        case 0x04:      /* Write Character to STDAUX */
            {
            }
            break;
        case 0x05:      /* Write Character to PRINTER */
            {
                break;
            }
        case 0x06:      /* Direct Console Output / Input */
            switch (reg_dl) {
                case 0xFF:  /* Input */
                    {   
                        //Simulate DOS overhead for timing sensitive games
                        //MM1
                        overhead();
                        //TODO Make this better according to standards
                        if (!DOS_GetSTDINStatus()) {
                            reg_al=0;
                            CALLBACK_SZF(true);
                            break;
                        }
                        Bit8u c;Bit16u n=1;
                        DOS_ReadFile(STDIN,&c,&n);
                        reg_al=c;
                        CALLBACK_SZF(false);
                        break;
                    }
                default:
                    {
                        Bit8u c = reg_dl;Bit16u n = 1;
                        dos.direct_output=true;
                        DOS_WriteFile(STDOUT,&c,&n);
                        dos.direct_output=false;
                        reg_al=c;
                    }
                    break;
            }
            break;
        case 0x07:      /* Character Input, without echo */
            {
                Bit8u c;Bit16u n=1;
                DOS_ReadFile (STDIN,&c,&n);
                reg_al=c;
                break;
            }
        case 0x08:      /* Direct Character Input, without echo (checks for breaks officially :)*/
            {
                Bit8u c;Bit16u n=1;
                DOS_ReadFile (STDIN,&c,&n);
                if (c == 3) {
                    DOS_BreakAction();
                    if (!DOS_BreakTest()) return CBRET_NONE;
                }
                reg_al=c;
                break;
            }
        case 0x09:      /* Write string to STDOUT */
            {   
                Bit8u c;Bit16u n=1;
                PhysPt buf=SegPhys(ds)+reg_dx;
                while ((c=mem_readb(buf++))!='$') {
                    DOS_WriteFile(STDOUT,&c,&n);
                }
                reg_al=c;
            }
            break;
        case 0x0a:      /* Buffered Input */
            {
                //TODO ADD Break checkin in STDIN but can't care that much for it
                PhysPt data=SegPhys(ds)+reg_dx;
                Bit8u free=mem_readb(data);
                Bit8u read=0;Bit8u c;Bit16u n=1;
                if (!free) break;
                free--;
                for(;;) {
                    if (!DOS_BreakTest()) return CBRET_NONE;
                    DOS_ReadFile(STDIN,&c,&n);
                    if (n == 0)				// End of file
                        E_Exit("DOS:0x0a:Redirected input reached EOF");
                    if (c == 10)			// Line feed
                        continue;
                    if (c == 8) {           // Backspace
                        if (read) { //Something to backspace.
                            // STDOUT treats backspace as non-destructive.
                            DOS_WriteFile(STDOUT,&c,&n);
                            c = ' '; DOS_WriteFile(STDOUT,&c,&n);
                            c = 8;   DOS_WriteFile(STDOUT,&c,&n);
                            --read;
                        }
                        continue;
                    }
                    if (c == 3) {   // CTRL+C
                        DOS_BreakAction();
                        if (!DOS_BreakTest()) return CBRET_NONE;
                    }
                    if (read == free && c != 13) {      // Keyboard buffer full
                        Bit8u bell = 7;
                        DOS_WriteFile(STDOUT, &bell, &n);
                        continue;
                    }
                    DOS_WriteFile(STDOUT,&c,&n);
                    mem_writeb(data+read+2,c);
                    if (c==13) 
                        break;
                    read++;
                }
                mem_writeb(data+1,read);
                break;
            }
        case 0x0b:      /* Get STDIN Status */
            if (!DOS_GetSTDINStatus()) {reg_al=0x00;}
            else {reg_al=0xFF;}
            //Simulate some overhead for timing issues
            //Tankwar menu (needs maybe even more)
            overhead();
            break;
        case 0x0c:      /* Flush Buffer and read STDIN call */
            {
                /* flush buffer if STDIN is CON */
                Bit8u handle=RealHandle(STDIN);
                if (handle!=0xFF && Files[handle] && Files[handle]->IsName("CON")) {
                    Bit8u c;Bit16u n;
                    while (DOS_GetSTDINStatus()) {
                        n=1;    DOS_ReadFile(STDIN,&c,&n);
                    }
                }
                switch (reg_al) {
                    case 0x1:
                    case 0x6:
                    case 0x7:
                    case 0x8:
                    case 0xa:
                        { 
                            Bit8u oldah=reg_ah;
                            reg_ah=reg_al;
                            DOS_21Handler();
                            reg_ah=oldah;
                        }
                        break;
                    default:
                        //              LOG_ERROR("DOS:0C:Illegal Flush STDIN Buffer call %d",reg_al);
                        reg_al=0;
                        break;
                }
            }
            break;
            //TODO Find out the values for when reg_al!=0
            //TODO Hope this doesn't do anything special
        case 0x0d:      /* Disk Reset */
            //Sure let's reset a virtual disk
            break;  
        case 0x0e:      /* Select Default Drive */
            DOS_SetDefaultDrive(reg_dl);
            reg_al=DOS_DRIVES;
            break;
        case 0x0f:      /* Open File using FCB */
            if(DOS_FCBOpen(SegValue(ds),reg_dx)){
                reg_al=0;
            }else{
                reg_al=0xff;
            }
            LOG(LOG_FCB,LOG_NORMAL)("DOS:0x0f FCB-fileopen used, result:al=%d",reg_al);
            break;
        case 0x10:      /* Close File using FCB */
            if(DOS_FCBClose(SegValue(ds),reg_dx)){
                reg_al=0;
            }else{
                reg_al=0xff;
            }
            LOG(LOG_FCB,LOG_NORMAL)("DOS:0x10 FCB-fileclose used, result:al=%d",reg_al);
            break;
        case 0x11:      /* Find First Matching File using FCB */
            if(DOS_FCBFindFirst(SegValue(ds),reg_dx)) reg_al = 0x00;
            else reg_al = 0xFF;
            LOG(LOG_FCB,LOG_NORMAL)("DOS:0x11 FCB-FindFirst used, result:al=%d",reg_al);
            break;
        case 0x12:      /* Find Next Matching File using FCB */
            if(DOS_FCBFindNext(SegValue(ds),reg_dx)) reg_al = 0x00;
            else reg_al = 0xFF;
            LOG(LOG_FCB,LOG_NORMAL)("DOS:0x12 FCB-FindNext used, result:al=%d",reg_al);
            break;
        case 0x13:      /* Delete File using FCB */
            if (DOS_FCBDeleteFile(SegValue(ds),reg_dx)) reg_al = 0x00;
            else reg_al = 0xFF;
            LOG(LOG_FCB,LOG_NORMAL)("DOS:0x16 FCB-Delete used, result:al=%d",reg_al);
            break;
        case 0x14:      /* Sequential read from FCB */
            reg_al = DOS_FCBRead(SegValue(ds),reg_dx,0);
            LOG(LOG_FCB,LOG_NORMAL)("DOS:0x14 FCB-Read used, result:al=%d",reg_al);
            break;
        case 0x15:      /* Sequential write to FCB */
            reg_al=DOS_FCBWrite(SegValue(ds),reg_dx,0);
            LOG(LOG_FCB,LOG_NORMAL)("DOS:0x15 FCB-Write used, result:al=%d",reg_al);
            break;
        case 0x16:      /* Create or truncate file using FCB */
            if (DOS_FCBCreate(SegValue(ds),reg_dx)) reg_al = 0x00;
            else reg_al = 0xFF;
            LOG(LOG_FCB,LOG_NORMAL)("DOS:0x16 FCB-Create used, result:al=%d",reg_al);
            break;
        case 0x17:      /* Rename file using FCB */     
            if (DOS_FCBRenameFile(SegValue(ds),reg_dx)) reg_al = 0x00;
            else reg_al = 0xFF;
            break;
        case 0x18:      /* NULL Function for CP/M compatibility or Extended rename FCB */
            goto default_fallthrough;
        case 0x19:      /* Get current default drive */
            reg_al = DOS_GetDefaultDrive();
            break;
        case 0x1a:      /* Set Disk Transfer Area Address */
            dos.dta(RealMakeSeg(ds, reg_dx));
            break;
        case 0x1b:      /* Get allocation info for default drive */ 
            if (!DOS_GetAllocationInfo(0,&reg_cx,&reg_al,&reg_dx)) reg_al=0xff;
            break;
        case 0x1c:      /* Get allocation info for specific drive */
            if (!DOS_GetAllocationInfo(reg_dl,&reg_cx,&reg_al,&reg_dx)) reg_al=0xff;
            break;
        case 0x1d:      /* NULL Function for CP/M compatibility or Extended rename FCB */
            goto default_fallthrough;
        case 0x1e:      /* NULL Function for CP/M compatibility or Extended rename FCB */
            goto default_fallthrough;
        case 0x1f: /* Get drive parameter block for default drive */
            goto case_0x32_fallthrough;
        case 0x20:      /* NULL Function for CP/M compatibility or Extended rename FCB */
            goto default_fallthrough;
        case 0x21:      /* Read random record from FCB */
            {
                Bit16u toread=1;
                reg_al = DOS_FCBRandomRead(SegValue(ds),reg_dx,&toread,true);
            }
            LOG(LOG_FCB,LOG_NORMAL)("DOS:0x21 FCB-Random read used, result:al=%d",reg_al);
            break;
        case 0x22:      /* Write random record to FCB */
            {
                Bit16u towrite=1;
                reg_al=DOS_FCBRandomWrite(SegValue(ds),reg_dx,&towrite,true);
            }
            LOG(LOG_FCB,LOG_NORMAL)("DOS:0x22 FCB-Random write used, result:al=%d",reg_al);
            break;
        case 0x23:      /* Get file size for FCB */
            if (DOS_FCBGetFileSize(SegValue(ds),reg_dx)) reg_al = 0x00;
            else reg_al = 0xFF;
            break;
        case 0x24:      /* Set Random Record number for FCB */
            DOS_FCBSetRandomRecord(SegValue(ds),reg_dx);
            break;
        case 0x25:      /* Set Interrupt Vector */
            RealSetVec(reg_al, RealMakeSeg(ds, reg_dx));
            break;
        case 0x26:      /* Create new PSP */
            /* TODO: DEBUG.EXE/DEBUG.COM as shipped with MS-DOS seems to reveal a bug where,
             *       when DEBUG.EXE calls this function and you're NOT loading a program to debug,
             *       the CP/M CALL FAR instruction's offset field will be off by 2. When does
             *       that happen, and how do we emulate that? */
            DOS_NewPSP(reg_dx, DOS_PSP(dos.psp()).GetSize());
            reg_al = 0xf0;    /* al destroyed */
            break;
        case 0x27:      /* Random block read from FCB */
            reg_al = DOS_FCBRandomRead(SegValue(ds),reg_dx,&reg_cx,false);
            LOG(LOG_FCB,LOG_NORMAL)("DOS:0x27 FCB-Random(block) read used, result:al=%d",reg_al);
            break;
        case 0x28:      /* Random Block write to FCB */
            reg_al=DOS_FCBRandomWrite(SegValue(ds),reg_dx,&reg_cx,false);
            LOG(LOG_FCB,LOG_NORMAL)("DOS:0x28 FCB-Random(block) write used, result:al=%d",reg_al);
            break;
        case 0x29:      /* Parse filename into FCB */
            {   
                Bit8u difference;
                char string[1024];
                MEM_StrCopy(SegPhys(ds)+reg_si,string,1023); // 1024 toasts the stack
                reg_al=FCB_Parsename(SegValue(es),reg_di,reg_al ,string, &difference);
                reg_si+=difference;
            }
            LOG(LOG_FCB,LOG_NORMAL)("DOS:29:FCB Parse Filename, result:al=%d",reg_al);
            break;
        case 0x2a:      /* Get System Date */
            break;
        case 0x2b:      /* Set System Date */
            break;
        case 0x2c:      /* Get System Time */
            break;
        case 0x2d:      /* Set System Time */
            break;
        case 0x2e:      /* Set Verify flag */
            dos.verify=(reg_al==1);
            break;
        case 0x2f:      /* Get Disk Transfer Area */
            SegSet16(es,RealSeg(dos.dta()));
            reg_bx=RealOff(dos.dta());
            break;
        case 0x30:      /* Get DOS Version */
            if (reg_al==0) reg_bh=0xFF;     /* Fake Microsoft DOS */
            if (reg_al==1 && DOS_IS_IN_HMA()) reg_bh=0x10;      /* DOS is in HMA? */
            reg_al=dos.version.major;
            reg_ah=dos.version.minor;
            /* Serialnumber */
            reg_bl=0x00;
            reg_cx=0x0000;
            break;
        case 0x31:      /* Terminate and stay resident */
            // Important: This service does not set the carry flag!
            DOS_ResizeMemory(dos.psp(),&reg_dx);
            DOS_Terminate(dos.psp(),true,reg_al);
            if (DOS_BreakINT23InProgress) throw int(0); /* HACK: Ick */
            dos_program_running = false;
            break;
        case 0x32: /* Get drive parameter block for specific drive */
            {   /* Officially a dpb should be returned as well. The disk detection part is implemented */
                case_0x32_fallthrough:
                Bit8u drive=reg_dl;
                if (!drive || reg_ah==0x1f) drive = DOS_GetDefaultDrive();
                else drive--;
                if (drive < DOS_DRIVES && Drives[drive] && !Drives[drive]->isRemovable()) {
                    reg_al = 0x00;
                    SegSet16(ds,dos.tables.dpb);
                    reg_bx = drive*dos.tables.dpb_size;
                    LOG(LOG_DOSMISC,LOG_NORMAL)("Get drive parameter block.");
                } else {
                    reg_al=0xff;
                }
            }
            break;
        case 0x33:      /* Extended Break Checking */
            switch (reg_al) {
                case 0:reg_dl=dos.breakcheck;break;         /* Get the breakcheck flag */
                case 1:dos.breakcheck=(reg_dl>0);break;     /* Set the breakcheck flag */
                case 2:{bool old=dos.breakcheck;dos.breakcheck=(reg_dl>0);reg_dl=old;}break;
                case 3: /* Get cpsw */
                       /* Fallthrough */
                case 4: /* Set cpsw */
                       LOG(LOG_DOSMISC,LOG_ERROR)("Someone playing with cpsw %x",reg_ax);
                       break;
                case 5:reg_dl=3;break;//TODO should be z                        /* Always boot from c: :) */
                case 6:                                         /* Get true version number */
                       reg_bl=dos.version.major;
                       reg_bh=dos.version.minor;
                       reg_dl=dos.version.revision;
                       reg_dh=DOS_IS_IN_HMA()?0x10:0x00;                       /* Dos in HMA?? */
                       break;
                case 7:
                       break;
                default:
                       LOG(LOG_DOSMISC,LOG_ERROR)("Weird 0x33 call %2X",reg_al);
                       reg_al =0xff;
                       break;
            }
            break;
        case 0x34:      /* Get INDos Flag */
            SegSet16(es,DOS_SDA_SEG);
            reg_bx=DOS_SDA_OFS + 0x01;
            break;
        case 0x35:      /* Get interrupt vector */
            reg_bx=real_readw(0,((Bit16u)reg_al)*4);
            SegSet16(es,real_readw(0,((Bit16u)reg_al)*4+2));
            break;
        case 0x36:      /* Get Free Disk Space */
            {
                Bit16u bytes,clusters,free;
                Bit8u sectors;
                if (DOS_GetFreeDiskSpace(reg_dl,&bytes,&sectors,&clusters,&free)) {
                    reg_ax=sectors;
                    reg_bx=free;
                    reg_cx=bytes;
                    reg_dx=clusters;
                } else {
                    Bit8u drive=reg_dl;
                    if (drive==0) drive=DOS_GetDefaultDrive();
                    else drive--;
                    if (drive<2) {
                        // floppy drive, non-present drivesdisks issue floppy check through int24
                        // (critical error handler); needed for Mixed up Mother Goose (hook)
                        //                  CALLBACK_RunRealInt(0x24);
                    }
                    reg_ax=0xffff;  // invalid drive specified
                }
            }
            break;
        case 0x37:      /* Get/Set Switch char Get/Set Availdev thing */
            //TODO  Give errors for these functions to see if anyone actually uses this shit-
            switch (reg_al) {
                case 0:
                    reg_al=0;reg_dl=0x2f;break;  /* always return '/' like dos 5.0+ */
                case 1:
                    LOG(LOG_MISC,LOG_DEBUG)("DOS:0x37:Attempted to set switch char");
                    reg_al=0;break;
                case 2:
                    reg_al=0;reg_dl=0xff;break;  /* AVAILDEV \DEV\ prefix optional */
                case 3:
                    LOG(LOG_MISC,LOG_DEBUG)("DOS:0x37:Attempted to set AVAILDEV \\DEV\\ prefix use");
                    reg_al=0;break;
            }
            break;
        case 0x38:                  /* Set Country Code */  
            if (reg_al==0) {        /* Get country specidic information */
                PhysPt dest = SegPhys(ds)+reg_dx;
                MEM_BlockWrite(dest,dos.tables.country,0x18);
                reg_ax = reg_bx = 0x01;
                CALLBACK_SCF(false);
                break;
            } else {                /* Set country code */
                LOG(LOG_MISC,LOG_ERROR)("DOS:Setting country code not supported");
            }
            CALLBACK_SCF(true);
            break;
        case 0x39:      /* MKDIR Create directory */
            MEM_StrCopy(SegPhys(ds)+reg_dx,name1,DOSNAMEBUF);
            if (DOS_MakeDir(name1)) {
                reg_ax=0x05;    /* ax destroyed */
                CALLBACK_SCF(false);
            } else {
                reg_ax=dos.errorcode;
                CALLBACK_SCF(true);
            }
            break;
        case 0x3a:      /* RMDIR Remove directory */
            MEM_StrCopy(SegPhys(ds)+reg_dx,name1,DOSNAMEBUF);
            if  (DOS_RemoveDir(name1)) {
                reg_ax=0x05;    /* ax destroyed */
                CALLBACK_SCF(false);
            } else {
                reg_ax=dos.errorcode;
                CALLBACK_SCF(true);
                LOG(LOG_MISC,LOG_NORMAL)("Remove dir failed on %s with error %X",name1,dos.errorcode);
            }
            break;
        case 0x3b:      /* CHDIR Set current directory */
            MEM_StrCopy(SegPhys(ds)+reg_dx,name1,DOSNAMEBUF);
            if  (DOS_ChangeDir(name1)) {
                reg_ax=0x00;    /* ax destroyed */
                CALLBACK_SCF(false);
            } else {
                reg_ax=dos.errorcode;
                CALLBACK_SCF(true);
            }
            break;
        case 0x3c:      /* CREATE Create or truncate file */
            unmask_irq0 |= disk_io_unmask_irq0;
            MEM_StrCopy(SegPhys(ds)+reg_dx,name1,DOSNAMEBUF);
            if (DOS_CreateFile(name1,reg_cx,&reg_ax)) {
                CALLBACK_SCF(false);
            } else {
                reg_ax=dos.errorcode;
                CALLBACK_SCF(true);
            }
            diskio_delay(2048);
            break;
        case 0x3d:      /* OPEN Open existing file */
            unmask_irq0 |= disk_io_unmask_irq0;
            MEM_StrCopy(SegPhys(ds)+reg_dx,name1,DOSNAMEBUF);
            if (DOS_OpenFile(name1,reg_al,&reg_ax)) {
                CALLBACK_SCF(false);
            } else {
                reg_ax=dos.errorcode;
                CALLBACK_SCF(true);
            }
            diskio_delay(1024);
            break;
        case 0x3e:      /* CLOSE Close file */
            unmask_irq0 |= disk_io_unmask_irq0;
            if (DOS_CloseFile(reg_bx)) {
                //          reg_al=0x01;    /* al destroyed. Refcount */
                CALLBACK_SCF(false);
            } else {
                reg_ax=dos.errorcode;
                CALLBACK_SCF(true);
            }
            diskio_delay(512);
            break;
        case 0x3f:      /* READ Read from file or device */
            unmask_irq0 |= disk_io_unmask_irq0;
            /* TODO: If handle is STDIN and not binary do CTRL+C checking */
            { 
                Bit16u toread=reg_cx;

                /* if the offset and size exceed the end of the 64KB segment,
                 * truncate the read according to observed MS-DOS 5.0 behavior
                 * where the actual byte count read is 64KB minus (reg_dx % 16).
                 *
                 * This is needed for "Dark Purpose" to read it's DAT file
                 * correctly, which calls INT 21h AH=3Fh with DX=0004h and CX=FFFFh
                 * and will mis-render it's fonts, images, and color palettes
                 * if we do not do this.
                 *
                 * Ref: http://files.scene.org/get/mirrors/hornet/demos/1995/d/darkp.zip */
                if (((uint32_t)toread+(uint32_t)reg_dx) > 0xFFFFUL && (reg_dx & 0xFU) != 0U) {
                    Bit16u nuread = (Bit16u)(0x10000UL - (reg_dx & 0xF)); /* FIXME: If MS-DOS 5.0 truncates it any farther I need to know! */

                    if (nuread > toread) nuread = toread;
                    LOG_MSG("INT 21h READ warning: DX=%04xh CX=%04xh exceeds 64KB, truncating to %04xh",reg_dx,toread,nuread);
                    toread = nuread;
                }

                dos.echo=true;
                if (DOS_ReadFile(reg_bx,dos_copybuf,&toread)) {
                    MEM_BlockWrite(SegPhys(ds)+reg_dx,dos_copybuf,toread);
                    reg_ax=toread;
                    CALLBACK_SCF(false);
                } else {
                    reg_ax=dos.errorcode;
                    CALLBACK_SCF(true);
                }
                diskio_delay(reg_ax);
                dos.echo=false;
                break;
            }
        case 0x40:                  /* WRITE Write to file or device */
            unmask_irq0 |= disk_io_unmask_irq0;
            {
                Bit16u towrite=reg_cx;

                /* if the offset and size exceed the end of the 64KB segment,
                 * truncate the write according to observed MS-DOS 5.0 READ behavior
                 * where the actual byte count written is 64KB minus (reg_dx % 16).
                 *
                 * This is copy-paste of AH=3Fh read handling because it's likely
                 * that MS-DOS probably does the same with write as well, though
                 * this has not yet been confirmed. --J.C. */
                if (((uint32_t)towrite+(uint32_t)reg_dx) > 0xFFFFUL && (reg_dx & 0xFU) != 0U) {
                    Bit16u nuwrite = (Bit16u)(0x10000UL - (reg_dx & 0xF)); /* FIXME: If MS-DOS 5.0 truncates it any farther I need to know! */

                    if (nuwrite > towrite) nuwrite = towrite;
                    LOG_MSG("INT 21h WRITE warning: DX=%04xh CX=%04xh exceeds 64KB, truncating to %04xh",reg_dx,towrite,nuwrite);
                    towrite = nuwrite;
                }

                MEM_BlockRead(SegPhys(ds)+reg_dx,dos_copybuf,towrite);
                if (DOS_WriteFile(reg_bx,dos_copybuf,&towrite)) {
                    reg_ax=towrite;
                    CALLBACK_SCF(false);
                } else {
                    reg_ax=dos.errorcode;
                    CALLBACK_SCF(true);
                }
                diskio_delay(reg_ax);
                break;
            }
        case 0x41:                  /* UNLINK Delete file */
            unmask_irq0 |= disk_io_unmask_irq0;
            MEM_StrCopy(SegPhys(ds)+reg_dx,name1,DOSNAMEBUF);
            if (DOS_UnlinkFile(name1)) {
                CALLBACK_SCF(false);
            } else {
                reg_ax=dos.errorcode;
                CALLBACK_SCF(true);
            }
            diskio_delay(1024);
            break;
        case 0x42:                  /* LSEEK Set current file position */
            unmask_irq0 |= disk_io_unmask_irq0;
            {
                Bit32u pos=((Bit32u)reg_cx << 16u) + reg_dx;
                if (DOS_SeekFile(reg_bx,&pos,reg_al)) {
                    reg_dx=(Bit16u)((unsigned int)pos >> 16u);
                    reg_ax=(Bit16u)(pos & 0xFFFF);
                    CALLBACK_SCF(false);
                } else {
                    reg_ax=dos.errorcode;
                    CALLBACK_SCF(true);
                }
                diskio_delay(32);
                break;
            }
        case 0x43:                  /* Get/Set file attributes */
            unmask_irq0 |= disk_io_unmask_irq0;
            MEM_StrCopy(SegPhys(ds)+reg_dx,name1,DOSNAMEBUF);
            switch (reg_al) {
                case 0x00:              /* Get */
                    {
                        Bit16u attr_val=reg_cx;
                        if (DOS_GetFileAttr(name1,&attr_val)) {
                            reg_cx=attr_val;
                            reg_ax=attr_val; /* Undocumented */   
                            CALLBACK_SCF(false);
                        } else {
                            CALLBACK_SCF(true);
                            reg_ax=dos.errorcode;
                        }
                        break;
                    }
                case 0x01:              /* Set */
                    LOG(LOG_MISC,LOG_ERROR)("DOS:Set File Attributes for %s not supported",name1);
                    if (DOS_SetFileAttr(name1,reg_cx)) {
                        reg_ax=0x202;   /* ax destroyed */
                        CALLBACK_SCF(false);
                    } else {
                        CALLBACK_SCF(true);
                        reg_ax=dos.errorcode;
                    }
                    break;
                default:
                    LOG(LOG_MISC,LOG_ERROR)("DOS:0x43:Illegal subfunction %2X",reg_al);
                    reg_ax=1;
                    CALLBACK_SCF(true);
                    break;
            }
            break;
        case 0x44:                  /* IOCTL Functions */
            if (DOS_IOCTL()) {
                CALLBACK_SCF(false);
            } else {
                reg_ax=dos.errorcode;
                CALLBACK_SCF(true);
            }
            break;
        case 0x45:                  /* DUP Duplicate file handle */
            if (DOS_DuplicateEntry(reg_bx,&reg_ax)) {
                CALLBACK_SCF(false);
            } else {
                reg_ax=dos.errorcode;
                CALLBACK_SCF(true);
            }
            break;
        case 0x46:                  /* DUP2,FORCEDUP Force duplicate file handle */
            if (DOS_ForceDuplicateEntry(reg_bx,reg_cx)) {
                reg_ax=reg_cx; //Not all sources agree on it.
                CALLBACK_SCF(false);
            } else {
                reg_ax=dos.errorcode;
                CALLBACK_SCF(true);
            }
            break;
        case 0x47:                  /* CWD Get current directory */
            if (DOS_GetCurrentDir(reg_dl,name1)) {
                MEM_BlockWrite(SegPhys(ds)+reg_si,name1,(Bitu)(strlen(name1)+1));   
                reg_ax=0x0100;
                CALLBACK_SCF(false);
            } else {
                reg_ax=dos.errorcode;
                CALLBACK_SCF(true);
            }
            break;
        case 0x48:                  /* Allocate memory */
            {
                Bit16u size=reg_bx;Bit16u seg;
                if (DOS_AllocateMemory(&seg,&size)) {
                    reg_ax=seg;
                    CALLBACK_SCF(false);
                } else {
                    reg_ax=dos.errorcode;
                    reg_bx=size;
                    CALLBACK_SCF(true);
                }
                break;
            }
        case 0x49:                  /* Free memory */
            if (DOS_FreeMemory(SegValue(es))) {
                CALLBACK_SCF(false);
            } else {            
                reg_ax=dos.errorcode;
                CALLBACK_SCF(true);
            }
            break;
        case 0x4a:                  /* Resize memory block */
            {
                Bit16u size=reg_bx;
                if (DOS_ResizeMemory(SegValue(es),&size)) {
                    reg_ax=SegValue(es);
                    CALLBACK_SCF(false);
                } else {            
                    reg_ax=dos.errorcode;
                    reg_bx=size;
                    CALLBACK_SCF(true);
                }
                break;
            }
        case 0x4b:                  /* EXEC Load and/or execute program */
            { 
                MEM_StrCopy(SegPhys(ds)+reg_dx,name1,DOSNAMEBUF);
                LOG(LOG_EXEC,LOG_NORMAL)("Execute %s %d",name1,reg_al);
                if (!DOS_Execute(name1,SegPhys(es)+reg_bx,reg_al)) {
                    reg_ax=dos.errorcode;
                    CALLBACK_SCF(true);
                }
                dos_program_running = true;
            }
            break;
            //TODO Check for use of execution state AL=5
        case 0x4c:                  /* EXIT Terminate with return code */
            DOS_Terminate(dos.psp(),false,reg_al);
            if (DOS_BreakINT23InProgress) throw int(0); /* HACK: Ick */
            dos_program_running = false;
            break;
        case 0x4d:                  /* Get Return code */
            reg_al=dos.return_code;/* Officially read from SDA and clear when read */
            reg_ah=dos.return_mode;
            break;
        case 0x4e:                  /* FINDFIRST Find first matching file */
            MEM_StrCopy(SegPhys(ds)+reg_dx,name1,DOSNAMEBUF);
            if (DOS_FindFirst(name1,reg_cx)) {
                CALLBACK_SCF(false);    
                reg_ax=0;           /* Undocumented */
            } else {
                reg_ax=dos.errorcode;
                CALLBACK_SCF(true);
            }
            break;       
        case 0x4f:                  /* FINDNEXT Find next matching file */
            if (DOS_FindNext()) {
                CALLBACK_SCF(false);
                /* reg_ax=0xffff;*/         /* Undocumented */
                reg_ax=0;               /* Undocumented:Qbix Willy beamish */
            } else {
                reg_ax=dos.errorcode;
                CALLBACK_SCF(true);
            }
            break;      
        case 0x50:                  /* Set current PSP */
            dos.psp(reg_bx);
            break;
        case 0x51:                  /* Get current PSP */
            reg_bx=dos.psp();
            break;
        case 0x52: {                /* Get list of lists */
            Bit8u count=2; // floppy drives always counted
            while (count<DOS_DRIVES && Drives[count] && !Drives[count]->isRemovable()) count++;
            dos_infoblock.SetBlockDevices(count);
            RealPt addr=dos_infoblock.GetPointer();
            SegSet16(es,RealSeg(addr));
            reg_bx=RealOff(addr);
            LOG(LOG_DOSMISC,LOG_NORMAL)("Call is made for list of lists - let's hope for the best");
            break; }
            //TODO Think hard how shit this is gonna be
            //And will any game ever use this :)
        case 0x53:                  /* Translate BIOS parameter block to drive parameter block */
            E_Exit("Unhandled Dos 21 call %02X",reg_ah);
            break;
        case 0x54:                  /* Get verify flag */
            reg_al=dos.verify?1:0;
            break;
        case 0x55:                  /* Create Child PSP*/
            DOS_ChildPSP(reg_dx,reg_si);
            dos.psp(reg_dx);
            reg_al=0xf0;    /* al destroyed */
            break;
        case 0x56:                  /* RENAME Rename file */
            MEM_StrCopy(SegPhys(ds)+reg_dx,name1,DOSNAMEBUF);
            MEM_StrCopy(SegPhys(es)+reg_di,name2,DOSNAMEBUF);
            if (DOS_Rename(name1,name2)) {
                CALLBACK_SCF(false);            
            } else {
                reg_ax=dos.errorcode;
                CALLBACK_SCF(true);
            }
            break;      
        case 0x57:                  /* Get/Set File's Date and Time */
            if (reg_al==0x00) {
                if (DOS_GetFileDate(reg_bx,&reg_cx,&reg_dx)) {
                    CALLBACK_SCF(false);
                } else {
                    CALLBACK_SCF(true);
                }
            } else if (reg_al==0x01) {
                if (DOS_SetFileDate(reg_bx,reg_cx,reg_dx)) {
                    CALLBACK_SCF(false);
                } else {
                    CALLBACK_SCF(true);
                }
            } else {
                LOG(LOG_DOSMISC,LOG_ERROR)("DOS:57:Unsupported subtion %X",reg_al);
            }
            break;
        case 0x58:                  /* Get/Set Memory allocation strategy */
            switch (reg_al) {
                case 0:                 /* Get Strategy */
                    reg_ax=DOS_GetMemAllocStrategy();
                    break;
                case 1:                 /* Set Strategy */
                    if (DOS_SetMemAllocStrategy(reg_bx)) CALLBACK_SCF(false);
                    else {
                        reg_ax=1;
                        CALLBACK_SCF(true);
                    }
                    break;
                case 2:                 /* Get UMB Link Status */
                    reg_al=dos_infoblock.GetUMBChainState()&1;
                    CALLBACK_SCF(false);
                    break;
                case 3:                 /* Set UMB Link Status */
                    if (DOS_LinkUMBsToMemChain(reg_bx)) CALLBACK_SCF(false);
                    else {
                        reg_ax=1;
                        CALLBACK_SCF(true);
                    }
                    break;
                default:
                    LOG(LOG_DOSMISC,LOG_ERROR)("DOS:58:Not Supported Set//Get memory allocation call %X",reg_al);
                    reg_ax=1;
                    CALLBACK_SCF(true);
            }
            break;
        case 0x59:                  /* Get Extended error information */
            reg_ax=dos.errorcode;
            if (dos.errorcode==DOSERR_FILE_NOT_FOUND || dos.errorcode==DOSERR_PATH_NOT_FOUND) {
                reg_bh=8;   //Not Found error class (Road Hog)
            } else {
                reg_bh=0;   //Unspecified error class
            }
            reg_bl=1;   //Retry retry retry
            reg_ch=0;   //Unkown error locus
            break;
        case 0x5a:                  /* Create temporary file */
            {
                Bit16u handle;
                MEM_StrCopy(SegPhys(ds)+reg_dx,name1,DOSNAMEBUF);
                if (DOS_CreateTempFile(name1,&handle)) {
                    reg_ax=handle;
                    MEM_BlockWrite(SegPhys(ds)+reg_dx,name1,(Bitu)(strlen(name1)+1));
                    CALLBACK_SCF(false);
                } else {
                    reg_ax=dos.errorcode;
                    CALLBACK_SCF(true);
                }
            }
            break;
        case 0x5b:                  /* Create new file */
            {
                MEM_StrCopy(SegPhys(ds)+reg_dx,name1,DOSNAMEBUF);
                Bit16u handle;
                if (DOS_OpenFile(name1,0,&handle)) {
                    DOS_CloseFile(handle);
                    DOS_SetError(DOSERR_FILE_ALREADY_EXISTS);
                    reg_ax=dos.errorcode;
                    CALLBACK_SCF(true);
                    break;
                }
                if (DOS_CreateFile(name1,reg_cx,&handle)) {
                    reg_ax=handle;
                    CALLBACK_SCF(false);
                } else {
                    reg_ax=dos.errorcode;
                    CALLBACK_SCF(true);
                }
                break;
            }
        case 0x5c:  {       /* FLOCK File region locking */
            /* ert, 20100711: Locking extensions */
            Bit32u pos=((unsigned int)reg_cx << 16u) + reg_dx;
            Bit32u size=((unsigned int)reg_si << 16u) + reg_di;
            //LOG_MSG("LockFile: BX=%d, AL=%d, POS=%d, size=%d", reg_bx, reg_al, pos, size);
            if (DOS_LockFile(reg_bx,reg_al,pos, size)) {
                reg_ax=0;
                CALLBACK_SCF(false);
            } else {
                reg_ax=dos.errorcode;
                CALLBACK_SCF(true);
            }
            break; }
            /*
               DOS_SetError(DOSERR_FUNCTION_NUMBER_INVALID);
               reg_ax = dos.errorcode;
               CALLBACK_SCF(true);
               break;
               */
        case 0x5d:                  /* Network Functions */
            if(reg_al == 0x06) {
                /* FIXME: I'm still not certain, @emendelson, why this matters so much
                 *        to WordPerfect 5.1 and 6.2 and why it causes problems otherwise.
                 *        DOSBox and DOSBox-X only use the first 0x1A bytes anyway. */
                SegSet16(ds,DOS_SDA_SEG);
                reg_si = DOS_SDA_OFS;
                reg_cx = DOS_SDA_SEG_SIZE;  // swap if in dos
                reg_dx = 0x1a;  // swap always (NTS: Size of DOS SDA structure in dos_inc)
                LOG(LOG_DOSMISC,LOG_NORMAL)("Get SDA, Let's hope for the best!");
            }
            break;
        case 0x5e:                  /* Network and printer functions */
            LOG(LOG_DOSMISC, LOG_ERROR)("DOS:5E Network and printer functions not implemented");
            goto default_fallthrough;
        case 0x5f:                  /* Network redirection */
            reg_ax=0x0001;          //Failing it
            CALLBACK_SCF(true);
            break; 
        case 0x60:                  /* Canonicalize filename or path */
            MEM_StrCopy(SegPhys(ds)+reg_si,name1,DOSNAMEBUF);
            if (DOS_Canonicalize(name1,name2)) {
                MEM_BlockWrite(SegPhys(es)+reg_di,name2,(Bitu)(strlen(name2)+1));   
                CALLBACK_SCF(false);
            } else {
                reg_ax=dos.errorcode;
                CALLBACK_SCF(true);
            }
            break;
        case 0x61:                  /* Unused (reserved for network use) */
            goto default_fallthrough;
        case 0x62:                  /* Get Current PSP Address */
            reg_bx=dos.psp();
            break;
        case 0x63:                  /* DOUBLE BYTE CHARACTER SET */
            if(reg_al == 0 && dos.tables.dbcs != 0) {
                SegSet16(ds,RealSeg(dos.tables.dbcs));
                reg_si=RealOff(dos.tables.dbcs);        
                reg_al = 0;
                CALLBACK_SCF(false); //undocumented
            } else reg_al = 0xff; //Doesn't officially touch carry flag
            break;
        case 0x64:                  /* Set device driver lookahead flag */
            LOG(LOG_DOSMISC,LOG_NORMAL)("set driver look ahead flag");
            break;
        case 0x65:                  /* Get extented country information and a lot of other useless shit*/
            { /* Todo maybe fully support this for now we set it standard for USA */ 
                LOG(LOG_DOSMISC,LOG_NORMAL)("DOS:65:Extended country information call %X",reg_ax);
                if((reg_al <=  0x07) && (reg_cx < 0x05)) {
                    DOS_SetError(DOSERR_FUNCTION_NUMBER_INVALID);
                    CALLBACK_SCF(true);
                    break;
                }
                Bitu len = 0; /* For 0x21 and 0x22 */
                PhysPt data=SegPhys(es)+reg_di;
                switch (reg_al) {
                    case 0x01:
                        mem_writeb(data + 0x00,reg_al);
                        mem_writew(data + 0x01,0x26);
                        mem_writew(data + 0x03,1);
                        if(reg_cx > 0x06 ) mem_writew(data+0x05,dos.loaded_codepage);
                        if(reg_cx > 0x08 ) {
                            Bitu amount = (reg_cx>=0x29u)?0x22u:(reg_cx-7u);
                            MEM_BlockWrite(data + 0x07,dos.tables.country,amount);
                            reg_cx=(reg_cx>=0x29)?0x29:reg_cx;
                        }
                        CALLBACK_SCF(false);
                        break;
                    case 0x05: // Get pointer to filename terminator table
                        mem_writeb(data + 0x00, reg_al);
                        mem_writed(data + 0x01, dos.tables.filenamechar);
                        reg_cx = 5;
                        CALLBACK_SCF(false);
                        break;
                    case 0x02: // Get pointer to uppercase table
                    case 0x04: // Get pointer to filename uppercase table
                        mem_writeb(data + 0x00, reg_al);
                        mem_writed(data + 0x01, dos.tables.upcase);
                        reg_cx = 5;
                        CALLBACK_SCF(false);
                        break;
                    case 0x06: // Get pointer to collating sequence table
                        mem_writeb(data + 0x00, reg_al);
                        mem_writed(data + 0x01, dos.tables.collatingseq);
                        reg_cx = 5;
                        CALLBACK_SCF(false);
                        break;
                    case 0x03: // Get pointer to lowercase table
                    case 0x07: // Get pointer to double byte char set table
                        if (dos.tables.dbcs != 0) {
                            mem_writeb(data + 0x00, reg_al);
                            mem_writed(data + 0x01, dos.tables.dbcs); //used to be 0
                            reg_cx = 5;
                            CALLBACK_SCF(false);
                        }
                        break;
                    case 0x20: /* Capitalize Character */
                        {
                            int in  = reg_dl;
                            int out = toupper(in);
                            reg_dl  = (Bit8u)out;
                        }
                        CALLBACK_SCF(false);
                        break;
                    case 0x21: /* Capitalize String (cx=length) */
                    case 0x22: /* Capatilize ASCIZ string */
                        data = SegPhys(ds) + reg_dx;
                        if(reg_al == 0x21) len = reg_cx; 
                        else len = mem_strlen(data); /* Is limited to 1024 */

                        if(len > DOS_COPYBUFSIZE - 1) E_Exit("DOS:0x65 Buffer overflow");
                        if(len) {
                            MEM_BlockRead(data,dos_copybuf,len);
                            dos_copybuf[len] = 0;
                            //No upcase as String(0x21) might be multiple asciz strings
                            for (Bitu count = 0; count < len;count++)
                                dos_copybuf[count] = (Bit8u)toupper(*reinterpret_cast<unsigned char*>(dos_copybuf+count));
                            MEM_BlockWrite(data,dos_copybuf,len);
                        }
                        CALLBACK_SCF(false);
                        break;
                    case 0x23: /* Determine if character represents yes/no response (MS-DOS 4.0+) */
                        /* DL = character
                         * DH = second char of double-byte char if DBCS */
                        /* response: CF=1 if error (what error?) or CF=0 and AX=response
                         *
                         * response values 0=no 1=yes 2=neither */
                        /* FORMAT.COM and FDISK.EXE rely on this call after prompting the user */
                        {
                            unsigned int c;

                            c = reg_dl; // SBCS

                            if (tolower(c) == 'y')
                                reg_ax = 1;/*yes*/
                            else if (tolower(c) == 'n')
                                reg_ax = 0;/*no*/
                            else
                                reg_ax = 2;/*neither*/
                        }
                        CALLBACK_SCF(false);
                        break;
                    default:
                        E_Exit("DOS:0x65:Unhandled country information call %2X",reg_al);   
                }
                break;
            }
        case 0x66:                  /* Get/Set global code page table  */
            if (reg_al==1) {
                LOG(LOG_DOSMISC,LOG_NORMAL)("Getting global code page table");
                reg_bx=reg_dx=dos.loaded_codepage;
                CALLBACK_SCF(false);
                break;
            }
            LOG(LOG_DOSMISC,LOG_ERROR)("DOS:Setting code page table is not supported");
            break;
        case 0x67:                  /* Set handle count */
            /* Weird call to increase amount of file handles needs to allocate memory if >20 */
            {
                DOS_PSP psp(dos.psp());
                psp.SetNumFiles(reg_bx);
                CALLBACK_SCF(false);
                break;
            }
        case 0x68:                  /* FFLUSH Commit file */
            case_0x68_fallthrough:
            if(DOS_FlushFile(reg_bl)) {
                CALLBACK_SCF(false);
            } else {
                reg_ax = dos.errorcode;
                CALLBACK_SCF(true);
            }
            break;
        case 0x69:                  /* Get/Set disk serial number */
            {
                Bit16u old_cx=reg_cx;
                switch(reg_al)      {
                    case 0x00:              /* Get */
                        LOG(LOG_DOSMISC,LOG_NORMAL)("DOS:Get Disk serial number");
                        reg_cl=0x66;// IOCTL function
                        break;
                    case 0x01:              /* Set */
                        LOG(LOG_DOSMISC,LOG_NORMAL)("DOS:Set Disk serial number");
                        reg_cl=0x46;// IOCTL function
                        break;
                    default:
                        E_Exit("DOS:Illegal Get Serial Number call %2X",reg_al);
                }
                reg_ch=0x08;    // IOCTL category: disk drive
                reg_ax=0x440d;  // Generic block device request
                DOS_21Handler();
                reg_cx=old_cx;
                break;
            }
        case 0x6a:                  /* Commit file */
            // Note: Identical to AH=68h in DOS 5.0-6.0; not known whether this is the case in DOS 4.x
            goto case_0x68_fallthrough;
        case 0x6b:                  /* NULL Function */
            goto default_fallthrough;
        case 0x6c:                  /* Extended Open/Create */
            MEM_StrCopy(SegPhys(ds)+reg_si,name1,DOSNAMEBUF);
            if (DOS_OpenFileExtended(name1,reg_bx,reg_cx,reg_dx,&reg_ax,&reg_cx)) {
                CALLBACK_SCF(false);
            } else {
                reg_ax=dos.errorcode;
                CALLBACK_SCF(true);
            }
            break;
        case 0x6d:                  /* ROM - Find first ROM program */
            LOG(LOG_DOSMISC, LOG_ERROR)("DOS:ROM - Find first ROM program not implemented");
            goto default_fallthrough;
        case 0x6e:                  /* ROM - Find next ROM program */
            LOG(LOG_DOSMISC, LOG_ERROR)("DOS:ROM - Find next ROM program not implemented");
            goto default_fallthrough;
        case 0x6f:                  /* ROM functions */
            LOG(LOG_DOSMISC, LOG_ERROR)("DOS:6F ROM functions not implemented");
            goto default_fallthrough;
        case 0x71:                  /* Unknown probably 4dos detection */
            reg_ax=0x7100;
            CALLBACK_SCF(true); //Check this! What needs this ? See default case
            LOG(LOG_DOSMISC,LOG_NORMAL)("DOS:Windows long file name support call %2X",reg_al);
            break;
        case 0xE0:
        case 0xEF:                  /* Used in Ancient Art Of War CGA */
        default:
            default_fallthrough:
            if (reg_ah < 0x6b) LOG(LOG_DOSMISC,LOG_ERROR)("DOS:Unhandled call %02X al=%02X. Set al to default of 0",reg_ah,reg_al); //Less errors. above 0x6c the functions are simply always skipped, only al is zeroed, all other registers untouched
            reg_al=0x00; /* default value */
            break;
    }

    /* if INT 21h involves any BIOS calls that need the timer, emulate the fact that tbe
     * BIOS might unmask IRQ 0 as part of the job (especially INT 13h disk I/O).
     *
     * Some DOS games & demos mask interrupts at the PIC level in a stingy manner that
     * apparently assumes DOS/BIOS will unmask some when called.
     *
     * Examples:
     *   Rebel by Arkham (without this fix, timer interrupt will not fire during demo and therefore music will not play). */
    if (unmask_irq0)
        PIC_SetIRQMask(0,false); /* Enable system timer */

    return CBRET_NONE;
}


static Bitu BIOS_1BHandler(void) {
    mem_writeb(BIOS_CTRL_BREAK_FLAG,0x00);

    /* take note (set flag) and return */
    /* FIXME: Don't forget that on "BOOT" this handler should be unassigned, though having it assigned
     *        to the guest OS causes no harm. */
    LOG_MSG("Note: default 1Bh handler invoked\n");
    DOS_BreakFlag = true;
    return CBRET_NONE;
}

static Bitu DOS_20Handler(void) {
	reg_ah=0x00;
	DOS_21Handler();
	return CBRET_NONE;
}

static Bitu DOS_CPMHandler(void) {
	// Convert a CPM-style call to a normal DOS call
	Bit16u flags=CPU_Pop16();
	CPU_Pop16();
	Bit16u caller_seg=CPU_Pop16();
	Bit16u caller_off=CPU_Pop16();
	CPU_Push16(flags);
	CPU_Push16(caller_seg);
	CPU_Push16(caller_off);
	if (reg_cl>0x24) {
		reg_al=0;
		return CBRET_NONE;
	}
	reg_ah=reg_cl;
	return DOS_21Handler();
}

static Bitu DOS_27Handler(void) {
	// Terminate & stay resident
	Bit16u para = (reg_dx/16)+((reg_dx % 16)>0);
	Bit16u psp = dos.psp(); //mem_readw(SegPhys(ss)+reg_sp+2);
	if (DOS_ResizeMemory(psp,&para)) {
		DOS_Terminate(psp,true,0);
		if (DOS_BreakINT23InProgress) throw int(0); /* HACK: Ick */
	}
	return CBRET_NONE;
}

static Bitu DOS_25Handler(void) {
    reg_ax = 0x8002;
    SETFLAGBIT(CF,true);
    return CBRET_NONE;
}
static Bitu DOS_26Handler(void) {
    reg_ax = 0x8002;
    SETFLAGBIT(CF,true);
    return CBRET_NONE;
}

bool enable_collating_uppercase = true;
bool keep_private_area_on_boot = false;
bool private_always_from_umb = false;
bool private_segment_in_umb = true;
Bit16u DOS_IHSEG = 0;

// NOTE about 0x70 and PC-98 emulation mode:
//
// I don't know exactly how things differ in NEC's PC-98 MS-DOS, but,
// according to some strange code in Touhou Project that's responsible
// for blanking the text layer, there's a "row count" variable at 0x70:0x12
// that holds (number of rows - 1). Leaving that byte value at zero prevents
// the game from clearing the screen (which also exposes the tile data and
// overdraw of the graphics layer). A value of zero instead just causes the
// first text character row to be filled in, not the whole visible text layer.
//
// Pseudocode of the routine:
//
// XOR AX,AX
// MOV ES,AX
// MOV AL,ES:[0712h]            ; AX = BYTE [0x70:0x12] zero extend (ex. 0x18 == 24)
// INC AX                       ; AX++                              (ex. becomes 0x19 == 25)
// MOV DX,AX
// SHL DX,1
// SHL DX,1                     ; DX *= 4
// ADD DX,AX                    ; DX += AX     equiv. DX = AX * 5
// MOV CL,4h
// SHL DX,CL                    ; DX <<= 4     equiv. DX = AX * 0x50  or  DX = AX * 80
// ...
// MOV AX,0A200h
// MOV ES,AX
// MOV AX,(solid black overlay block attribute)
// MOV CX,DX
// REP STOSW
//
// When the routine is done, the graphics layer is obscured by text character cells that
// represent all black (filled in) so that the game can later "punch out" the regions
// of the graphics layer it wants you to see. TH02 relies on this as well to flash the
// screen and open from the center to show the title screen. During gameplay, the text
// layer is used to obscure sprite overdraw when a sprite is partially off-screen as well
// as hidden tile data on the right hand half of the screen that the game read/write
// copies through the GDC pattern/tile registers to make the background. When the text
// layer is not present it's immediately apparent that the sprite renderer makes no attempt
// to clip sprites within the screen, but instead relies on the text overlay to hide the
// overdraw.
//
// this means that on PC-98 one of two things are true. either:
//  - NEC's variation of MS-DOS loads the base kernel higher up (perhaps at 0x80:0x00?)
//    and the BIOS data area lies from 0x40:00 to 0x7F:00
//
//    or
//
//  - NEC's variation loads at 0x70:0x00 (same as IBM PC MS-DOS) and Touhou Project
//    is dead guilty of reaching directly into MS-DOS kernel memory to read
//    internal variables it shouldn't be reading directly!
//
// Ick...

void DOS_GetMemory_reset();
void DOS_GetMemory_Choose();
Bitu MEM_PageMask(void);

#include <assert.h>

extern bool dos_con_use_int16_to_detect_input;
extern bool dbg_zero_on_dos_allocmem;
extern bool log_dev_con;

class DOS:public Module_base{
private:
	CALLBACK_HandlerObject callback[9];
	RealPt int30,int31;

public:
    void DOS_Write_HMA_CPM_jmp(void) {
        // HMA mirror of CP/M entry point.
        // this is needed for "F01D:FEF0" to be a valid jmp whether or not A20 is enabled
        if (dos_in_hma &&
            cpm_compat_mode != CPM_COMPAT_OFF &&
            cpm_compat_mode != CPM_COMPAT_DIRECT) {
            LOG(LOG_MISC,LOG_DEBUG)("Writing HMA mirror of CP/M entry point");

            Bitu was_a20 = XMS_GetEnabledA20();

            XMS_EnableA20(true);

            mem_writeb(0x1000C0,(Bit8u)0xea);		// jmpf
            mem_unalignedwrited(0x1000C0+1,callback[8].Get_RealPointer());

            if (!was_a20) XMS_EnableA20(false);
        }
    }

    Bit32u DOS_Get_CPM_entry_direct(void) {
        return callback[8].Get_RealPointer();
    }

	DOS(Section* configuration):Module_base(configuration){
		Section_prop * section=static_cast<Section_prop *>(configuration);

        ::disk_data_rate = section->Get_int("hard drive data rate limit");
        if (::disk_data_rate < 0) {
            ::disk_data_rate = 3500000; /* Probably an average IDE data rate for early 1990s ISA IDE controllers in PIO mode */
        }

        dos_in_hma = section->Get_bool("dos in hma");
        dos_sda_size = section->Get_int("dos sda size");
        log_dev_con = control->opt_log_con || section->Get_bool("log console");
		enable_dbcs_tables = section->Get_bool("dbcs");
		enable_share_exe_fake = section->Get_bool("share");
		enable_filenamechar = section->Get_bool("filenamechar");
		dos_initial_hma_free = section->Get_int("hma free space");
        minimum_mcb_free = section->Get_hex("minimum mcb free");
		minimum_mcb_segment = section->Get_hex("minimum mcb segment");
		private_segment_in_umb = section->Get_bool("private area in umb");
		enable_collating_uppercase = section->Get_bool("collating and uppercase");
		private_always_from_umb = section->Get_bool("kernel allocation in umb");
		minimum_dos_initial_private_segment = section->Get_hex("minimum dos initial private segment");
		dos_con_use_int16_to_detect_input = section->Get_bool("con device use int 16h to detect keyboard input");
		dbg_zero_on_dos_allocmem = section->Get_bool("zero memory on int 21h memory allocation");
		MAXENV = (unsigned int)section->Get_int("maximum environment block size on exec");
		ENV_KEEPFREE = (unsigned int)section->Get_int("additional environment block size on exec");
		enable_dummy_device_mcb = section->Get_bool("enable dummy device mcb");
		int15_wait_force_unmask_irq = section->Get_bool("int15 wait force unmask irq");
        disk_io_unmask_irq0 = section->Get_bool("unmask timer on disk io");

        if (dos_initial_hma_free > 0x10000)
            dos_initial_hma_free = 0x10000;

        std::string cpmcompat = section->Get_string("cpm compatibility mode");

        if (cpmcompat == "")
            cpmcompat = "auto";

        if (cpmcompat == "msdos2")
            cpm_compat_mode = CPM_COMPAT_MSDOS2;
        else if (cpmcompat == "msdos5")
            cpm_compat_mode = CPM_COMPAT_MSDOS5;
        else if (cpmcompat == "direct")
            cpm_compat_mode = CPM_COMPAT_DIRECT;
        else if (cpmcompat == "auto")
            cpm_compat_mode = CPM_COMPAT_MSDOS5; /* MS-DOS 5.x is default */
        else
            cpm_compat_mode = CPM_COMPAT_OFF;

        /* FIXME: Boot up an MS-DOS system and look at what INT 21h on Microsoft's MS-DOS returns
         *        for SDA size and location, then use that here.
         *
         *        Why does this value matter so much to WordPerfect 5.1? */
        if (dos_sda_size == 0)
            DOS_SDA_SEG_SIZE = 0x560;
        else if (dos_sda_size < 0x1A)
            DOS_SDA_SEG_SIZE = 0x1A;
        else if (dos_sda_size > 32768)
            DOS_SDA_SEG_SIZE = 32768;
        else
            DOS_SDA_SEG_SIZE = (dos_sda_size + 0xF) & (~0xF); /* round up to paragraph */

        /* msdos 2.x and msdos 5.x modes, if HMA is involved, require us to take the first 256 bytes of HMA
         * in order for "F01D:FEF0" to work properly whether or not A20 is enabled. Our direct mode doesn't
         * jump through that address, and therefore doesn't need it. */
        if (dos_in_hma &&
            cpm_compat_mode != CPM_COMPAT_OFF &&
            cpm_compat_mode != CPM_COMPAT_DIRECT) {
            LOG(LOG_MISC,LOG_DEBUG)("DOS: CP/M compatibility method with DOS in HMA requires mirror of entry point in HMA.");
            if (dos_initial_hma_free > 0xFF00) {
                dos_initial_hma_free = 0xFF00;
                LOG(LOG_MISC,LOG_DEBUG)("DOS: CP/M compatibility method requires reduction of HMA free space to accomodate.");
            }
        }

		if ((int)MAXENV < 0) MAXENV = 65535;
		if ((int)ENV_KEEPFREE < 0) ENV_KEEPFREE = 1024;

		LOG(LOG_MISC,LOG_DEBUG)("DOS: MAXENV=%u ENV_KEEPFREE=%u",MAXENV,ENV_KEEPFREE);

		if (ENV_KEEPFREE < 83)
			LOG_MSG("DOS: ENV_KEEPFREE is below 83 bytes. DOS programs that rely on undocumented data following the environment block may break.");

		if (dbg_zero_on_dos_allocmem) {
			LOG_MSG("Debug option enabled: INT 21h memory allocation will always clear memory block before returning\n");
		}

		if (minimum_mcb_segment > 0x8000) minimum_mcb_segment = 0x8000; /* FIXME: Clip against available memory */

        /* we make use of the DOS_GetMemory() function for the dynamic allocation */
        if (private_always_from_umb) {
            DOS_GetMemory_Choose(); /* the pool starts in UMB */
            if (minimum_mcb_segment == 0)
                DOS_MEM_START = 0x70; /* funny behavior in some games suggests the MS-DOS kernel loads a bit higher on PC-98 */
            else
                DOS_MEM_START = minimum_mcb_segment;

            if (DOS_MEM_START < 0x40)
                LOG_MSG("DANGER, DANGER! DOS_MEM_START has been set to within the interrupt vector table! Proceed at your own risk!");
            else if (DOS_MEM_START < 0x50)
                LOG_MSG("WARNING: DOS_MEM_START has been assigned to the BIOS data area! Proceed at your own risk!");
            else if (DOS_MEM_START < 0x51)
                LOG_MSG("WARNING: DOS_MEM_START has been assigned to segment 0x50, which some programs may use as the Print Screen flag");
            else if (DOS_MEM_START < 0x70)
                LOG_MSG("CAUTION: DOS_MEM_START is less than 0x70 which may cause problems with some DOS games or applications");
        }
        else {
            if (minimum_dos_initial_private_segment == 0)
                DOS_PRIVATE_SEGMENT = 0x70; /* funny behavior in some games suggests the MS-DOS kernel loads a bit higher on PC-98 */
            else
                DOS_PRIVATE_SEGMENT = minimum_dos_initial_private_segment;

            if (DOS_PRIVATE_SEGMENT < 0x50)
                LOG_MSG("DANGER, DANGER! DOS_PRIVATE_SEGMENT has been set too low!");

            if (MEM_TotalPages() > 0x9C)
                DOS_PRIVATE_SEGMENT_END = 0x9C00;
            else
                DOS_PRIVATE_SEGMENT_END = (Bit16u)((MEM_TotalPages() << (12 - 4)) - 1); /* NTS: Remember DOSBox's implementation reuses the last paragraph for UMB linkage */
        }

        LOG(LOG_MISC,LOG_DEBUG)("DOS kernel structures will be allocated from pool 0x%04x-0x%04x",
                DOS_PRIVATE_SEGMENT,DOS_PRIVATE_SEGMENT_END-1);

        DOS_IHSEG = DOS_GetMemory(1,"DOS_IHSEG");

        /* DOS_INFOBLOCK_SEG contains the entire List of Lists, though the INT 21h call returns seg:offset with offset nonzero */
        DOS_INFOBLOCK_SEG = DOS_GetMemory(0xC0,"DOS_INFOBLOCK_SEG");	// was 0x80

        DOS_CONDRV_SEG = DOS_GetMemory(0x08,"DOS_CONDRV_SEG");		// was 0xA0
        DOS_CONSTRING_SEG = DOS_GetMemory(0x0A,"DOS_CONSTRING_SEG");	// was 0xA8
        DOS_SDA_SEG = DOS_GetMemory(DOS_SDA_SEG_SIZE>>4,"DOS_SDA_SEG");		// was 0xB2  (0xB2 + 0x56 = 0x108)
        DOS_SDA_OFS = 0;
        DOS_CDS_SEG = DOS_GetMemory(0x10,"DOS_CDA_SEG");		// was 0x108

		LOG(LOG_MISC,LOG_DEBUG)("DOS kernel alloc:");
		LOG(LOG_MISC,LOG_DEBUG)("   IHSEG:        seg 0x%04x",DOS_IHSEG);
		LOG(LOG_MISC,LOG_DEBUG)("   infoblock:    seg 0x%04x",DOS_INFOBLOCK_SEG);
		LOG(LOG_MISC,LOG_DEBUG)("   condrv:       seg 0x%04x",DOS_CONDRV_SEG);
		LOG(LOG_MISC,LOG_DEBUG)("   constring:    seg 0x%04x",DOS_CONSTRING_SEG);
		LOG(LOG_MISC,LOG_DEBUG)("   SDA:          seg 0x%04x:0x%04x %u bytes",DOS_SDA_SEG,DOS_SDA_OFS,DOS_SDA_SEG_SIZE);
		LOG(LOG_MISC,LOG_DEBUG)("   CDS:          seg 0x%04x",DOS_CDS_SEG);
		LOG(LOG_MISC,LOG_DEBUG)("[private segment @ this point 0x%04x-0x%04x mem=0x%04lx]",
			DOS_PRIVATE_SEGMENT,DOS_PRIVATE_SEGMENT_END,
			(unsigned long)(MEM_TotalPages() << (12 - 4)));

		callback[0].Install(DOS_20Handler,CB_IRET,"DOS Int 20");
		callback[0].Set_RealVec(0x20);

		callback[1].Install(DOS_21Handler,CB_INT21,"DOS Int 21");
		callback[1].Set_RealVec(0x21);
	//Pseudo code for int 21
	// sti
	// callback 
	// iret
	// retf  <- int 21 4c jumps here to mimic a retf Cyber

		callback[2].Install(DOS_25Handler,CB_RETF_STI,"DOS Int 25");
		callback[2].Set_RealVec(0x25);

		callback[3].Install(DOS_26Handler,CB_RETF_STI,"DOS Int 26");
		callback[3].Set_RealVec(0x26);

		callback[4].Install(DOS_27Handler,CB_IRET,"DOS Int 27");
		callback[4].Set_RealVec(0x27);

		callback[5].Install(NULL,CB_IRET/*CB_INT28*/,"DOS idle");
		callback[5].Set_RealVec(0x28);

        {
            // FIXME: Really? Considering the main CON device emulation has ANSI.SYS emulation
            //        you'd think that this would route it through the same.
            callback[6].Install(NULL,CB_INT29,"CON Output Int 29");
            callback[6].Set_RealVec(0x29);
            // pseudocode for CB_INT29:
            //	push ax
            //	mov ah, 0x0e
            //	int 0x10
            //	pop ax
            //	iret
        }

        /* DOS installs a handler for INT 1Bh */
        callback[7].Install(BIOS_1BHandler,CB_IRET,"BIOS 1Bh");
        callback[7].Set_RealVec(0x1B);

		callback[8].Install(DOS_CPMHandler,CB_CPM,"DOS/CPM Int 30-31");
		int30=RealGetVec(0x30);
		int31=RealGetVec(0x31);
		mem_writeb(0x30*4,(Bit8u)0xea);		// jmpf
		mem_unalignedwrited(0x30*4+1,callback[8].Get_RealPointer());
		// pseudocode for CB_CPM:
		//	pushf
		//	... the rest is like int 21

        /* NTS: HMA support requires XMS. EMS support may switch on A20 if VCPI emulation requires the odd megabyte */
        if ((!dos_in_hma || !section->Get_bool("xms")) && (MEM_A20_Enabled() || strcmp(section->Get_string("ems"),"false") != 0) &&
            cpm_compat_mode != CPM_COMPAT_OFF && cpm_compat_mode != CPM_COMPAT_DIRECT) {
            /* hold on, only if more than 1MB of RAM and memory access permits it */
            if (MEM_TotalPages() > 0x100 && MEM_PageMask() > 0xff/*more than 20-bit decoding*/) {
                LOG(LOG_MISC,LOG_WARN)("DOS not in HMA or XMS is disabled. This may break programs using the CP/M compatibility call method if the A20 gate is switched on.");
            }
        }

		DOS_FILES = (unsigned int)section->Get_int("files");
		DOS_SetupFiles();								/* Setup system File tables */
		DOS_SetupDevices();							/* Setup dos devices */
		DOS_SetupTables();

		/* move the private segment elsewhere to avoid conflict with the MCB structure.
		 * either set to 0 to cause the decision making to choose an upper memory address,
		 * or allocate an additional private area and start the MCB just after that */
		if (!private_always_from_umb) {
			DOS_MEM_START = DOS_GetMemory(0,"DOS_MEM_START");		// was 0x158 (pass 0 to alloc nothing, get the pointer)

			DOS_GetMemory_reset();
			DOS_PRIVATE_SEGMENT = 0;
			DOS_PRIVATE_SEGMENT_END = 0;
			if (!private_segment_in_umb) {
				/* If private segment is not being placed in UMB, then it must follow the DOS kernel. */
				unsigned int seg;
				unsigned int segend;

				seg = DOS_MEM_START;
				DOS_MEM_START += (Bit16u)DOS_PRIVATE_SEGMENT_Size;
				segend = DOS_MEM_START;

				if (segend >= (MEM_TotalPages() << (12 - 4)))
					E_Exit("Insufficient room for private area");

				DOS_PRIVATE_SEGMENT = seg;
				DOS_PRIVATE_SEGMENT_END = segend;
				DOS_MEM_START = DOS_PRIVATE_SEGMENT_END;
				DOS_GetMemory_reset();
				LOG_MSG("Private area, not stored in UMB on request, occupies 0x%04x-0x%04x [dynamic]\n",
					DOS_PRIVATE_SEGMENT,DOS_PRIVATE_SEGMENT_END-1);
			}
		}

		if (minimum_mcb_segment != 0) {
			if (DOS_MEM_START < minimum_mcb_segment)
				DOS_MEM_START = minimum_mcb_segment;
		}

		LOG(LOG_MISC,LOG_DEBUG)("   mem start:    seg 0x%04x",DOS_MEM_START);

		/* carry on setup */
		DOS_SetupMemory();								/* Setup first MCB */

        /* NTS: The reason PC-98 has a higher minimum free is that the MS-DOS kernel
         *      has a larger footprint in memory, including fixed locations that
         *      some PC-98 games will read directly, and an ANSI driver.
         *
         *      Some PC-98 games will have problems if loaded below a certain
         *      threshhold as well.
         *
         *        Valkyrie: 0xE10 is not enough for the game to run. If a specific
         *                  FM music selection is chosen, the remaining memory is
         *                  insufficient for the game to start the battle.
         *
         *      The default assumes a DOS kernel and lower memory region of 32KB,
         *      which might be a reasonable compromise so far.
         *
         * NOTES: A minimum mcb free value of at least 0xE10 is needed for Windows 3.1
         *        386 enhanced to start, else it will complain about insufficient memory (?).
         *        To get Windows 3.1 to run, either set "minimum mcb free=e10" or run
         *        "LOADFIX" before starting Windows 3.1 */

        /* NTS: There is a mysterious memory corruption issue with some DOS games
         *      and applications when they are loaded at or around segment 0x800.
         *      This should be looked into. In the meantime, setting the MCB
         *      start segment before or after 0x800 helps to resolve these issues.
         *      It also puts DOSBox-X at parity with main DOSBox SVN behavior. */
        if (minimum_mcb_free == 0)
            minimum_mcb_free = 0x100;
        else if (minimum_mcb_free < minimum_mcb_segment)
            minimum_mcb_free = minimum_mcb_segment;

        LOG(LOG_MISC,LOG_DEBUG)("   min free:     seg 0x%04x",minimum_mcb_free);

        if (DOS_MEM_START < minimum_mcb_free) {
            Bit16u sg=0,tmp;

            dos.psp(8); // DOS ownership

            tmp = 1; // start small
            if (DOS_AllocateMemory(&sg,&tmp)) {
                if (sg < minimum_mcb_free) {
                    LOG(LOG_MISC,LOG_DEBUG)("   min free pad: seg 0x%04x",sg);
                }
                else {
                    DOS_FreeMemory(sg);
                    sg = 0;
                }
            }
            else {
                sg=0;
            }

            if (sg != 0 && sg < minimum_mcb_free) {
                tmp = minimum_mcb_free - sg;
                if (!DOS_ResizeMemory(sg,&tmp)) {
                    LOG(LOG_MISC,LOG_DEBUG)("    WARNING: cannot resize min free pad");
                }
            }
        }

		DOS_SetupPrograms();
		DOS_SetupMisc();							/* Some additional dos interrupts */
		DOS_SDA(DOS_SDA_SEG,DOS_SDA_OFS).SetDrive(25); /* Else the next call gives a warning. */
		DOS_SetDefaultDrive(25);

		keep_private_area_on_boot = section->Get_bool("keep private area on boot");
	
		dos.version.major=5;
		dos.version.minor=0;
		dos.direct_output=false;
		dos.internal_output=false;

		std::string ver = section->Get_string("ver");
		if (!ver.empty()) {
			const char *s = ver.c_str();

			if (isdigit(*s)) {
				dos.version.minor=0;
				dos.version.major=(int)strtoul(s,(char**)(&s),10);
				if (*s == '.') {
					s++;
					if (isdigit(*s)) {
						dos.version.minor=(int)strtoul(s,(char**)(&s),10);
					}
				}

				/* warn about unusual version numbers */
				if (dos.version.major >= 10 && dos.version.major <= 30) {
					LOG_MSG("WARNING, DOS version %u.%u: the major version is set to a "
						"range that may cause some DOS programs to think they are "
						"running from within an OS/2 DOS box.",
						dos.version.major, dos.version.minor);
				}
				else if (dos.version.major == 0 || dos.version.major > 8 || dos.version.minor > 90)
					LOG_MSG("WARNING: DOS version %u.%u is unusual, may confuse DOS programs",
						dos.version.major, dos.version.minor);
			}
		}
	}
	~DOS(){
		/* NTS: We do NOT free the drives! The OS may use them later! */
		void DOS_ShutdownFiles();
		DOS_ShutdownFiles();
		void DOS_ShutdownDevices(void);
		DOS_ShutdownDevices();
		RealSetVec(0x30,int30);
		RealSetVec(0x31,int31);
	}
};

static DOS* test = NULL;

void DOS_Write_HMA_CPM_jmp(void) {
    assert(test != NULL);
    test->DOS_Write_HMA_CPM_jmp();
}

Bit32u DOS_Get_CPM_entry_direct(void) {
    assert(test != NULL);
    return test->DOS_Get_CPM_entry_direct();
}

void DOS_ShutdownFiles() {
	if (Files != NULL) {
		for (Bitu i=0;i<DOS_FILES;i++) {
			if (Files[i] != NULL) {
				delete Files[i];
				Files[i] = NULL;
			}
		}
		delete[] Files;
		Files = NULL;
	}
}

void DOS_ShutdownDrives() {
	for (Bit16u i=0;i<DOS_DRIVES;i++) {
		delete Drives[i];
		Drives[i] = NULL;
	}
}

void DOS_Casemap_Free();

extern Bit8u ZDRIVE_NUM;

void DOS_DoShutDown() {
	if (test != NULL) {
		delete test;
		test = NULL;
	}

    DOS_Casemap_Free();
}

void DOS_ShutDown(Section* /*sec*/) {
	DOS_DoShutDown();
}

void DOS_GetMemory_reinit();

void DOS_OnReset(Section* /*sec*/) {
	DOS_DoShutDown();
    DOS_GetMemory_reinit();
}

void DOS_Startup(Section* sec) {
    (void)sec;//UNUSED

	if (test == NULL) {
        DOS_GetMemLog.clear();
        DOS_GetMemory_reinit();
        LOG(LOG_MISC,LOG_DEBUG)("Allocating DOS kernel");
		test = new DOS(control->GetSection("dos"));
	}
}

void DOS_Init() {
	LOG(LOG_MISC,LOG_DEBUG)("Initializing DOS kernel (DOS_Init)");

	AddExitFunction(AddExitFunctionFuncPair(DOS_ShutDown),false);
	AddVMEventFunction(VM_EVENT_RESET,AddVMEventFunctionFuncPair(DOS_OnReset));
	AddVMEventFunction(VM_EVENT_DOS_EXIT_KERNEL,AddVMEventFunctionFuncPair(DOS_ShutDown));
	AddVMEventFunction(VM_EVENT_DOS_EXIT_REBOOT_KERNEL,AddVMEventFunctionFuncPair(DOS_ShutDown));
	AddVMEventFunction(VM_EVENT_DOS_SURPRISE_REBOOT,AddVMEventFunctionFuncPair(DOS_OnReset));
}

