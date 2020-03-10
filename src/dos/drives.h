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


#ifndef _DRIVES_H__
#define _DRIVES_H__

#include <vector>
#include <sys/types.h>
#include "dos_system.h"
#include "shell.h" /* for DOS_Shell */

bool WildFileCmp(const char * file, const char * wild);
void Set_Label(char const * const input, char * const output, bool cdrom);

class DriveManager {
public:
	static void AppendDisk(int drive, DOS_Drive* disk);
	static void InitializeDrive(int drive);
	static int UnmountDrive(int drive);
//	static void CycleDrive(bool pressed);
//	static void CycleDisk(bool pressed);
	static void CycleDisks(int drive, bool notify);
	static void CycleAllDisks(void);
	static void CycleAllCDs(void);
	static void Init(Section* s);
	
	static void SaveState( std::ostream& stream );
	static void LoadState( std::istream& stream );
	
private:
	static struct DriveInfo {
		std::vector<DOS_Drive*> disks;
		Bit32u currentDisk = 0;
	} driveInfos[DOS_DRIVES];
	
	static int currentDrive;
};

class localDrive : public DOS_Drive {
public:
	localDrive(const char * startdir,Bit16u _bytes_sector,Bit8u _sectors_cluster,Bit16u _total_clusters,Bit16u _free_clusters,Bit8u _mediaid);
	virtual bool FileOpen(DOS_File * * file,const char * name,Bit32u flags);
	virtual FILE *GetSystemFilePtr(char const * const name, char const * const type); 
	virtual bool GetSystemFilename(char* sysName, char const * const dosName); 
	virtual bool FileCreate(DOS_File * * file,const char * name,Bit16u attributes);
	virtual bool FileUnlink(const char * name);
	virtual bool RemoveDir(const char * dir);
	virtual bool MakeDir(const char * dir);
	virtual bool TestDir(const char * dir);
	virtual bool FindFirst(const char * _dir,DOS_DTA & dta,bool fcb_findfirst=false);
	virtual bool FindNext(DOS_DTA & dta);
	virtual bool GetFileAttr(const char * name,Bit16u * attr);
	virtual bool Rename(const char * oldname,const char * newname);
	virtual bool AllocationInfo(Bit16u * _bytes_sector,Bit8u * _sectors_cluster,Bit16u * _total_clusters,Bit16u * _free_clusters);
	virtual bool FileExists(const char* name);
	virtual bool FileStat(const char* name, FileStat_Block * const stat_block);
	virtual Bit8u GetMediaByte(void);
	virtual bool isRemote(void);
	virtual bool isRemovable(void);
	virtual Bits UnMount(void);
	virtual char const * GetLabel(){return dirCache.GetLabel();};
	virtual void SetLabel(const char *label, bool iscdrom, bool updatable) { dirCache.SetLabel(label,iscdrom,updatable); };
	virtual void *opendir(const char *name);
	virtual void closedir(void *handle);
	virtual bool read_directory_first(void *handle, char* entry_name, bool& is_directory);
	virtual bool read_directory_next(void *handle, char* entry_name, bool& is_directory);

	virtual void EmptyCache(void) { dirCache.EmptyCache(); };
	virtual void MediaChange() {};

protected:
	DOS_Drive_Cache dirCache;
	char basedir[CROSS_LEN];
	friend void DOS_Shell::CMD_SUBST(char* args); 	
	struct {
		char srch_dir[CROSS_LEN];
    } srchInfo[MAX_OPENDIRS] = {};

	struct {
		Bit16u bytes_sector;
		Bit8u sectors_cluster;
		Bit16u total_clusters;
		Bit16u free_clusters;
		Bit8u mediaid;
	} allocation;
};

struct VFILE_Block;

class Virtual_Drive: public DOS_Drive {
public:
	Virtual_Drive();
	bool FileOpen(DOS_File * * file,const char * name,Bit32u flags);
	bool FileCreate(DOS_File * * file,const char * name,Bit16u attributes);
	bool FileUnlink(const char * name);
	bool RemoveDir(const char * dir);
	bool MakeDir(const char * dir);
	bool TestDir(const char * dir);
	bool FindFirst(const char * _dir,DOS_DTA & dta,bool fcb_findfirst);
	bool FindNext(DOS_DTA & dta);
	bool GetFileAttr(const char * name,Bit16u * attr);
	bool Rename(const char * oldname,const char * newname);
	bool AllocationInfo(Bit16u * _bytes_sector,Bit8u * _sectors_cluster,Bit16u * _total_clusters,Bit16u * _free_clusters);
	bool FileExists(const char* name);
	bool FileStat(const char* name, FileStat_Block* const stat_block);
	virtual void MediaChange() {}
	Bit8u GetMediaByte(void);
	void EmptyCache(void){}
	bool isRemote(void);
	virtual bool isRemovable(void);
	virtual Bits UnMount(void);
	virtual char const* GetLabel(void);
private:
	VFILE_Block * search_file;
};



#endif
