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
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <string>
#include <vector>
#include "programs.h"
#include "support.h"
#include "drives.h"
#include "cross.h"
#include "regs.h"
#include "ide.h"
#include "cpu.h"
#include "callback.h"
#include "dos_system.h"
#include "dos_inc.h"
#include "bios.h"
#include "inout.h"
#include "setup.h"
#include "control.h"
#include <time.h>
#include "menu.h"
#include "render.h"
#include "mouse.h"
bool Mouse_Drv=true;
bool Mouse_Vertical = false;
bool force_nocachedir = false;

#if defined(OS2)
#define INCL DOSFILEMGR
#define INCL_DOSERRORS
#include "os2.h"
#endif

#if defined(WIN32)
#ifndef S_ISDIR
#define S_ISDIR(m) (((m)&S_IFMT)==S_IFDIR)
#endif
#endif

#if defined(RISCOS)
#include <unixlib/local.h>
#include <limits.h>
#endif

#if C_DEBUG
Bitu DEBUG_EnableDebugger(void);
#endif

void MSCDEX_SetCDInterface(int intNr, int forceCD);
Bit8u ZDRIVE_NUM = 25;

static const char* UnmountHelper(char umount) {
    int i_drive;
    if (umount < '0' || umount > 3+'0')
        i_drive = toupper(umount) - 'A';
    else
        i_drive = umount - '0';

    if (i_drive >= DOS_DRIVES || i_drive < 0)
        return MSG_Get("PROGRAM_MOUNT_UMOUNT_NOT_MOUNTED");

    if (Drives[i_drive]) {
        switch (DriveManager::UnmountDrive(i_drive)) {
            case 1: return MSG_Get("PROGRAM_MOUNT_UMOUNT_NO_VIRTUAL");
            case 2: return MSG_Get("MSCDEX_ERROR_MULTIPLE_CDROMS");
        }
        Drives[i_drive] = 0;
        mem_writeb(Real2Phys(dos.tables.mediaid)+(unsigned int)i_drive*dos.tables.dpb_size,0);
        if (i_drive == DOS_GetDefaultDrive()) {
            DOS_SetDrive(ZDRIVE_NUM);
        }

    }

    return MSG_Get("PROGRAM_MOUNT_UMOUNT_SUCCESS");
}

void MenuUnmountDrive(char drv) {
    UnmountHelper(drv);
}

class MOUNT : public Program {
public:
    void ListMounts(void) {
        char name[DOS_NAMELENGTH_ASCII];Bit32u size;Bit16u date;Bit16u time;Bit8u attr;
        /* Command uses dta so set it to our internal dta */
        RealPt save_dta = dos.dta();
        dos.dta(dos.tables.tempdta);
        DOS_DTA dta(dos.dta());

        WriteOut(MSG_Get("PROGRAM_MOUNT_STATUS_1"));
        WriteOut(MSG_Get("PROGRAM_MOUNT_STATUS_FORMAT"),"Drive","Type","Label");
        for(int p = 0;p < 8;p++) WriteOut("----------");

        for (int d = 0;d < DOS_DRIVES;d++) {
            if (!Drives[d]) continue;

            char root[7] = {(char)('A'+d),':','\\','*','.','*',0};
            bool ret = DOS_FindFirst(root,DOS_ATTR_VOLUME);
            if (ret) {
                dta.GetResult(name,size,date,time,attr);
                DOS_FindNext(); //Mark entry as invalid
            } else name[0] = 0;

            /* Change 8.3 to 11.0 */
            char* dot = strchr(name,'.');
            if(dot && (dot - name == 8) ) { 
                name[8] = name[9];name[9] = name[10];name[10] = name[11];name[11] = 0;
            }

            root[1] = 0; //This way, the format string can be reused.
            WriteOut(MSG_Get("PROGRAM_MOUNT_STATUS_FORMAT"),root, Drives[d]->GetInfo(),name);       
        }
        dos.dta(save_dta);
    }

    void Run(void) {
        DOS_Drive *newdrive = NULL;
        std::string label;
        std::string umount;
        std::string newz;
        bool quiet=false;
        char drive;

        //Hack To allow long commandlines
        ChangeToLongCmd();
        /* Parse the command line */
        /* if the command line is empty show current mounts */
        if (!cmd->GetCount()) {
            ListMounts();
            return;
        }

        /* In secure mode don't allow people to change mount points. 
         * Neither mount nor unmount */
        if(control->SecureMode()) {
            WriteOut(MSG_Get("PROGRAM_CONFIG_SECURE_DISALLOW"));
            return;
        }
        bool path_relative_to_last_config = false;
        if (cmd->FindExist("-pr",true)) path_relative_to_last_config = true;

        if (cmd->FindExist("-q",false))
            quiet = true;

        /* Check for unmounting */
        if (cmd->FindString("-u",umount,false)) {
            WriteOut(UnmountHelper(umount[0]), toupper(umount[0]));
            return;
        }
        
        /* Check for moving Z: */
        /* Only allowing moving it once. It is merely a convenience added for the wine team */
        if (ZDRIVE_NUM == 25 && cmd->FindString("-z", newz,false)) {
            newz[0] = toupper(newz[0]);
            int i_newz = (int)newz[0] - (int)'A';
            if (i_newz >= 0 && i_newz < DOS_DRIVES-1 && !Drives[i_newz]) {
                ZDRIVE_NUM = i_newz;
                /* remap drives */
                Drives[i_newz] = Drives[25];
                Drives[25] = 0;
                if (!first_shell) return; //Should not be possible
                /* Update environment */
                std::string line = "";
                char ppp[2] = {newz[0],0};
                std::string tempenv = ppp; tempenv += ":\\";
                if (first_shell->GetEnvStr("PATH",line)){
                    std::string::size_type idx = line.find('=');
                    std::string value = line.substr(idx +1 , std::string::npos);
                    while ( (idx = value.find("Z:\\")) != std::string::npos ||
                            (idx = value.find("z:\\")) != std::string::npos  )
                        value.replace(idx,3,tempenv);
                    line = value;
                }
                if (!line.size()) line = tempenv;
                first_shell->SetEnv("PATH",line.c_str());
                tempenv += "COMMAND.COM";
                first_shell->SetEnv("COMSPEC",tempenv.c_str());

                /* Update batch file if running from Z: (very likely: autoexec) */
                if(first_shell->bf) {
                    std::string &name = first_shell->bf->filename;
                    if(name.length() >2 &&  name[0] == 'Z' && name[1] == ':') name[0] = newz[0];
                }
                /* Change the active drive */
                if (DOS_GetDefaultDrive() == 25) DOS_SetDrive(i_newz);
            }
            return;
        }
        /* Show list of cdroms */
        if (cmd->FindExist("-cd",false)) {
#if !defined(C_SDL2)
            int num = SDL_CDNumDrives();
            WriteOut(MSG_Get("PROGRAM_MOUNT_CDROMS_FOUND"),num);
            for (int i=0; i<num; i++) {
                WriteOut("%2d. %s\n",i,SDL_CDName(i));
            }
#endif
            return;
        }

        bool nocachedir = false;

        if (force_nocachedir)
            nocachedir = true;

        if (cmd->FindExist("-nocachedir",true))
            nocachedir = true;

        bool readonly = false;
        if (cmd->FindExist("-ro",true))
            readonly = true;
        if (cmd->FindExist("-rw",true))
            readonly = false;

        std::string type="dir";
        cmd->FindString("-t",type,true);
        bool iscdrom = (type =="cdrom"); //Used for mscdex bug cdrom label name emulation
        if (type=="floppy" || type=="dir" || type=="cdrom") {
            Bit16u sizes[4];
            Bit8u mediaid;
            std::string str_size;
            if (type=="floppy") {
                str_size="512,1,2880,2880";/* All space free */
                mediaid=0xF0;       /* Floppy 1.44 media */
            } else if (type=="dir") {
                // 512*32*32765==~500MB total size
                // 512*32*16000==~250MB total free size
#if defined(__WIN32__) && !defined(C_SDL2) && !defined(HX_DOS)
                GetDefaultSize();
                str_size=hdd_size;
#else
                str_size="512,32,32765,16000";
#endif
                mediaid=0xF8;       /* Hard Disk */
            } else if (type=="cdrom") {
                str_size="2048,1,65535,0";
                mediaid=0xF8;       /* Hard Disk */
            } else {
                WriteOut(MSG_Get("PROGAM_MOUNT_ILL_TYPE"),type.c_str());
                return;
            }
            /* Parse the free space in mb's (kb's for floppies) */
            std::string mb_size;
            if(cmd->FindString("-freesize",mb_size,true)) {
                char teststr[1024];
                Bit16u freesize = static_cast<Bit16u>(atoi(mb_size.c_str()));
                if (type=="floppy") {
                    // freesize in kb
                    sprintf(teststr,"512,1,2880,%d",freesize*1024/(512*1));
                } else {
                    Bit32u total_size_cyl=32765;
                    Bit32u free_size_cyl=(Bit32u)freesize*1024*1024/(512*32);
                    if (free_size_cyl>65534) free_size_cyl=65534;
                    if (total_size_cyl<free_size_cyl) total_size_cyl=free_size_cyl+10;
                    if (total_size_cyl>65534) total_size_cyl=65534;
                    sprintf(teststr,"512,32,%u,%u",total_size_cyl,free_size_cyl);
                }
                str_size=teststr;
            }
           
            cmd->FindString("-size",str_size,true);
            char number[21] = { 0 }; const char* scan = str_size.c_str();
            Bitu index = 0; Bitu count = 0;
            /* Parse the str_size string */
            while (*scan && index < 20 && count < 4) {
                if (*scan==',') {
                    number[index] = 0;
                    sizes[count++] = atoi(number);
                    index = 0;
                } else number[index++] = *scan;
                scan++;
            }
            if (count < 4) {
                number[index] = 0; //always goes correct as index is max 20 at this point.
                sizes[count] = atoi(number);
            }
        
            // get the drive letter
            cmd->FindCommand(1,temp_line);
            if ((temp_line.size() > 2) || ((temp_line.size()>1) && (temp_line[1]!=':'))) goto showusage;
            int i_drive = toupper(temp_line[0]);
            if (!isalpha(i_drive)) goto showusage;
            if ((i_drive - 'A') >= DOS_DRIVES || (i_drive - 'A') < 0) goto showusage;
            drive = static_cast<char>(i_drive);

            if (!cmd->FindCommand(2,temp_line)) goto showusage;
            if (!temp_line.size()) goto showusage;
            if(path_relative_to_last_config && control->configfiles.size() && !Cross::IsPathAbsolute(temp_line)) {
		        std::string lastconfigdir(control->configfiles[control->configfiles.size()-1]);
                std::string::size_type pos = lastconfigdir.rfind(CROSS_FILESPLIT);
                if(pos == std::string::npos) pos = 0; //No directory then erase string
                lastconfigdir.erase(pos);
                if (lastconfigdir.length())	temp_line = lastconfigdir + CROSS_FILESPLIT + temp_line;
            }
            bool is_physfs = temp_line.find(':',((temp_line[0]|0x20) >= 'a' && (temp_line[0]|0x20) <= 'z')?2:0) != std::string::npos;
            struct stat test;
            //Win32 : strip tailing backslashes
            //os2: some special drive check
            //rest: substitute ~ for home
            bool failed = false;

            (void)failed;// MAY BE UNUSED

#if defined (RISCOS)
            // If the user provided a RISC OS style path, convert it to a Unix style path
            // TODO: Disable UnixLib's automatic path conversion and use RISC OS style paths internally?
            if (temp_line.find('$',0) != std::string::npos) {
                char fname[PATH_MAX];
                is_physfs = false;
                __unixify_std(temp_line.c_str(), fname, sizeof(fname), 0);
		temp_line = fname;
            }
#endif

#if defined (WIN32) || defined(OS2)
            /* nothing */
#else
            // Linux: Convert backslash to forward slash
            if (!is_physfs && temp_line.size() > 0) {
                for (size_t i=0;i < temp_line.size();i++) {
                    if (temp_line[i] == '\\')
                        temp_line[i] = '/';
                }
            }
#endif

#if defined (WIN32) || defined(OS2)
            /* Removing trailing backslash if not root dir so stat will succeed */
            if(temp_line.size() > 3 && temp_line[temp_line.size()-1]=='\\') temp_line.erase(temp_line.size()-1,1);
            if (!is_physfs && stat(temp_line.c_str(),&test)) {
#endif
#if defined(WIN32)
// Nothing to do here.
#elif defined (OS2)
                if (temp_line.size() <= 2) // Seems to be a drive.
                {
                    failed = true;
                    HFILE cdrom_fd = 0;
                    ULONG ulAction = 0;

                    APIRET rc = DosOpen((unsigned char*)temp_line.c_str(), &cdrom_fd, &ulAction, 0L, FILE_NORMAL, OPEN_ACTION_OPEN_IF_EXISTS,
                        OPEN_FLAGS_DASD | OPEN_SHARE_DENYNONE | OPEN_ACCESS_READONLY, 0L);
                    DosClose(cdrom_fd);
                    if (rc != NO_ERROR && rc != ERROR_NOT_READY)
                    {
                        failed = true;
                    } else {
                        failed = false;
                    }
                }
            }
            if (failed) {
#else
            if (!is_physfs && stat(temp_line.c_str(),&test)) {
                failed = true;
                Cross::ResolveHomedir(temp_line);
                //Try again after resolving ~
                if(!stat(temp_line.c_str(),&test)) failed = false;
            }
            if(failed) {
#endif
                WriteOut(MSG_Get("PROGRAM_MOUNT_ERROR_1"),temp_line.c_str());
                return;
            }
            /* Not a switch so a normal directory/file */
            if (!is_physfs && !S_ISDIR(test.st_mode)) {
#ifdef OS2
                HFILE cdrom_fd = 0;
                ULONG ulAction = 0;

                APIRET rc = DosOpen((unsigned char*)temp_line.c_str(), &cdrom_fd, &ulAction, 0L, FILE_NORMAL, OPEN_ACTION_OPEN_IF_EXISTS,
                    OPEN_FLAGS_DASD | OPEN_SHARE_DENYNONE | OPEN_ACCESS_READONLY, 0L);
                DosClose(cdrom_fd);
                if (rc != NO_ERROR && rc != ERROR_NOT_READY) {
                WriteOut(MSG_Get("PROGRAM_MOUNT_ERROR_2"),temp_line.c_str());
                return;
            }
#else
                WriteOut(MSG_Get("PROGRAM_MOUNT_ERROR_2"),temp_line.c_str());
                return;
#endif

            }

            if (temp_line[temp_line.size()-1]!=CROSS_FILESPLIT) temp_line+=CROSS_FILESPLIT;
            Bit8u bit8size=(Bit8u) sizes[1];
            if (type=="cdrom") {
                return;
            } else {
                /* Give a warning when mount c:\ or the / */
#if defined (WIN32) || defined(OS2)
                if( (temp_line == "c:\\") || (temp_line == "C:\\") || 
                    (temp_line == "c:/") || (temp_line == "C:/")    )   
                    WriteOut(MSG_Get("PROGRAM_MOUNT_WARNING_WIN"));
#else
                if(temp_line == "/") WriteOut(MSG_Get("PROGRAM_MOUNT_WARNING_OTHER"));
#endif
                if (is_physfs) {
                    LOG_MSG("ERROR:This build does not support physfs");
                } else {
                    newdrive=new localDrive(temp_line.c_str(),sizes[0],bit8size,sizes[2],sizes[3],mediaid);
                    newdrive->nocachedir = nocachedir;
                    newdrive->readonly = readonly;
                }
            }
        } else {
            WriteOut(MSG_Get("PROGRAM_MOUNT_ILL_TYPE"),type.c_str());
            return;
        }
        if (Drives[drive-'A']) {
            WriteOut(MSG_Get("PROGRAM_MOUNT_ALREADY_MOUNTED"),drive,Drives[drive-'A']->GetInfo());
            if (newdrive) delete newdrive;
            return;
        }
        if (!newdrive) E_Exit("DOS:Can't create drive");
        Drives[drive-'A']=newdrive;
        /* Set the correct media byte in the table */
        mem_writeb(Real2Phys(dos.tables.mediaid)+((unsigned int)drive-'A')*dos.tables.dpb_size,newdrive->GetMediaByte());
        if (!quiet) WriteOut(MSG_Get("PROGRAM_MOUNT_STATUS_2"),drive,newdrive->GetInfo());
        /* check if volume label is given and don't allow it to updated in the future */
        if (cmd->FindString("-label",label,true)) newdrive->SetLabel(label.c_str(),iscdrom,false);
        /* For hard drives set the label to DRIVELETTER_Drive.
         * For floppy drives set the label to DRIVELETTER_Floppy.
         * This way every drive except cdroms should get a label.*/
        else if(type == "dir") { 
#if defined (WIN32) || defined(OS2)
            if(temp_line.size()==3 && toupper(drive) == toupper(temp_line[0]))  {
                // automatic mount
            } else {
                label = drive; label += "_DRIVE";
                newdrive->SetLabel(label.c_str(),iscdrom,false);
            }
#endif
        } else if(type == "floppy") {
#if defined (WIN32) || defined(OS2)
            if(temp_line.size()==3 && toupper(drive) == toupper(temp_line[0]))  {
                // automatic mount
            } else {
                label = drive; label += "_FLOPPY";
                newdrive->SetLabel(label.c_str(),iscdrom,true);
            }
#endif
        }
        return;
showusage:
#if defined (WIN32) || defined(OS2)
       WriteOut(MSG_Get("PROGRAM_MOUNT_USAGE"),"d:\\dosprogs","d:\\dosprogs");
#else
       WriteOut(MSG_Get("PROGRAM_MOUNT_USAGE"),"~/dosprogs","~/dosprogs");         
#endif
        return;
    }
};

static void MOUNT_ProgramStart(Program * * make) {
    *make=new MOUNT;
}

extern bool custom_bios;
extern Bit32u floppytype;
extern bool dos_kernel_disabled;
extern bool boot_debug_break;
extern Bitu BIOS_bootfail_code_offset;

void EMS_DoShutDown();
void XMS_DoShutDown();
void DOS_DoShutDown();
void GUS_DOS_Shutdown();
void SBLASTER_DOS_Shutdown();

extern int swapInDisksSpecificDrive;

unsigned char PC98_ITF_ROM[0x8000];
bool PC98_ITF_ROM_init = false;
unsigned char PC98_BANK_Select = 0x12;

#include "mem.h"
#include "paging.h"

class PC98ITFPageHandler : public PageHandler {
public:
    PC98ITFPageHandler() : PageHandler(PFLAG_READABLE|PFLAG_HASROM) {}
    PC98ITFPageHandler(Bitu flags) : PageHandler(flags) {}
    HostPt GetHostReadPt(Bitu phys_page) {
        return PC98_ITF_ROM+(phys_page&0x7)*MEM_PAGESIZE;
    }
    HostPt GetHostWritePt(Bitu phys_page) {
        return PC98_ITF_ROM+(phys_page&0x7)*MEM_PAGESIZE;
    }
    void writeb(PhysPt addr,Bit8u val){
        LOG(LOG_CPU,LOG_ERROR)("Write %x to rom at %x",(int)val,(int)addr);
    }
    void writew(PhysPt addr,Bit16u val){
        LOG(LOG_CPU,LOG_ERROR)("Write %x to rom at %x",(int)val,(int)addr);
    }
    void writed(PhysPt addr,Bit32u val){
        LOG(LOG_CPU,LOG_ERROR)("Write %x to rom at %x",(int)val,(int)addr);
    }
};

PC98ITFPageHandler          mem_itf_rom;

void MEM_RegisterHandler(Bitu phys_page,PageHandler * handler,Bitu page_range);
void MEM_ResetPageHandler_Unmapped(Bitu phys_page, Bitu pages);
bool MEM_map_ROM_physmem(Bitu start,Bitu end);
PageHandler &Get_ROM_page_handler(void);

// LOADFIX

class LOADFIX : public Program {
public:
    void Run(void);
};

bool XMS_Active(void);
Bitu XMS_AllocateMemory(Bitu size, Bit16u& handle);

void LOADFIX::Run(void) 
{
    Bit16u commandNr    = 1;
    Bitu kb             = 64;
    bool xms            = false;

    if (cmd->FindExist("-xms",true)) {
        xms = true;
        kb = 1024;
    }

    if (cmd->FindExist("-?", false)) {
        WriteOut(MSG_Get("PROGRAM_LOADFIX_HELP"));
        return;
    }

    if (cmd->FindCommand(commandNr,temp_line)) {
        if (temp_line[0]=='-') {
            char ch = temp_line[1];
            if ((*upcase(&ch)=='D') || (*upcase(&ch)=='F')) {
                // Deallocate all
                if (xms) {
                    WriteOut("XMS deallocation not yet implemented\n");
                }
                else {
                    DOS_FreeProcessMemory(0x40);
                    WriteOut(MSG_Get("PROGRAM_LOADFIX_DEALLOCALL"),kb);
                }
                return;
            } else {
                // Set mem amount to allocate
                kb = (Bitu)atoi(temp_line.c_str()+1);
                if (kb==0) kb=xms?1024:64;
                commandNr++;
            }
        }
    }

    // Allocate Memory
    if (xms) {
        if (XMS_Active()) {
            Bit16u handle;
            Bitu err;

            err = XMS_AllocateMemory(kb,/*&*/handle);
            if (err == 0) {
                WriteOut("XMS block allocated (%uKB)\n",kb);
            }
            else {
                WriteOut("Unable to allocate XMS block\n");
            }
        }
        else {
            WriteOut("XMS not active\n");
        }
    }
    else {
        Bit16u segment;
        Bit16u blocks = (Bit16u)(kb*1024/16);
        if (DOS_AllocateMemory(&segment,&blocks)) {
            DOS_MCB mcb((Bit16u)(segment-1));
            mcb.SetPSPSeg(0x40);            // use fake segment
            WriteOut(MSG_Get("PROGRAM_LOADFIX_ALLOC"),kb);
            // Prepare commandline...
            if (cmd->FindCommand(commandNr++,temp_line)) {
                // get Filename
                char filename[128];
                safe_strncpy(filename,temp_line.c_str(),128);
                // Setup commandline
                bool ok;
                char args[256];
                args[0] = 0;
                do {
                    ok = cmd->FindCommand(commandNr++,temp_line);
                    if(sizeof(args)-strlen(args)-1 < temp_line.length()+1)
                        break;
                    strcat(args,temp_line.c_str());
                    strcat(args," ");
                } while (ok);           
                // Use shell to start program
                DOS_Shell shell;
                shell.Execute(filename,args);
                DOS_FreeMemory(segment);        
                WriteOut(MSG_Get("PROGRAM_LOADFIX_DEALLOC"),kb);
            }
        } else {
            WriteOut(MSG_Get("PROGRAM_LOADFIX_ERROR"),kb);  
        }
    }
}

static void LOADFIX_ProgramStart(Program * * make) {
    *make=new LOADFIX;
}

// RESCAN

class RESCAN : public Program {
public:
    void Run(void);
};

void RESCAN::Run(void) 
{
    bool all = false;
    
    Bit8u drive = DOS_GetDefaultDrive();
    
    if(cmd->FindCommand(1,temp_line)) {
        //-A -All /A /All 
        if(temp_line.size() >= 2 && (temp_line[0] == '-' ||temp_line[0] =='/')&& (temp_line[1] == 'a' || temp_line[1] =='A') ) all = true;
        else if(temp_line.size() == 2 && temp_line[1] == ':') {
            lowcase(temp_line);
            drive  = temp_line[0] - 'a';
        }
    }
    // Get current drive
    if (all) {
        for(Bitu i =0; i<DOS_DRIVES;i++) {
            if (Drives[i]) Drives[i]->EmptyCache();
        }
        WriteOut(MSG_Get("PROGRAM_RESCAN_SUCCESS"));
    } else {
        if (drive < DOS_DRIVES && Drives[drive]) {
            Drives[drive]->EmptyCache();
            WriteOut(MSG_Get("PROGRAM_RESCAN_SUCCESS"));
        }
    }
}

static void RESCAN_ProgramStart(Program * * make) {
    *make=new RESCAN;
}

/* TODO: This menu code sucks. Write a better one. */
class INTRO : public Program {
public:
    void DisplayMount(void) {
        /* Basic mounting has a version for each operating system.
         * This is done this way so both messages appear in the language file*/
        WriteOut(MSG_Get("PROGRAM_INTRO_MOUNT_START"));
#if (WIN32)
        WriteOut(MSG_Get("PROGRAM_INTRO_MOUNT_WINDOWS"));
#else           
        WriteOut(MSG_Get("PROGRAM_INTRO_MOUNT_OTHER"));
#endif
        WriteOut(MSG_Get("PROGRAM_INTRO_MOUNT_END"));
    }
    void DisplayUsage(void) {
        Bit8u c;Bit16u n=1;
        WriteOut(MSG_Get("PROGRAM_INTRO_USAGE_TOP"));
        WriteOut(MSG_Get("PROGRAM_INTRO_USAGE_1"));
        DOS_ReadFile (STDIN,&c,&n);
        WriteOut(MSG_Get("PROGRAM_INTRO_USAGE_TOP"));
        WriteOut(MSG_Get("PROGRAM_INTRO_USAGE_2"));
        DOS_ReadFile (STDIN,&c,&n);
        WriteOut(MSG_Get("PROGRAM_INTRO_USAGE_TOP"));
        WriteOut(MSG_Get("PROGRAM_INTRO_USAGE_3"));
        DOS_ReadFile (STDIN,&c,&n);
    }
    void DisplayIntro(void) {
        WriteOut(MSG_Get("PROGRAM_INTRO"));
        WriteOut(MSG_Get("PROGRAM_INTRO_MENU_UP"));
    }
    void DisplayMenuBefore(void) { WriteOut("\033[44m\033[K\033[33m\033[1m   \033[0m "); }
    void DisplayMenuCursorStart(void) {
        WriteOut("\033[44m\033[K\033[1m\033[33;44m  \x10\033[0m\033[5;37;44m ");
    }
    void DisplayMenuCursorEnd(void) { WriteOut("\033[0m\n"); }
    void DisplayMenuNone(void) { WriteOut("\033[44m\033[K\033[0m\n"); }

    bool CON_IN(Bit8u * data) {
        Bit8u c;
        Bit16u n=1;

        /* handle arrow keys vs normal input,
         * with special conditions for PC-98 and IBM PC */
        if (!DOS_ReadFile(STDIN,&c,&n) || n == 0) return false;

        {
            if (c == 0) {
                if (!DOS_ReadFile(STDIN,&c,&n) || n == 0) return false;
                *data = c | 0x80; /* extended code */
            }
            else {
                *data = c;
            }
        }

        return true;
    }

    void Run(void) {
        std::string menuname = "BASIC"; // default
        /* Only run if called from the first shell (Xcom TFTD runs any intro file in the path) */
        if(DOS_PSP(dos.psp()).GetParent() != DOS_PSP(DOS_PSP(dos.psp()).GetParent()).GetParent()) return;
        if(cmd->FindExist("cdrom",false)) {
            WriteOut(MSG_Get("PROGRAM_INTRO_CDROM"));
            return;
        }
        if(cmd->FindExist("mount",false)) {
            WriteOut("\033[2J");//Clear screen before printing
            DisplayMount();
            return;
        }

        if(cmd->FindExist("usage",false)) { DisplayUsage(); return; }
        Bit8u c;Bit16u n=1;

#define CURSOR(option) \
    if (menuname==option) DisplayMenuCursorStart(); \
    else DisplayMenuBefore(); \
    WriteOut(MSG_Get("PROGRAM_INTRO_MENU_" option "")); \
    if (menuname==option) DisplayMenuCursorEnd(); \
    else WriteOut("\n");

        /* Intro */

menufirst:
        DisplayIntro();
        CURSOR("BASIC")
        CURSOR("CDROM")
        CURSOR("USAGE")
        DisplayMenuNone(); // None
        CURSOR("INFO")
        CURSOR("QUIT")
        DisplayMenuNone(); // None

        if (menuname=="BASIC") goto basic;
        else if (menuname=="CDROM") goto cdrom;
        else if (menuname=="USAGE") goto usage;
        else if (menuname=="INFO") goto info;
        else if (menuname=="QUIT") goto quit;
        else if (menuname=="GOTO_EXIT") goto goto_exit;

goto_exit:
        WriteOut("\n"); // Give a line
        return;

basic:
        menuname="BASIC";
        WriteOut(MSG_Get("PROGRAM_INTRO_MENU_BASIC_HELP")); 
        CON_IN(&c);
        do switch (c) {
            case 0x48|0x80: menuname="QUIT"; goto menufirst; // Up
            case 0x50|0x80: menuname="CDROM"; goto menufirst; // Down
            case 0xD:   // Run
                WriteOut("\033[2J");
                WriteOut(MSG_Get("PROGRAM_INTRO"));
                WriteOut("\n");
                DisplayMount();
                DOS_ReadFile (STDIN,&c,&n);
                goto menufirst;
        } while (CON_IN(&c));

cdrom:
        menuname="CDROM";
        CON_IN(&c);
        do switch (c) {
            case 0x48|0x80: menuname="BASIC"; goto menufirst; // Up
            case 0x50|0x80: menuname="USAGE"; goto menufirst; // Down
            case 0xD:   // Run
                DOS_ReadFile (STDIN,&c,&n);
                goto menufirst;
        } while (CON_IN(&c));

usage:
        menuname="USAGE";
        WriteOut(MSG_Get("PROGRAM_INTRO_MENU_USAGE_HELP")); 
        CON_IN(&c);
        do switch (c) {
            case 0x48|0x80: menuname="CDROM"; goto menufirst; // Down
            case 0x50|0x80: menuname="INFO"; goto menufirst; // Down
            case 0xD:   // Run
                DisplayUsage();
                goto menufirst;
        } while (CON_IN(&c));

info:
        menuname="INFO";
        WriteOut(MSG_Get("PROGRAM_INTRO_MENU_INFO_HELP"));
        CON_IN(&c);
        do switch (c) {
            case 0x48|0x80: menuname="USAGE"; goto menufirst; // Up
            case 0x50|0x80: menuname="QUIT"; goto menufirst; // Down
            case 0xD:   // Run
                WriteOut("\033[2J");
                WriteOut(MSG_Get("PROGRAM_INTRO"));
                WriteOut("\n");
                WriteOut(MSG_Get("PROGRAM_INTRO_INFO"));
                DOS_ReadFile (STDIN,&c,&n);
                goto menufirst;
        } while (CON_IN(&c));

quit:
        menuname="QUIT";
        WriteOut(MSG_Get("PROGRAM_INTRO_MENU_QUIT_HELP")); 
        CON_IN(&c);
        do switch (c) {
            case 0x48|0x80: menuname="INFO"; goto menufirst; // Up
            case 0x50|0x80: menuname="BASIC"; goto menufirst; // Down
            case 0xD:   // Run
                menuname="GOTO_EXIT";
                goto menufirst;
        } while (CON_IN(&c));
    }   
};

bool ElTorito_ChecksumRecord(unsigned char *entry/*32 bytes*/) {
    unsigned int chk=0,i;

    for (i=0;i < 16;i++) {
        unsigned int word = ((unsigned int)entry[0]) + ((unsigned int)entry[1] << 8);
        chk += word;
        entry += 2;
    }
    chk &= 0xFFFF;
    return (chk == 0);
}

static void INTRO_ProgramStart(Program * * make) {
    *make=new INTRO;
}

// MODE

class MODE : public Program {
public:
    void Run(void);
};

void MODE::Run(void) {
    Bit16u rate=0,delay=0,mode;
    if (!cmd->FindCommand(1,temp_line) || temp_line=="/?") {
        WriteOut(MSG_Get("PROGRAM_MODE_USAGE"));
        return;
    }
    else if (strcasecmp(temp_line.c_str(),"con")==0 || strcasecmp(temp_line.c_str(),"con:")==0) {
        if (cmd->GetCount()!=3) goto modeparam;
        if (cmd->FindStringBegin("rate=", temp_line,false)) rate= atoi(temp_line.c_str());
        if (cmd->FindStringBegin("delay=",temp_line,false)) delay=atoi(temp_line.c_str());
        if (rate<1 || rate>32 || delay<1 || delay>4) goto modeparam;
        IO_Write(0x60,0xf3); IO_Write(0x60,(Bit8u)(((delay-1)<<5)|(32-rate)));
        return;
    }
    else if (cmd->GetCount()>1) goto modeparam;
    else if (strcasecmp(temp_line.c_str(),"mono")==0) mode=7;
    else if (strcasecmp(temp_line.c_str(),"co80")==0) mode=3;
    else if (strcasecmp(temp_line.c_str(),"bw80")==0) mode=2;
    else if (strcasecmp(temp_line.c_str(),"co40")==0) mode=1;
    else if (strcasecmp(temp_line.c_str(),"bw40")==0) mode=0;
    else goto modeparam;
    mem_writeb(BIOS_CONFIGURATION,(mem_readb(BIOS_CONFIGURATION)&0xcf)|((mode==7)?0x30:0x20));
    reg_ax=mode;
    CALLBACK_RunRealInt(0x10);
    return;
modeparam:
    WriteOut(MSG_Get("PROGRAM_MODE_INVALID_PARAMETERS"));
    return;
}

static void MODE_ProgramStart(Program * * make) {
    *make=new MODE;
}
/*
// MORE
class MORE : public Program {
public:
    void Run(void);
};

void MORE::Run(void) {
    if (cmd->GetCount()) {
        WriteOut(MSG_Get("PROGRAM_MORE_USAGE"));
        return;
    }
    Bit16u ncols=mem_readw(BIOS_SCREEN_COLUMNS);
    Bit16u nrows=(Bit16u)mem_readb(BIOS_ROWS_ON_SCREEN_MINUS_1);
    Bit16u col=1,row=1;
    Bit8u c;Bit16u n=1;
    WriteOut("\n");
    while (n) {
        DOS_ReadFile(STDIN,&c,&n);
        if (n==0 || c==0x1a) break; // stop at EOF
        switch (c) {
            case 0x07: break;
            case 0x08: if (col>1) col--; break;
            case 0x09: col=((col+7)&~7)+1; break;
            case 0x0a: row++; break;
            case 0x0d: col=1; break;
            default: col++; break;
        }
        if (col>ncols) {col=1;row++;}
        DOS_WriteFile(STDOUT,&c,&n);
        if (row>=nrows) {
            WriteOut(MSG_Get("PROGRAM_MORE_MORE"));
            DOS_ReadFile(STDERR,&c,&n);
            if (c==0) DOS_ReadFile(STDERR,&c,&n); // read extended key
            WriteOut("\n\n");
            col=row=1;
        }
    }
}

static void MORE_ProgramStart(Program * * make) {
    *make=new MORE;
}
*/

void REDOS_ProgramStart(Program * * make);
void A20GATE_ProgramStart(Program * * make);

class NMITEST : public Program {
public:
    void Run(void) {
        CPU_Raise_NMI();
    }
};

static void NMITEST_ProgramStart(Program * * make) {
    *make=new NMITEST;
}

class CAPMOUSE : public Program
{
public:
	void Run() override
    {
        auto val = 0;
        auto tmp = std::string("");

        if(cmd->GetCount() == 0 || cmd->FindExist("/?", true))
            val = 0;
        else if(cmd->FindExist("/C", false))
            val = 1;
        else if(cmd->FindExist("/R", false))
            val = 2;

        auto cap = false;
        switch(val)
        {
        case 2:
            break;
        case 1:
            cap = true;
            break;
        case 0:
        default:
            WriteOut("Mouse capture/release.\n\n");
            WriteOut("CAPMOUSE /[?|C|R]\n");
            WriteOut("  /? help\n");
            WriteOut("  /C capture mouse\n");
            WriteOut("  /R release mouse\n");
            return;
        }

        CaptureMouseNotify(!cap);
        GFX_CaptureMouse(cap);
        std::string msg;
        msg.append("Mouse ");
        msg.append(Mouse_IsLocked() ? "captured" : "released");
        msg.append("\n");
        WriteOut(msg.c_str());
    }
};

class LABEL : public Program
{
public:
    void Help() {
        WriteOut("LABEL [drive:][label]\n");
    }
	void Run() override
    {
        /* MS-DOS behavior: If no label provided at the command line, prompt for one.
         *
         * LABEL [drive:] [label]
         *
         * No options are supported in MS-DOS, and the label can have spaces in it.
         * This is valid, apparently:
         *
         * LABEL H E L L O
         *
         * Will set the volume label to "H E L L O"
         *
         * Label /? will print help.
         */
        std::string label;
    	Bit8u drive = DOS_GetDefaultDrive();
        const char *raw = cmd->GetRawCmdline().c_str();

        /* skip space */
        while (*raw == ' ') raw++;

        /* options */
        if (raw[0] == '/') {
            raw++;
            if (raw[0] == '?') {
                Help();
                return;
            }
        }

        /* is the next part a drive letter? */
        if (raw[0] != 0 && raw[1] != 0) {
            if (isalpha(raw[0]) && raw[1] == ':') {
                drive = tolower(raw[0]) - 'a';
                raw += 2;
                while (*raw == ' ') raw++;
            }
        }

        /* then the label. MS-DOS behavior is to treat the rest of the command line, spaces and all, as the label */
        if (*raw != 0) {
            label = raw;
        }

        /* if the label is longer than 11 chars or contains a dot, MS-DOS will reject it and then prompt for another label */
        if (label.length() > 11) {
            WriteOut("Label is too long (more than 11 chars)\n");
            label.clear();
        }
        else if (label.find_first_of(".:/\\") != std::string::npos) {
            WriteOut("Label has invalid chars.\n");
            label.clear();
        }

        /* if no label provided, MS-DOS will display the current label and serial number and prompt the user to type in a new label. */
        if (label.empty()) {
            std::string clabel = Drives[drive]->GetLabel();

            if (!clabel.empty())
                WriteOut("Volume in drive %c is %s\n",drive+'A',clabel.c_str());
            else
                WriteOut("Volume in drive %c has no label\n",drive+'A');
        }

        /* If no label is provided, MS-DOS will prompt the user whether to delete the label. */
        if (label.empty()) {
            Bit8u c,ans=0;
            Bit16u s;

            do {
                WriteOut("Delete the volume label (Y/N)? ");
                s = 1;
                DOS_ReadFile(STDIN,&c,&s);
                WriteOut("\n");
                if (s != 1) return;
                ans = Bit8u(tolower(char(c)));
            } while (!(ans == 'y' || ans == 'n'));

            if (ans != 'y') return;
        }

        /* delete then create the label */
		Drives[drive]->SetLabel("",false,true);
		Drives[drive]->SetLabel(label.c_str(),false,true);
    }
};

void LABEL_ProgramStart(Program** make)
{
	*make = new LABEL;
}

void DOS_SetupPrograms(void) {
    /*Add Messages */

    MSG_Add("PROGRAM_MOUNT_CDROMS_FOUND","CDROMs found: %d\n");
    MSG_Add("PROGRAM_MOUNT_STATUS_FORMAT","%-5s  %-58s %-12s\n");
    MSG_Add("PROGRAM_MOUNT_STATUS_ELTORITO", "Drive %c is mounted as el torito floppy\n");
    MSG_Add("PROGRAM_MOUNT_STATUS_RAMDRIVE", "Drive %c is mounted as ram drive\n");
    MSG_Add("PROGRAM_MOUNT_STATUS_2","Drive %c is mounted as %s\n");
    MSG_Add("PROGRAM_MOUNT_STATUS_1","The currently mounted drives are:\n");
    MSG_Add("PROGRAM_MOUNT_ERROR_1","Directory %s doesn't exist.\n");
    MSG_Add("PROGRAM_MOUNT_ERROR_2","%s isn't a directory\n");
    MSG_Add("PROGRAM_MOUNT_ILL_TYPE","Illegal type %s\n");
    MSG_Add("PROGRAM_MOUNT_ALREADY_MOUNTED","Drive %c already mounted with %s\n");
    MSG_Add("PROGRAM_MOUNT_USAGE",
        "Usage \033[34;1mMOUNT Drive-Letter Local-Directory\033[0m\n"
        "For example: MOUNT c %s\n"
        "This makes the directory %s act as the C: drive inside DOSBox.\n"
        "The directory has to exist.\n");
    MSG_Add("PROGRAM_MOUNT_UMOUNT_NOT_MOUNTED","Drive %c isn't mounted.\n");
    MSG_Add("PROGRAM_MOUNT_UMOUNT_SUCCESS","Drive %c has successfully been removed.\n");
    MSG_Add("PROGRAM_MOUNT_UMOUNT_NO_VIRTUAL","Virtual Drives can not be unMOUNTed.\n");
    MSG_Add("PROGRAM_MOUNT_WARNING_WIN","\033[31;1mMounting c:\\ is NOT recommended. Please mount a (sub)directory next time.\033[0m\n");
    MSG_Add("PROGRAM_MOUNT_WARNING_OTHER","\033[31;1mMounting / is NOT recommended. Please mount a (sub)directory next time.\033[0m\n");

    MSG_Add("PROGRAM_LOADFIX_ALLOC","%d kb allocated.\n");
    MSG_Add("PROGRAM_LOADFIX_DEALLOC","%d kb freed.\n");
    MSG_Add("PROGRAM_LOADFIX_DEALLOCALL","Used memory freed.\n");
    MSG_Add("PROGRAM_LOADFIX_ERROR","Memory allocation error.\n");
    MSG_Add("PROGRAM_LOADFIX_HELP",
        "Reduces the amount of available conventional or XMS memory\n\n"
        "LOADFIX [-xms] [-{ram}] [{program}]\n"
        "LOADFIX -f [-xms]\n\n"
        "  -xms        Allocates memory from XMS rather than conventional memory\n"
        "  -{ram}      Specifies the amount of memory to allocate in KB\n"
        "                 Defaults to 64kb for conventional memory; 1MB for XMS memory\n"
        "  -f          Frees previously allocated memory\n"
        "  {program}   Runs the specified program\n\n"
        "Examples:\n"
        "  LOADFIX game.exe     Allocates 64KB of conventional memory and runs game.exe\n"
        "  LOADFIX -128         Allocates 128KB of conventional memory\n"
        "  LOADFIX -xms         Allocates 1MB of XMS memory\n"
        "  LOADFIX -f           Frees allocated conventional memory\n");

    MSG_Add("MSCDEX_SUCCESS","MSCDEX installed.\n");
    MSG_Add("MSCDEX_ERROR_MULTIPLE_CDROMS","MSCDEX: Failure: Drive-letters of multiple CD-ROM drives have to be continuous.\n");
    MSG_Add("MSCDEX_ERROR_NOT_SUPPORTED","MSCDEX: Failure: Not yet supported.\n");
    MSG_Add("MSCDEX_ERROR_PATH","MSCDEX: Specified location is not a CD-ROM drive.\n");
    MSG_Add("MSCDEX_ERROR_OPEN","MSCDEX: Failure: Invalid file or unable to open.\n");
    MSG_Add("MSCDEX_TOO_MANY_DRIVES","MSCDEX: Failure: Too many CD-ROM drives (max: 5). MSCDEX Installation failed.\n");
    MSG_Add("MSCDEX_LIMITED_SUPPORT","MSCDEX: Mounted subdirectory: limited support.\n");
    MSG_Add("MSCDEX_INVALID_FILEFORMAT","MSCDEX: Failure: File is either no ISO/CUE image or contains errors.\n");
    MSG_Add("MSCDEX_UNKNOWN_ERROR","MSCDEX: Failure: Unknown error.\n");

    MSG_Add("PROGRAM_RESCAN_SUCCESS","Drive cache cleared.\n");

    MSG_Add("PROGRAM_INTRO",
        "\033[2J\033[32;1mWelcome to DOSBox\033[0m, an x86 emulator with sound and graphics.\n"
        "DOSBox creates a shell for you which looks like old plain DOS.\n"
        "\n"
        "\033[31;1mDOSBox will stop/exit without a warning if an error occurred!\033[0m\n"
        "\n"
        "\n" );
    {
        MSG_Add("PROGRAM_INTRO_MENU_UP",
            "\033[44m\033[K\033[0m\n"
            "\033[44m\033[K\033[1m\033[1m\t\t\t\t\t\t\t  DOSBox Introduction \033[0m\n"
            "\033[44m\033[K\033[1m\033[1m \xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\033[0m\n"
            "\033[44m\033[K\033[0m\n"
            );
    }
    MSG_Add("PROGRAM_INTRO_MENU_BASIC","Basic mount");
    MSG_Add("PROGRAM_INTRO_MENU_CDROM","CD-ROM support");
    MSG_Add("PROGRAM_INTRO_MENU_USAGE","Usage");
    MSG_Add("PROGRAM_INTRO_MENU_INFO","Information");
    MSG_Add("PROGRAM_INTRO_MENU_QUIT","Quit");
    MSG_Add("PROGRAM_INTRO_MENU_BASIC_HELP","\n\033[1m   \033[1m\033[KMOUNT allows you to connect real hardware to DOSBox's emulated PC.\033[0m\n");
    MSG_Add("PROGRAM_INTRO_MENU_USAGE_HELP","\n\033[1m   \033[1m\033[KAn overview of the command line options you can give to DOSBox.\033[0m\n");
    MSG_Add("PROGRAM_INTRO_MENU_INFO_HELP","\n\033[1m   \033[1m\033[KHow to get more information about DOSBox.\033[0m\n");
    MSG_Add("PROGRAM_INTRO_MENU_QUIT_HELP","\n\033[1m   \033[1m\033[KExit from Intro.\033[0m\n");
    MSG_Add("PROGRAM_INTRO_USAGE_TOP",
        "\033[2J\033[32;1mAn overview of the command line options you can give to DOSBox.\033[0m\n"
        "Windows Users must open cmd.exe or command.com or edit the shortcut to\n"
        "DOSBox.exe for this.\n\n"
        "dosbox [name] [-exit] [-c command] [-fullscreen] [-conf congfigfile]\n"
        "       [-lang languagefile] [-machine machinetype] [-noconsole]\n"
        "       [-startmapper] [-noautoexec] [-scaler scaler | -forcescaler scaler]\n       [-version]\n\n"
        );
    MSG_Add("PROGRAM_INTRO_USAGE_1",
        "\033[33;1m  name\033[0m\n"
        "\tIf name is a directory it will mount that as the C: drive.\n"
        "\tIf name is an executable it will mount the directory of name\n"
        "\tas the C: drive and execute name.\n\n"
        "\033[33;1m  -exit\033[0m\n"
        "\tDOSBox will close itself when the DOS application name ends.\n\n"
        "\033[33;1m  -c\033[0m command\n"
        "\tRuns the specified command before running name. Multiple commands\n"
        "\tcan be specified. Each command should start with -c, though.\n"
        "\tA command can be: an Internal Program, a DOS command or an executable\n"
        "\ton a mounted drive.\n"
        );
    MSG_Add("PROGRAM_INTRO_USAGE_2",
        "\033[33;1m  -fullscreen\033[0m\n"
        "\tStarts DOSBox in fullscreen mode.\n\n"
        "\033[33;1m  -conf\033[0m configfile\n"
        "\tStart DOSBox with the options specified in configfile.\n"
        "\tSee README for more details.\n\n"
        "\033[33;1m  -lang\033[0m languagefile\n"
        "\tStart DOSBox using the language specified in languagefile.\n\n"
        "\033[33;1m  -noconsole\033[0m (Windows Only)\n"
        "\tStart DOSBox without showing the console window. Output will\n"
        "\tbe redirected to stdout.txt and stderr.txt\n"
        );
    MSG_Add("PROGRAM_INTRO_USAGE_3",
        "\033[33;1m  -machine\033[0m machinetype\n"
        "\tSetup DOSBox to emulate a specific type of machine. Valid choices are:\n"
        "\thercules, cga, pcjr, tandy, vga (default). The machinetype affects\n"
        "\tboth the videocard and the available soundcards.\n\n"
        "\033[33;1m  -startmapper\033[0m\n"
        "\tEnter the keymapper directly on startup. Useful for people with\n"
        "\tkeyboard problems.\n\n"
        "\033[33;1m  -noautoexec\033[0m\n"
        "\tSkips the [autoexec] section of the loaded configuration file.\n\n"
        "\033[33;1m  -version\033[0m\n"
        "\toutput version information and exit. Useful for frontends.\n"
        );
    MSG_Add("PROGRAM_INTRO_INFO",
        "\033[32;1mInformation:\033[0m\n\n"
        "For information about basic mount, type \033[34;1mintro mount\033[0m\n"
        "For information about CD-ROM support, type \033[34;1mintro cdrom\033[0m\n"
        "For information about special keys, type \033[34;1mintro special\033[0m\n"
        "For information about usage, type \033[34;1mintro usage\033[0m\n\n"
        "For the latest version of DOSBox, go to \033[34;1mhttp://www.dosbox.com\033[0m\n"
        "\n"
        "For more information about DOSBox, read README first!\n"
        "\n"
        "\033[34;1mhttp://www.dosbox.com/wiki\033[0m\n"
        "\033[34;1mhttp://vogons.zetafleet.com\033[0m\n"
        );
    MSG_Add("PROGRAM_INTRO_MOUNT_START",
        "\033[32;1mHere are some commands to get you started:\033[0m\n"
        "Before you can use the files located on your own filesystem,\n"
        "you have to mount the directory containing the files.\n"
        "\n"
        );
    {
        MSG_Add("PROGRAM_INTRO_MOUNT_WINDOWS",
            "\033[44;1m\xC9\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD"
            "\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD"
            "\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xBB\n"
            "\xBA \033[32mmount c c:\\dosgames\\\033[37m will create a C drive with c:\\dosgames as contents.\xBA\n"
            "\xBA                                                                         \xBA\n"
            "\xBA \033[32mc:\\dosgames\\\033[37m is an example. Replace it with your own games directory.  \033[37m \xBA\n"
            "\xC8\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD"
            "\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD"
            "\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xBC\033[0m\n"
            );
        MSG_Add("PROGRAM_INTRO_MOUNT_OTHER",
            "\033[44;1m\xC9\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD"
            "\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD"
            "\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xBB\n"
            "\xBA \033[32mmount c ~/dosgames\033[37m will create a C drive with ~/dosgames as contents.\xBA\n"
            "\xBA                                                                      \xBA\n"
            "\xBA \033[32m~/dosgames\033[37m is an example. Replace it with your own games directory.\033[37m  \xBA\n"
            "\xC8\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD"
            "\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD"
            "\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xBC\033[0m\n"
            );
        MSG_Add("PROGRAM_INTRO_MOUNT_END",
            "When the mount has successfully completed you can type \033[34;1mc:\033[0m to go to your freshly\n"
            "mounted C-drive. Typing \033[34;1mdir\033[0m there will show its contents."
            " \033[34;1mcd\033[0m will allow you to\n"
            "enter a directory (recognised by the \033[33;1m[]\033[0m in a directory listing).\n"
            "You can run programs/files which end with \033[31m.exe .bat\033[0m and \033[31m.com\033[0m.\n"
            );
    }
    MSG_Add("PROGRAM_INTRO_CDROM",
        "\033[2J\033[32;1mHow to mount a Real/Virtual CD-ROM Drive in DOSBox:\033[0m\n"
        "DOSBox provides CD-ROM emulation on several levels.\n"
        "\n"
        "The \033[33mbasic\033[0m level works on all CD-ROM drives and normal directories.\n"
        "It installs MSCDEX and marks the files read-only.\n"
        "Usually this is enough for most games:\n"
        "\033[34;1mmount d \033[0;31mD:\\\033[34;1m -t cdrom\033[0m   or   \033[34;1mmount d C:\\example -t cdrom\033[0m\n"
        "If it doesn't work you might have to tell DOSBox the label of the CD-ROM:\n"
        "\033[34;1mmount d C:\\example -t cdrom -label CDLABEL\033[0m\n"
        "\n"
        "The \033[33mnext\033[0m level adds some low-level support.\n"
        "Therefore only works on CD-ROM drives:\n"
        "\033[34;1mmount d \033[0;31mD:\\\033[34;1m -t cdrom -usecd \033[33m0\033[0m\n"
        "\n"
        "The \033[33mlast\033[0m level of support depends on your Operating System:\n"
        "For \033[1mWindows 2000\033[0m, \033[1mWindows XP\033[0m and \033[1mLinux\033[0m:\n"
        "\033[34;1mmount d \033[0;31mD:\\\033[34;1m -t cdrom -usecd \033[33m0 \033[34m-ioctl\033[0m\n"
        "For \033[1mWindows 9x\033[0m with a ASPI layer installed:\n"
        "\033[34;1mmount d \033[0;31mD:\\\033[34;1m -t cdrom -usecd \033[33m0 \033[34m-aspi\033[0m\n"
        "\n"
        "Replace \033[0;31mD:\\\033[0m with the location of your CD-ROM.\n"
        "Replace the \033[33;1m0\033[0m in \033[34;1m-usecd \033[33m0\033[0m with the number reported for your CD-ROM if you type:\n"
        "\033[34;1mmount -cd\033[0m\n"
        );
    MSG_Add("PROGRAM_BOOT_NOT_EXIST","Bootdisk file does not exist.  Failing.\n");
    MSG_Add("PROGRAM_BOOT_NOT_OPEN","Cannot open bootdisk file.  Failing.\n");
    MSG_Add("PROGRAM_BOOT_WRITE_PROTECTED","Image file is read-only! Might create problems.\n");
    MSG_Add("PROGRAM_BOOT_UNABLE","Unable to boot off of drive %c");
    MSG_Add("PROGRAM_BOOT_IMAGE_OPEN","Opening image file: %s\n");
    MSG_Add("PROGRAM_BOOT_IMAGE_NOT_OPEN","Cannot open %s");
    MSG_Add("PROGRAM_BOOT_BOOT","Booting from drive %c...\n");
    MSG_Add("PROGRAM_BOOT_CART_WO_PCJR","PCjr cartridge found, but machine is not PCjr");
    MSG_Add("PROGRAM_BOOT_CART_LIST_CMDS","Available PCjr cartridge commandos:%s");
    MSG_Add("PROGRAM_BOOT_CART_NO_CMDS","No PCjr cartridge commandos found");

    MSG_Add("VHD_ERROR_OPENING", "Could not open the specified VHD file.\n");
    MSG_Add("VHD_INVALID_DATA", "The specified VHD file is corrupt and cannot be opened.\n");
    MSG_Add("VHD_UNSUPPORTED_TYPE", "The specified VHD file is of an unsupported type.\n");
    MSG_Add("VHD_ERROR_OPENING_PARENT", "The parent of the specified VHD file could not be found.\n");
    MSG_Add("VHD_PARENT_INVALID_DATA", "The parent of the specified VHD file is corrupt and cannot be opened.\n");
    MSG_Add("VHD_PARENT_UNSUPPORTED_TYPE", "The parent of the specified VHD file is of an unsupported type.\n");
    MSG_Add("VHD_PARENT_INVALID_MATCH", "The parent of the specified VHD file does not contain the expected identifier.\n");
    MSG_Add("VHD_PARENT_INVALID_DATE", "The parent of the specified VHD file has been changed and cannot be loaded.\n");

    MSG_Add("PROGRAM_MODE_USAGE",
            "\033[34;1mMODE\033[0m display-type       :display-type codes are "
            "\033[1mCO80\033[0m, \033[1mBW80\033[0m, \033[1mCO40\033[0m, \033[1mBW40\033[0m, or \033[1mMONO\033[0m\n"
            "\033[34;1mMODE CON RATE=\033[0mr \033[34;1mDELAY=\033[0md :typematic rates, r=1-32 (32=fastest), d=1-4 (1=lowest)\n");
    MSG_Add("PROGRAM_MODE_INVALID_PARAMETERS","Invalid parameter(s).\n");
    //MSG_Add("PROGRAM_MORE_USAGE","Usage: \033[34;1mMORE <\033[0m text-file\n");
    //MSG_Add("PROGRAM_MORE_MORE","-- More --");

    /*regular setup*/
    PROGRAMS_MakeFile("MOUNT.COM",MOUNT_ProgramStart);
    PROGRAMS_MakeFile("LOADFIX.COM",LOADFIX_ProgramStart);
    PROGRAMS_MakeFile("RESCAN.COM",RESCAN_ProgramStart);
    PROGRAMS_MakeFile("INTRO.COM",INTRO_ProgramStart);

        PROGRAMS_MakeFile("MODE.COM", MODE_ProgramStart);

    PROGRAMS_MakeFile("A20GATE.COM",A20GATE_ProgramStart);
    PROGRAMS_MakeFile("NMITEST.COM",NMITEST_ProgramStart);
    PROGRAMS_MakeFile("RE-DOS.COM",REDOS_ProgramStart);

    PROGRAMS_MakeFile("LABEL.COM", LABEL_ProgramStart);
}
