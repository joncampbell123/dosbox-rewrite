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


/* Character displaying moving functions */

#include "dosbox.h"
#include "bios.h"
#include "mem.h"
#include "inout.h"
#include "int10.h"
#include "shiftjis.h"
#include "callback.h"

Bit8u DefaultANSIAttr();

#if defined(_MSC_VER)
# pragma warning(disable:4244) /* const fmath::local::uint64_t to double possible loss of data */
#endif

static void TEXT_CopyRow(Bit8u cleft,Bit8u cright,Bit8u rold,Bit8u rnew,PhysPt base) {
    PhysPt src,dest;
    src=base+(rold*CurMode->twidth+cleft)*2u;
    dest=base+(rnew*CurMode->twidth+cleft)*2u;
    MEM_BlockCopy(dest,src,(Bitu)(cright-cleft)*2u);
}

static void TEXT_FillRow(Bit8u cleft,Bit8u cright,Bit8u row,PhysPt base,Bit8u attr) {
    /* Do some filing */
    PhysPt dest;
    dest=base+(row*CurMode->twidth+cleft)*2;
    Bit16u fill=(attr<<8)+' ';
    for (Bit8u x=0;x<(Bitu)(cright-cleft);x++) {
        mem_writew(dest,fill);
        dest+=2;
    }
}

void INT10_ScrollWindow(Bit8u rul,Bit8u cul,Bit8u rlr,Bit8u clr,Bit8s nlines,Bit8u attr,Bit8u page) {
    (void)page;
/* Do some range checking */
    if (CurMode->type!=M_TEXT) page=0xff;
    BIOS_NCOLS;BIOS_NROWS;
    if(rul>rlr) return;
    if(cul>clr) return;
    if(rlr>=nrows) rlr=(Bit8u)nrows-1;
    if(clr>=ncols) clr=(Bit8u)ncols-1;
    clr++;

    /* Get the correct page: current start address for current page (0xFF),
       otherwise calculate from page number and page size */
    PhysPt base=CurMode->pstart;
    
    /* See how much lines need to be copied */
    Bit8u start,end;Bits next;
    /* Copy some lines */
    if (nlines>0) {
        start=rlr-nlines+1;
        end=rul;
        next=-1;
    } else if (nlines<0) {
        start=rul-nlines-1;
        end=rlr;
        next=1;
    } else {
        nlines=rlr-rul+1;
        goto filling;
    }
    while (start!=end) {
        start+=next;
        switch (CurMode->type) {
        case M_TEXT:
            TEXT_CopyRow(cul,clr,start,start+nlines,base);break;
        default:
            LOG(LOG_INT10,LOG_ERROR)("Unhandled mode %d for scroll",CurMode->type);
        }   
    } 
    /* Fill some lines */
filling:
    if (nlines>0) {
        start=rul;
    } else {
        nlines=-nlines;
        start=rlr-nlines+1;
    }
    for (;nlines>0;nlines--) {
        switch (CurMode->type) {
        case M_TEXT:
            TEXT_FillRow(cul,clr,start,base,attr);break;
        default:
            LOG(LOG_INT10,LOG_ERROR)("Unhandled mode %d for scroll",CurMode->type);
        }   
        start++;
    } 
}

void INT10_GetScreenColumns(Bit16u *cols)
{
    *cols = real_readw(BIOSMEM_SEG, BIOSMEM_NB_COLS);
}

void INT10_GetCursorPos(Bit8u *row, Bit8u*col, const Bit8u page)
{
    {
        *col = real_readb(BIOSMEM_SEG, BIOSMEM_CURSOR_POS + page * 2u);
        *row = real_readb(BIOSMEM_SEG, BIOSMEM_CURSOR_POS + page * 2u + 1u);
    }
}

void INT10_SetCursorPos(Bit8u row,Bit8u col,Bit8u page) {
    if (page>7) LOG(LOG_INT10,LOG_ERROR)("INT10_SetCursorPos page %d",page);
    // Bios cursor pos
    {
        real_writeb(BIOSMEM_SEG,BIOSMEM_CURSOR_POS+page*2u,col);
        real_writeb(BIOSMEM_SEG,BIOSMEM_CURSOR_POS+page*2u+1u,row);
    }
    // Set the hardware cursor
    Bit8u current=real_readb(BIOSMEM_SEG,BIOSMEM_CURRENT_PAGE);
    if(page==current) {
        // Get the dimensions
        BIOS_NCOLS;
        // Calculate the address knowing nbcols nbrows and page num
        // NOTE: BIOSMEM_CURRENT_START counts in colour/flag pairs
        Bit16u address=(ncols*row)+col+(real_readw(BIOSMEM_SEG,BIOSMEM_CURRENT_START)/2);
        {
            // CRTC regs 0x0e and 0x0f
            Bit16u base=0x3D4;
            IO_Write(base,0x0eu);
            IO_Write(base+1u,(Bit8u)(address>>8u));
            IO_Write(base,0x0fu);
            IO_Write(base+1u,(Bit8u)address);
        }
    }
}

void ReadCharAttr(Bit16u col,Bit16u row,Bit8u page,Bit16u * result) {
    (void)page;
    /* Externally used by the mouse routine */
    Bit16u cols = real_readw(BIOSMEM_SEG,BIOSMEM_NB_COLS);
    switch (CurMode->type) {
    case M_TEXT:
        {   
            // Compute the address  
            Bit16u address=(row*cols+col)*2;
            // read the char 
            PhysPt where = CurMode->pstart+address;
            *result=mem_readw(where);
        }
        return;
    default:
        break;
    }
    *result = 0;
}
void INT10_ReadCharAttr(Bit16u * result,Bit8u page) {
    if(page==0xFF) page=real_readb(BIOSMEM_SEG,BIOSMEM_CURRENT_PAGE);
    Bit8u cur_row=CURSOR_POS_ROW(page);
    Bit8u cur_col=CURSOR_POS_COL(page);
    ReadCharAttr(cur_col,cur_row,page,result);
}

void WriteChar(Bit16u col,Bit16u row,Bit8u page,Bit16u chr,Bit8u attr,bool useattr) {
    (void)page;

    /* Externally used by the mouse routine */
    Bit16u cols = real_readw(BIOSMEM_SEG,BIOSMEM_NB_COLS);

    chr &= 0xFF;

    switch (CurMode->type) {
    case M_TEXT:
        {   
            // Compute the address  
            Bit16u address=(row*cols+col)*2;
            // Write the char 
            PhysPt where = CurMode->pstart+address;
            mem_writeb(where,chr);
            if (useattr) mem_writeb(where+1,attr);
        }
        return;
    default:
        break;
    }
}

void INT10_WriteChar(Bit16u chr,Bit8u attr,Bit8u page,Bit16u count,bool showattr) {
    Bit8u pospage=page;
    if (CurMode->type!=M_TEXT) {
        showattr=true; //Use attr in graphics mode always
        switch (machine) {
            case EGAVGA_ARCH_CASE:
                page%=CurMode->ptotal;
                pospage=page;
                break;
            default:
                break;
        }
    }

    Bit8u cur_row=CURSOR_POS_ROW(pospage);
    Bit8u cur_col=CURSOR_POS_COL(pospage);
    BIOS_NCOLS;
    while (count>0) {
        WriteChar(cur_col,cur_row,page,chr,attr,showattr);
        count--;
        cur_col++;
        if(cur_col==ncols) {
            cur_col=0;
            cur_row++;
        }
    }
}

static void INT10_TeletypeOutputAttr(Bit8u chr,Bit8u attr,bool useattr,Bit8u page) {
    BIOS_NCOLS;BIOS_NROWS;
    Bit8u cur_row=CURSOR_POS_ROW(page);
    Bit8u cur_col=CURSOR_POS_COL(page);
    switch (chr) {
    case 7: /* Beep */
        // Prepare PIT counter 2 for ~900 Hz square wave
        IO_Write(0x43, 0xb6);
        IO_Write(0x42, 0x28);
        IO_Write(0x42, 0x05);
        // Speaker on
        IO_Write(0x61, IO_Read(0x61) | 0x3);
        // Idle for 1/3rd of a second
        double start;
        start = PIC_FullIndex();
        while ((PIC_FullIndex() - start) < 333.0) CALLBACK_Idle();
        // Speaker off
        IO_Write(0x61, IO_Read(0x61) & ~0x3);
        // No change in position
        return;
    case 8:
        if(cur_col>0) cur_col--;
        break;
    case '\r':
        cur_col=0;
        break;
    case '\n':
//      cur_col=0; //Seems to break an old chess game
        cur_row++;
        break;
    default:
        /* Draw the actual Character */
        WriteChar(cur_col,cur_row,page,chr,attr,useattr);
        cur_col++;
    }
    if(cur_col==ncols) {
        cur_col=0;
        cur_row++;
    }
    // Do we need to scroll ?
    if(cur_row==nrows) {
        //Fill with black on non-text modes
        Bit8u fill = 0;
        if (CurMode->type==M_TEXT) {
            //Fill with attribute at cursor on textmode
            Bit16u chat;
            INT10_ReadCharAttr(&chat,page);
            fill=(Bit8u)(chat>>8);
        }
        INT10_ScrollWindow(0,0,(Bit8u)(nrows-1),(Bit8u)(ncols-1),-1,fill,page);
        cur_row--;
    }
    // Set the cursor for the page
    INT10_SetCursorPos(cur_row,cur_col,page);
}

void INT10_TeletypeOutputAttr(Bit8u chr,Bit8u attr,bool useattr) {
    INT10_TeletypeOutputAttr(chr,attr,useattr,real_readb(BIOSMEM_SEG,BIOSMEM_CURRENT_PAGE));
}

void INT10_TeletypeOutput(Bit8u chr,Bit8u attr) {
    INT10_TeletypeOutputAttr(chr,attr,CurMode->type!=M_TEXT);
}

void INT10_WriteString(Bit8u row,Bit8u col,Bit8u flag,Bit8u attr,PhysPt string,Bit16u count,Bit8u page) {
    Bit8u cur_row=CURSOR_POS_ROW(page);
    Bit8u cur_col=CURSOR_POS_COL(page);
    
    // if row=0xff special case : use current cursor position
    if (row==0xff) {
        row=cur_row;
        col=cur_col;
    }
    INT10_SetCursorPos(row,col,page);
    while (count>0) {
        Bit8u chr=mem_readb(string);
        string++;
        if (flag&2) {
            attr=mem_readb(string);
            string++;
        }
        INT10_TeletypeOutputAttr(chr,attr,true,page);
        count--;
    }
    if (!(flag&1)) {
        INT10_SetCursorPos(cur_row,cur_col,page);
    }
}

bool INT10_GetInsertState()
{
    {
        const auto flags = mem_readb(BIOS_KEYBOARD_FLAGS1);
        const auto state =static_cast<bool>(flags & BIOS_KEYBOARD_FLAGS1_INSERT_ACTIVE);
        return state;
    }
}
