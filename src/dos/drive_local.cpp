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
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <limits.h>

#include "dosbox.h"
#include "dos_inc.h"
#include "drives.h"
#include "support.h"
#include "cross.h"
#include "inout.h"
#ifndef WIN32
#include <utime.h>
#else
#include <sys/utime.h>
#include <sys/locking.h>
#endif

#include "cp437_uni.h"
#include "cp932_uni.h"

#if defined(PATH_MAX) && !defined(MAX_PATH)
#define MAX_PATH PATH_MAX
#endif

#if defined(WIN32)
// Windows: Use UTF-16 (wide char)
// TODO: Offer an option to NOT use wide char on Windows if directed by config.h
//       for people who compile this code for Windows 95 or earlier where some
//       widechar functions are missing.
typedef wchar_t host_cnv_char_t;
# define host_cnv_use_wchar
# define _HT(x) L##x
# if defined(__MINGW32__) /* TODO: Get MinGW to support 64-bit file offsets, at least targeting Windows XP! */
#  define ht_stat_t struct _stat
#  define ht_stat(x,y) _wstat(x,y)
# else
#  define ht_stat_t struct _stat64i32 /* WTF Microsoft?? Why aren't _stat and _wstat() consistent on stat struct type? */
#  define ht_stat(x,y) _wstat64i32(x,y)
# endif
# define ht_access(x,y) _waccess(x,y)
# define ht_strdup(x) _wcsdup(x)
# define ht_unlink(x) _wunlink(x)
#else
// Linux: Use UTF-8
typedef char host_cnv_char_t;
# define _HT(x) x
# define ht_stat_t struct stat
# define ht_stat(x,y) stat(x,y)
# define ht_access(x,y) access(x,y)
# define ht_strdup(x) strdup(x)
# define ht_unlink(x) unlink(x)
#endif

static host_cnv_char_t cpcnv_temp[4096];

bool String_ASCII_TO_HOST(host_cnv_char_t *d/*CROSS_LEN*/,const char *s/*CROSS_LEN*/) {
	host_cnv_char_t *df = d + CROSS_LEN - 1;
	const char *sf = s + CROSS_LEN - 1;

    while (*s != 0 && s < sf) {
        unsigned char ic = (unsigned char)(*s++);
        if (ic < 32 || ic > 127) return false; // non-representable

#if defined(host_cnv_use_wchar)
        *d++ = (host_cnv_char_t)ic;
#else
        if (utf8_encode(&d,df,(uint32_t)ic) < 0) // will advance d by however many UTF-8 bytes are needed
            return false; // non-representable, or probably just out of room
#endif
    }

    assert(d <= df);
    *d = 0;

    return true;
}

template <class MT> bool String_SBCS_TO_HOST(host_cnv_char_t *d/*CROSS_LEN*/,const char *s/*CROSS_LEN*/,const MT *map,const size_t map_max) {
	host_cnv_char_t *df = d + CROSS_LEN - 1;
	const char *sf = s + CROSS_LEN - 1;

    while (*s != 0 && s < sf) {
        unsigned char ic = (unsigned char)(*s++);
        if (ic >= map_max) return false; // non-representable
        MT wc = map[ic]; // output: unicode character

#if defined(host_cnv_use_wchar)
        *d++ = (host_cnv_char_t)wc;
#else
        if (utf8_encode(&d,df,(uint32_t)wc) < 0) // will advance d by however many UTF-8 bytes are needed
            return false; // non-representable, or probably just out of room
#endif
    }

    assert(d <= df);
    *d = 0;

    return true;
}

template <class MT> bool String_DBCS_TO_HOST_SHIFTJIS(host_cnv_char_t *d/*CROSS_LEN*/,const char *s/*CROSS_LEN*/,const MT *hitbl,const MT *rawtbl,const size_t rawtbl_max) {
	host_cnv_char_t *df = d + CROSS_LEN - 1;
	const char *sf = s + CROSS_LEN - 1;

    while (*s != 0 && s < sf) {
        uint16_t ic = (unsigned char)(*s++);
        if ((ic & 0xE0) == 0x80 || (ic & 0xE0) == 0xE0) {
            if (*s == 0) return false;
            ic <<= 8U;
            ic += (unsigned char)(*s++);
        }

        MT rawofs = hitbl[ic >> 6];
        if (rawofs == 0xFFFF)
            return false;

        assert((size_t)(rawofs+ (Bitu)0x40) <= rawtbl_max);
        MT wc = rawtbl[rawofs + (ic & 0x3F)];
        if (wc == 0x0000)
            return false;

#if defined(host_cnv_use_wchar)
        *d++ = (host_cnv_char_t)wc;
#else
        if (utf8_encode(&d,df,(uint32_t)wc) < 0) // will advance d by however many UTF-8 bytes are needed
            return false; // non-representable, or probably just out of room
#endif
    }

    assert(d <= df);
    *d = 0;

    return true;
}

// TODO: This is SLOW. Optimize.
template <class MT> int SBCS_From_Host_Find(int c,const MT *map,const size_t map_max) {
    for (size_t i=0;i < map_max;i++) {
        if ((MT)c == map[i])
            return (int)i;
    }

    return -1;
}

// TODO: This is SLOW. Optimize.
template <class MT> int DBCS_SHIFTJIS_From_Host_Find(int c,const MT *hitbl,const MT *rawtbl,const size_t rawtbl_max) {
    for (size_t h=0;h < 1024;h++) {
        MT ofs = hitbl[h];

        if (ofs == 0xFFFF) continue;
        assert((size_t)(ofs+ (Bitu)0x40) <= rawtbl_max);

        for (size_t l=0;l < 0x40;l++) {
            if ((MT)c == rawtbl[ofs+l])
                return (int)((h << 6) + l);
        }
    }

    return -1;
}

template <class MT> bool String_HOST_TO_DBCS_SHIFTJIS(char *d/*CROSS_LEN*/,const host_cnv_char_t *s/*CROSS_LEN*/,const MT *hitbl,const MT *rawtbl,const size_t rawtbl_max) {
    const host_cnv_char_t *sf = s + CROSS_LEN - 1;
    char *df = d + CROSS_LEN - 1;

    while (*s != 0 && s < sf) {
        int ic;
#if defined(host_cnv_use_wchar)
        ic = (int)(*s++);
#else
        if ((ic=utf8_decode(&s,sf)) < 0)
            return false; // non-representable
#endif

        int oc = DBCS_SHIFTJIS_From_Host_Find<MT>(ic,hitbl,rawtbl,rawtbl_max);
        if (oc < 0)
            return false; // non-representable

        if (oc >= 0x100) {
            if ((d+1) >= df) return false;
            *d++ = (char)(oc >> 8U);
            *d++ = (char)oc;
        }
        else {
            if (d >= df) return false;
            *d++ = (char)oc;
        }
    }

    assert(d <= df);
    *d = 0;

    return true;
}

template <class MT> bool String_HOST_TO_SBCS(char *d/*CROSS_LEN*/,const host_cnv_char_t *s/*CROSS_LEN*/,const MT *map,const size_t map_max) {
    const host_cnv_char_t *sf = s + CROSS_LEN - 1;
    char *df = d + CROSS_LEN - 1;

    while (*s != 0 && s < sf) {
        int ic;
#if defined(host_cnv_use_wchar)
        ic = (int)(*s++);
#else
        if ((ic=utf8_decode(&s,sf)) < 0)
            return false; // non-representable
#endif

        int oc = SBCS_From_Host_Find<MT>(ic,map,map_max);
        if (oc < 0)
            return false; // non-representable

        if (d >= df) return false;
        *d++ = (char)oc;
    }

    assert(d <= df);
    *d = 0;

    return true;
}

bool String_HOST_TO_ASCII(char *d/*CROSS_LEN*/,const host_cnv_char_t *s/*CROSS_LEN*/) {
    const host_cnv_char_t *sf = s + CROSS_LEN - 1;
    char *df = d + CROSS_LEN - 1;

    while (*s != 0 && s < sf) {
        int ic;
#if defined(host_cnv_use_wchar)
        ic = (int)(*s++);
#else
        if ((ic=utf8_decode(&s,sf)) < 0)
            return false; // non-representable
#endif

        if (ic < 32 || ic > 127)
            return false; // non-representable

        if (d >= df) return false;
        *d++ = (char)ic;
    }

    assert(d <= df);
    *d = 0;

    return true;
}

bool cpwarn_once = false;

bool CodePageHostToGuest(char *d/*CROSS_LEN*/,const host_cnv_char_t *s/*CROSS_LEN*/) {
    switch (dos.loaded_codepage) {
        case 437:
            return String_HOST_TO_SBCS<uint16_t>(d,s,cp437_to_unicode,sizeof(cp437_to_unicode)/sizeof(cp437_to_unicode[0]));
        case 932:
            return String_HOST_TO_DBCS_SHIFTJIS<uint16_t>(d,s,cp932_to_unicode_hitbl,cp932_to_unicode_raw,sizeof(cp932_to_unicode_raw)/sizeof(cp932_to_unicode_raw[0]));
        default:
            /* at this time, it would be cruel and unusual to not allow any file I/O just because
             * our code page support is so limited. */
            if (!cpwarn_once) {
                cpwarn_once = true;
                LOG_MSG("WARNING: No translation support (to guest) for code page %u",dos.loaded_codepage);
            }
            return String_HOST_TO_ASCII(d,s);
    }

    return false;
}

bool CodePageGuestToHost(host_cnv_char_t *d/*CROSS_LEN*/,const char *s/*CROSS_LEN*/) {
    switch (dos.loaded_codepage) {
        case 437:
            return String_SBCS_TO_HOST<uint16_t>(d,s,cp437_to_unicode,sizeof(cp437_to_unicode)/sizeof(cp437_to_unicode[0]));
        case 932:
            return String_DBCS_TO_HOST_SHIFTJIS<uint16_t>(d,s,cp932_to_unicode_hitbl,cp932_to_unicode_raw,sizeof(cp932_to_unicode_raw)/sizeof(cp932_to_unicode_raw[0]));
        default:
            /* at this time, it would be cruel and unusual to not allow any file I/O just because
             * our code page support is so limited. */
            if (!cpwarn_once) {
                cpwarn_once = true;
                LOG_MSG("WARNING: No translation support (to host) for code page %u",dos.loaded_codepage);
            }
            return String_ASCII_TO_HOST(d,s);
    }

    return false;
}

host_cnv_char_t *CodePageGuestToHost(const char *s) {
    if (!CodePageGuestToHost(cpcnv_temp,s))
        return NULL;

    return cpcnv_temp;
}

char *CodePageHostToGuest(const host_cnv_char_t *s) {
    if (!CodePageHostToGuest((char*)cpcnv_temp,s))
        return NULL;

    return (char*)cpcnv_temp;
}

bool localDrive::FileCreate(DOS_File * * file,const char * name,Bit16u attributes) {
    if (nocachedir) EmptyCache();

    if (readonly) {
		DOS_SetError(DOSERR_WRITE_PROTECTED);
        return false;
    }

    if (attributes & DOS_ATTR_VOLUME) {
        // Real MS-DOS 6.22 and earlier behavior says that creating a volume label
        // without first deleting the existing volume label does nothing but add
        // yet another volume label entry to the root directory and the reported
        // volume label does not change. MS-DOS 7.0 and later appear to have fixed
        // this considering the re-use of bits [3:0] == 0xF to carry LFN entries.
        //
        // Emulate this behavior by setting the volume label ONLY IF there is no
        // volume label. If the DOS application knows how to do it properly it will
        // first issue an FCB delete with attr = DOS_ATTR_VOLUME and ????????.???
        // OR (more common in MS-DOS 6.22 and later) an FCB delete with
        // attr = DOS_ATTR_VOLUME and an explicit copy of the volume label obtained
        // from FCB FindFirst.
        //
        // Volume label handling always affects the root directory.
        //
        // This function is called with file == NULL for volume labels.
        if (*GetLabel() == 0)
            SetLabel(name,false,true);

        return true;
    }

    assert(file != NULL);

//TODO Maybe care for attributes but not likely
	char newname[CROSS_LEN];
	strcpy(newname,basedir);
	strcat(newname,name);
	CROSS_FILENAME(newname);
	char* temp_name = dirCache.GetExpandName(newname); //Can only be used in till a new drive_cache action is preformed */
	/* Test if file exists (so we need to truncate it). don't add to dirCache then */
	bool existing_file=false;

    // guest to host code page translation
    host_cnv_char_t *host_name = CodePageGuestToHost(temp_name);
    if (host_name == NULL) {
        LOG_MSG("%s: Filename '%s' from guest is non-representable on the host filesystem through code page conversion",__FUNCTION__,newname);
		DOS_SetError(DOSERR_FILE_NOT_FOUND); // FIXME
        return false;
    }

#ifdef host_cnv_use_wchar
    FILE * test=_wfopen(host_name,L"rb+");
#else
    FILE * test=fopen(host_name,"rb+");
#endif
	if(test) {
		fclose(test);
		existing_file=true;
	}
	
#ifdef host_cnv_use_wchar
	FILE * hand=_wfopen(host_name,L"wb+");
#else
	FILE * hand=fopen(host_name,"wb+");
#endif
	if (!hand){
		LOG_MSG("Warning: file creation failed: %s",newname);
		return false;
	}
   
	if(!existing_file) {
		strcpy(newname,basedir);
		strcat(newname,name);
		CROSS_FILENAME(newname);
		dirCache.AddEntry(newname, true);
	}

	/* Make the 16 bit device information */
	*file=new localFile(name,hand);
	(*file)->flags=OPEN_READWRITE;

	return true;
}

bool localDrive::FileOpen(DOS_File * * file,const char * name,Bit32u flags) {
    if (nocachedir) EmptyCache();

    if (readonly) {
        if ((flags&0xf) == OPEN_WRITE || (flags&0xf) == OPEN_READWRITE) {
            DOS_SetError(DOSERR_WRITE_PROTECTED);
            return false;
        }
    }

	const host_cnv_char_t * type;
	switch (flags&0xf) {
	case OPEN_READ:        type = _HT("rb");  break;
	case OPEN_WRITE:       type = _HT("rb+"); break;
	case OPEN_READWRITE:   type = _HT("rb+"); break;
	case OPEN_READ_NO_MOD: type = _HT("rb");  break; //No modification of dates. LORD4.07 uses this
	default:
		DOS_SetError(DOSERR_ACCESS_CODE_INVALID);
		return false;
	}
	char newname[CROSS_LEN];
	strcpy(newname,basedir);
	strcat(newname,name);
	CROSS_FILENAME(newname);
	dirCache.ExpandName(newname);

	//Flush the buffer of handles for the same file. (Betrayal in Antara)
	Bit8u i,drive=DOS_DRIVES;
	localFile *lfp;
	for (i=0;i<DOS_DRIVES;i++) {
		if (Drives[i]==this) {
			drive=i;
			break;
		}
	}
	for (i=0;i<DOS_FILES;i++) {
		if (Files[i] && Files[i]->IsOpen() && Files[i]->GetDrive()==drive && Files[i]->IsName(name)) {
			lfp=dynamic_cast<localFile*>(Files[i]);
			if (lfp) lfp->Flush();
		}
	}

    // guest to host code page translation
    host_cnv_char_t *host_name = CodePageGuestToHost(newname);
    if (host_name == NULL) {
        LOG_MSG("%s: Filename '%s' from guest is non-representable on the host filesystem through code page conversion",__FUNCTION__,newname);
		DOS_SetError(DOSERR_FILE_NOT_FOUND);
        return false;
    }

#ifdef host_cnv_use_wchar
	FILE * hand=_wfopen(host_name,type);
#else
	FILE * hand=fopen(host_name,type);
#endif
//	Bit32u err=errno;
	if (!hand) { 
		if((flags&0xf) != OPEN_READ) {
#ifdef host_cnv_use_wchar
			FILE * hmm=_wfopen(host_name,L"rb");
#else
			FILE * hmm=fopen(host_name,"rb");
#endif
			if (hmm) {
				fclose(hmm);
#ifdef host_cnv_use_wchar
				LOG_MSG("Warning: file %ls exists and failed to open in write mode.\nPlease Remove write-protection",host_name);
#else
				LOG_MSG("Warning: file %s exists and failed to open in write mode.\nPlease Remove write-protection",host_name);
#endif
			}
		}
		return false;
	}

	*file=new localFile(name,hand);
	(*file)->flags=flags;  //for the inheritance flag and maybe check for others.
//	(*file)->SetFileName(host_name);
	return true;
}

FILE * localDrive::GetSystemFilePtr(char const * const name, char const * const type) {

	char newname[CROSS_LEN];
	strcpy(newname,basedir);
	strcat(newname,name);
	CROSS_FILENAME(newname);
	dirCache.ExpandName(newname);

    // guest to host code page translation
    host_cnv_char_t *host_name = CodePageGuestToHost(newname);
    if (host_name == NULL) {
        LOG_MSG("%s: Filename '%s' from guest is non-representable on the host filesystem through code page conversion",__FUNCTION__,newname);
        return NULL;
    }

#ifdef host_cnv_use_wchar
    wchar_t wtype[8];
    unsigned int tis;

    // "type" always has ANSI chars (like "rb"), nothing fancy
    for (tis=0;tis < 7 && type[tis] != 0;tis++) wtype[tis] = (wchar_t)type[tis];
    assert(tis < 7); // guard
    wtype[tis] = 0;

	return _wfopen(host_name,wtype);
#else
	return fopen(host_name,type);
#endif
}

bool localDrive::GetSystemFilename(char *sysName, char const * const dosName) {

	strcpy(sysName, basedir);
	strcat(sysName, dosName);
	CROSS_FILENAME(sysName);
	dirCache.ExpandName(sysName);

    // guest to host code page translation
    host_cnv_char_t *host_name = CodePageGuestToHost(sysName);
    if (host_name == NULL) {
        LOG_MSG("%s: Filename '%s' from guest is non-representable on the host filesystem through code page conversion",__FUNCTION__,sysName);
        return false;
    }

#ifdef host_cnv_use_wchar
    // FIXME: GetSystemFilename as implemented cannot return the wide char filename
    return false;
#else
    strcpy(sysName,host_name);
    return true;
#endif
}

bool localDrive::FileUnlink(const char * name) {
    if (readonly) {
        DOS_SetError(DOSERR_WRITE_PROTECTED);
        return false;
    }

	char newname[CROSS_LEN];
	strcpy(newname,basedir);
	strcat(newname,name);
	CROSS_FILENAME(newname);
	char *fullname = dirCache.GetExpandName(newname);

    // guest to host code page translation
    host_cnv_char_t *host_name = CodePageGuestToHost(fullname);
    if (host_name == NULL) {
        LOG_MSG("%s: Filename '%s' from guest is non-representable on the host filesystem through code page conversion",__FUNCTION__,fullname);
		DOS_SetError(DOSERR_FILE_NOT_FOUND);
        return false;
    }

	if (ht_unlink(host_name)) {
		//Unlink failed for some reason try finding it.
		ht_stat_t buffer;
		if(ht_stat(host_name,&buffer)) return false; // File not found.

#ifdef host_cnv_use_wchar
		FILE* file_writable = _wfopen(host_name,L"rb+");
#else
		FILE* file_writable = fopen(host_name,"rb+");
#endif
		if(!file_writable) return false; //No acces ? ERROR MESSAGE NOT SET. FIXME ?
		fclose(file_writable);

		//File exists and can technically be deleted, nevertheless it failed.
		//This means that the file is probably open by some process.
		//See if We have it open.
		bool found_file = false;
		for(Bitu i = 0;i < DOS_FILES;i++){
			if(Files[i] && Files[i]->IsName(name)) {
				Bitu max = DOS_FILES;
				while(Files[i]->IsOpen() && max--) {
					Files[i]->Close();
					if (Files[i]->RemoveRef()<=0) break;
				}
				found_file=true;
			}
		}
		if(!found_file) return false;
		if (!ht_unlink(host_name)) {
			dirCache.DeleteEntry(newname);
			return true;
		}
		return false;
	} else {
		dirCache.DeleteEntry(newname);
		return true;
	}
}

bool localDrive::FindFirst(const char * _dir,DOS_DTA & dta,bool fcb_findfirst) {
	char tempDir[CROSS_LEN];
	strcpy(tempDir,basedir);
	strcat(tempDir,_dir);
	CROSS_FILENAME(tempDir);

    if (nocachedir) EmptyCache();

	if (allocation.mediaid==0xF0 ) {
		EmptyCache(); //rescan floppie-content on each findfirst
	}
    
	
	if (tempDir[strlen(tempDir)-1]!=CROSS_FILESPLIT) {
		char end[2]={CROSS_FILESPLIT,0};
		strcat(tempDir,end);
	}
	
	Bit16u id;
	if (!dirCache.FindFirst(tempDir,id)) {
		DOS_SetError(DOSERR_PATH_NOT_FOUND);
		return false;
	}
	strcpy(srchInfo[id].srch_dir,tempDir);
	dta.SetDirID(id);
	
	Bit8u sAttr;
	dta.GetSearchParams(sAttr,tempDir);

	if (this->isRemote() && this->isRemovable()) {
		// cdroms behave a bit different than regular drives
		if (sAttr == DOS_ATTR_VOLUME) {
			dta.SetResult(dirCache.GetLabel(),0,0,0,DOS_ATTR_VOLUME);
			return true;
		}
	} else {
		if (sAttr == DOS_ATTR_VOLUME) {
			if ( strcmp(dirCache.GetLabel(), "") == 0 ) {
//				LOG(LOG_DOSMISC,LOG_ERROR)("DRIVELABEL REQUESTED: none present, returned  NOLABEL");
//				dta.SetResult("NO_LABEL",0,0,0,DOS_ATTR_VOLUME);
//				return true;
				DOS_SetError(DOSERR_NO_MORE_FILES);
				return false;
			}
			dta.SetResult(dirCache.GetLabel(),0,0,0,DOS_ATTR_VOLUME);
			return true;
		} else if ((sAttr & DOS_ATTR_VOLUME)  && (*_dir == 0) && !fcb_findfirst) { 
		//should check for a valid leading directory instead of 0
		//exists==true if the volume label matches the searchmask and the path is valid
			if (WildFileCmp(dirCache.GetLabel(),tempDir)) {
				dta.SetResult(dirCache.GetLabel(),0,0,0,DOS_ATTR_VOLUME);
				return true;
			}
		}
	}
	return FindNext(dta);
}

char * shiftjis_upcase(char * str);

bool localDrive::FindNext(DOS_DTA & dta) {

	char * dir_ent;
	ht_stat_t stat_block;
	char full_name[CROSS_LEN];
	char dir_entcopy[CROSS_LEN];

	Bit8u srch_attr;char srch_pattern[DOS_NAMELENGTH_ASCII];
	Bit8u find_attr;

	dta.GetSearchParams(srch_attr,srch_pattern);
	Bit16u id = dta.GetDirID();

again:
	if (!dirCache.FindNext(id,dir_ent)) {
		DOS_SetError(DOSERR_NO_MORE_FILES);
		return false;
	}
	if(!WildFileCmp(dir_ent,srch_pattern)) goto again;

	strcpy(full_name,srchInfo[id].srch_dir);
	strcat(full_name,dir_ent);
	
	//GetExpandName might indirectly destroy dir_ent (by caching in a new directory 
	//and due to its design dir_ent might be lost.)
	//Copying dir_ent first
	strcpy(dir_entcopy,dir_ent);

    char *temp_name = dirCache.GetExpandName(full_name);

    // guest to host code page translation
    host_cnv_char_t *host_name = CodePageGuestToHost(temp_name);
    if (host_name == NULL) {
        LOG_MSG("%s: Filename '%s' from guest is non-representable on the host filesystem through code page conversion",__FUNCTION__,temp_name);
		goto again;//No symlinks and such
    }

	if (ht_stat(host_name,&stat_block)!=0) { 
		goto again;//No symlinks and such
	}	

	if(stat_block.st_mode & S_IFDIR) find_attr=DOS_ATTR_DIRECTORY;
	else find_attr=DOS_ATTR_ARCHIVE;
 	if (~srch_attr & find_attr & (DOS_ATTR_DIRECTORY | DOS_ATTR_HIDDEN | DOS_ATTR_SYSTEM)) goto again;
	
	/*file is okay, setup everything to be copied in DTA Block */
	char find_name[DOS_NAMELENGTH_ASCII];Bit16u find_date,find_time;Bit32u find_size;

	if(strlen(dir_entcopy)<DOS_NAMELENGTH_ASCII){
		strcpy(find_name,dir_entcopy);
        upcase(find_name);
    } 

	find_size=(Bit32u) stat_block.st_size;
	struct tm *time;
	if((time=localtime(&stat_block.st_mtime))!=0){
		find_date=DOS_PackDate((Bit16u)(time->tm_year+1900),(Bit16u)(time->tm_mon+1),(Bit16u)time->tm_mday);
		find_time=DOS_PackTime((Bit16u)time->tm_hour,(Bit16u)time->tm_min,(Bit16u)time->tm_sec);
	} else {
		find_time=6; 
		find_date=4;
	}
	dta.SetResult(find_name,find_size,find_date,find_time,find_attr);
	return true;
}

bool localDrive::GetFileAttr(const char * name,Bit16u * attr) {
    if (nocachedir) EmptyCache();

	char newname[CROSS_LEN];
	strcpy(newname,basedir);
	strcat(newname,name);
	CROSS_FILENAME(newname);
	dirCache.ExpandName(newname);

    // guest to host code page translation
    host_cnv_char_t *host_name = CodePageGuestToHost(newname);
    if (host_name == NULL) {
        LOG_MSG("%s: Filename '%s' from guest is non-representable on the host filesystem through code page conversion",__FUNCTION__,newname);
		DOS_SetError(DOSERR_FILE_NOT_FOUND);
        return false;
    }

	ht_stat_t status;
	if (ht_stat(host_name,&status)==0) {
		*attr=DOS_ATTR_ARCHIVE;
		if(status.st_mode & S_IFDIR) *attr|=DOS_ATTR_DIRECTORY;
		return true;
	}
	*attr=0;
	return false; 
}

bool localDrive::MakeDir(const char * dir) {
    if (nocachedir) EmptyCache();

    if (readonly) {
        DOS_SetError(DOSERR_WRITE_PROTECTED);
        return false;
    }

	char newdir[CROSS_LEN];
	strcpy(newdir,basedir);
	strcat(newdir,dir);
	CROSS_FILENAME(newdir);

    char *temp_name = dirCache.GetExpandName(newdir);

    // guest to host code page translation
    host_cnv_char_t *host_name = CodePageGuestToHost(temp_name);
    if (host_name == NULL) {
        LOG_MSG("%s: Filename '%s' from guest is non-representable on the host filesystem through code page conversion",__FUNCTION__,newdir);
		DOS_SetError(DOSERR_FILE_NOT_FOUND); // FIXME
        return false;
    }

#if defined (WIN32)						/* MS Visual C++ */
	int temp=_wmkdir(host_name);
#else
	int temp=mkdir(host_name,0700);
#endif
	if (temp==0) dirCache.CacheOut(newdir,true);

	return (temp==0);// || ((temp!=0) && (errno==EEXIST));
}

bool localDrive::RemoveDir(const char * dir) {
    if (nocachedir) EmptyCache();

    if (readonly) {
        DOS_SetError(DOSERR_WRITE_PROTECTED);
        return false;
    }

	char newdir[CROSS_LEN];
	strcpy(newdir,basedir);
	strcat(newdir,dir);
	CROSS_FILENAME(newdir);

    char *temp_name = dirCache.GetExpandName(newdir);

    // guest to host code page translation
    host_cnv_char_t *host_name = CodePageGuestToHost(temp_name);
    if (host_name == NULL) {
        LOG_MSG("%s: Filename '%s' from guest is non-representable on the host filesystem through code page conversion",__FUNCTION__,newdir);
		DOS_SetError(DOSERR_FILE_NOT_FOUND);
        return false;
    }

#if defined (WIN32)						/* MS Visual C++ */
	int temp=_wrmdir(host_name);
#else
	int temp=rmdir(host_name);
#endif
	if (temp==0) dirCache.DeleteEntry(newdir,true);
	return (temp==0);
}

bool localDrive::TestDir(const char * dir) {
    if (nocachedir) EmptyCache();

	char newdir[CROSS_LEN];
	strcpy(newdir,basedir);
	strcat(newdir,dir);
	CROSS_FILENAME(newdir);
	dirCache.ExpandName(newdir);

    // guest to host code page translation
    host_cnv_char_t *host_name = CodePageGuestToHost(newdir);
    if (host_name == NULL) {
        LOG_MSG("%s: Filename '%s' from guest is non-representable on the host filesystem through code page conversion",__FUNCTION__,newdir);
        return false;
    }

	// Skip directory test, if "\"
	size_t len = strlen(newdir);
	if (len && (newdir[len-1]!='\\')) {
		// It has to be a directory !
		ht_stat_t test;
		if (ht_stat(host_name,&test))		return false;
		if ((test.st_mode & S_IFDIR)==0)	return false;
	}
	int temp=ht_access(host_name,F_OK);
	return (temp==0);
}

bool localDrive::Rename(const char * oldname,const char * newname) {
    if (readonly) {
        DOS_SetError(DOSERR_WRITE_PROTECTED);
        return false;
    }

    host_cnv_char_t *ht;

	char newold[CROSS_LEN];
	strcpy(newold,basedir);
	strcat(newold,oldname);
	CROSS_FILENAME(newold);
	dirCache.ExpandName(newold);
	
	char newnew[CROSS_LEN];
	strcpy(newnew,basedir);
	strcat(newnew,newname);
	CROSS_FILENAME(newnew);
    dirCache.ExpandName(newnew);

    // guest to host code page translation
    ht = CodePageGuestToHost(newold);
    if (ht == NULL) {
        LOG_MSG("%s: Filename '%s' from guest is non-representable on the host filesystem through code page conversion",__FUNCTION__,newold);
		DOS_SetError(DOSERR_FILE_NOT_FOUND);
        return false;
    }
    host_cnv_char_t *o_temp_name = ht_strdup(ht);

    // guest to host code page translation
    ht = CodePageGuestToHost(newnew);
    if (ht == NULL) {
        free(o_temp_name);
        LOG_MSG("%s: Filename '%s' from guest is non-representable on the host filesystem through code page conversion",__FUNCTION__,newnew);
		DOS_SetError(DOSERR_FILE_NOT_FOUND); // FIXME
        return false;
    }
    host_cnv_char_t *n_temp_name = ht_strdup(ht);

#ifdef host_cnv_use_wchar
	int temp=_wrename(o_temp_name,n_temp_name);
#else
	int temp=rename(o_temp_name,n_temp_name);
#endif

	if (temp==0) dirCache.CacheOut(newnew);

    free(o_temp_name);
    free(n_temp_name);

	return (temp==0);

}

bool localDrive::AllocationInfo(Bit16u * _bytes_sector,Bit8u * _sectors_cluster,Bit16u * _total_clusters,Bit16u * _free_clusters) {
	*_bytes_sector=allocation.bytes_sector;
	*_sectors_cluster=allocation.sectors_cluster;
	*_total_clusters=allocation.total_clusters;
	*_free_clusters=allocation.free_clusters;
	return true;
}

bool localDrive::FileExists(const char* name) {
    if (nocachedir) EmptyCache();

	char newname[CROSS_LEN];
	strcpy(newname,basedir);
	strcat(newname,name);
	CROSS_FILENAME(newname);
	dirCache.ExpandName(newname);

    // guest to host code page translation
    host_cnv_char_t *host_name = CodePageGuestToHost(newname);
    if (host_name == NULL) {
        LOG_MSG("%s: Filename '%s' from guest is non-representable on the host filesystem through code page conversion",__FUNCTION__,newname);
        return false;
    }

	ht_stat_t temp_stat;
	if(ht_stat(host_name,&temp_stat)!=0) return false;
	if(temp_stat.st_mode & S_IFDIR) return false;
	return true;
}

bool localDrive::FileStat(const char* name, FileStat_Block * const stat_block) {
    if (nocachedir) EmptyCache();

	char newname[CROSS_LEN];
	strcpy(newname,basedir);
	strcat(newname,name);
	CROSS_FILENAME(newname);
	dirCache.ExpandName(newname);

    // guest to host code page translation
    host_cnv_char_t *host_name = CodePageGuestToHost(newname);
    if (host_name == NULL) {
        LOG_MSG("%s: Filename '%s' from guest is non-representable on the host filesystem through code page conversion",__FUNCTION__,newname);
        return false;
    }

	ht_stat_t temp_stat;
	if(ht_stat(host_name,&temp_stat)!=0) return false;

	/* Convert the stat to a FileStat */
	struct tm *time;
	if((time=localtime(&temp_stat.st_mtime))!=0) {
		stat_block->time=DOS_PackTime((Bit16u)time->tm_hour,(Bit16u)time->tm_min,(Bit16u)time->tm_sec);
		stat_block->date=DOS_PackDate((Bit16u)(time->tm_year+1900),(Bit16u)(time->tm_mon+1),(Bit16u)time->tm_mday);
	}
	stat_block->size=(Bit32u)temp_stat.st_size;
	return true;
}


Bit8u localDrive::GetMediaByte(void) {
	return allocation.mediaid;
}

bool localDrive::isRemote(void) {
	return false;
}

bool localDrive::isRemovable(void) {
	return false;
}

Bits localDrive::UnMount(void) { 
	delete this;
	return 0; 
}

/* helper functions for drive cache */
void *localDrive::opendir(const char *name) {
    // guest to host code page translation
    host_cnv_char_t *host_name = CodePageGuestToHost(name);
    if (host_name == NULL) {
        LOG_MSG("%s: Filename '%s' from guest is non-representable on the host filesystem through code page conversion",__FUNCTION__,name);
        return NULL;
    }

	return open_directoryw(host_name);
}

void localDrive::closedir(void *handle) {
	close_directory((dir_information*)handle);
}

bool localDrive::read_directory_first(void *handle, char* entry_name, bool& is_directory) {
    host_cnv_char_t tmp[MAX_PATH+1];

    if (::read_directory_firstw((dir_information*)handle, tmp, is_directory)) {
        // guest to host code page translation
        char *n_temp_name = CodePageHostToGuest(tmp);
        if (n_temp_name == NULL) {
#ifdef host_cnv_use_wchar
            LOG_MSG("%s: Filename '%ls' from host is non-representable on the guest filesystem through code page conversion",__FUNCTION__,tmp);
#else
            LOG_MSG("%s: Filename '%s' from host is non-representable on the guest filesystem through code page conversion",__FUNCTION__,tmp);
#endif
            return false;
        }
        strcpy(entry_name,n_temp_name);
        return true;
    }

    return false;
}

bool localDrive::read_directory_next(void *handle, char* entry_name, bool& is_directory) {
    host_cnv_char_t tmp[MAX_PATH+1];

next:
    if (::read_directory_nextw((dir_information*)handle, tmp, is_directory)) {
        // guest to host code page translation
        char *n_temp_name = CodePageHostToGuest(tmp);
        if (n_temp_name == NULL) {
#ifdef host_cnv_use_wchar
            LOG_MSG("%s: Filename '%ls' from host is non-representable on the guest filesystem through code page conversion",__FUNCTION__,tmp);
#else
            LOG_MSG("%s: Filename '%s' from host is non-representable on the guest filesystem through code page conversion",__FUNCTION__,tmp);
#endif
            goto next;
        }
        strcpy(entry_name,n_temp_name);
        return true;
    }

    return false;
}

localDrive::localDrive(const char * startdir,Bit16u _bytes_sector,Bit8u _sectors_cluster,Bit16u _total_clusters,Bit16u _free_clusters,Bit8u _mediaid) {
	strcpy(basedir,startdir);
	sprintf(info,"local directory %s",startdir);
	allocation.bytes_sector=_bytes_sector;
	allocation.sectors_cluster=_sectors_cluster;
	allocation.total_clusters=_total_clusters;
	allocation.free_clusters=_free_clusters;
	allocation.mediaid=_mediaid;

	dirCache.SetBaseDir(basedir,this);
}


//TODO Maybe use fflush, but that seemed to fuck up in visual c
bool localFile::Read(Bit8u * data,Bit16u * size) {
	if ((this->flags & 0xf) == OPEN_WRITE) {	// check if file opened in write-only mode
		DOS_SetError(DOSERR_ACCESS_DENIED);
		return false;
	}
	if (last_action==WRITE) fseek(fhandle,ftell(fhandle),SEEK_SET);
	last_action=READ;
	*size=(Bit16u)fread(data,1,*size,fhandle);
	/* Fake harddrive motion. Inspector Gadget with soundblaster compatible */
	/* Same for Igor */
	/* hardrive motion => unmask irq 2. Only do it when it's masked as unmasking is realitively heavy to emulate */
    {
        Bit8u mask = IO_Read(0x21);
        if(mask & 0x4 ) IO_Write(0x21,mask&0xfb);
    }

	return true;
}

bool localFile::Write(const Bit8u * data,Bit16u * size) {
	Bit32u lastflags = this->flags & 0xf;
	if (lastflags == OPEN_READ || lastflags == OPEN_READ_NO_MOD) {	// check if file opened in read-only mode
		DOS_SetError(DOSERR_ACCESS_DENIED);
		return false;
	}
	if (last_action==READ) fseek(fhandle,ftell(fhandle),SEEK_SET);
	last_action=WRITE;
	if(*size==0){  
        return (!ftruncate(fileno(fhandle),ftell(fhandle)));
    }
    else 
    {
		*size=(Bit16u)fwrite(data,1,*size,fhandle);
		return true;
    }
}

/* ert, 20100711: Locking extensions */
#ifdef WIN32
#include <sys/locking.h>
bool localFile::LockFile(Bit8u mode, Bit32u pos, Bit16u size) {
	HANDLE hFile = (HANDLE)_get_osfhandle(_fileno(fhandle));
	BOOL bRet;

	switch (mode)
	{
	case 0: bRet = ::LockFile (hFile, pos, 0, size, 0); break;
	case 1: bRet = ::UnlockFile(hFile, pos, 0, size, 0); break;
	default: 
		DOS_SetError(DOSERR_FUNCTION_NUMBER_INVALID);
		return false;
	}
	//LOG_MSG("::LockFile %s", name);

	if (!bRet)
	{
		switch (GetLastError())
		{
		case ERROR_ACCESS_DENIED:
		case ERROR_LOCK_VIOLATION:
		case ERROR_NETWORK_ACCESS_DENIED:
		case ERROR_DRIVE_LOCKED:
		case ERROR_SEEK_ON_DEVICE:
		case ERROR_NOT_LOCKED:
		case ERROR_LOCK_FAILED:
			DOS_SetError(0x21);
			break;
		case ERROR_INVALID_HANDLE:
			DOS_SetError(DOSERR_INVALID_HANDLE);
			break;
		case ERROR_INVALID_FUNCTION:
		default:
			DOS_SetError(DOSERR_FUNCTION_NUMBER_INVALID);
			break;
		}
	}
	return bRet;
}
#endif

bool localFile::Seek(Bit32u * pos,Bit32u type) {
	int seektype;
	switch (type) {
	case DOS_SEEK_SET:seektype=SEEK_SET;break;
	case DOS_SEEK_CUR:seektype=SEEK_CUR;break;
	case DOS_SEEK_END:seektype=SEEK_END;break;
	default:
	//TODO Give some doserrorcode;
		return false;//ERROR
	}
	int ret=fseek(fhandle,*reinterpret_cast<Bit32s*>(pos),seektype);
	if (ret!=0) {
		// Out of file range, pretend everythings ok 
		// and move file pointer top end of file... ?! (Black Thorne)
		fseek(fhandle,0,SEEK_END);
	}
#if 0
	fpos_t temppos;
	fgetpos(fhandle,&temppos);
	Bit32u * fake_pos=(Bit32u*)&temppos;
	*pos=*fake_pos;
#endif
	*pos=(Bit32u)ftell(fhandle);
	last_action=NONE;
	return true;
}

bool localFile::Close() {
	if (newtime && fhandle) {
        // force STDIO to flush buffers on this file handle, or else fclose() will write buffered data
        // and cause mtime to reset back to current time.
        fflush(fhandle);

 		// backport from DOS_PackDate() and DOS_PackTime()
		struct tm tim = { 0 };
		tim.tm_sec  = (time&0x1f)*2;
		tim.tm_min  = (time>>5)&0x3f;
		tim.tm_hour = (time>>11)&0x1f;
		tim.tm_mday = date&0x1f;
		tim.tm_mon  = ((date>>5)&0x0f)-1;
		tim.tm_year = (date>>9)+1980-1900;
        // sanitize the date in case of invalid timestamps (such as 0x0000 date/time fields)
        if (tim.tm_mon < 0) tim.tm_mon = 0;
        if (tim.tm_mday == 0) tim.tm_mday = 1;
		//  have the C run-time library code compute whether standard time or daylight saving time is in effect.
		tim.tm_isdst = -1;
		// serialize time
		mktime(&tim);

        // change file time by file handle (while we still have it open)
        // so that we do not have to duplicate guest to host filename conversion here.
        // This should help Yksoft1 with file date/time, PC-98, and Shift-JIS Japanese filenames as well on Windows.

#if defined(WIN32) /* TODO: What about MinGW? */
        struct _utimbuf ftim;
        ftim.actime = ftim.modtime = mktime(&tim);

        if (_futime(fileno(fhandle), &ftim)) {
            extern int errno; 
            LOG_MSG("Set time failed (%s)", strerror(errno));
        }
#elif !defined(RISCOS) // Linux (TODO: What about Mac OS X/Darwin?)
        // NTS: Do not attempt futime, Linux doesn't have it.
        //      Do not attempt futimes, Linux man pages LIE about having it. It's even there in the freaking header, but not recognized!
        //      Use futimens. Modern stuff should have it. [https://pubs.opengroup.org/onlinepubs/9699919799/functions/futimens.html]
        struct timespec ftsp[2];
        ftsp[0].tv_sec =  ftsp[1].tv_sec =  mktime(&tim);
        ftsp[0].tv_nsec = ftsp[1].tv_nsec = 0;

        if (futimens(fileno(fhandle), ftsp)) {
            extern int errno; 
            LOG_MSG("Set time failed (%s)", strerror(errno));
        }
#endif
	}

	// only close if one reference left
	if (refCtr==1) {
		if(fhandle) fclose(fhandle); 
		fhandle = 0;
		open = false;
	}

	return true;
}

Bit16u localFile::GetInformation(void) {
	return read_only_medium?0x40:0;
}
	

Bit32u localFile::GetSeekPos() {
	return (Bit32u)ftell( fhandle );
}


localFile::localFile(const char* _name, FILE * handle) {
	fhandle=handle;
	open=true;
	localFile::UpdateDateTimeFromHost();

	attr=DOS_ATTR_ARCHIVE;
	last_action=NONE;
	read_only_medium=false;

	name=0;
	SetName(_name);
}

void localFile::FlagReadOnlyMedium(void) {
	read_only_medium = true;
}

bool localFile::UpdateDateTimeFromHost(void) {
	if(!open) return false;
	struct stat temp_stat;
	fstat(fileno(fhandle),&temp_stat);
	struct tm * ltime;
	if((ltime=localtime(&temp_stat.st_mtime))!=0) {
		time=DOS_PackTime((Bit16u)ltime->tm_hour,(Bit16u)ltime->tm_min,(Bit16u)ltime->tm_sec);
		date=DOS_PackDate((Bit16u)(ltime->tm_year+1900),(Bit16u)(ltime->tm_mon+1),(Bit16u)ltime->tm_mday);
	} else {
		time=1;date=1;
	}
	return true;
}

void localFile::Flush(void) {
	if (last_action==WRITE) {
		fseek(fhandle,ftell(fhandle),SEEK_SET);
        fflush(fhandle);
		last_action=NONE;
	}
}

