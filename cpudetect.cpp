
#ifdef HAVE_CONFIG_H
# include "config.h" // must be first
#endif

#include <stdio.h>

#include "dosboxxr/lib/hostcpudetect/caps.h"

static const char *yesno[2] = {"no","yes"};

int main() {
    hostCPUcaps.detect();

    fprintf(stdout,"CPU detect results:\n");
    fprintf(stdout,"  Method:           %s\n",hostCPUcaps.detect_method);
#if defined(__i386__) || defined(__x86_64__)
    fprintf(stdout,"  MMX:              %s\n",yesno[hostCPUcaps.mmx?1:0]);
    fprintf(stdout,"  SSE:              %s\n",yesno[hostCPUcaps.sse?1:0]);
    fprintf(stdout,"  SSE2:             %s\n",yesno[hostCPUcaps.sse2?1:0]);
    fprintf(stdout,"  SSE3:             %s\n",yesno[hostCPUcaps.sse3?1:0]);
    fprintf(stdout,"  SSSE3:            %s\n",yesno[hostCPUcaps.ssse3?1:0]);
    fprintf(stdout,"  AVX:              %s\n",yesno[hostCPUcaps.avx?1:0]);
    fprintf(stdout,"  AVX2:             %s\n",yesno[hostCPUcaps.avx2?1:0]);
#endif

    return 0;
}

