
#ifdef HAVE_CONFIG_H
# include "config.h" // must be first
#endif

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>

#if HAVE_ENDIAN_H
# include <endian.h>
#else
# include "dosboxxr/lib/util/endian.h"
#endif

#ifndef O_BINARY
#define O_BINARY 0
#endif

#include "dosboxxr/lib/cpu/ipdec.h"
#include "dosboxxr/lib/cpu/x86ModRegRm.h"
#include "dosboxxr/lib/cpu/x86ScaleIndexBase.h"
#include "dosboxxr/lib/cpu/memreftypes.h"
#include "dosboxxr/lib/util/case_groups.h"
#include "cpustep86common.h"

void IPDec_Everything(x86_offset_t ip);
void IPExec_Everything(x86_offset_t ip);

x86_offset_t            exe_ip = 0;
unsigned char*          exe_ip_ptr = NULL;
unsigned char*          exe_image = NULL;
unsigned char*          exe_image_fence = NULL;
bool                    exe_code32 = false;

#include "dosboxxr/lib/cpu/ipdec_pre_core.h"

/*----------------------------------------------*/
x86_cpu_state               cpu;

uint8_t * const cpuref_regs_8[8] = {
    &cpu.reg[CPU_REGI_AX].b.l,
    &cpu.reg[CPU_REGI_CX].b.l,
    &cpu.reg[CPU_REGI_DX].b.l,
    &cpu.reg[CPU_REGI_BX].b.l,
    &cpu.reg[CPU_REGI_AX].b.h,
    &cpu.reg[CPU_REGI_CX].b.h,
    &cpu.reg[CPU_REGI_DX].b.h,
    &cpu.reg[CPU_REGI_BX].b.h
};

uint16_t * const cpuref_regs_16[8] = {
    &cpu.reg[CPU_REGI_AX].w.w,
    &cpu.reg[CPU_REGI_CX].w.w,
    &cpu.reg[CPU_REGI_DX].w.w,
    &cpu.reg[CPU_REGI_BX].w.w,
    &cpu.reg[CPU_REGI_SP].w.w,
    &cpu.reg[CPU_REGI_BP].w.w,
    &cpu.reg[CPU_REGI_SI].w.w,
    &cpu.reg[CPU_REGI_DI].w.w
};

uint32_t * const cpuref_regs_32[8] = {
    &cpu.reg[CPU_REGI_AX].d,
    &cpu.reg[CPU_REGI_CX].d,
    &cpu.reg[CPU_REGI_DX].d,
    &cpu.reg[CPU_REGI_BX].d,
    &cpu.reg[CPU_REGI_SP].d,
    &cpu.reg[CPU_REGI_BP].d,
    &cpu.reg[CPU_REGI_SI].d,
    &cpu.reg[CPU_REGI_DI].d
};
/*----------------------------------------------*/

static void (*IPDec)(x86_offset_t ip) = IPDec_Everything;
static void (*IPExec)(x86_offset_t ip) = IPExec_Everything;

int main(int argc,char **argv) {
    const char *src_file = NULL,*arg;
    off_t file_size;
    int fd;
    int i;

    for (i=1;i < argc;) {
        arg = argv[i++];

        if (*arg == '-') {
            do { arg++; } while (*arg == '-');

            if (!strcmp(arg,"i")) {
                src_file = argv[i++];
            }
            else if (!strcmp(arg,"code32")) {
                exe_code32 = true;
            }
            else if (!strcmp(arg,"cpu")) {
                const char *a = argv[i++];

                if (!strcmp(a,"everything"))
                    IPDec = IPDec_Everything;
                else {
                    fprintf(stderr,"Unknown CPU\n");
                    return 1;
                }
            }
            else {
                fprintf(stderr,"Unknown sw %s\n",arg);
                return 1;
            }
        }
        else {
            fprintf(stderr,"Unexpected arg %s\n",arg);
            return 1;
        }
    }

    if (src_file == NULL) {
        fprintf(stderr,"Must specify file\n");
        return 1;
    }

    fd = open(src_file,O_RDONLY|O_BINARY);
    if (fd < 0) {
        fprintf(stderr,"Cannot open input file\n");
        return 1;
    }
    file_size = lseek(fd,0,SEEK_END);
    if (lseek(fd,0,SEEK_SET) != (off_t)0 || file_size <= (off_t)0 || file_size > (off_t)0xFFFFU) {
        fprintf(stderr,"Bad file size\n");
        return 1;
    }

    exe_image = (unsigned char*)malloc((size_t)file_size + 32);
    if (exe_image == NULL) {
        fprintf(stderr,"Cannot alloc EXE image\n");
        return 1;
    }
    if (read(fd,exe_image,file_size) != (int)file_size) {
        fprintf(stderr,"Cannot read EXE image\n");
        return 1;
    }
    exe_image_fence = exe_image + (size_t)file_size;

    // decompile
    exe_ip = 0x100;
    exe_ip_ptr = exe_image;
    while (exe_ip_ptr < exe_image_fence) {
        x86_offset_t c_exe_ip = exe_ip;
        unsigned char* c_exe_ip_ptr = exe_ip_ptr;

        {
            unsigned char *i_ptr = exe_ip_ptr;
            unsigned int bip;

            printf(">>>> A=0x%08lX B=0x%08lX C=0x%08lX D=0x%08lX SP=0x%08lX BP=0x%08lX SI=0x%08lX DI=0x%08lX F=0x%08lx\n",
                (unsigned long)cpu.eax(),
                (unsigned long)cpu.ebx(),
                (unsigned long)cpu.ecx(),
                (unsigned long)cpu.edx(),
                (unsigned long)cpu.esp(),
                (unsigned long)cpu.ebp(),
                (unsigned long)cpu.esi(),
                (unsigned long)cpu.edi(),
                (unsigned long)cpu.eflags());

            IPDec(exe_ip);
            printf("%04X: ",IPDecIP);

            for (bip=0;bip < (exe_ip-IPDecIP);bip++)
                printf("%02X ",i_ptr[bip]);
            while (bip < 8) {
                printf("   ");
                bip++;
            }

            printf("%s\n",IPDecStr);
        
            exe_ip = c_exe_ip;
            exe_ip_ptr = c_exe_ip_ptr;
        }

        IPExec(exe_ip);
    }

    free(exe_image);
    exe_image = NULL;
    exe_image_fence = NULL;
    close(fd);
    return 0;
}

