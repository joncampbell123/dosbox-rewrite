; Everything test instruction set

    cpu     386,pentium4,FPU,Undoc,Obsolete,sse3,ssse3,sse4,Privileged,SVM,Sandybridge
    cpu     Obsolete
    cpu     Undoc

    bits    16
    org     100h

    sti
    cli
    sti
    cld
    std
    clc
    stc
    cld
    stc
    cmc
    cmc

    cli
    nop
    nop
    nop
    mov     ax,1
    mov     bx,2
    mov     cx,3
    mov     dx,4
    mov     sp,5
    mov     bp,6
    mov     si,7
    mov     di,8
    mov     ah,0x11
    mov     al,0x22
    mov     bh,0x33
    mov     bl,0x44
    mov     ch,0x55
    mov     cl,0x66
    mov     dh,0x77
    mov     dl,0x88
    mov     eax,0x11111111
    mov     ebx,0x22222222
    mov     ecx,0x33333333
    mov     edx,0x44444444
    mov     esp,0x55555555
    mov     ebp,0x66666666
    mov     esi,0x77777777
    mov     edi,0x88888888

    lea     ax,[0x1234]
    lea     bx,[0x5678]
    a32 lea     eax,[0x87654321]
    lea     ax,[bx]

    mov     bx,0x89AB
    mov     si,0x4444
    lea     ax,[bx+si]
    lea     ax,[bx+si+0x11]
    lea     ax,[bx+si+0x3211] ; sum to 0

    mov     eax,0x12345678
    lea     ebx,[eax]           ; EAX
    lea     ebx,[eax+eax]       ; EAX*2
    lea     ebx,[eax*2+eax]     ; EAX*3
    lea     ebx,[eax*4]         ; EAX*4
    lea     ebx,[eax*4+eax]     ; EAX*5
    lea     ebx,[eax*8]         ; EAX*8
    lea     ebx,[eax*8+eax]     ; EAX*9
    lea     ebx,[eax+1]
    lea     ebx,[eax+0x12345678]
    lea     ebx,[eax*2+1]
    lea     ebx,[eax*2+0x12345678]
    lea     ebx,[eax*2+eax+1]
    lea     ebx,[eax*2+eax+0x12345678]

    mov     ebx,0x87654321
    lea     ax,[ebx]

    nop

