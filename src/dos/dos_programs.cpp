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
#include "bios_disk.h"
#include "dos_system.h"
#include "dos_inc.h"
#include "bios.h"
#include "inout.h"
#include "bios_disk.h"
#include "qcow2_disk.h"
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

    if (i_drive < MAX_DISK_IMAGES && Drives[i_drive] == NULL && imageDiskList[i_drive] == NULL)
        return MSG_Get("PROGRAM_MOUNT_UMOUNT_NOT_MOUNTED");

    if (i_drive >= MAX_DISK_IMAGES && Drives[i_drive] == NULL)
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

    if (i_drive < MAX_DISK_IMAGES && imageDiskList[i_drive]) {
        delete imageDiskList[i_drive];
        imageDiskList[i_drive] = NULL;
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
        if(type == "floppy") incrementFDD();
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

class SHOWGUI : public Program {
public:
    void Run(void) {
    }
};

static void SHOWGUI_ProgramStart(Program * * make) {
    *make=new SHOWGUI;
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

// Normal BIOS is in the BIOS memory area
// ITF is in it's own buffer, served by mem_itf_rom
void PC98_BIOS_Bank_Switch(void) {
    if (PC98_BANK_Select == 0x00) {
        MEM_RegisterHandler(0xF8,&mem_itf_rom,0x8);
    }
    else {
        MEM_RegisterHandler(0xF8,&Get_ROM_page_handler(),0x8);
    }

    PAGING_ClearTLB();
}

/*! \brief          BOOT.COM utility to boot a floppy or hard disk device.
 *
 *  \description    Users will use this command to boot a guest operating system from
 *                  a disk image. Options are provided to specify the device to boot
 *                  from (if the image is already assigned) or a floppy disk image
 *                  specified on the command line.
 */
class BOOT : public Program {
public:
    BOOT() {
        for (size_t i=0;i < MAX_SWAPPABLE_DISKS;i++) newDiskSwap[i] = NULL;
    }
    virtual ~BOOT() {
        for (size_t i=0;i < MAX_SWAPPABLE_DISKS;i++) {
            if (newDiskSwap[i] != NULL) {
                newDiskSwap[i]->Release();
                newDiskSwap[i] = NULL;
            }
        }
    }
public:
    /*! \brief      Array of disk images to add to floppy swaplist
     */
    imageDisk* newDiskSwap[MAX_SWAPPABLE_DISKS] = {};

private:

    /*! \brief      Open a file as a disk image and return FILE* handle and size
     */
    FILE *getFSFile_mounted(char const* filename, Bit32u *ksize, Bit32u *bsize, Bit8u *error) {
        //if return NULL then put in error the errormessage code if an error was requested
        bool tryload = (*error)?true:false;
        *error = 0;
        Bit8u drive;
        char fullname[DOS_PATHLENGTH];

        localDrive* ldp=0;
        if (!DOS_MakeName(const_cast<char*>(filename),fullname,&drive)) return NULL;

        try {       
            ldp=dynamic_cast<localDrive*>(Drives[drive]);
            if(!ldp) return NULL;

            FILE *tmpfile = ldp->GetSystemFilePtr(fullname, "rb");
            if(tmpfile == NULL) {
                if (!tryload) *error=1;
                return NULL;
            }

            // get file size
            fseek(tmpfile,0L, SEEK_END);
            *ksize = Bit32u(ftell(tmpfile) / 1024);
            *bsize = Bit32u(ftell(tmpfile));
            fclose(tmpfile);

            tmpfile = ldp->GetSystemFilePtr(fullname, "rb+");
            if(tmpfile == NULL) {
//              if (!tryload) *error=2;
//              return NULL;
                WriteOut(MSG_Get("PROGRAM_BOOT_WRITE_PROTECTED"));
                tmpfile = ldp->GetSystemFilePtr(fullname, "rb");
                if(tmpfile == NULL) {
                    if (!tryload) *error=1;
                    return NULL;
                }
            }

            return tmpfile;
        }
        catch(...) {
            return NULL;
        }
    }

    /*! \brief      Open a file as a disk image and return FILE* handle and size
     */
    FILE *getFSFile(char const * filename, Bit32u *ksize, Bit32u *bsize,bool tryload=false) {
        Bit8u error = tryload?1:0;
        FILE* tmpfile = getFSFile_mounted(filename,ksize,bsize,&error);
        if(tmpfile) return tmpfile;
        //File not found on mounted filesystem. Try regular filesystem
        std::string filename_s(filename);
        Cross::ResolveHomedir(filename_s);
        tmpfile = fopen(filename_s.c_str(),"rb+");
        if(!tmpfile) {
            if( (tmpfile = fopen(filename_s.c_str(),"rb")) ) {
                //File exists; So can't be opened in correct mode => error 2
//              fclose(tmpfile);
//              if(tryload) error = 2;
                WriteOut(MSG_Get("PROGRAM_BOOT_WRITE_PROTECTED"));
                fseek(tmpfile,0L, SEEK_END);
                *ksize = Bit32u(ftell(tmpfile) / 1024);
                *bsize = Bit32u(ftell(tmpfile));
                return tmpfile;
            }
            // Give the delayed errormessages from the mounted variant (or from above)
            if(error == 1) WriteOut(MSG_Get("PROGRAM_BOOT_NOT_EXIST"));
            if(error == 2) WriteOut(MSG_Get("PROGRAM_BOOT_NOT_OPEN"));
            return NULL;
        }
        fseek(tmpfile,0L, SEEK_END);
        *ksize = Bit32u(ftell(tmpfile) / 1024);
        *bsize = Bit32u(ftell(tmpfile));
        return tmpfile;
    }

    /*! \brief      Utility function to print generic boot error
     */
    void printError(void) {
        WriteOut(MSG_Get("PROGRAM_BOOT_PRINT_ERROR"));
    }

public:
   
    /*! \brief      Program entry point, when the command is run
     */
    void Run(void) {
        std::string boothax_str;
        bool force = false;

        //Hack To allow long commandlines
        ChangeToLongCmd();

        boot_debug_break = false;
        if (cmd->FindExist("-debug",true))
            boot_debug_break = true;

        if (cmd->FindExist("-force",true))
            force = true;

        cmd->FindString("-boothax",boothax_str,true);

        if (boothax_str == "msdos") // WARNING: For MS-DOS only, or the real-mode portion of Windows 95/98/ME.
            boothax = BOOTHAX_MSDOS; // do NOT use while in the graphical portion of Windows 95/98/ME especially a DOS VM.
        else if (boothax_str == "")
            boothax = BOOTHAX_NONE;
        else {
            WriteOut("Unknown boothax mode");
            return;
        }

        /* In secure mode don't allow people to boot stuff. 
         * They might try to corrupt the data on it */
        if(control->SecureMode()) {
            WriteOut(MSG_Get("PROGRAM_CONFIG_SECURE_DISALLOW"));
            return;
        }

        bool bootbyDrive=false;
        Bitu i=0; 
        Bit32u floppysize=0;
        Bit8u drive = 'A';
        std::string cart_cmd="";
        Bitu max_seg;

        /* IBM PC:
         *    CS:IP = 0000:7C00     Load = 07C0:0000
         *    SS:SP = ???
         *
         * PC-98:
         *    CS:IP = 1FE0:0000     Load = 1FE0:0000
         *    SS:SP = 0030:00D8
         */
        Bitu stack_seg=0x7000;
        Bitu load_seg;

        if (MEM_TotalPages() > 0x9C)
            max_seg = 0x9C00;
        else
            max_seg = MEM_TotalPages() << (12 - 4);

        if ((stack_seg+0x20) > max_seg)
            stack_seg = max_seg - 0x20;

        if(!cmd->GetCount()) {
            printError();
            return;
        }
        while(i<cmd->GetCount()) {
            if(cmd->FindCommand((unsigned int)(i+1), temp_line)) {
                if((temp_line == "-l") || (temp_line == "-L")) {
                    /* Specifying drive... next argument then is the drive */
                    bootbyDrive = true;
                    i++;
                    if(cmd->FindCommand((unsigned int)(i+1), temp_line)) {
                        drive=toupper(temp_line[0]);
                        if ((drive != 'A') && (drive != 'C') && (drive != 'D')) {
                            printError();
                            return;
                        }

                    } else {
                        printError();
                        return;
                    }
                    i++;
                    continue;
                }

                if((temp_line == "-e") || (temp_line == "-E")) {
                    /* Command mode for PCJr cartridges */
                    i++;
                    if(cmd->FindCommand((unsigned int)(i + 1), temp_line)) {
                        for(size_t ct = 0;ct < temp_line.size();ct++) temp_line[ct] = toupper(temp_line[ct]);
                        cart_cmd = temp_line;
                    } else {
                        printError();
                        return;
                    }
                    i++;
                    continue;
                }

                if (i >= MAX_SWAPPABLE_DISKS) {
                    return; //TODO give a warning.
                }

                Bit32u rombytesize=0;
                WriteOut(MSG_Get("PROGRAM_BOOT_IMAGE_OPEN"), temp_line.c_str());
                FILE *usefile = getFSFile(temp_line.c_str(), &floppysize, &rombytesize);
                if(usefile != NULL) {
                    char tmp[256];

                    if (newDiskSwap[i] != NULL) {
                        newDiskSwap[i]->Release();
                        newDiskSwap[i] = NULL;
                    }

                    fseeko64(usefile, 0L, SEEK_SET);
                    size_t readResult = fread(tmp,256,1,usefile); // look for magic signatures
                    if (readResult != 1) {
                        LOG(LOG_IO, LOG_ERROR) ("Reading error in Run\n");
                        return;
                    }

                    const char *ext = strrchr(temp_line.c_str(),'.');

                    if (ext != NULL && !strcasecmp(ext, ".d88")) {
                        newDiskSwap[i] = new imageDiskD88(usefile, (Bit8u *)temp_line.c_str(), floppysize, false);
                    }
                    else if (!memcmp(tmp,"VFD1.",5)) { /* FDD files */
                        newDiskSwap[i] = new imageDiskVFD(usefile, (Bit8u *)temp_line.c_str(), floppysize, false);
                    }
                    else if (!memcmp(tmp,"T98FDDIMAGE.R0\0\0",16)) {
                        newDiskSwap[i] = new imageDiskNFD(usefile, (Bit8u *)temp_line.c_str(), floppysize, false, 0);
                    }
                    else if (!memcmp(tmp,"T98FDDIMAGE.R1\0\0",16)) {
                        newDiskSwap[i] = new imageDiskNFD(usefile, (Bit8u *)temp_line.c_str(), floppysize, false, 1);
                    }
                    else {
                        newDiskSwap[i] = new imageDisk(usefile, (Bit8u *)temp_line.c_str(), floppysize, false);
                    }
                    newDiskSwap[i]->Addref();
                    if (newDiskSwap[i]->active && !newDiskSwap[i]->hardDrive) incrementFDD(); //moved from imageDisk constructor
                } else {
                    WriteOut(MSG_Get("PROGRAM_BOOT_IMAGE_NOT_OPEN"), temp_line.c_str());
                    return;
                }

            }
            i++;
        }

        if (!bootbyDrive) {
            if (i == 0) {
                WriteOut("No images specified");
                return;
            }

            /* attach directly without using the swap list */
            if (imageDiskList[drive-65] != NULL) {
                imageDiskChange[drive-65] = true;
                imageDiskList[drive-65]->Release();
                imageDiskList[drive-65] = NULL;
            }

            imageDiskList[drive-65] = newDiskSwap[0];
            newDiskSwap[0] = NULL;
        }

        if(imageDiskList[drive-65]==NULL) {
            WriteOut(MSG_Get("PROGRAM_BOOT_UNABLE"), drive);
            return;
        }

        // .D88 images come from PC-88 which usually means the boot sector is full
        // of Z80 executable code, therefore it's very unlikely the boot sector will
        // work with our x86 emulation!
        //
        // If the user is REALLY REALLY SURE they want to try executing Z80 bootsector
        // code as x86, they're free to use --force.
        //
        // However PC-98 games are also distributed as .D88 images and therefore
        // we probably CAN boot the image.
        //
        // It depends on the fd_type field of the image.
        if (!force && imageDiskList[drive-65]->class_id == imageDisk::ID_D88) {
            if (reinterpret_cast<imageDiskD88*>(imageDiskList[drive-65])->fd_type_major == imageDiskD88::DISKTYPE_2D) {
                WriteOut("The D88 image appears to target PC-88 and cannot be booted.");
                return;
            }
        }


        bootSector bootarea;

        if (imageDiskList[drive-65]->getSectSize() > sizeof(bootarea)) {
            WriteOut("Bytes/sector too large");
            return;
        }

        /* clear the disk change flag.
         * Most OSes don't expect the disk change error signal when they first boot up */
        imageDiskChange[drive-65] = false;

        bool has_read = false;
        unsigned int bootsize = imageDiskList[drive-65]->getSectSize();

        /* NTS: Load address is 128KB - sector size */
        load_seg=0x07C0;

        if (!has_read) {
            if (imageDiskList[drive - 65]->Read_Sector(0, 0, 1, (Bit8u *)&bootarea) != 0) {
                WriteOut("Error reading drive");
                return;
            }
        }
        
        {
            extern const char* RunningProgram;

            if (max_seg < 0x0800) {
                /* TODO: For the adventerous, add a configuration option or command line switch to "BOOT"
                 *       that allows us to boot the guest OS anyway in a manner that is non-standard. */
                WriteOut("32KB of RAM is required to boot a guest OS\n");
                return;
            }

            /* Other versions of MS-DOS/PC-DOS have their own requirements about memory:
             *    - IBM PC-DOS 1.0/1.1: not too picky, will boot with as little as 32KB even though
             *                          it was intended for the average model with 64KB of RAM.
             *
             *    - IBM PC-DOS 2.1: requires at least 44KB of RAM. will crash on boot otherwise.
             *
             *    - MS-DOS 3.2: requires at least 64KB to boot without crashing, 80KB to make it
             *                  to the command line without halting at "configuration too big for
             *                  memory"*/

            /* TODO: Need a configuration option or a BOOT command option where the user can
             *       dictate where we put the stack: if we put it at 0x7000 or top of memory
             *       (default) or just below the boot sector, or... */

            if((bootarea.rawdata[0]==0) && (bootarea.rawdata[1]==0)) {
                WriteOut_NoParsing("PROGRAM_BOOT_UNABLE");
                return;
            }

            WriteOut(MSG_Get("PROGRAM_BOOT_BOOT"), drive);

            {
                for(i=0;i<bootsize;i++) real_writeb(0, (Bit16u)((load_seg<<4) + i), bootarea.rawdata[i]);
            }

            /* debug */
            LOG_MSG("Booting guest OS stack_seg=0x%04x load_seg=0x%04x\n",(int)stack_seg,(int)load_seg);
            RunningProgram = "Guest OS";

            if (drive == 'A' || drive == 'B') {
                incrementFDD();
            }

            /* standard method */
            {
                SegSet16(cs, 0);
                SegSet16(ds, 0);
                SegSet16(es, 0);
                reg_ip = (Bit16u)(load_seg<<4);
                reg_ebx = (Bit32u)(load_seg<<4); //Real code probably uses bx to load the image
                reg_esp = 0x100;
                /* set up stack at a safe place */
                SegSet16(ss, (Bit16u)stack_seg);
                reg_esi = 0;
                reg_ecx = 1;
                reg_ebp = 0;
                reg_eax = 0;
                reg_edx = 0; //Head 0
                if (drive >= 'A' && drive <= 'B')
                    reg_edx += (unsigned int)(drive-'A');
                else if (drive >= 'C' && drive <= 'Z')
                    reg_edx += 0x80u+(unsigned int)(drive-'C');
            }
#ifdef __WIN32__
            // let menu know it boots
            menu.boot=true;
#endif

            /* forcibly exit the shell, the DOS kernel, and anything else by throwing an exception */
            throw int(2);
        }
    }
};

static void BOOT_ProgramStart(Program * * make) {
    *make=new BOOT;
}

class LOADROM : public Program {
public:
    void Run(void) {
        if (!(cmd->FindCommand(1, temp_line))) {
            WriteOut(MSG_Get("PROGRAM_LOADROM_SPECIFY_FILE"));
            return;
        }

        Bit8u drive;
        char fullname[DOS_PATHLENGTH];
        localDrive* ldp=0;
        if (!DOS_MakeName((char *)temp_line.c_str(),fullname,&drive)) return;

        try {
            /* try to read ROM file into buffer */
            ldp=dynamic_cast<localDrive*>(Drives[drive]);
            if(!ldp) return;

            FILE *tmpfile = ldp->GetSystemFilePtr(fullname, "rb");
            if(tmpfile == NULL) {
                WriteOut(MSG_Get("PROGRAM_LOADROM_CANT_OPEN"));
                return;
            }
            fseek(tmpfile, 0L, SEEK_END);
            if (ftell(tmpfile)>0x8000) {
                WriteOut(MSG_Get("PROGRAM_LOADROM_TOO_LARGE"));
                fclose(tmpfile);
                return;
            }
            fseek(tmpfile, 0L, SEEK_SET);
            Bit8u rom_buffer[0x8000];
            Bitu data_read = fread(rom_buffer, 1, 0x8000, tmpfile);
            fclose(tmpfile);

            /* try to identify ROM type */
            PhysPt rom_base = 0;
            if (data_read >= 0x4000 && rom_buffer[0] == 0x55 && rom_buffer[1] == 0xaa &&
                (rom_buffer[3] & 0xfc) == 0xe8 && strncmp((char*)(&rom_buffer[0x1e]), "IBM", 3) == 0) {

                if (!IS_EGAVGA_ARCH) {
                    WriteOut(MSG_Get("PROGRAM_LOADROM_INCOMPATIBLE"));
                    return;
                }
                rom_base = PhysMake(0xc000, 0); // video BIOS
            }
            else if (data_read == 0x8000 && rom_buffer[0] == 0xe9 && rom_buffer[1] == 0x8f &&
                rom_buffer[2] == 0x7e && strncmp((char*)(&rom_buffer[0x4cd4]), "IBM", 3) == 0) {

                rom_base = PhysMake(0xf600, 0); // BASIC
            }

            if (rom_base) {
                /* write buffer into ROM */
                for (Bitu i=0; i<data_read; i++) phys_writeb((PhysPt)(rom_base + i), rom_buffer[i]);

                if (rom_base == 0xc0000) {
                    /* initialize video BIOS */
                    phys_writeb(PhysMake(0xf000, 0xf065), 0xcf);
                    reg_flags &= ~FLAG_IF;
                    CALLBACK_RunRealFar(0xc000, 0x0003);
                    LOG_MSG("Video BIOS ROM loaded and initialized.");
                }
                else WriteOut(MSG_Get("PROGRAM_LOADROM_BASIC_LOADED"));
            }
            else WriteOut(MSG_Get("PROGRAM_LOADROM_UNRECOGNIZED"));
        }
        catch(...) {
            return;
        }
    }
};

static void LOADROM_ProgramStart(Program * * make) {
    *make=new LOADROM;
}

const Bit8u freedos_mbr[] = {
    0x33,0xC0,0x8E,0xC0,0x8E,0xD8,0x8E,0xD0,0xBC,0x00,0x7C,0xFC,0x8B,0xF4,0xBF,0x00, 
    0x06,0xB9,0x00,0x01,0xF2,0xA5,0xEA,0x67,0x06,0x00,0x00,0x8B,0xD5,0x58,0xA2,0x4F, // 10h
    0x07,0x3C,0x35,0x74,0x23,0xB4,0x10,0xF6,0xE4,0x05,0xAE,0x04,0x8B,0xF0,0x80,0x7C, // 20h
    0x04,0x00,0x74,0x44,0x80,0x7C,0x04,0x05,0x74,0x3E,0xC6,0x04,0x80,0xE8,0xDA,0x00, 
    0x8A,0x74,0x01,0x8B,0x4C,0x02,0xEB,0x08,0xE8,0xCF,0x00,0xB9,0x01,0x00,0x32,0xD1, // 40h
    0xBB,0x00,0x7C,0xB8,0x01,0x02,0xCD,0x13,0x72,0x1E,0x81,0xBF,0xFE,0x01,0x55,0xAA, 
    0x75,0x16,0xEA,0x00,0x7C,0x00,0x00,0x80,0xFA,0x81,0x74,0x02,0xB2,0x80,0x8B,0xEA, 
    0x42,0x80,0xF2,0xB3,0x88,0x16,0x41,0x07,0xBF,0xBE,0x07,0xB9,0x04,0x00,0xC6,0x06, 
    0x34,0x07,0x31,0x32,0xF6,0x88,0x2D,0x8A,0x45,0x04,0x3C,0x00,0x74,0x23,0x3C,0x05, // 80h
    0x74,0x1F,0xFE,0xC6,0xBE,0x31,0x07,0xE8,0x71,0x00,0xBE,0x4F,0x07,0x46,0x46,0x8B, 
    0x1C,0x0A,0xFF,0x74,0x05,0x32,0x7D,0x04,0x75,0xF3,0x8D,0xB7,0x7B,0x07,0xE8,0x5A, 
    0x00,0x83,0xC7,0x10,0xFE,0x06,0x34,0x07,0xE2,0xCB,0x80,0x3E,0x75,0x04,0x02,0x74, 
    0x0B,0xBE,0x42,0x07,0x0A,0xF6,0x75,0x0A,0xCD,0x18,0xEB,0xAC,0xBE,0x31,0x07,0xE8, 
    0x39,0x00,0xE8,0x36,0x00,0x32,0xE4,0xCD,0x1A,0x8B,0xDA,0x83,0xC3,0x60,0xB4,0x01, 
    0xCD,0x16,0xB4,0x00,0x75,0x0B,0xCD,0x1A,0x3B,0xD3,0x72,0xF2,0xA0,0x4F,0x07,0xEB, 
    0x0A,0xCD,0x16,0x8A,0xC4,0x3C,0x1C,0x74,0xF3,0x04,0xF6,0x3C,0x31,0x72,0xD6,0x3C, 
    0x35,0x77,0xD2,0x50,0xBE,0x2F,0x07,0xBB,0x1B,0x06,0x53,0xFC,0xAC,0x50,0x24,0x7F, //100h
    0xB4,0x0E,0xCD,0x10,0x58,0xA8,0x80,0x74,0xF2,0xC3,0x56,0xB8,0x01,0x03,0xBB,0x00, //110h
    0x06,0xB9,0x01,0x00,0x32,0xF6,0xCD,0x13,0x5E,0xC6,0x06,0x4F,0x07,0x3F,0xC3,0x0D, //120h
    0x8A,0x0D,0x0A,0x46,0x35,0x20,0x2E,0x20,0x2E,0x20,0x2E,0xA0,0x64,0x69,0x73,0x6B, 
    0x20,0x32,0x0D,0x0A,0x0A,0x44,0x65,0x66,0x61,0x75,0x6C,0x74,0x3A,0x20,0x46,0x31, //140h
    0xA0,0x00,0x01,0x00,0x04,0x00,0x06,0x03,0x07,0x07,0x0A,0x0A,0x63,0x0E,0x64,0x0E, 
    0x65,0x14,0x80,0x19,0x81,0x19,0x82,0x19,0x83,0x1E,0x93,0x24,0xA5,0x2B,0x9F,0x2F, 
    0x75,0x33,0x52,0x33,0xDB,0x36,0x40,0x3B,0xF2,0x41,0x00,0x44,0x6F,0xF3,0x48,0x70, 
    0x66,0xF3,0x4F,0x73,0xB2,0x55,0x6E,0x69,0xF8,0x4E,0x6F,0x76,0x65,0x6C,0xEC,0x4D, //180h
    0x69,0x6E,0x69,0xF8,0x4C,0x69,0x6E,0x75,0xF8,0x41,0x6D,0x6F,0x65,0x62,0xE1,0x46, 
    0x72,0x65,0x65,0x42,0x53,0xC4,0x42,0x53,0x44,0xE9,0x50,0x63,0x69,0xF8,0x43,0x70, 
    0xED,0x56,0x65,0x6E,0x69,0xF8,0x44,0x6F,0x73,0x73,0x65,0xE3,0x3F,0xBF,0x00,0x00, //1B0h
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x55,0xAA
    };
#ifdef WIN32
#include <winioctl.h>
#endif

class IMGMAKE : public Program {
public:
#ifdef WIN32
    bool OpenDisk(HANDLE* f, OVERLAPPED* o, Bit8u* name) {
        o->hEvent = INVALID_HANDLE_VALUE;
        *f = CreateFile( (LPCSTR)name, GENERIC_READ | GENERIC_WRITE,
            0,    // exclusive access 
            NULL, // default security attributes 
            OPEN_EXISTING,
            FILE_FLAG_OVERLAPPED,
            NULL );

        if (*f == INVALID_HANDLE_VALUE) return false;

        // init OVERLAPPED 
        o->Internal = 0;
        o->InternalHigh = 0;
        o->Offset = 0;
        o->OffsetHigh = 0;
        o->hEvent = CreateEvent(
            NULL,   // default security attributes 
            TRUE,   // manual-reset event 
            FALSE,  // not signaled 
            NULL    // no name
            );
        return true;
    }

    void CloseDisk(HANDLE f, OVERLAPPED* o) {
        if(f != INVALID_HANDLE_VALUE) CloseHandle(f);
        if(o->hEvent != INVALID_HANDLE_VALUE) CloseHandle(o->hEvent);
    }

    bool StartReadDisk(HANDLE f, OVERLAPPED* o, Bit8u* buffer, Bitu offset, Bitu size) { 
        o->Offset = (DWORD)offset;
        if (!ReadFile(f, buffer, (DWORD)size, NULL, o) &&
            (GetLastError()==ERROR_IO_PENDING)) return true;
        return false;
    }

    // 0=still waiting, 1=catastrophic faliure, 2=success, 3=sector not found, 4=crc error 
    Bitu CheckDiskReadComplete(HANDLE f, OVERLAPPED* o) {
        DWORD numret;
        BOOL b = GetOverlappedResult( f, o, &numret,false); 
        if(b) return 2;
        else {
            int error = GetLastError();
            if(error==ERROR_IO_INCOMPLETE)          return 0;
            if(error==ERROR_FLOPPY_UNKNOWN_ERROR)   return 5;
            if(error==ERROR_CRC)                    return 4;
            if(error==ERROR_SECTOR_NOT_FOUND)       return 3;
            return 1;   
        }
    }

    Bitu ReadDisk(FILE* f, Bit8u driveletter, Bitu retries_max) {
        unsigned char data[36*2*512];
        HANDLE hFloppy;
        DWORD numret;
        OVERLAPPED o;
        DISK_GEOMETRY geom;

        Bit8u drivestring[] = "\\\\.\\x:"; drivestring[4]=driveletter;
        if(!OpenDisk(&hFloppy, &o, drivestring)) return false;

        // get drive geom
        DeviceIoControl( hFloppy, IOCTL_DISK_GET_DRIVE_GEOMETRY,NULL,0,
        &geom,sizeof(DISK_GEOMETRY),&numret,NULL);

        switch(geom.MediaType) {
            case F5_1Pt2_512: case F3_1Pt44_512: case F3_2Pt88_512: case F3_720_512:
            case F5_360_512: case F5_320_512: case F5_180_512: case F5_160_512:
                break;
            default:
                CloseDisk(hFloppy,&o);
                return false;
        }
        Bitu total_sect_per_cyl = geom.SectorsPerTrack * geom.TracksPerCylinder;
        Bitu cyln_size = 512 * total_sect_per_cyl;
        
        WriteOut(MSG_Get("PROGRAM_IMGMAKE_FLREAD"),
            geom.Cylinders.LowPart,geom.TracksPerCylinder,
            geom.SectorsPerTrack,(cyln_size*geom.Cylinders.LowPart)/1024);
        WriteOut(MSG_Get("PROGRAM_IMGMAKE_FLREAD2"));
            
        for(Bitu i = 0; i < geom.Cylinders.LowPart; i++) {
            Bitu result;
            // for each cylinder
            WriteOut("%2u",i);

            if(!StartReadDisk(hFloppy, &o, &data[0], cyln_size*i, cyln_size)){
                CloseDisk(hFloppy,&o);
                return false;
            }
            do {
                result = CheckDiskReadComplete(hFloppy, &o);
                CALLBACK_Idle();
            }
            while (result==0);

            switch(result) {
            case 1:
                CloseDisk(hFloppy,&o);
                return false;
            case 2: // success
                for(Bitu m=0; m < cyln_size/512; m++) WriteOut("\xdb");
                break;
            case 3:
            case 4: // data errors
            case 5:
                for(Bitu k=0; k < total_sect_per_cyl; k++) {
                    Bitu retries=retries_max;
restart_int:
                    StartReadDisk(hFloppy, &o, &data[512*k], cyln_size*i + 512*k, 512);
                    do {
                        result = CheckDiskReadComplete(hFloppy, &o);
                        CALLBACK_Idle();
                    }
                    while (result==0);
                                        
                    switch(result) {
                    case 1: // bad error
                        CloseDisk(hFloppy,&o);
                        return false;
                    case 2: // success
                        if(retries==retries_max) WriteOut("\xdb");
                        else WriteOut("\b\b\b\xb1");
                        break;
                    case 3:
                    case 4: // read errors
                    case 5:
                        if(retries!=retries_max) WriteOut("\b\b\b");
                        retries--;
                        switch(result) {
                            case 3: WriteOut("x");
                            case 4: WriteOut("!");
                            case 5: WriteOut("?");
                        }
                        WriteOut("%2d",retries);

                        if(retries) goto restart_int;
                        const Bit8u badfood[]="IMGMAKE BAD FLOPPY SECTOR \xBA\xAD\xF0\x0D";
                        for(Bit8u z = 0; z < 512/32; z++)
                            memcpy(&data[512*k+z*32],badfood,31);
                        WriteOut("\b\b");
                        break;
                    }
                }
                break;
            }
            fwrite(data, 512, total_sect_per_cyl, f);
            WriteOut("%2x%2x\n", data[0], data[1]);
        }
        // seek to 0
        StartReadDisk(hFloppy, &o, &data[0], 0, 512);
        CloseDisk(hFloppy,&o);
        return true;
    }
#endif

    void Run(void) {
        std::string disktype;
        std::string src;
        std::string filename;
        std::string dpath;

        unsigned int c, h, s, sectors; 
        Bit64u size = 0;

        if(cmd->FindExist("-?")) {
            printHelp();
            return;
        }

/*
        this stuff is too frustrating

        // when only a filename is passed try to create the file on the current DOS path
        // if directory+filename are passed first see if directory is a host path, if not
        // maybe it is a DOSBox path.
        
        // split filename and path
        std::string path = "";
        Bitu spos = temp_line.rfind('\\');
        if(spos==std::string::npos) {
            temp_line.rfind('/');
        }

        if(spos==std::string::npos) {
            // no path separator
            filename=temp_line;
        } else {
            path=temp_line.substr(0,spos);
            filename=temp_line.substr(spos+1,std::string::npos);
        }
        if(filename=="") 

        char tbuffer[DOS_PATHLENGTH]= { 0 };
        if(path=="") {
            if(!DOS_GetCurrentDir(DOS_GetDefaultDrive()+1,tbuffer)){
                printHelp();
                return;
            }
            dpath=(std::string)tbuffer;
        }       
        WriteOut("path %s, filename %s, dpath %s",
            path.c_str(),filename.c_str(),dpath.c_str());
        return;
*/
            
#ifdef WIN32
        // read from real floppy?
        if(cmd->FindString("-source",src,true)) {
            int retries = 10;
            cmd->FindInt("-retries",retries,true);
            if((retries < 1)||(retries > 99))  {
                printHelp();
                return;
            }
            if((src.length()!=1) || !isalpha(src.c_str()[0])) {
                // only one letter allowed
                printHelp();
                return;
            }

            // temp_line is the filename
            if (!(cmd->FindCommand(1, temp_line))) {
                printHelp();
                return;
            }

            // don't trash user's files
            FILE* f = fopen(temp_line.c_str(),"r");
            if(f) {
                fclose(f);
                WriteOut(MSG_Get("PROGRAM_IMGMAKE_FILE_EXISTS"),temp_line.c_str());
                return;
            }
            f = fopen(temp_line.c_str(),"wb+");
            if (!f) {
                WriteOut(MSG_Get("PROGRAM_IMGMAKE_CANNOT_WRITE"),temp_line.c_str());
                return;
            }
            // maybe delete f if it failed?
            if(!ReadDisk(f, src.c_str()[0],retries))
                WriteOut(MSG_Get("PROGRAM_IMGMAKE_CANT_READ_FLOPPY"));
            fclose(f);
            return;
        }
#endif
        // disk type
        if (!(cmd->FindString("-t",disktype,true))) {
            printHelp();
            return;
        }

        Bit8u mediadesc = 0xF8; // media descriptor byte; also used to differ fd and hd
        Bit16u root_ent = 512; // FAT root directory entries: 512 is for harddisks
        if(disktype=="fd_160") {
            c = 40; h = 1; s = 8; mediadesc = 0xFE; root_ent = 56; // root_ent?
        } else if(disktype=="fd_180") {
            c = 40; h = 1; s = 9; mediadesc = 0xFC; root_ent = 56; // root_ent?
        } else if(disktype=="fd_200") {
            c = 40; h = 1; s = 10; mediadesc = 0xFC; root_ent = 56; // root_ent?
        } else if(disktype=="fd_320") {
            c = 40; h = 2; s = 8; mediadesc = 0xFF; root_ent = 112; // root_ent?
        } else if(disktype=="fd_360") {
            c = 40; h = 2; s = 9; mediadesc = 0xFD; root_ent = 112;
        } else if(disktype=="fd_400") {
            c = 40; h = 2; s = 10; mediadesc = 0xFD; root_ent = 112; // root_ent?
        } else if(disktype=="fd_720") {
            c = 80; h = 2; s = 9; mediadesc = 0xF9; root_ent = 112;
        } else if(disktype=="fd_1200") {
            c = 80; h = 2; s = 15; mediadesc = 0xF9; root_ent = 224;
        } else if(disktype=="fd_1440") {
            c = 80; h = 2; s = 18; mediadesc = 0xF0; root_ent = 224;
        } else if(disktype=="fd_2880") {
            c = 80; h = 2; s = 36; mediadesc = 0xF0; root_ent = 512; // root_ent?
        } else if(disktype=="hd_250") {
            c = 489; h = 16; s = 63;
        } else if(disktype=="hd_520") {
            c = 1023; h = 16; s = 63;
        } else if(disktype=="hd_2gig") {
            c = 1023; h = 64; s = 63;
        } else if(disktype=="hd_4gig") { // fseek only supports 2gb
            c = 1023; h = 130; s = 63;
        } else if(disktype=="hd_8gig") { // fseek only supports 2gb
            c = 1023; h = 255; s = 63;
        } else if(disktype=="hd_st251") { // old 40mb drive
            c = 820; h = 6; s = 17;
        } else if(disktype=="hd_st225") { // even older 20mb drive
            c = 615; h = 4; s = 17;
        } else if(disktype=="hd") {
            // get size from parameter
            std::string isize;
            if (!(cmd->FindString("-size",isize,true))) {
                // maybe -chs?
                if (!(cmd->FindString("-chs",isize,true))){
                        // user forgot -size and -chs
                        printHelp();
                        return;
                    }
                else {
                    // got chs data: -chs 1023,16,63
                    if(sscanf(isize.c_str(),"%u,%u,%u",&c,&h,&s) != 3) {
                        printHelp();
                        return;
                    }
                    // sanity-check chs values
                    if((h>255)||(c>1023)||(s>63)) {
                        printHelp();
                        return;
                    }
                    size = (unsigned long long)c * (unsigned long long)h * (unsigned long long)s * 512ULL;
                    if((size < 3u*1024u*1024u) || (size > 0x1FFFFFFFFLL)) {
                        // user picked geometry resulting in wrong size
                        printHelp();
                        return;
                    }
                }
            } else {
                // got -size
                std::istringstream stream(isize);
                stream >> size;
                size *= 1024*1024LL; // size in megabytes
                // low limit: 3 megs, high limit: 2 gigs
                // Int13 limit would be 8 gigs
                if((size < 3*1024*1024LL) || (size > 0x1FFFFFFFFLL)) {
                    // wrong size
                    printHelp();
                    return;
                }
                sectors = (unsigned int)(size / 512);

                // Now that we finally have the proper size, figure out good CHS values
                h=2;
                while(h*1023*63 < sectors) h <<= 1;
                if(h>255) h=255;
                s=8;
                while(h*s*1023 < sectors) s *= 2;
                if(s>63) s=63;
                c=sectors/(h*s);
                if(c>1023) c=1023;
            }
        } else {
            // user passed a wrong -t argument
            printHelp();
            return;
        }

        std::string t2 = "";
        if(cmd->FindExist("-bat",true)) {
            t2 = "-bat";
        }

        size = (unsigned long long)c * (unsigned long long)h * (unsigned long long)s * 512ULL;
        Bits bootsect_pos = 0; // offset of the boot sector in clusters
        if(cmd->FindExist("-nofs",true) || (size>(2048*1024*1024LL))) {
            bootsect_pos = -1;
        }

        // temp_line is the filename
        if (!(cmd->FindCommand(1, temp_line))) {
            printHelp();
            return;
        }

        // don't trash user's files
        FILE* f = fopen(temp_line.c_str(),"r");
        if(f) {
            fclose(f);
            WriteOut(MSG_Get("PROGRAM_IMGMAKE_FILE_EXISTS"),temp_line.c_str());
            return;
        }

        WriteOut(MSG_Get("PROGRAM_IMGMAKE_PRINT_CHS"),c,h,s);
        WriteOut("%s\r\n",temp_line.c_str());
        LOG_MSG(MSG_Get("PROGRAM_IMGMAKE_PRINT_CHS"),c,h,s);

        // do it again for fixed chs values
        sectors = (unsigned int)(size / 512);

        // create the image file
        f = fopen64(temp_line.c_str(),"wb+");
        if (!f) {
            WriteOut(MSG_Get("PROGRAM_IMGMAKE_CANNOT_WRITE"),temp_line.c_str());
            return;
        }
        if(fseeko64(f,static_cast<off_t>(size - 1ull),SEEK_SET)) {
            WriteOut(MSG_Get("PROGRAM_IMGMAKE_NOT_ENOUGH_SPACE"),size);
            fclose(f);
            return;
        }
        Bit8u bufferbyte=0;
        if(fwrite(&bufferbyte,1,1,f)!=1) {
            WriteOut(MSG_Get("PROGRAM_IMGMAKE_NOT_ENOUGH_SPACE"),size);
            fclose(f);
            return;
        }

        // Format the image if not unrequested (and image size<2GB)
        if(bootsect_pos > -1) {
            Bit8u sbuf[512];
            if(mediadesc == 0xF8) {
                // is a harddisk: write MBR
                memcpy(sbuf,freedos_mbr,512);
                // active partition
                sbuf[0x1be]=0x80;
                // start head - head 0 has the partition table, head 1 first partition
                sbuf[0x1bf]=1;
                // start sector with bits 8-9 of start cylinder in bits 6-7
                sbuf[0x1c0]=1;
                // start cylinder bits 0-7
                sbuf[0x1c1]=0;
                // OS indicator: DOS what else ;)
                sbuf[0x1c2]=0x06;
                // end head (0-based)
                sbuf[0x1c3]= h-1;
                // end sector with bits 8-9 of end cylinder (0-based) in bits 6-7
                sbuf[0x1c4]=s|(((c-1)&0x300)>>2);
                // end cylinder (0-based) bits 0-7
                sbuf[0x1c5]=(c-1)&0xFF;
                // sectors preceding partition1 (one head)
                host_writed(&sbuf[0x1c6],s);
                // length of partition1, align to chs value
                host_writed(&sbuf[0x1ca],((c-1)*h+(h-1))*s);

                // write partition table
                fseeko64(f,0,SEEK_SET);
                fwrite(&sbuf,512,1,f);
                bootsect_pos = (Bits)s;
            }

            // set boot sector values
            memset(sbuf,0,512);
            // TODO boot code jump
            sbuf[0]=0xEB; sbuf[1]=0x3c; sbuf[2]=0x90;
            // OEM
            sprintf((char*)&sbuf[0x03],"MSDOS5.0");
            // bytes per sector: always 512
            host_writew(&sbuf[0x0b],512);
            // sectors per cluster: 1,2,4,8,16,...
            if(mediadesc == 0xF8) {
                Bitu cval = 1;
                while((sectors/cval) >= 65525) cval <<= 1;
                sbuf[0x0d]=(Bit8u)cval;
            } else sbuf[0x0d]=sectors/0x1000 + 1; // FAT12 can hold 0x1000 entries TODO
            // TODO small floppys have 2 sectors per cluster?
            // reserverd sectors: 1 ( the boot sector)
            host_writew(&sbuf[0x0e],1);
            // Number of FATs - always 2
            sbuf[0x10] = 2;
            // Root entries - how are these made up? - TODO
            host_writew(&sbuf[0x11],root_ent);
            // sectors (under 32MB) - will OSes be sore if all HD's use large size?
            if(mediadesc != 0xF8) host_writew(&sbuf[0x13],c*h*s);
            // media descriptor
            sbuf[0x15]=mediadesc;
            // sectors per FAT
            // needed entries: (sectors per cluster)
            Bitu sect_per_fat=0;
            Bitu clusters = (sectors-1)/sbuf[0x0d]; // TODO subtract root dir too maybe
            if(mediadesc == 0xF8) sect_per_fat = (clusters*2)/512+1;
            else sect_per_fat = ((clusters*3)/2)/512+1;
            host_writew(&sbuf[0x16],(Bit16u)sect_per_fat);
            // sectors per track
            host_writew(&sbuf[0x18],s);
            // heads
            host_writew(&sbuf[0x1a],h);
            // hidden sectors
            host_writed(&sbuf[0x1c],(Bit32u)bootsect_pos);
            if(mediadesc == 0xF8) {
                // sectors (large disk) - this is the same as partition length in MBR
                host_writed(&sbuf[0x20],sectors-s);
                // BIOS drive
                sbuf[0x24]=0x80;
            }
            else sbuf[0x24]=0x00;
            // ext. boot signature
            sbuf[0x26]=0x29;
            // volume serial number
            // let's use the BIOS time (cheap, huh?)
            host_writed(&sbuf[0x27],mem_readd(BIOS_TIMER));
            // Volume label
            sprintf((char*)&sbuf[0x2b],"NO NAME    ");
            // file system type
            if(mediadesc == 0xF8) sprintf((char*)&sbuf[0x36],"FAT16   ");
            else sprintf((char*)&sbuf[0x36],"FAT12   ");
            // boot sector signature
            host_writew(&sbuf[0x1fe],0xAA55);

            // write the boot sector
            fseeko64(f,bootsect_pos*512,SEEK_SET);
            fwrite(&sbuf,512,1,f);
            // write FATs
            memset(sbuf,0,512);
            if(mediadesc == 0xF8) host_writed(&sbuf[0],0xFFFFFFF8);
            else host_writed(&sbuf[0],0xFFFFF0);
            // 1st FAT
            fseeko64(f,(off_t)((bootsect_pos+1ll)*512ll),SEEK_SET);
            fwrite(&sbuf,512,1,f);
            // 2nd FAT
            fseeko64(f,(off_t)(((unsigned long long)bootsect_pos+1ull+(unsigned long long)sect_per_fat)*512ull),SEEK_SET);
            fwrite(&sbuf,512,1,f);
        }
        // write VHD footer if requested, largely copied from RAW2VHD program, no license was included
        if((mediadesc == 0xF8) && (temp_line.find(".vhd")) != std::string::npos) {
            int i;
            Bit8u footer[512];
            // basic information
            memcpy(footer,"conectix" "\0\0\0\2\0\1\0\0" "\xff\xff\xff\xff\xff\xff\xff\xff" "????rawv" "\0\1\0\0Wi2k",40);
            memset(footer+40,0,512-40);
            // time
            struct tm tm20000101 = { /*sec*/0,/*min*/0,/*hours*/0, /*day of month*/1,/*month*/0,/*year*/100, /*wday*/0,/*yday*/0,/*isdst*/0 };
            time_t basetime = mktime(&tm20000101);
            time_t vhdtime = time(NULL) - basetime;
#if defined (_MSC_VER)
            *(Bit32u*)(footer+0x18) = SDL_SwapBE32((__time32_t)vhdtime);
#else
            *(Bit32u*)(footer+0x18) = Bit32u(SDL_SwapBE32((Uint32)vhdtime));
#endif
            // size and geometry
            *(Bit64u*)(footer+0x30) = *(Bit64u*)(footer+0x28) = SDL_SwapBE64(size);

            *(Bit16u*)(footer+0x38) = SDL_SwapBE16(c);
            *(Bit8u*)( footer+0x3A) = h;
            *(Bit8u*)( footer+0x3B) = s;
            *(Bit32u*)(footer+0x3C) = SDL_SwapBE32(2);

            // generate UUID
            for (i=0; i<16; ++i) {
                *(footer+0x44+i) = (Bit8u)(rand()>>4);
            }

            // calculate checksum
            Bit32u sum;
            for (i=0,sum=0; i<512; ++i) {
                sum += footer[i];
            }

            *(Bit32u*)(footer+0x40) = SDL_SwapBE32(~sum);

            // write footer
            fseeko64(f, 0L, SEEK_END);
            fwrite(&footer,512,1,f);
        }
        fclose(f);

        // create the batch file
        if(t2 == "-bat") {
            if(temp_line.length() > 3) {
                t2 = temp_line.substr(0,temp_line.length()-4);
                t2 = t2.append(".bat");
            } else {
                t2 = temp_line.append(".bat");
            }
            WriteOut("%s\n",t2.c_str());
            f = fopen(t2.c_str(),"wb+");
            if (!f) {
                WriteOut(MSG_Get("PROGRAM_IMGMAKE_CANNOT_WRITE"),t2.c_str());
                return;
            }
            fprintf(f,"imgmount c %s -size 512,%u,%u,%u\r\n",temp_line.c_str(),s,h,c);
            fclose(f);
        }
        return;
    }
    void printHelp() { // maybe hint parameter?
        WriteOut(MSG_Get("PROGRAM_IMGMAKE_SYNTAX"));
    }
};

static void IMGMAKE_ProgramStart(Program * * make) {
    *make=new IMGMAKE;
}

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

Bitu DOS_SwitchKeyboardLayout(const char* new_layout, Bit32s& tried_cp);
Bitu DOS_LoadKeyboardLayout(const char * layoutname, Bit32s codepage, const char * codepagefile);
const char* DOS_GetLoadedLayout(void);

class KEYB : public Program {
public:
    void Run(void);
};

void KEYB::Run(void) {
    // codepage 949 start
    std::string temp_codepage;
    temp_codepage="949";
    if (cmd->FindString("ko",temp_codepage,false)) {
        dos.loaded_codepage=949;
        const char* layout_name = DOS_GetLoadedLayout();
        WriteOut(MSG_Get("PROGRAM_KEYB_INFO_LAYOUT"),dos.loaded_codepage,layout_name);
        return;
    }
    // codepage 949 end
    if (cmd->FindCommand(1,temp_line)) {
        if (cmd->FindString("?",temp_line,false)) {
            WriteOut(MSG_Get("PROGRAM_KEYB_SHOWHELP"));
        } else {
            /* first parameter is layout ID */
            Bitu keyb_error=0;
            std::string cp_string;
            Bit32s tried_cp = -1;
            if (cmd->FindCommand(2,cp_string)) {
                /* second parameter is codepage number */
                tried_cp=atoi(cp_string.c_str());
                char cp_file_name[256];
                if (cmd->FindCommand(3,cp_string)) {
                    /* third parameter is codepage file */
                    strcpy(cp_file_name, cp_string.c_str());
                } else {
                    /* no codepage file specified, use automatic selection */
                    strcpy(cp_file_name, "auto");
                }

                keyb_error=DOS_LoadKeyboardLayout(temp_line.c_str(), tried_cp, cp_file_name);
            } else {
                keyb_error=DOS_SwitchKeyboardLayout(temp_line.c_str(), tried_cp);
            }
            switch (keyb_error) {
                case KEYB_NOERROR:
                    WriteOut(MSG_Get("PROGRAM_KEYB_NOERROR"),temp_line.c_str(),dos.loaded_codepage);
                    break;
                case KEYB_FILENOTFOUND:
                    WriteOut(MSG_Get("PROGRAM_KEYB_FILENOTFOUND"),temp_line.c_str());
                    WriteOut(MSG_Get("PROGRAM_KEYB_SHOWHELP"));
                    break;
                case KEYB_INVALIDFILE:
                    WriteOut(MSG_Get("PROGRAM_KEYB_INVALIDFILE"),temp_line.c_str());
                    break;
                case KEYB_LAYOUTNOTFOUND:
                    WriteOut(MSG_Get("PROGRAM_KEYB_LAYOUTNOTFOUND"),temp_line.c_str(),tried_cp);
                    break;
                case KEYB_INVALIDCPFILE:
                    WriteOut(MSG_Get("PROGRAM_KEYB_INVCPFILE"),temp_line.c_str());
                    WriteOut(MSG_Get("PROGRAM_KEYB_SHOWHELP"));
                    break;
                default:
                    LOG(LOG_DOSMISC,LOG_ERROR)("KEYB:Invalid returncode %x",(int)keyb_error);
                    break;
            }
        }
    } else {
        /* no parameter in the command line, just output codepage info and possibly loaded layout ID */
        const char* layout_name = DOS_GetLoadedLayout();
        if (layout_name==NULL) {
            WriteOut(MSG_Get("PROGRAM_KEYB_INFO"),dos.loaded_codepage);
        } else {
            WriteOut(MSG_Get("PROGRAM_KEYB_INFO_LAYOUT"),dos.loaded_codepage,layout_name);
        }
    }
}

static void KEYB_ProgramStart(Program * * make) {
    *make=new KEYB;
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

    MSG_Add("PROGRAM_LOADROM_SPECIFY_FILE","Must specify ROM file to load.\n");
    MSG_Add("PROGRAM_LOADROM_CANT_OPEN","ROM file not accessible.\n");
    MSG_Add("PROGRAM_LOADROM_TOO_LARGE","ROM file too large.\n");
    MSG_Add("PROGRAM_LOADROM_INCOMPATIBLE","Video BIOS not supported by machine type.\n");
    MSG_Add("PROGRAM_LOADROM_UNRECOGNIZED","ROM file not recognized.\n");
    MSG_Add("PROGRAM_LOADROM_BASIC_LOADED","BASIC ROM loaded.\n");

    MSG_Add("VHD_ERROR_OPENING", "Could not open the specified VHD file.\n");
    MSG_Add("VHD_INVALID_DATA", "The specified VHD file is corrupt and cannot be opened.\n");
    MSG_Add("VHD_UNSUPPORTED_TYPE", "The specified VHD file is of an unsupported type.\n");
    MSG_Add("VHD_ERROR_OPENING_PARENT", "The parent of the specified VHD file could not be found.\n");
    MSG_Add("VHD_PARENT_INVALID_DATA", "The parent of the specified VHD file is corrupt and cannot be opened.\n");
    MSG_Add("VHD_PARENT_UNSUPPORTED_TYPE", "The parent of the specified VHD file is of an unsupported type.\n");
    MSG_Add("VHD_PARENT_INVALID_MATCH", "The parent of the specified VHD file does not contain the expected identifier.\n");
    MSG_Add("VHD_PARENT_INVALID_DATE", "The parent of the specified VHD file has been changed and cannot be loaded.\n");
    MSG_Add("PROGRAM_IMGMAKE_FLREAD",
        "Disk geometry: %d Cylinders, %d Heads, %d Sectors, %d Kilobytes\n\n");
    MSG_Add("PROGRAM_IMGMAKE_FLREAD2",
        "\xdb =good, \xb1 =good after retries, ! =CRC error, x =sector not found, ? =unknown\n\n");
    MSG_Add("PROGRAM_IMGMAKE_FILE_EXISTS","The file \"%s\" already exists.\n");
    MSG_Add("PROGRAM_IMGMAKE_CANNOT_WRITE","The file \"%s\" cannot be opened for writing.\n");
    MSG_Add("PROGRAM_IMGMAKE_NOT_ENOUGH_SPACE","Not enough space availible for the image file. Need %u bytes.\n");
    MSG_Add("PROGRAM_IMGMAKE_PRINT_CHS","Creating an image file with %u cylinders, %u heads and %u sectors\n");
    MSG_Add("PROGRAM_IMGMAKE_CANT_READ_FLOPPY","\n\nUnable to read floppy.");

    MSG_Add("PROGRAM_KEYB_INFO","Codepage %i has been loaded\n");
    MSG_Add("PROGRAM_KEYB_INFO_LAYOUT","Codepage %i has been loaded for layout %s\n");
    MSG_Add("PROGRAM_KEYB_SHOWHELP",
        "\033[32;1mKEYB\033[0m [keyboard layout ID[ codepage number[ codepage file]]]\n\n"
        "Some examples:\n"
        "  \033[32;1mKEYB\033[0m: Display currently loaded codepage.\n"
        "  \033[32;1mKEYB\033[0m sp: Load the spanish (SP) layout, use an appropriate codepage.\n"
        "  \033[32;1mKEYB\033[0m sp 850: Load the spanish (SP) layout, use codepage 850.\n"
        "  \033[32;1mKEYB\033[0m sp 850 mycp.cpi: Same as above, but use file mycp.cpi.\n");
    MSG_Add("PROGRAM_KEYB_NOERROR","Keyboard layout %s loaded for codepage %i\n");
    MSG_Add("PROGRAM_KEYB_FILENOTFOUND","Keyboard file %s not found\n\n");
    MSG_Add("PROGRAM_KEYB_INVALIDFILE","Keyboard file %s invalid\n");
    MSG_Add("PROGRAM_KEYB_LAYOUTNOTFOUND","No layout in %s for codepage %i\n");
    MSG_Add("PROGRAM_KEYB_INVCPFILE","None or invalid codepage file for layout %s\n\n");
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
    PROGRAMS_MakeFile("BOOT.COM",BOOT_ProgramStart);

        PROGRAMS_MakeFile("LOADROM.COM", LOADROM_ProgramStart);

    PROGRAMS_MakeFile("IMGMAKE.COM", IMGMAKE_ProgramStart);

        PROGRAMS_MakeFile("MODE.COM", MODE_ProgramStart);

    //PROGRAMS_MakeFile("MORE.COM", MORE_ProgramStart);

        PROGRAMS_MakeFile("KEYB.COM", KEYB_ProgramStart);

    PROGRAMS_MakeFile("A20GATE.COM",A20GATE_ProgramStart);
    PROGRAMS_MakeFile("SHOWGUI.COM",SHOWGUI_ProgramStart);
    PROGRAMS_MakeFile("NMITEST.COM",NMITEST_ProgramStart);
    PROGRAMS_MakeFile("RE-DOS.COM",REDOS_ProgramStart);

    PROGRAMS_MakeFile("LABEL.COM", LABEL_ProgramStart);
}
