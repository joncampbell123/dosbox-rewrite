
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
#include "dosboxxr/lib/cpu/memreftypes.h"
#include "dosboxxr/lib/util/case_groups.h"

x86_offset_t            exe_ip = 0;
unsigned char*          exe_ip_ptr = NULL;
unsigned char*          exe_image = NULL;
unsigned char*          exe_image_fence = NULL;

// include header core requires this
static inline bool IPcontinue(void) {
    return (exe_ip_ptr < exe_image_fence);
}

// include header core requires this
static inline x86_offset_t IPval(void) {
    return exe_ip;
}

// include header core requires this
static inline uint8_t IPFB(void) {
    const uint8_t r = *((const uint8_t*)exe_ip_ptr);
    exe_ip_ptr += 1;
    exe_ip += 1;
    return r;
}

// include header core requires this
static inline uint16_t IPFW(void) {
    const uint16_t r = le16toh(*((const uint16_t*)exe_ip_ptr));
    exe_ip_ptr += 2;
    exe_ip += 2;
    return r;
}

// include header core requires this
static inline uint32_t IPFDW(void) {
    const uint32_t r = le32toh(*((const uint32_t*)exe_ip_ptr));
    exe_ip_ptr += 4;
    exe_ip += 4;
    return r;
}

#include "dosboxxr/lib/cpu/ipdec_pre_core.h"

//// CPU CORE
#define DECOMPILEMODE

static void IPDec_8086(x86_offset_t ip) {
    char *w = IPDecStr,*wf = IPDecStr+sizeof(IPDecStr)-1;
    x86_offset_t disp;
    x86ModRegRm mrm;
    uint8_t op1,v8;
    uint16_t v16b;
    uint16_t v16;

    {
        /* one instruction only */
        IPDecStr[0] = 0;
        IPDecIP = ip;
after_prefix:
#include "dosboxxr/lib/cpu/core/intel8086/coreloop.h"
        goto done;
invalidopcode:
done:
        { }
    }
}
//// END CORE

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

        IPDec_8086(exe_ip);
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

