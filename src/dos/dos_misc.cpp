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


#include "dosbox.h"
#include "callback.h"
#include "mem.h"
#include "regs.h"
#include "dos_inc.h"
#include <list>

Bit32u DOS_HMA_LIMIT();
Bit32u DOS_HMA_FREE_START();
Bit32u DOS_HMA_GET_FREE_SPACE();
void DOS_HMA_CLAIMED(Bit16u bytes);

extern bool enable_share_exe_fake;

bool enable_a20_on_windows_init = false;

static Bitu call_int2f,call_int2a;

static std::list<MultiplexHandler*> Multiplex;
typedef std::list<MultiplexHandler*>::iterator Multiplex_it;

const char *Win_NameThatVXD(Bit16u devid) {
	switch (devid) {
		case 0x0006:	return "V86MMGR";
		case 0x000C:	return "VMD";
		case 0x000D:	return "VKD";
		case 0x0010:	return "BLOCKDEV";
		case 0x0014:	return "VNETBIOS";
		case 0x0015:	return "DOSMGR";
		case 0x0018:	return "VMPOLL";
		case 0x0021:	return "PAGEFILE";
		case 0x002D:	return "W32S";
		case 0x0040:	return "IFSMGR";
		case 0x0446:	return "VADLIBD";
		case 0x0484:	return "IFSMGR";
		case 0x0487:	return "NWSUP";
		case 0x28A1:	return "PHARLAP";
		case 0x7A5F:	return "SIWVID";
	}

	return NULL;
}

void DOS_AddMultiplexHandler(MultiplexHandler * handler) {
	Multiplex.push_front(handler);
}

void DOS_DelMultiplexHandler(MultiplexHandler * handler) {
	for(Multiplex_it it =Multiplex.begin();it != Multiplex.end();++it) {
		if(*it == handler) {
			Multiplex.erase(it);
			return;
		}
	}
}

static Bitu INT2F_Handler(void) {
	for(Multiplex_it it = Multiplex.begin();it != Multiplex.end();++it)
		if( (*it)() ) return CBRET_NONE;
   
	LOG(LOG_DOSMISC,LOG_ERROR)("DOS:INT 2F Unhandled call AX=%4X",reg_ax);
	return CBRET_NONE;
}


static Bitu INT2A_Handler(void) {
	return CBRET_NONE;
}

extern RealPt DOS_DriveDataListHead;       // INT 2Fh AX=0803h DRIVER.SYS drive data table list

// INT 2F
static bool DOS_MultiplexFunctions(void) {
	switch (reg_ax) {
    case 0x0800:    /* DRIVER.SYS function */
    case 0x0801:    /* DRIVER.SYS function */
    case 0x0802:    /* DRIVER.SYS function */
        LOG(LOG_MISC,LOG_DEBUG)("Unhandled DRIVER.SYS call AX=%04x BX=%04x CX=%04x DX=%04x BP=%04x",reg_ax,reg_bx,reg_cx,reg_dx,reg_bp);
        break;
    case 0x0803:    /* DRIVER.SYS function */
        LOG(LOG_MISC,LOG_DEBUG)("Unhandled DRIVER.SYS call AX=%04x BX=%04x CX=%04x DX=%04x BP=%04x",reg_ax,reg_bx,reg_cx,reg_dx,reg_bp);
        // FIXME: Windows 95 SCANDISK.EXE relies on the drive data table list pointer provided by this call.
        //        Returning DS:DI unmodified or set to 0:0 will only send it off into the weeds chasing random data
        //        as a linked list. However looking at the code DI=0xFFFF is sufficient to prevent that until
        //        DOSBox-X emulates DRIVER.SYS functions and provides the information it expects according to RBIL.
        //        BUT, Windows 95 setup checks if the pointer is NULL, and considers 0:FFFF valid >_<.
        //        It's just easier to return a pointer to a dummy table.
        //        [http://www.ctyme.com/intr/rb-4283.htm]
        SegSet16(ds,DOS_DriveDataListHead >> 16);
        reg_di = DOS_DriveDataListHead;
        break;
	/* ert, 20100711: Locking extensions */
	case 0x1000:	/* SHARE.EXE installation check */
		if (enable_share_exe_fake) {
			reg_ax=0xffff; /* Pretend that share.exe is installed.. Of course it's a bloody LIE! */
		}
		else {
			return false; /* pass it on */
		}
		break;
	case 0x1216:	/* GET ADDRESS OF SYSTEM FILE TABLE ENTRY */
		// reg_bx is a system file table entry, should coincide with
		// the file handle so just use that
		LOG(LOG_DOSMISC,LOG_ERROR)("Some BAD filetable call used bx=%X",reg_bx);
		if(reg_bx <= DOS_FILES) CALLBACK_SCF(false);
		else CALLBACK_SCF(true);
		if (reg_bx<16) {
			RealPt sftrealpt=mem_readd(Real2Phys(dos_infoblock.GetPointer())+4);
			PhysPt sftptr=Real2Phys(sftrealpt);
			Bit32u sftofs=0x06u+reg_bx*0x3bu;

			if (Files[reg_bx]) mem_writeb(sftptr+sftofs, (Bit8u)(Files[reg_bx]->refCtr));
			else mem_writeb(sftptr+sftofs,0);

			if (!Files[reg_bx]) return true;

			Bit32u handle=RealHandle(reg_bx);
			if (handle>=DOS_FILES) {
				mem_writew(sftptr+sftofs+0x02,0x02);	// file open mode
				mem_writeb(sftptr+sftofs+0x04,0x00);	// file attribute
				mem_writew(sftptr+sftofs+0x05,Files[reg_bx]->GetInformation());	// device info word
				mem_writed(sftptr+sftofs+0x07,0);		// device driver header
				mem_writew(sftptr+sftofs+0x0d,0);		// packed time
				mem_writew(sftptr+sftofs+0x0f,0);		// packed date
				mem_writew(sftptr+sftofs+0x11,0);		// size
				mem_writew(sftptr+sftofs+0x15,0);		// current position
			} else {
				Bit8u drive=Files[reg_bx]->GetDrive();

				mem_writew(sftptr+sftofs+0x02,(Bit16u)(Files[reg_bx]->flags&3));	// file open mode
				mem_writeb(sftptr+sftofs+0x04,(Bit8u)(Files[reg_bx]->attr));		// file attribute
				mem_writew(sftptr+sftofs+0x05,0x40|drive);							// device info word
				mem_writed(sftptr+sftofs+0x07,RealMake(dos.tables.dpb,drive*dos.tables.dpb_size));	// dpb of the drive
				mem_writew(sftptr+sftofs+0x0d,Files[reg_bx]->time);					// packed file time
				mem_writew(sftptr+sftofs+0x0f,Files[reg_bx]->date);					// packed file date
				Bit32u curpos=0;
				Files[reg_bx]->Seek(&curpos,DOS_SEEK_CUR);
				Bit32u endpos=0;
				Files[reg_bx]->Seek(&endpos,DOS_SEEK_END);
				mem_writed(sftptr+sftofs+0x11,endpos);		// size
				mem_writed(sftptr+sftofs+0x15,curpos);		// current position
				Files[reg_bx]->Seek(&curpos,DOS_SEEK_SET);
			}

			// fill in filename in fcb style
			// (space-padded name (8 chars)+space-padded extension (3 chars))
			const char* filename=(const char*)Files[reg_bx]->GetName();
			if (strrchr(filename,'\\')) filename=strrchr(filename,'\\')+1;
			if (strrchr(filename,'/')) filename=strrchr(filename,'/')+1;
			if (!filename) return true;
			const char* dotpos=strrchr(filename,'.');
			if (dotpos) {
				dotpos++;
				size_t nlen=strlen(filename);
				size_t extlen=strlen(dotpos);
				Bits nmelen=(Bits)nlen-(Bits)extlen;
				if (nmelen<1) return true;
				nlen-=(extlen+1);

				if (nlen>8) nlen=8;
				size_t i;

				for (i=0; i<nlen; i++)
					mem_writeb((PhysPt)(sftptr+sftofs+0x20u+i),(unsigned char)filename[i]);
				for (i=nlen; i<8; i++)
					mem_writeb((PhysPt)(sftptr+sftofs+0x20u+i),(unsigned char)' ');
				
				if (extlen>3) extlen=3;
				for (i=0; i<extlen; i++)
					mem_writeb((PhysPt)(sftptr+sftofs+0x28u+i),(unsigned char)dotpos[i]);
				for (i=extlen; i<3; i++)
					mem_writeb((PhysPt)(sftptr+sftofs+0x28u+i),(unsigned char)' ');
			} else {
				size_t i;
				size_t nlen=strlen(filename);
				if (nlen>8) nlen=8;
				for (i=0; i<nlen; i++)
					mem_writeb((PhysPt)(sftptr+sftofs+0x20u+i),(unsigned char)filename[i]);
				for (i=nlen; i<11; i++)
					mem_writeb((PhysPt)(sftptr+sftofs+0x20u+i),(unsigned char)' ');
			}

			SegSet16(es,RealSeg(sftrealpt));
			reg_di=RealOff(sftrealpt+sftofs);
			reg_ax=0xc000;

		}
		return true;
	case 0x1600:	/* Windows enhanced mode installation check */
		// Leave AX as 0x1600, indicating that neither Windows 3.x enhanced mode nor Windows/386 2.x is running, nor is XMS version 1 driver installed
		return true;
    }

	return false;
}

void DOS_SetupMisc(void) {
	/* Setup the dos multiplex interrupt */
	call_int2f=CALLBACK_Allocate();
	CALLBACK_Setup(call_int2f,&INT2F_Handler,CB_IRET,"DOS Int 2f");
	RealSetVec(0x2f,CALLBACK_RealPointer(call_int2f));
	DOS_AddMultiplexHandler(DOS_MultiplexFunctions);
	/* Setup the dos network interrupt */
	call_int2a=CALLBACK_Allocate();
	CALLBACK_Setup(call_int2a,&INT2A_Handler,CB_IRET,"DOS Int 2a");
	RealSetVec(0x2A,CALLBACK_RealPointer(call_int2a));
}

void CALLBACK_DeAllocate(Bitu in);

void DOS_UninstallMisc(void) {
	/* these vectors shouldn't exist when booting a guest OS */
	if (call_int2a) {
		RealSetVec(0x2a,0);
		CALLBACK_DeAllocate(call_int2a);
		call_int2a=0;
	}
	if (call_int2f) {
		RealSetVec(0x2f,0);
		CALLBACK_DeAllocate(call_int2f);
		call_int2f=0;
	}
}

