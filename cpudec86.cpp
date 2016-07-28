
#ifdef HAVE_CONFIG_H
# include "config.h" // must be first
#endif

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <endian.h>

#include "dosboxxr/lib/cpu/ipdec.h"
#include "dosboxxr/lib/cpu/x86ModRegRm.h"
#include "dosboxxr/lib/cpu/x86ScaleIndexBase.h"
#include "dosboxxr/lib/cpu/memreftypes.h"
#include "dosboxxr/lib/util/case_groups.h"
#include "cpudec86common.h"

void IPDec_8086(x86_offset_t ip);
void IPDec_80286(x86_offset_t ip);
void IPDec_80386(x86_offset_t ip);
void IPDec_80386_generic(x86_offset_t ip);
void IPDec_80486(x86_offset_t ip);
void IPDec_80486_generic(x86_offset_t ip);
void IPDec_Pentium(x86_offset_t ip);
void IPDec_Pentium_generic(x86_offset_t ip);
void IPDec_PentiumPro(x86_offset_t ip);
void IPDec_PentiumPro_generic(x86_offset_t ip);
void IPDec_PentiumMMX(x86_offset_t ip);
void IPDec_PentiumMMX_generic(x86_offset_t ip);
void IPDec_PentiumProMMX(x86_offset_t ip);
void IPDec_PentiumProMMX_generic(x86_offset_t ip);
void IPDec_Pentium2(x86_offset_t ip);
void IPDec_Pentium2_generic(x86_offset_t ip);
void IPDec_Pentium3(x86_offset_t ip);
void IPDec_Pentium3_generic(x86_offset_t ip);

x86_offset_t            exe_ip = 0;
unsigned char*          exe_ip_ptr = NULL;
unsigned char*          exe_image = NULL;
unsigned char*          exe_image_fence = NULL;
bool                    exe_code32 = false;

#include "dosboxxr/lib/cpu/ipdec_pre_core.h"

static void (*IPDec)(x86_offset_t ip) = IPDec_8086;

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

                if (!strcmp(a,"intel8086") || !strcmp(a,"8086") || !strcmp(a,"8088"))
                    IPDec = IPDec_8086;
                else if (!strcmp(a,"intel80286") || !strcmp(a,"intel286") || !strcmp(a,"286"))
                    IPDec = IPDec_80286;
                else if (!strcmp(a,"intel80386") || !strcmp(a,"intel386") || !strcmp(a,"386"))
                    IPDec = IPDec_80386;
                else if (!strcmp(a,"intel80386g") || !strcmp(a,"intel386g") || !strcmp(a,"386g"))
                    IPDec = IPDec_80386_generic;
                else if (!strcmp(a,"intel80486") || !strcmp(a,"intel486") || !strcmp(a,"486"))
                    IPDec = IPDec_80486;
                else if (!strcmp(a,"intel80486g") || !strcmp(a,"intel486g") || !strcmp(a,"486g"))
                    IPDec = IPDec_80486_generic;
                else if (!strcmp(a,"intelpentium") || !strcmp(a,"intel586") || !strcmp(a,"586"))
                    IPDec = IPDec_Pentium;
                else if (!strcmp(a,"intelpentiumg") || !strcmp(a,"intel586g") || !strcmp(a,"586g"))
                    IPDec = IPDec_Pentium_generic;
                else if (!strcmp(a,"intelpentiumpro") || !strcmp(a,"intel586pro") || !strcmp(a,"586pro"))
                    IPDec = IPDec_PentiumPro;
                else if (!strcmp(a,"intelpentiumprog") || !strcmp(a,"intel586prog") || !strcmp(a,"586prog"))
                    IPDec = IPDec_PentiumPro_generic;
                else if (!strcmp(a,"intelpentiummmx") || !strcmp(a,"intel586mmx") || !strcmp(a,"586mmx"))
                    IPDec = IPDec_PentiumMMX;
                else if (!strcmp(a,"intelpentiummmxg") || !strcmp(a,"intel586mmxg") || !strcmp(a,"586mmxg"))
                    IPDec = IPDec_PentiumMMX_generic;
                else if (!strcmp(a,"intelpentiumprommx") || !strcmp(a,"intel586prommx") || !strcmp(a,"586prommx"))
                    IPDec = IPDec_PentiumProMMX;
                else if (!strcmp(a,"intelpentiumprommxg") || !strcmp(a,"intel586prommxg") || !strcmp(a,"586prommxg"))
                    IPDec = IPDec_PentiumProMMX_generic;
                else if (!strcmp(a,"intelpentium2") || !strcmp(a,"intelp2") || !strcmp(a,"686"))
                    IPDec = IPDec_Pentium2;
                else if (!strcmp(a,"intelpentium2g") || !strcmp(a,"intelp2g") || !strcmp(a,"686"))
                    IPDec = IPDec_Pentium2_generic;
                else if (!strcmp(a,"intelpentium3") || !strcmp(a,"intelp3"))
                    IPDec = IPDec_Pentium3;
                else if (!strcmp(a,"intelpentium3g") || !strcmp(a,"intelp3g"))
                    IPDec = IPDec_Pentium3_generic;
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

    fd = open(src_file,O_RDONLY);
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
        unsigned char *i_ptr = exe_ip_ptr;
        unsigned int bip;

        IPDec(exe_ip);
        printf("%04X: ",IPDecIP);

        for (bip=0;bip < (exe_ip-IPDecIP);bip++)
            printf("%02X ",i_ptr[bip]);
        while (bip < 8) {
            printf("   ");
            bip++;
        }

        printf("%s\n",IPDecStr);
    }

    free(exe_image);
    exe_image = NULL;
    exe_image_fence = NULL;
    close(fd);
    return 0;
}

