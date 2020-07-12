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


#include <vector>
#include <sstream>
#include <ctype.h>
#include <assert.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "programs.h"
#include "callback.h"
#include "regs.h"
#include "support.h"
#include "cross.h"
#include "control.h"
#include "shell.h"

Bitu call_program;

extern bool dos_kernel_disabled;

/* This registers a file on the virtual drive and creates the correct structure for it*/

static Bit8u exe_block[]={
	0xbc,0x00,0x04,					//0x100 MOV SP,0x400  decrease stack size
	0xbb,0x40,0x00,					//0x103 MOV BX,0x0040 for memory resize
	0xb4,0x4a,					//0x106 MOV AH,0x4A   Resize memory block
	0xcd,0x21,					//0x108 INT 0x21      ...
	0x30,0xc0,					//0x10A XOR AL,AL     Clear AL (exit code). Program will write AL to modify exit status
//pos 14 is callback number
	0xFE,0x38,0x00,0x00,				//0x10C CALLBack number
	0xb4,0x4c,					//0x110 Mov AH,0x4C   Prepare to exit, preserve AL
	0xcd,0x21					//0x112 INT 0x21      Exit to DOS
};							//0x114 --DONE--

#define CB_POS 14

class InternalProgramEntry {
public:
	InternalProgramEntry() {
		main = NULL;
		comsize = 0;
		comdata = NULL;
	}
	~InternalProgramEntry() {
		if (comdata != NULL) free(comdata);
		comdata = NULL;
		comsize = 0;
		main = NULL;
	}
public:
	std::string	name;
	Bit8u*		comdata;
	Bit32u		comsize;
	PROGRAMS_Main*	main;
};

static std::vector<InternalProgramEntry*> internal_progs;

void PROGRAMS_Shutdown(void) {
	LOG(LOG_MISC,LOG_DEBUG)("Shutting down internal programs list");

	for (size_t i=0;i < internal_progs.size();i++) {
		if (internal_progs[i] != NULL) {
			delete internal_progs[i];
			internal_progs[i] = NULL;
		}
	}
	internal_progs.clear();
}

void PROGRAMS_MakeFile(char const * const name,PROGRAMS_Main * main) {
	Bit32u size=sizeof(exe_block)+sizeof(Bit8u);
	InternalProgramEntry *ipe;
	Bit8u *comdata;
	Bit8u index;

	/* Copy save the pointer in the vector and save it's index */
	if (internal_progs.size()>255) E_Exit("PROGRAMS_MakeFile program size too large (%d)",static_cast<int>(internal_progs.size()));

	index = (Bit8u)internal_progs.size();
	comdata = (Bit8u *)malloc(32); //MEM LEAK
	memcpy(comdata,&exe_block,sizeof(exe_block));
	memcpy(&comdata[sizeof(exe_block)],&index,sizeof(index));
	comdata[CB_POS]=(Bit8u)(call_program&0xff);
	comdata[CB_POS+1]=(Bit8u)((call_program>>8)&0xff);

	ipe = new InternalProgramEntry();
	ipe->main = main;
	ipe->name = name;
	ipe->comsize = size;
	ipe->comdata = comdata;
	internal_progs.push_back(ipe);
	VFILE_Register(name,ipe->comdata,ipe->comsize);
}

static Bitu PROGRAMS_Handler(void) {
	/* This sets up everything for a program start up call */
	Bitu size=sizeof(Bit8u);
	Bit8u index;
	/* Read the index from program code in memory */
	PhysPt reader=PhysMake(dos.psp(),256+sizeof(exe_block));
	HostPt writer=(HostPt)&index;
	for (;size>0;size--) *writer++=mem_readb(reader++);
	Program * new_program = NULL;
	if (index >= internal_progs.size()) E_Exit("something is messing with the memory");
	InternalProgramEntry *ipe = internal_progs[index];
	if (ipe == NULL) E_Exit("Attempt to run internal program slot with nothing allocated");
	if (ipe->main == NULL) return CBRET_NONE;
	PROGRAMS_Main * handler = internal_progs[index]->main;
	(*handler)(&new_program);

	try { /* "BOOT" can throw an exception (int(2)) */
		new_program->Run();
		delete new_program;
	}
	catch (...) { /* well if it happened, free the program anyway to avert memory leaks */
		delete new_program;
		throw; /* pass it on */
	}

	return CBRET_NONE;
}

/* Main functions used in all program */

Program::Program() {
	/* Find the command line and setup the PSP */
	psp = new DOS_PSP(dos.psp());
	/* Scan environment for filename */
	PhysPt envscan=PhysMake(psp->GetEnvironment(),0);
	while (mem_readb(envscan)) envscan+=(PhysPt)(mem_strlen(envscan)+1);	
	envscan+=3;
	CommandTail tail;
	MEM_BlockRead(PhysMake(dos.psp(),128),&tail,128);
	if (tail.count<127) tail.buffer[tail.count]=0;
	else tail.buffer[126]=0;
	char filename[256+1];
	MEM_StrCopy(envscan,filename,256);
	cmd = new CommandLine(filename,tail.buffer);
	exit_status = 0;
}

extern std::string full_arguments;

void Program::WriteExitStatus() {
	/* the exe block was modified that on return to DOS only AH is modified, leaving AL normally
	 * zero but we're free to set AL to any other value to set exit code */
	reg_al = exit_status;
}

void Program::ChangeToLongCmd() {
	/* 
	 * Get arguments directly from the shell instead of the psp.
	 * this is done in securemode: (as then the arguments to mount and friends
	 * can only be given on the shell ( so no int 21 4b) 
	 * Securemode part is disabled as each of the internal command has already
	 * protection for it. (and it breaks games like cdman)
	 * it is also done for long arguments to as it is convient (as the total commandline can be longer then 127 characters.
	 * imgmount with lot's of parameters
	 * Length of arguments can be ~120. but switch when above 100 to be sure
	 */

	if (/*control->SecureMode() ||*/ cmd->Get_arglength() > 100) {	
		CommandLine* temp = new CommandLine(cmd->GetFileName(),full_arguments.c_str());
		delete cmd;
		cmd = temp;
	}
	full_arguments.assign(""); //Clear so it gets even more save
}

static char last_written_character = 0;//For 0xA to OxD 0xA expansion
void Program::WriteOut(const char * format,...) {
	char buf[2048];
	va_list msg;
	
	va_start(msg,format);
	vsnprintf(buf,2047,format,msg);
	va_end(msg);

	Bit16u size = (Bit16u)strlen(buf);
	dos.internal_output=true;
	for(Bit16u i = 0; i < size;i++) {
		Bit8u out;Bit16u s=1;
		if (buf[i] == 0xA && last_written_character != 0xD) {
			out = 0xD;DOS_WriteFile(STDOUT,&out,&s);
		}
		last_written_character = (char)(out = (Bit8u)buf[i]);
		DOS_WriteFile(STDOUT,&out,&s);
	}
	dos.internal_output=false;
	
//	DOS_WriteFile(STDOUT,(Bit8u *)buf,&size);
}

void Program::WriteOut_NoParsing(const char * format) {
	Bit16u size = (Bit16u)strlen(format);
	char const* buf = format;
	dos.internal_output=true;
	for(Bit16u i = 0; i < size;i++) {
		Bit8u out;Bit16u s=1;
		if (buf[i] == 0xA && last_written_character != 0xD) {
			out = 0xD;DOS_WriteFile(STDOUT,&out,&s);
		}
		last_written_character = (char)(out = (Bit8u)buf[i]);
		DOS_WriteFile(STDOUT,&out,&s);
	}
	dos.internal_output=false;

//	DOS_WriteFile(STDOUT,(Bit8u *)format,&size);
}

static bool LocateEnvironmentBlock(PhysPt &env_base,PhysPt &env_fence,Bitu env_seg) {
	if (env_seg == 0) {
		/* The DOS program might have freed it's environment block perhaps. */
		return false;
	}

	DOS_MCB env_mcb((Bit16u)(env_seg-1)); /* read the environment block's MCB to determine how large it is */
	env_base = PhysMake((Bit16u)env_seg,0);
	env_fence = env_base + (PhysPt)(env_mcb.GetSize() << 4u);
	return true;
}

int EnvPhys_StrCmp(PhysPt es,PhysPt ef,const char *ls) {
    (void)ef;//UNUSED
	while (1) {
		unsigned char a = mem_readb(es++);
		unsigned char b = (unsigned char)(*ls++);
		if (a == '=') a = 0;
		if (a == 0 && b == 0) break;
		if (a == b) continue;
		return (int)a - (int)b;
	}

	return 0;
}

void EnvPhys_StrCpyToCPPString(std::string &result,PhysPt &env_scan,PhysPt env_fence) {
	char tmp[512],*w=tmp,*wf=tmp+sizeof(tmp)-1;

	result.clear();
	while (env_scan < env_fence) {
		char c;
		if ((c=(char)mem_readb(env_scan++)) == 0) break;

		if (w >= wf) {
			*w = 0;
			result += tmp;
			w = tmp;
		}

		assert(w < wf);
		*w++ = c;
	}
	if (w != tmp) {
		*w = 0;
		result += tmp;
	}
}

bool EnvPhys_ScanUntilNextString(PhysPt &env_scan,PhysPt env_fence) {
	/* scan until end of block or NUL */
	while (env_scan < env_fence && mem_readb(env_scan) != 0) env_scan++;

	/* if we hit the fence, that's something to warn about. */
	if (env_scan >= env_fence) {
		LOG_MSG("Warning: environment string scan hit the end of the environment block without terminating NUL\n");
		return false;
	}

	/* if we stopped at anything other than a NUL, that's something to warn about */
	if (mem_readb(env_scan) != 0) {
		LOG_MSG("Warning: environment string scan scan stopped without hitting NUL\n");
		return false;
	}

	env_scan++; /* skip NUL */
	return true;
}

bool Program::GetEnvStr(const char * entry,std::string & result) {
	PhysPt env_base,env_fence,env_scan;

	if (dos_kernel_disabled) {
		LOG_MSG("BUG: Program::GetEnvNum() called with DOS kernel disabled (such as OS boot).\n");
		return false;
	}

	if (!LocateEnvironmentBlock(env_base,env_fence,psp->GetEnvironment())) {
		LOG_MSG("Warning: GetEnvCount() was not able to locate the program's environment block\n");
		return false;
	}

	std::string bigentry(entry);
	for (std::string::iterator it = bigentry.begin(); it != bigentry.end(); ++it) *it = toupper(*it);

	env_scan = env_base;
	while (env_scan < env_fence) {
		/* "NAME" + "=" + "VALUE" + "\0" */
		/* end of the block is a NULL string meaning a \0 follows the last string's \0 */
		if (mem_readb(env_scan) == 0) break; /* normal end of block */

		if (EnvPhys_StrCmp(env_scan,env_fence,bigentry.c_str()) == 0) {
			EnvPhys_StrCpyToCPPString(result,env_scan,env_fence);
			return true;
		}

		if (!EnvPhys_ScanUntilNextString(env_scan,env_fence)) break;
	}

	return false;
}

bool Program::GetEnvNum(Bitu want_num,std::string & result) {
	PhysPt env_base,env_fence,env_scan;
	Bitu num = 0;

	if (dos_kernel_disabled) {
		LOG_MSG("BUG: Program::GetEnvNum() called with DOS kernel disabled (such as OS boot).\n");
		return false;
	}

	if (!LocateEnvironmentBlock(env_base,env_fence,psp->GetEnvironment())) {
		LOG_MSG("Warning: GetEnvCount() was not able to locate the program's environment block\n");
		return false;
	}

	result.clear();
	env_scan = env_base;
	while (env_scan < env_fence) {
		/* "NAME" + "=" + "VALUE" + "\0" */
		/* end of the block is a NULL string meaning a \0 follows the last string's \0 */
		if (mem_readb(env_scan) == 0) break; /* normal end of block */

		if (num == want_num) {
			EnvPhys_StrCpyToCPPString(result,env_scan,env_fence);
			return true;
		}

		num++;
		if (!EnvPhys_ScanUntilNextString(env_scan,env_fence)) break;
	}

	return false;
}

Bitu Program::GetEnvCount(void) {
	PhysPt env_base,env_fence,env_scan;
	Bitu num = 0;

	if (dos_kernel_disabled) {
		LOG_MSG("BUG: Program::GetEnvCount() called with DOS kernel disabled (such as OS boot).\n");
		return 0;
	}

	if (!LocateEnvironmentBlock(env_base,env_fence,psp->GetEnvironment())) {
		LOG_MSG("Warning: GetEnvCount() was not able to locate the program's environment block\n");
		return false;
	}

	env_scan = env_base;
	while (env_scan < env_fence) {
		/* "NAME" + "=" + "VALUE" + "\0" */
		/* end of the block is a NULL string meaning a \0 follows the last string's \0 */
		if (mem_readb(env_scan++) == 0) break; /* normal end of block */
		num++;
		if (!EnvPhys_ScanUntilNextString(env_scan,env_fence)) break;
	}

	return num;
}

void Program::DebugDumpEnv() {
	PhysPt env_base,env_fence,env_scan;
	unsigned char c;
	std::string tmp;

	if (dos_kernel_disabled)
		return;

	if (!LocateEnvironmentBlock(env_base,env_fence,psp->GetEnvironment()))
		return;

	env_scan = env_base;
	LOG_MSG("DebugDumpEnv()");
	while (env_scan < env_fence) {
		if (mem_readb(env_scan) == 0) break;

		while (env_scan < env_fence) {
			if ((c=mem_readb(env_scan++)) == 0) break;
			tmp += (char)c;
		}

		LOG_MSG("...%s",tmp.c_str());
		tmp = "";
	}
}

/* NTS: "entry" string must have already been converted to uppercase */
bool Program::SetEnv(const char * entry,const char * new_string) {
	PhysPt env_base,env_fence,env_scan;
	size_t nsl = 0,el = 0,needs;

	if (dos_kernel_disabled) {
		LOG_MSG("BUG: Program::SetEnv() called with DOS kernel disabled (such as OS boot).\n");
		return false;
	}

	if (!LocateEnvironmentBlock(env_base,env_fence,psp->GetEnvironment())) {
		LOG_MSG("Warning: SetEnv() was not able to locate the program's environment block\n");
		return false;
	}

	std::string bigentry(entry);
	for (std::string::iterator it = bigentry.begin(); it != bigentry.end(); ++it) *it = toupper(*it);

	el = strlen(bigentry.c_str());
	if (*new_string != 0) nsl = strlen(new_string);
	needs = nsl+1+el+1+1; /* entry + '=' + new_string + '\0' + '\0' */

	/* look for the variable in the block. break the loop if found */
	env_scan = env_base;
	while (env_scan < env_fence) {
		if (mem_readb(env_scan) == 0) break;

		if (EnvPhys_StrCmp(env_scan,env_fence,bigentry.c_str()) == 0) {
			/* found it. remove by shifting the rest of the environment block over */
			int zeroes=0;
			PhysPt s,d;

			/* before we remove it: is there room for the new value? */
			if (nsl != 0) {
				if ((env_scan+needs) > env_fence) {
					LOG_MSG("Program::SetEnv() error, insufficient room for environment variable %s=%s (replacement)\n",bigentry.c_str(),new_string);
					DebugDumpEnv();
					return false;
				}
			}

			s = env_scan; d = env_scan;
			while (s < env_fence && mem_readb(s) != 0) s++;
			if (s < env_fence && mem_readb(s) == 0) s++;

			while (s < env_fence) {
				unsigned char b = mem_readb(s++);

				if (b == 0) zeroes++;
				else zeroes=0;

				mem_writeb(d++,b);
				if (zeroes >= 2) break; /* two consecutive zeros means the end of the block */
			}
		}
		else {
			/* scan to next string */
			if (!EnvPhys_ScanUntilNextString(env_scan,env_fence)) break;
		}
	}

	/* At this point, env_scan points to the first byte beyond the block */
	/* add the string to the end of the block */
	if (*new_string != 0) {
		if ((env_scan+needs) > env_fence) {
			LOG_MSG("Program::SetEnv() error, insufficient room for environment variable %s=%s (addition)\n",bigentry.c_str(),new_string);
			DebugDumpEnv();
			return false;
		}

		assert(env_scan < env_fence);
		for (const char *s=bigentry.c_str();*s != 0;) mem_writeb(env_scan++,(Bit8u)(*s++));
		mem_writeb(env_scan++,'=');

		assert(env_scan < env_fence);
		for (const char *s=new_string;*s != 0;) mem_writeb(env_scan++,(Bit8u)(*s++));
		mem_writeb(env_scan++,0);
		mem_writeb(env_scan++,0);

		assert(env_scan <= env_fence);
	}

	return true;
}

bool MSG_Write(const char *);

void PROGRAMS_DOS_Boot(Section *) {
}

/* FIXME: Rename the function to clarify it does not init programs, it inits the callback mechanism
 *        that program generation on drive Z: needs to tie a .COM executable to a callback */
void PROGRAMS_Init() {
	LOG(LOG_MISC,LOG_DEBUG)("PROGRAMS_Init(): initializing Z: drive .COM stub and program management");

	/* Setup a special callback to start virtual programs */
	call_program=CALLBACK_Allocate();
	CALLBACK_Setup(call_program,&PROGRAMS_Handler,CB_RETF,"internal program");

    AddVMEventFunction(VM_EVENT_DOS_INIT_KERNEL_READY,AddVMEventFunctionFuncPair(PROGRAMS_DOS_Boot));

	// listconf
	MSG_Add("PROGRAM_CONFIG_NOCONFIGFILE","No config file loaded!\n");
	MSG_Add("PROGRAM_CONFIG_PRIMARY_CONF","Primary config file: \n%s\n");
	MSG_Add("PROGRAM_CONFIG_ADDITIONAL_CONF","Additional config files:\n");
	MSG_Add("PROGRAM_CONFIG_CONFDIR","DOSBox %s configuration directory: \n%s\n\n");
	
	// writeconf
	MSG_Add("PROGRAM_CONFIG_FILE_ERROR","\nCan't open file %s\n");
	MSG_Add("PROGRAM_CONFIG_FILE_WHICH","Writing config file %s");
	
	// help
	MSG_Add("PROGRAM_CONFIG_USAGE","Config tool:\n"\
		"-writeconf or -wc without parameter: write to primary loaded config file.\n"\
		"-writeconf or -wc with filename: write file to config directory.\n"\
		"Use -writelang or -wl filename to write the current language strings.\n"\
		"-all  Use -all with -wc and -writeconf to write ALL options to the file.\n"\
		"-wcp [filename]\n Write config file to the program directory, dosbox.conf or the specified \n filename.\n"\
		"-wcd\n Write to the default config file in the config directory.\n"\
		"-l lists configuration parameters.\n"\
		"-h, -help, -? sections / sectionname / propertyname\n"\
		" Without parameters, displays this help screen. Add \"sections\" for a list of\n sections."\
		" For info about a specific section or property add its name behind.\n"\
		"-axclear clears the autoexec section.\n"\
		"-axadd [line] adds a line to the autoexec section.\n"\
		"-axtype prints the content of the autoexec section.\n"\
		"-securemode\n"\
        " Switches to secure mode where MOUNT, IMGMOUNT and BOOT will be disabled\n"\
        " as well as the ability to create config and language files.\n"\
		"-get \"section property\" returns the value of the property.\n"\
		"-set \"section property=value\" sets the value." );
	MSG_Add("PROGRAM_CONFIG_HLP_PROPHLP","Purpose of property \"%s\" (contained in section \"%s\"):\n%s\n\nPossible Values: %s\nDefault value: %s\nCurrent value: %s\n");
	MSG_Add("PROGRAM_CONFIG_HLP_LINEHLP","Purpose of section \"%s\":\n%s\nCurrent value:\n%s\n");
	MSG_Add("PROGRAM_CONFIG_HLP_NOCHANGE","This property cannot be changed at runtime.\n");
	MSG_Add("PROGRAM_CONFIG_HLP_POSINT","positive integer"); 
	MSG_Add("PROGRAM_CONFIG_HLP_SECTHLP","Section %s contains the following properties:\n");				
	MSG_Add("PROGRAM_CONFIG_HLP_SECTLIST","DOSBox configuration contains the following sections:\n\n");

	MSG_Add("PROGRAM_CONFIG_SECURE_ON","Switched to secure mode.\n");
	MSG_Add("PROGRAM_CONFIG_SECURE_DISALLOW","This operation is not permitted in secure mode.\n");
	MSG_Add("PROGRAM_CONFIG_SECTION_ERROR","Section %s doesn't exist.\n");
	MSG_Add("PROGRAM_CONFIG_VALUE_ERROR","\"%s\" is not a valid value for property %s.\n");
	MSG_Add("PROGRAM_CONFIG_PROPERTY_ERROR","No such section or property.\n");
	MSG_Add("PROGRAM_CONFIG_NO_PROPERTY","There is no property %s in section %s.\n");
	MSG_Add("PROGRAM_CONFIG_SET_SYNTAX","Correct syntax: config -set \"section property\".\n");
	MSG_Add("PROGRAM_CONFIG_GET_SYNTAX","Correct syntax: config -get \"section property\".\n");
	MSG_Add("PROGRAM_CONFIG_PRINT_STARTUP","\nDOSBox was started with the following command line parameters:\n%s");
	MSG_Add("PROGRAM_CONFIG_MISSINGPARAM","Missing parameter.");
}
