
#ifdef HAVE_CONFIG_H
# include "config.h" // must be first
#endif

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>

#include "dosboxxr/lib/cpu/x86ModRegRm.h"
#include "dosboxxr/lib/cpu/memreftypes.h"

x86_offset_t            exe_ip = 0;
unsigned char*          exe_ip_ptr = NULL;
unsigned char*          exe_image = NULL;
unsigned char*          exe_image_fence = NULL;

#include <endian.h>

static const char *CPUregs16[8] = {
    "AX","CX","DX","BX", "SP","BP","SI","DI"
};

static const char *CPUregs8[8] = {
    "AL","CL","DL","BL", "AH","CH","DH","BH"
};

//// CPU CORE
#define DECOMPILEMODE

static x86_offset_t IPDecIP;
static char IPDecStr[256];

static inline bool IPcontinue(void) {
    return (exe_ip_ptr < exe_image_fence);
}

static inline uint8_t IPFB(void) {
    const uint8_t r = *((const uint8_t*)exe_ip_ptr);
    exe_ip_ptr += 1;
    exe_ip += 1;
    return r;
}

static inline void IPFModRegRm(x86ModRegRm &m) {
    m.byte = IPFB();
}

static inline uint16_t IPFW(void) {
    const uint16_t r = le16toh(*((const uint16_t*)exe_ip_ptr));
    exe_ip_ptr += 2;
    exe_ip += 2;
    return r;
}

static inline uint32_t IPFDW(void) {
    const uint32_t r = le32toh(*((const uint32_t*)exe_ip_ptr));
    exe_ip_ptr += 4;
    exe_ip += 4;
    return r;
}

void IPDec(x86_offset_t ip) {
    char *w = IPDecStr,*wf = IPDecStr+sizeof(IPDecStr)-1;
    uint8_t op1,v8;
    uint16_t v16;

    {
#ifdef DECOMPILEMODE
        /* one instruction only */
        IPDecStr[0] = 0;
        IPDecIP = ip;
#else
        while (IPcontinue())
#endif
        {
            switch (op1=IPFB()) {
                case 0x40: case 0x41: case 0x42: case 0x43:
                case 0x44: case 0x45: case 0x46: case 0x47:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"INC %s",CPUregs16[op1&7]);
#endif
                    break;

                case 0x48: case 0x49: case 0x4A: case 0x4B:
                case 0x4C: case 0x4D: case 0x4E: case 0x4F:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"DEC %s",CPUregs16[op1&7]);
#endif
                    break;

                case 0x50: case 0x51: case 0x52: case 0x53:
                case 0x54: case 0x55: case 0x56: case 0x57:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"PUSH %s",CPUregs16[op1&7]);
#endif
                    break;

                case 0x58: case 0x59: case 0x5A: case 0x5B:
                case 0x5C: case 0x5D: case 0x5E: case 0x5F:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"POP %s",CPUregs16[op1&7]);
#endif
                    break;

                case 0x90:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"NOP");
#endif
                    break;

                case 0x98:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"CBW");
#endif
                    break;

                case 0x99:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"CWD");
#endif
                    break;

                case 0x9C:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"PUSHF");
#endif
                    break;

                case 0x9D:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"POPF");
#endif
                    break;

                case 0xC2:
                    v16 = IPFW();
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"RET %04xh",v16);
#endif
                    break;

                case 0xC3:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"RET");
#endif
                    break;

                case 0xCA:
                    v16 = IPFW();
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"RETF %04xh",v16);
#endif
                    break;

                case 0xCB:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"RETF");
#endif
                    break;

                case 0xCC:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"INT3");
#endif
                    break;

                case 0xCD:
                    v8 = IPFB();
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"INT %02xh",v8);
#endif
                    break;

                case 0xCF:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"IRET");
#endif
                    break;

                case 0xE4:
                    v8 = IPFB(); // immediate port number
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"IN AL,%02xh",v8);
#endif
                    break;

                case 0xE5:
                    v8 = IPFB(); // immediate port number
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"IN AX,%02xh",v8);
#endif
                    break;

                case 0xE6:
                    v8 = IPFB(); // immediate port number
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"OUT %02xh,AL",v8);
#endif
                    break;

                case 0xE7:
                    v8 = IPFB(); // immediate port number
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"OUT %02xh,AX",v8);
#endif
                    break;

                case 0xEC:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"IN AL,DX");
#endif
                    break;

                case 0xED:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"IN AX,DX");
#endif
                    break;

                case 0xEE:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"OUT DX,AL");
#endif
                    break;

                case 0xEF:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"OUT DX,AX");
#endif
                    break;

                case 0xFA:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"CLI");
#endif
                    break;

                case 0xFB:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"STI");
#endif
                    break;

                default:
#ifdef DECOMPILEMODE
                    w += snprintf(w,(size_t)(wf-w),"(invalid opcode %02xh)",op1);
                    break;
#else
                    goto invalidopcode;
#endif
            }
        }
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
        IPDec(exe_ip);
        printf("%04x: %s\n",IPDecIP,IPDecStr);
    }

    free(exe_image);
    exe_image = NULL;
    exe_image_fence = NULL;
    close(fd);
    return 0;
}

