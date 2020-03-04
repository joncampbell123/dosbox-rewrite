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


#include "dos_inc.h"
#include "../ints/int10.h"
#include <string.h>
#include "inout.h"
#include "shiftjis.h"
#include "callback.h"

#define NUMBER_ANSI_DATA 10

extern bool DOS_BreakFlag;

Bitu INT10_Handler(void);
Bitu INT16_Handler_Wrap(void);

ShiftJISDecoder con_sjis;

Bit16u last_int16_code = 0;

static size_t dev_con_pos=0,dev_con_max=0;
static unsigned char dev_con_readbuf[64];

Bit8u DefaultANSIAttr() {
	return 0x07;
}

class device_CON : public DOS_Device {
public:
	device_CON();
	bool Read(Bit8u * data,Bit16u * size);
	bool Write(const Bit8u * data,Bit16u * size);
	bool Seek(Bit32u * pos,Bit32u type);
	bool Close();
	Bit16u GetInformation(void);
	bool ReadFromControlChannel(PhysPt bufptr,Bit16u size,Bit16u * retcode) { (void)bufptr; (void)size; (void)retcode; return false; }
	bool WriteToControlChannel(PhysPt bufptr,Bit16u size,Bit16u * retcode) { (void)bufptr; (void)size; (void)retcode; return false; }
private:
	void ClearAnsi(void);
	void Output(Bit8u chr);
	Bit8u readcache;
	struct ansi { /* should create a constructor, which would fill them with the appropriate values */
		bool esc;
		bool sci;
        bool equcurp;       // ????? ESC = Y X      cursor pos    (not sure if PC-98 specific or general to DOS ANSI.SYS)
        bool pc98rab;       // PC-98 ESC [ > ...    (right angle bracket) I will rename this variable if MS-DOS ANSI.SYS also supports this sequence
		bool enabled;
		Bit8u attr;         // machine-specific
		Bit8u data[NUMBER_ANSI_DATA];
		Bit8u numberofarg;
		Bit16u nrows;
		Bit16u ncols;
		Bit8u savecol;
		Bit8u saverow;
		bool warned;
	} ansi;

    // ESC M
    void ESC_M(void) {
        LineFeedRev();
        ClearAnsi();
    }

    // ESC D
    void ESC_D(void) {
        LineFeed();
        ClearAnsi();
    }

    // ESC E
    void ESC_E(void) {
        Real_INT10_TeletypeOutputAttr('\n',ansi.attr,ansi.enabled);
        ClearAnsi();
    }

    // ESC [ A
    void ESC_BRACKET_A(void) {
        Bit8u page=real_readb(BIOSMEM_SEG,BIOSMEM_CURRENT_PAGE);
        Bit8u tempdata;
        Bit8u col,row;

        col=CURSOR_POS_COL(page) ;
        row=CURSOR_POS_ROW(page) ;
        tempdata = (ansi.data[0]? ansi.data[0] : 1);
        if(tempdata > row) { row=0; } 
        else { row-=tempdata;}
        Real_INT10_SetCursorPos(row,col,page);
        ClearAnsi();
    }

    // ESC [ B
    void ESC_BRACKET_B(void) {
        Bit8u page=real_readb(BIOSMEM_SEG,BIOSMEM_CURRENT_PAGE);
        Bit8u tempdata;
        Bit8u col,row;

        col=CURSOR_POS_COL(page) ;
        row=CURSOR_POS_ROW(page) ;
        ansi.nrows = real_readb(BIOSMEM_SEG,BIOSMEM_NB_ROWS) + 1;
        tempdata = (ansi.data[0]? ansi.data[0] : 1);
        if(tempdata + static_cast<Bitu>(row) >= ansi.nrows)
        { row = ansi.nrows - 1;}
        else	{ row += tempdata; }
        Real_INT10_SetCursorPos(row,col,page);
        ClearAnsi();
    }

    // ESC [ C
    void ESC_BRACKET_C(void) {
        Bit8u page=real_readb(BIOSMEM_SEG,BIOSMEM_CURRENT_PAGE);
        Bit8u tempdata;
        Bit8u col,row;

        col=CURSOR_POS_COL(page);
        row=CURSOR_POS_ROW(page);
        ansi.ncols = real_readw(BIOSMEM_SEG,BIOSMEM_NB_COLS);
        tempdata=(ansi.data[0]? ansi.data[0] : 1);
        if(tempdata + static_cast<Bitu>(col) >= ansi.ncols) 
        { col = ansi.ncols - 1;} 
        else	{ col += tempdata;}
        Real_INT10_SetCursorPos(row,col,page);
        ClearAnsi();
    }

    // ESC [ D
    void ESC_BRACKET_D(void) {
        Bit8u page=real_readb(BIOSMEM_SEG,BIOSMEM_CURRENT_PAGE);
        Bit8u tempdata;
        Bit8u col,row;

        col=CURSOR_POS_COL(page);
        row=CURSOR_POS_ROW(page);
        tempdata=(ansi.data[0]? ansi.data[0] : 1);
        if(tempdata > col) {col = 0;}
        else { col -= tempdata;}
        Real_INT10_SetCursorPos(row,col,page);
        ClearAnsi();
    }

    // ESC = Y X
    void ESC_EQU_cursor_pos(void) {
        Bit8u page=real_readb(BIOSMEM_SEG,BIOSMEM_CURRENT_PAGE);

        /* This is what the PC-98 ANSI driver does */
        if(ansi.data[0] >= 0x20) ansi.data[0] -= 0x20;
        else ansi.data[0] = 0;
        if(ansi.data[1] >= 0x20) ansi.data[1] -= 0x20;
        else ansi.data[1] = 0;

        ansi.ncols = real_readw(BIOSMEM_SEG,BIOSMEM_NB_COLS);
        ansi.nrows = real_readb(BIOSMEM_SEG,BIOSMEM_NB_ROWS) + 1;
        /* Turn them into positions that are on the screen */
        if(ansi.data[0] >= ansi.nrows) ansi.data[0] = (Bit8u)ansi.nrows - 1;
        if(ansi.data[1] >= ansi.ncols) ansi.data[1] = (Bit8u)ansi.ncols - 1;
        Real_INT10_SetCursorPos(ansi.data[0],ansi.data[1],page);

        ClearAnsi();
    }

	static void Real_INT10_SetCursorPos(Bit8u row,Bit8u col,Bit8u page) {
		Bit16u		oldax,oldbx,olddx;

		oldax=reg_ax;
		oldbx=reg_bx;
		olddx=reg_dx;

		reg_ah=0x2;
		reg_dh=row;
		reg_dl=col;
		reg_bh=page;

        CALLBACK_RunRealInt(0x10);

		reg_ax=oldax;
		reg_bx=oldbx;
		reg_dx=olddx;
	}

	static void Real_INT10_TeletypeOutput(Bit8u xChar,Bit8u xAttr) {
        Bit16u oldax,oldbx;
        oldax=reg_ax;
        oldbx=reg_bx;
            
        reg_ah=0xE;
        reg_al=xChar;
        reg_bl=xAttr;
            
        CALLBACK_RunRealInt(0x10);
        
        reg_ax=oldax;
        reg_bx=oldbx;
    }


	static void Real_WriteChar(Bit8u cur_col,Bit8u cur_row,
					Bit8u page,Bit8u chr,Bit8u attr,Bit8u useattr) {
		//Cursor position
		Real_INT10_SetCursorPos(cur_row,cur_col,page);

		//Write the character
		Bit16u		oldax,oldbx,oldcx;
		oldax=reg_ax;
		oldbx=reg_bx;
		oldcx=reg_cx;

		reg_al=chr;
		reg_bl=attr;
		reg_bh=page;
		reg_cx=1;
		if(useattr)
				reg_ah=0x9;
		else	reg_ah=0x0A;

        /* FIXME: PC-98 emulation should eventually use CONIO emulation that
         *        better emulates the actual platform. The purpose of this
         *        hack is to allow our code to call into INT 10h without
         *        setting up an INT 10h vector */
        CALLBACK_RunRealInt(0x10);
        
		reg_ax=oldax;
		reg_bx=oldbx;
		reg_cx=oldcx;
	}//static void Real_WriteChar(cur_col,cur_row,page,chr,attr,useattr)

    static void LineFeedRev(void) { // ESC M
		BIOS_NCOLS;BIOS_NROWS;
		auto defattr = DefaultANSIAttr();
		Bit8u page=real_readb(BIOSMEM_SEG,BIOSMEM_CURRENT_PAGE);
		Bit8u cur_row=CURSOR_POS_ROW(page);
		Bit8u cur_col=CURSOR_POS_COL(page);

		if(cur_row==0) 
		{
            INT10_ScrollWindow(0,0,(Bit8u)(nrows-1),(Bit8u)(ncols-1),1,defattr,0);
        }
        else {
            cur_row--;
        }

		Real_INT10_SetCursorPos(cur_row,cur_col,page);	
    }

    static void LineFeed(void) { // ESC D
		BIOS_NCOLS;BIOS_NROWS;
		auto defattr = DefaultANSIAttr();
		Bit8u page=real_readb(BIOSMEM_SEG,BIOSMEM_CURRENT_PAGE);
		Bit8u cur_row=CURSOR_POS_ROW(page);
		Bit8u cur_col=CURSOR_POS_COL(page);

        if (cur_row < nrows) cur_row++;

		if(cur_row==nrows) 
		{
            INT10_ScrollWindow(0,0,(Bit8u)(nrows-1),(Bit8u)(ncols-1),-1,defattr,0);
            cur_row--;
		}

		Real_INT10_SetCursorPos(cur_row,cur_col,page);	
    }
	
	static void AdjustCursorPosition(Bit8u& cur_col,Bit8u& cur_row) {
		BIOS_NCOLS;BIOS_NROWS;
		auto defattr = DefaultANSIAttr();
		//Need a new line?
		if(cur_col==ncols) 
		{
			cur_col=0;
			cur_row++;

            Real_INT10_TeletypeOutput('\r',defattr);
        }
		
		//Reached the bottom?
		if(cur_row==nrows) 
		{
            Real_INT10_TeletypeOutput('\n',defattr);	//Scroll up

            cur_row--;
		}
	}


	void Real_INT10_TeletypeOutputAttr(Bit8u chr,Bit8u attr,bool useattr) {
		//TODO Check if this page thing is correct
		Bit8u page=real_readb(BIOSMEM_SEG,BIOSMEM_CURRENT_PAGE);
//		BIOS_NCOLS;BIOS_NROWS;
		Bit8u cur_row=CURSOR_POS_ROW(page);
		Bit8u cur_col=CURSOR_POS_COL(page);
		switch (chr) 
		{
		case 7: {
			// set timer (this should not be needed as the timer already is programmed 
			// with those values, but the speaker stays silent without it)
			IO_Write(0x43,0xb6);
			IO_Write(0x42,1320&0xff);
			IO_Write(0x42,1320>>8);
			// enable speaker
			IO_Write(0x61,IO_Read(0x61)|0x3);
			for(Bitu i=0; i < 333; i++) CALLBACK_Idle();
			IO_Write(0x61,IO_Read(0x61)&~0x3);
			break;
		}
		case 8:
			if(cur_col>0)
				cur_col--;
			break;
		case '\r':
			cur_col=0;
			break;
		case '\n':
			cur_col=0;
			cur_row++;
			break;
		case '\t':
			do {
				Real_INT10_TeletypeOutputAttr(' ',attr,useattr);
				cur_row=CURSOR_POS_ROW(page);
				cur_col=CURSOR_POS_COL(page);
			} while(cur_col%8);
			break;
		default:
			//* Draw the actual Character
            Real_WriteChar(cur_col,cur_row,page,chr,attr,useattr);
            cur_col++;
		}
		
		AdjustCursorPosition(cur_col,cur_row);
		Real_INT10_SetCursorPos(cur_row,cur_col,page);	
	}//void Real_INT10_TeletypeOutputAttr(Bit8u chr,Bit8u attr,bool useattr) 
};

bool device_CON::Read(Bit8u * data,Bit16u * size) {
	Bit16u oldax=reg_ax;
	Bit16u count=0;
	auto defattr=DefaultANSIAttr();
	INT10_SetCurMode();
	if ((readcache) && (*size)) {
		data[count++]=readcache;
		if(dos.echo) Real_INT10_TeletypeOutput(readcache,defattr);
		readcache=0;
	}
	while (*size>count) {
        if (dev_con_pos < dev_con_max) {
            data[count++] = (Bit8u)dev_con_readbuf[dev_con_pos++];
            continue;
        }

		reg_ah=(IS_EGAVGA_ARCH)?0x10:0x0;

        /* FIXME: PC-98 emulation should eventually use CONIO emulation that
         *        better emulates the actual platform. The purpose of this
         *        hack is to allow our code to call into INT 16h without
         *        setting up an INT 16h vector */
        CALLBACK_RunRealInt(0x16);

        /* hack for DOSKEY emulation */
        last_int16_code = reg_ax;

		switch(reg_al) {
		case 13:
			data[count++]=0x0D;
			if (*size>count) data[count++]=0x0A;    // it's only expanded if there is room for it. (NO cache)
			*size=count;
			reg_ax=oldax;
			if(dos.echo) { 
				Real_INT10_TeletypeOutput(13,defattr); //maybe don't do this ( no need for it actually ) (but it's compatible)
				Real_INT10_TeletypeOutput(10,defattr);
			}
			return true;
			break;
		case 8:
			if(*size==1) data[count++]=reg_al;  //one char at the time so give back that BS
			else if(count) {                    //Remove data if it exists (extended keys don't go right)
				data[count--]=0;
				Real_INT10_TeletypeOutput(8,defattr);
				Real_INT10_TeletypeOutput(' ',defattr);
			} else {
				continue;                       //no data read yet so restart whileloop.
			}
			break;
		case 0xe0: /* Extended keys in the  int 16 0x10 case */
			if(!reg_ah) { /*extended key if reg_ah isn't 0 */
				data[count++] = reg_al;
			} else {
				data[count++] = 0;
				if (*size>count) data[count++] = reg_ah;
				else readcache = reg_ah;
			}
			break;
		case 0: /* Extended keys in the int 16 0x0 case */
            {
                /* IBM PC/XT/AT signals extended code by entering AL, AH.
                 * Arrow keys for example become 0x00 0x48, 0x00 0x50, etc. */
    			data[count++]=reg_al;
	    		if (*size>count) data[count++]=reg_ah;
		    	else readcache=reg_ah;
            }
			break;
		default:
			data[count++]=reg_al;
			break;
		}
		if(dos.echo) { //what to do if *size==1 and character is BS ?????
			// TODO: If CTRL+C checking is applicable do not echo (reg_al == 3)
			Real_INT10_TeletypeOutput(reg_al,defattr);
		}
	}
	*size=count;
	reg_ax=oldax;
	return true;
}

bool log_dev_con = false;
std::string log_dev_con_str;

bool device_CON::Write(const Bit8u * data,Bit16u * size) {
    Bit16u count=0;
    Bitu i;
    Bit8u col,row,page;

    INT10_SetCurMode();

    while (*size>count) {
        if (log_dev_con) {
            if (log_dev_con_str.size() >= 255 || data[count] == '\n' || data[count] == 27) {
                LOG_MSG("DOS CON: %s",log_dev_con_str.c_str());
                log_dev_con_str.clear();
            }

            if (data[count] != '\n' && data[count] != '\r')
                log_dev_con_str += (char)data[count];
        }

        if (!ansi.esc){
            if(data[count]=='\033') {
                /*clear the datastructure */
                ClearAnsi();
                /* start the sequence */
                ansi.esc=true;
                count++;
                continue;
            } else if(data[count] == '\t' && !dos.direct_output) {
                /* expand tab if not direct output */
                page = real_readb(BIOSMEM_SEG,BIOSMEM_CURRENT_PAGE);
                do {
                    Output(' ');
                    col=CURSOR_POS_COL(page);
                } while(col%8);
                count++;
                continue;
            } else { 
                Output(data[count]);
                count++;
                continue;
            }
        }

        if(!ansi.sci){

            switch(data[count]){
                case '[': 
                    ansi.sci=true;
                    break;
                case '*':/* PC-98: clear screen (same code path as CTRL+Z) */
                    LOG(LOG_IOCTL,LOG_NORMAL)("ANSI: unknown char %c after a esc",data[count]); /*prob () */
                    ClearAnsi();
                    break;
                case 'D':/* cursor DOWN (with scrolling) */
                    ESC_D();
                    break;
                case 'E':/* cursor DOWN, carriage return (with scrolling) */
                    ESC_E();
                    break;
                case 'M':/* cursor UP (with scrolling) */ 
                    ESC_M();
                    break;
                case '=':/* cursor position */
                    ansi.equcurp=true;
                    ansi.sci=true;
                    break;
                case '7': /* save cursor pos + attr TODO */
                case '8': /* restore this TODO */
                default:
                    LOG(LOG_IOCTL,LOG_NORMAL)("ANSI: unknown char %c after a esc",data[count]); /*prob () */
                    ClearAnsi();
                    break;
            }
            count++;
            continue;
        }
        /*ansi.esc and ansi.sci are true */
        if (!dos.internal_output) ansi.enabled=true;
        page = real_readb(BIOSMEM_SEG,BIOSMEM_CURRENT_PAGE);
        if (ansi.equcurp) { /* proprietary ESC = Y X command */
            ansi.data[ansi.numberofarg++] = data[count];
            if (ansi.numberofarg >= 2) {
                ESC_EQU_cursor_pos(); /* clears ANSI state */
            }
        }
        else if (isdigit(data[count])) {
            assert(ansi.numberofarg < NUMBER_ANSI_DATA);
            ansi.data[ansi.numberofarg]=10*ansi.data[ansi.numberofarg]+(data[count]-'0');
        }
        else if (data[count] == ';') {
            if ((ansi.numberofarg+1) < NUMBER_ANSI_DATA)
                ansi.numberofarg++;
        }
        else {
            switch(data[count]){
                case 'm':               /* SGR */
                    // NEC's ANSI driver always resets at the beginning
                    for(i=0;i<=ansi.numberofarg;i++){ 
                        const Bit8u COLORFLAGS[][8] = {
                        //  Black   Red Green Yellow Blue  Pink  Cyan  White
                            { 0x0,  0x4,  0x2,  0x6,  0x1,  0x5,  0x3,  0x7 }, /*   IBM */
                        };
                        const auto &flagset = COLORFLAGS[0];

                        switch(ansi.data[i]){
                            case 0: /* normal */
                                //Real ansi does this as well. (should do current defaults)
                                ansi.attr = DefaultANSIAttr();
                                break;
                            case 1: /* bold mode on*/
                                ansi.attr |= 0x08;
                                break;
                            case 2: /* PC-98 "Bit 4" */
                                ansi.attr |= 0;
                                break;
                            case 4: /* underline */
                                LOG(LOG_IOCTL, LOG_NORMAL)("ANSI:no support for underline yet");
                                break;
                            case 5: /* blinking */
                                ansi.attr |= 0x80;
                                break;
                            case 7: /* reverse */
                                ansi.attr = 0x70;
                                break;
                            case 8: /* PC-98 secret */
                            case 16:
                                ansi.attr &= 0xFF;
                                break;
                            case 30: /* fg color black */
                            case 31: /* fg color red */
                            case 32: /* fg color green */
                            case 33: /* fg color yellow */
                            case 34: /* fg color blue */
                            case 35: /* fg color magenta */
                            case 36: /* fg color cyan */
                            case 37: /* fg color white */
                                ansi.attr &= ~(flagset[7]);
                                ansi.attr |= (flagset[ansi.data[i] - 30]);
                                break;
                            case 40:
                            case 41:
                            case 42:
                            case 43:
                            case 44:
                            case 45:
                            case 46:
                            case 47: {
                                Bit8u shift = 4;
                                ansi.attr &= ~(flagset[7] << shift);
                                ansi.attr |= (flagset[ansi.data[i] - 40] << shift);
                                ansi.attr |= 0;
                                break;
                            }
                            default:
                                break;
                        }
                    }
                    ClearAnsi();
                    break;
                case 'f':
                case 'H':/* Cursor Pos*/
                    if(!ansi.warned) { //Inform the debugger that ansi is used.
                        ansi.warned = true;
                        LOG(LOG_IOCTL,LOG_WARN)("ANSI SEQUENCES USED");
                    }
                    ansi.ncols = real_readw(BIOSMEM_SEG,BIOSMEM_NB_COLS);
                    ansi.nrows = real_readb(BIOSMEM_SEG,BIOSMEM_NB_ROWS) + 1;
                    /* Turn them into positions that are on the screen */
                    if(ansi.data[0] == 0) ansi.data[0] = 1;
                    if(ansi.data[1] == 0) ansi.data[1] = 1;
                    if(ansi.data[0] > ansi.nrows) ansi.data[0] = (Bit8u)ansi.nrows;
                    if(ansi.data[1] > ansi.ncols) ansi.data[1] = (Bit8u)ansi.ncols;
                    Real_INT10_SetCursorPos(--(ansi.data[0]),--(ansi.data[1]),page); /*ansi=1 based, int10 is 0 based */
                    ClearAnsi();
                    break;
                    /* cursor up down and forward and backward only change the row or the col not both */
                case 'A': /* cursor up*/
                    ESC_BRACKET_A();
                    break;
                case 'B': /*cursor Down */
                    ESC_BRACKET_B();
                    break;
                case 'C': /*cursor forward */
                    ESC_BRACKET_C();
                    break;
                case 'D': /*Cursor Backward  */
                    ESC_BRACKET_D();
                    break;
                case 'J': /*erase screen and move cursor home*/
                    if(ansi.data[0]==0) ansi.data[0]=2;
                    if(ansi.data[0]!=2) {/* every version behaves like type 2 */
                        LOG(LOG_IOCTL,LOG_NORMAL)("ANSI: esc[%dJ called : not supported handling as 2",ansi.data[0]);
                    }
                    INT10_ScrollWindow(0,0,255,255,0,ansi.attr,page);
                    ClearAnsi();
                    Real_INT10_SetCursorPos(0,0,page);
                    break;
                case 'h': /* SET   MODE (if code =7 enable linewrap) */
                case 'I': /* RESET MODE */
                    LOG(LOG_IOCTL,LOG_NORMAL)("ANSI: set/reset mode called(not supported)");
                    ClearAnsi();
                    break;
                case 'u': /* Restore Cursor Pos */
                    Real_INT10_SetCursorPos(ansi.saverow,ansi.savecol,page);
                    ClearAnsi();
                    break;
                case 's': /* SAVE CURSOR POS */
                    ansi.savecol=CURSOR_POS_COL(page);
                    ansi.saverow=CURSOR_POS_ROW(page);
                    ClearAnsi();
                    break;
                case 'K': /* erase till end of line (don't touch cursor) */
                    col = CURSOR_POS_COL(page);
                    row = CURSOR_POS_ROW(page);
                    ansi.ncols = real_readw(BIOSMEM_SEG,BIOSMEM_NB_COLS);
					INT10_WriteChar(' ',ansi.attr,page,ansi.ncols-col,true); //Real_WriteChar(ansi.ncols-col,row,page,' ',ansi.attr,true);

                    //for(i = col;i<(Bitu) ansi.ncols; i++) INT10_TeletypeOutputAttr(' ',ansi.attr,true);
                    Real_INT10_SetCursorPos(row,col,page);
                    ClearAnsi();
                    break;
                case 'M': /* delete line (NANSI) */
                    row = CURSOR_POS_ROW(page);
                    ansi.ncols = real_readw(BIOSMEM_SEG,BIOSMEM_NB_COLS);
                    ansi.nrows = real_readb(BIOSMEM_SEG,BIOSMEM_NB_ROWS) + 1;
					INT10_ScrollWindow(row,0,ansi.nrows-1,ansi.ncols-1,ansi.data[0]? -ansi.data[0] : -1,ansi.attr,0xFF);
                    ClearAnsi();
                    break;
                case '>':/* proprietary NEC PC-98 MS-DOS codes (??) */
                    LOG(LOG_IOCTL,LOG_NORMAL)("ANSI: ESC [ > not supported outside PC-98 mode");
                    ClearAnsi();
                    break;
                case 'l':/* (if code =7) disable linewrap */
                case 'p':/* reassign keys (needs strings) */
                case 'i':/* printer stuff */
                default:
                    LOG(LOG_IOCTL,LOG_NORMAL)("ANSI: unhandled char %c in esc[",data[count]);
                    ClearAnsi();
                    break;
            }
        }
        count++;
    }
    *size=count;
    return true;
}

bool device_CON::Seek(Bit32u * pos,Bit32u type) {
    (void)pos; // UNUSED
    (void)type; // UNUSED
	// seek is valid
	*pos = 0;
	return true;
}

bool device_CON::Close() {
	return true;
}

extern bool dos_con_use_int16_to_detect_input;

Bit16u device_CON::GetInformation(void) {
	if (dos_con_use_int16_to_detect_input) {
		Bit16u ret = 0x80D3; /* No Key Available */

		/* DOSBox-X behavior: Use INT 16h AH=0x11 Query keyboard status/preview key.
		 * The reason we do this is some DOS programs actually rely on hooking INT 16h
		 * to manipulate, hide, or transform what the DOS CON driver sees as well as
		 * itself. Perhaps the most disgusting example of this behavior would be the
		 * SCANDISK.EXE utility in Microsoft MS-DOS 6.22, which apparently relies on
		 * hooking INT 16h in this way to catch the Escape, CTRL+C, and some other
		 * scan codes in order to "eat" the scan codes before they get back to DOS.
		 * The reason they can get away with it apparently and still respond properly
		 * to those keys, is because the MS-DOS 6.22 CON driver always calls INT 16h
		 * AH=0x11 first before calling INT 16h AH=0x10 to fetch the scan code.
		 *
		 * Without this fix, SCANDISK.EXE does not respond properly to Escape and
		 * a few other keys. Pressing Escape will do nothing until you hit any other
		 * key, at which point it suddenly acts upon the Escape key.
		 *
		 * Since Scandisk is using INT 21h AH=0x0B to query STDIN during this time,
		 * this implementation is a good "halfway" compromise in that this call
		 * will trigger the INT 16h AH=0x11 hook it relies on. */
		if (readcache || dev_con_pos < dev_con_max) return 0x8093; /* key available */

		Bit16u saved_ax = reg_ax;

		reg_ah = (IS_EGAVGA_ARCH)?0x11:0x1; // check for keystroke

        /* FIXME: PC-98 emulation should eventually use CONIO emulation that
         *        better emulates the actual platform. The purpose of this
         *        hack is to allow our code to call into INT 16h without
         *        setting up an INT 16h vector */
        CALLBACK_RunRealInt(0x16);

        if (!GETFLAG(ZF)) { /* key is present, waiting to be returned on AH=0x10 or AH=0x00 */
            ret = 0x8093; /* Key Available */
        }

		reg_ax = saved_ax;
		return ret;
	}
	else {
		/* DOSBox mainline behavior: alternate "fast" way through direct manipulation of keyboard scan buffer */
		Bit16u head=mem_readw(BIOS_KEYBOARD_BUFFER_HEAD);
		Bit16u tail=mem_readw(BIOS_KEYBOARD_BUFFER_TAIL);

		if ((head==tail) && !readcache) return 0x80D3;	/* No Key Available */
		if (readcache || real_readw(0x40,head)) return 0x8093;		/* Key Available */

		/* remove the zero from keyboard buffer */
		Bit16u start=mem_readw(BIOS_KEYBOARD_BUFFER_START);
		Bit16u end	=mem_readw(BIOS_KEYBOARD_BUFFER_END);
		head+=2;
		if (head>=end) head=start;
		mem_writew(BIOS_KEYBOARD_BUFFER_HEAD,head);
	}

	return 0x80D3; /* No Key Available */
}

device_CON::device_CON() {
	SetName("CON");
	readcache=0;
	ansi.enabled=false;
	ansi.attr=DefaultANSIAttr();
	ansi.saverow=0;
	ansi.savecol=0;
	ansi.warned=false;
	ClearAnsi();
}

void device_CON::ClearAnsi(void){
	for(Bit8u i=0; i<NUMBER_ANSI_DATA;i++) ansi.data[i]=0;
    ansi.pc98rab=false;
    ansi.equcurp=false;
	ansi.esc=false;
	ansi.sci=false;
	ansi.numberofarg=0;
}

void device_CON::Output(Bit8u chr) {
	if (dos.internal_output || ansi.enabled) {
		if (CurMode->type==M_TEXT) {
			Bit8u page=real_readb(BIOSMEM_SEG,BIOSMEM_CURRENT_PAGE);
			Bit8u col=CURSOR_POS_COL(page);
			Bit8u row=CURSOR_POS_ROW(page);
			BIOS_NCOLS;BIOS_NROWS;
			if (nrows==row+1 && (chr=='\n' || (ncols==col+1 && chr!='\r' && chr!=8 && chr!=7))) {
				INT10_ScrollWindow(0,0,(Bit8u)(nrows-1),(Bit8u)(ncols-1),-1,ansi.attr,page);
				INT10_SetCursorPos(row-1,col,page);
			}
		}
		Real_INT10_TeletypeOutputAttr(chr,ansi.attr,true);
	} else Real_INT10_TeletypeOutput(chr,DefaultANSIAttr());
 }
