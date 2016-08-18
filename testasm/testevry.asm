; Everything test instruction set

    cpu     386,pentium4,FPU,Undoc,Obsolete,sse3,ssse3,sse4,Privileged,SVM,Sandybridge
    cpu     Obsolete
    cpu     Undoc

    bits    16
    org     100h

    nop
    cbw
    cwd
    cdq
    cli
    sti
    pushf
    pushfd
    popf
    popfd
    ret
    o32 ret
    a32 ret
    ret     4
    o32 ret 4
    a32 ret 4
    retf
    o32 retf
    a32 retf
    retf    4
    o32 retf 4
    a32 retf 4
    iret
    o32 iret
    a32 iret
    int     1
    int     3
    int3
    int     0x10
    int     0x21
    int     0xAB
    in      al,0x44
    in      al,dx
    in      ax,0x44
    in      ax,dx
    in      eax,0x44
    in      eax,dx
    out     0x44,al
    out     dx,al
    out     0x44,ax
    out     dx,ax
    out     0x44,eax
    out     dx,eax

    inc     ax
    inc     cx
    inc     dx
    inc     bx
    inc     sp
    inc     bp
    inc     si
    inc     di

    dec     ax
    dec     cx
    dec     dx
    dec     bx
    dec     sp
    dec     bp
    dec     si
    dec     di

    inc     eax
    inc     ecx
    inc     edx
    inc     ebx
    inc     esp
    inc     ebp
    inc     esi
    inc     edi

    dec     eax
    dec     ecx
    dec     edx
    dec     ebx
    dec     esp
    dec     ebp
    dec     esi
    dec     edi

    push    ax
    push    cx
    push    dx
    push    bx
    push    sp
    push    bp
    push    si
    push    di

    pop     ax
    pop     cx
    pop     dx
    pop     bx
    pop     sp
    pop     bp
    pop     si
    pop     di

    push    eax
    push    ecx
    push    edx
    push    ebx
    push    esp
    push    ebp
    push    esi
    push    edi

    pop     eax
    pop     ecx
    pop     edx
    pop     ebx
    pop     esp
    pop     ebp
    pop     esi
    pop     edi

    cs
    nop
    ds
    nop
    es
    nop
    ss
    nop

    xlatb
    a32 xlatb

    movsb
    movsw
    movsd
    cmpsb
    cmpsw
    cmpsd
    stosb
    stosw
    stosd
    lodsb
    lodsw
    lodsd
    scasb
    scasw
    scasd

    hlt
    cmc
    nop
    clc
    stc
    cli
    sti
    cld
    std

    daa
    aaa
    das
    aas

    aam
    aad
    aam     0x17
    aad     0x17

    repz
    movsb
    repz
    movsw
    repz
    movsd

    repz
    cmpsb
    repz
    cmpsw
    repz
    cmpsd

    repz
    stosb
    repz
    stosw
    repz
    stosd

    repz
    lodsb
    repz
    lodsw
    repz
    lodsd

    repz
    scasb
    repz
    scasw
    repz
    scasd

    repnz
    movsb
    repnz
    movsw
    repnz
    movsd

    repnz
    cmpsb
    repnz
    cmpsw
    repnz
    cmpsd

    repnz
    stosb
    repnz
    stosw
    repnz
    stosd

    repnz
    lodsb
    repnz
    lodsw
    repnz
    lodsd

    repnz
    scasb
    repnz
    scasw
    repnz
    scasd

    lock    xchg    ax,ax

    xchg    cx,ax
    xchg    dx,ax
    xchg    bx,ax
    xchg    sp,ax
    xchg    bp,ax
    xchg    si,ax
    xchg    di,ax

    xchg    ecx,eax
    xchg    edx,eax
    xchg    ebx,eax
    xchg    esp,eax
    xchg    ebp,eax
    xchg    esi,eax
    xchg    edi,eax

    wait
    sahf
    lahf

    push    es
    pop     es
    push    cs
    push    ss
    pop     ss
    push    ds
    pop     ds
    push    fs
    pop     fs
    push    gs
    pop     gs

    o32 push    es
    o32 pop     es
    o32 push    cs
    o32 push    ss
    o32 pop     ss
    o32 push    ds
    o32 pop     ds
    o32 push    fs
    o32 pop     fs
    o32 push    gs
    o32 pop     gs

jmp1:
    jo      short jmp1
    jno     short jmp1
    jb      short jmp1
    jnb     short jmp1
    jz      short jmp1
    jnz     short jmp1
    jbe     short jmp1
    ja      short jmp1
    js      short jmp1
    jns     short jmp1
    jpe     short jmp1
    jpo     short jmp1
    jl      short jmp1
    jge     short jmp1
    jle     short jmp1
    jg      short jmp1
    jmp     short jmp1

    o32 jo  short jmp1
    o32 jno short jmp1
    o32 jb  short jmp1
    o32 jnb short jmp1
    o32 jz  short jmp1
    o32 jnz short jmp1
    o32 jbe short jmp1
    o32 ja  short jmp1
    o32 js  short jmp1
    o32 jns short jmp1
    o32 jpe short jmp1
    o32 jpo short jmp1
    o32 jl  short jmp1
    o32 jge short jmp1
    o32 jle short jmp1
    o32 jg  short jmp1
    o32 jmp short jmp1

;--------------byte
    add     byte [bx+si],bl ; 00 m/r/m mod=0
    add     byte [bx+di],bl ; 00 m/r/m mod=0
    add     byte [bp+si],bl ; 00 m/r/m mod=0
    add     byte [bp+di],bl ; 00 m/r/m mod=0
    add     byte [si],bl    ; 00 m/r/m mod=0
    add     byte [di],bl    ; 00 m/r/m mod=0
    add     byte [0x1234],bl; 00 m/r/m mod=0
    add     byte [bx],bl    ; 00 m/r/m mod=0

    add     byte [bx+si+0x5F],bl ; 40 m/r/m mod=1
    add     byte [bx+si-0x5F],bl
    add     byte [bx+di+0x5F],bl ; 40 m/r/m mod=1
    add     byte [bx+di-0x5F],bl
    add     byte [bp+si+0x5F],bl ; 40 m/r/m mod=1
    add     byte [bp+si-0x5F],bl
    add     byte [bp+di+0x5F],bl ; 40 m/r/m mod=1
    add     byte [bp+di-0x5F],bl
    add     byte [si+0x5F],bl ; 40 m/r/m mod=1
    add     byte [si-0x5F],bl
    add     byte [di+0x5F],bl ; 40 m/r/m mod=1
    add     byte [di-0x5F],bl
    add     byte [bp+0x5F],bl ; 40 m/r/m mod=1
    add     byte [bp-0x5F],bl
    add     byte [bx+0x5F],bl ; 40 m/r/m mod=1
    add     byte [bx-0x5F],bl

    add     byte [bx+si+0x1234],bl
    add     byte [bx+di+0x1234],bl
    add     byte [bp+si+0x1234],bl
    add     byte [bp+di+0x1234],bl
    add     byte [si+0x1234],bl
    add     byte [di+0x1234],bl
    add     byte [bp+0x1234],bl
    add     byte [bx+0x1234],bl
    add     bl,al           ; 00 m/r/m mod=3
    add     bl,cl           ; 00 m/r/m mod=3
    add     bl,dl           ; 00 m/r/m mod=3
    add     bl,bl           ; 00 m/r/m mod=3
    add     bl,ah
    add     bl,ch
    add     bl,dh
    add     bl,bh

;--------------word
    add     word [bx+si],bx ; 00 m/r/m mod=0
    add     word [bx+di],bx ; 00 m/r/m mod=0
    add     word [bp+si],bx ; 00 m/r/m mod=0
    add     word [bp+di],bx ; 00 m/r/m mod=0
    add     word [si],bx    ; 00 m/r/m mod=0
    add     word [di],bx    ; 00 m/r/m mod=0
    add     word [0x1234],bx; 00 m/r/m mod=0
    add     word [bx],bx    ; 00 m/r/m mod=0

    add     word [bx+si+0x5F],bx ; 40 m/r/m mod=1
    add     word [bx+si-0x5F],bx
    add     word [bx+di+0x5F],bx ; 40 m/r/m mod=1
    add     word [bx+di-0x5F],bx
    add     word [bp+si+0x5F],bx ; 40 m/r/m mod=1
    add     word [bp+si-0x5F],bx
    add     word [bp+di+0x5F],bx ; 40 m/r/m mod=1
    add     word [bp+di-0x5F],bx
    add     word [si+0x5F],bx ; 40 m/r/m mod=1
    add     word [si-0x5F],bx
    add     word [di+0x5F],bx ; 40 m/r/m mod=1
    add     word [di-0x5F],bx
    add     word [bp+0x5F],bx ; 40 m/r/m mod=1
    add     word [bp-0x5F],bx
    add     word [bx+0x5F],bx ; 40 m/r/m mod=1
    add     word [bx-0x5F],bx

    add     word [bx+si+0x1234],bx
    add     word [bx+di+0x1234],bx
    add     word [bp+si+0x1234],bx
    add     word [bp+di+0x1234],bx
    add     word [si+0x1234],bx
    add     word [di+0x1234],bx
    add     word [bp+0x1234],bx
    add     word [bx+0x1234],bx
    add     bx,ax           ; 00 m/r/m mod=3
    add     bx,cx           ; 00 m/r/m mod=3
    add     bx,dx           ; 00 m/r/m mod=3
    add     bx,bx           ; 00 m/r/m mod=3
    add     bx,si
    add     bx,di
    add     bx,bp
    add     bx,sp

;--------------dword
    add     byte [ebx+esi],bl
    add     word [ebx+esi],bx
    add     dword [bx+si],ebx
    add     dword [ebx+esi],ebx ; 00 m/r/m mod=0
    add     dword [ebx+edi],ebx ; 00 m/r/m mod=0
    add     dword [ebp+esi],ebx ; 00 m/r/m mod=0
    add     dword [ebp+edi],ebx ; 00 m/r/m mod=0
    add     dword [esi],ebx    ; 00 m/r/m mod=0
    add     dword [edi],ebx    ; 00 m/r/m mod=0
    a32 add dword [0x12345678],ebx; 00 m/r/m mod=0
    add     dword [ebx],ebx    ; 00 m/r/m mod=0

    add     dword [ebx+esi+0x5F],ebx ; 40 m/r/m mod=1
    add     dword [ebx+esi-0x5F],ebx
    add     dword [ebx+edi+0x5F],ebx ; 40 m/r/m mod=1
    add     dword [ebx+edi-0x5F],ebx
    add     dword [ebp+esi+0x5F],ebx ; 40 m/r/m mod=1
    add     dword [ebp+esi-0x5F],ebx
    add     dword [ebp+edi+0x5F],ebx ; 40 m/r/m mod=1
    add     dword [ebp+edi-0x5F],ebx
    add     dword [esi+0x5F],ebx ; 40 m/r/m mod=1
    add     dword [esi-0x5F],ebx
    add     dword [edi+0x5F],ebx ; 40 m/r/m mod=1
    add     dword [edi-0x5F],ebx
    add     dword [ebp+0x5F],ebx ; 40 m/r/m mod=1
    add     dword [ebp-0x5F],ebx
    add     dword [ebx+0x5F],ebx ; 40 m/r/m mod=1
    add     dword [ebx-0x5F],ebx

    add     dword [ebx+esi+0x1234],ebx
    add     dword [ebx+edi+0x1234],ebx
    add     dword [ebp+esi+0x1234],ebx
    add     dword [ebp+edi+0x1234],ebx
    add     dword [esi+0x1234],ebx
    add     dword [edi+0x1234],ebx
    add     dword [ebp+0x1234],ebx
    add     dword [ebx+0x1234],ebx
    add     ebx,eax           ; 00 m/r/m mod=3
    add     ebx,ecx           ; 00 m/r/m mod=3
    add     ebx,edx           ; 00 m/r/m mod=3
    add     ebx,ebx           ; 00 m/r/m mod=3
    add     ebx,esi
    add     ebx,edi
    add     ebx,ebp
    add     ebx,esp

;--------------byte
    add     bl,byte [bx+si] ; 00 m/r/m mod=0
    add     bl,byte [bx+di] ; 00 m/r/m mod=0
    add     bl,byte [bp+si] ; 00 m/r/m mod=0
    add     bl,byte [bp+di] ; 00 m/r/m mod=0
    add     bl,byte [si]    ; 00 m/r/m mod=0
    add     bl,byte [di]    ; 00 m/r/m mod=0
    add     bl,byte [0x1234]; 00 m/r/m mod=0
    add     bl,byte [bx]    ; 00 m/r/m mod=0

    add     bl,byte [bx+si+0x5F] ; 40 m/r/m mod=1
    add     bl,byte [bx+si-0x5F]
    add     bl,byte [bx+di+0x5F] ; 40 m/r/m mod=1
    add     bl,byte [bx+di-0x5F]
    add     bl,byte [bp+si+0x5F] ; 40 m/r/m mod=1
    add     bl,byte [bp+si-0x5F]
    add     bl,byte [bp+di+0x5F] ; 40 m/r/m mod=1
    add     bl,byte [bp+di-0x5F]
    add     bl,byte [si+0x5F] ; 40 m/r/m mod=1
    add     bl,byte [si-0x5F]
    add     bl,byte [di+0x5F] ; 40 m/r/m mod=1
    add     bl,byte [di-0x5F]
    add     bl,byte [bp+0x5F] ; 40 m/r/m mod=1
    add     bl,byte [bp-0x5F]
    add     bl,byte [bx+0x5F] ; 40 m/r/m mod=1
    add     bl,byte [bx-0x5F]

    add     bl,byte [bx+si+0x1234]
    add     bl,byte [bx+di+0x1234]
    add     bl,byte [bp+si+0x1234]
    add     bl,byte [bp+di+0x1234]
    add     bl,byte [si+0x1234]
    add     bl,byte [di+0x1234]
    add     bl,byte [bp+0x1234]
    add     bl,byte [bx+0x1234]

;--------------word
    add     bx,word [bx+si] ; 00 m/r/m mod=0
    add     bx,word [bx+di] ; 00 m/r/m mod=0
    add     bx,word [bp+si] ; 00 m/r/m mod=0
    add     bx,word [bp+di] ; 00 m/r/m mod=0
    add     bx,word [si]    ; 00 m/r/m mod=0
    add     bx,word [di]    ; 00 m/r/m mod=0
    add     bx,word [0x1234]; 00 m/r/m mod=0
    add     bx,word [bx]    ; 00 m/r/m mod=0

    add     bx,word [bx+si+0x5F] ; 40 m/r/m mod=1
    add     bx,word [bx+si-0x5F]
    add     bx,word [bx+di+0x5F] ; 40 m/r/m mod=1
    add     bx,word [bx+di-0x5F]
    add     bx,word [bp+si+0x5F] ; 40 m/r/m mod=1
    add     bx,word [bp+si-0x5F]
    add     bx,word [bp+di+0x5F] ; 40 m/r/m mod=1
    add     bx,word [bp+di-0x5F]
    add     bx,word [si+0x5F] ; 40 m/r/m mod=1
    add     bx,word [si-0x5F]
    add     bx,word [di+0x5F] ; 40 m/r/m mod=1
    add     bx,word [di-0x5F]
    add     bx,word [bp+0x5F] ; 40 m/r/m mod=1
    add     bx,word [bp-0x5F]
    add     bx,word [bx+0x5F] ; 40 m/r/m mod=1
    add     bx,word [bx-0x5F]

    add     bx,word [bx+si+0x1234]
    add     bx,word [bx+di+0x1234]
    add     bx,word [bp+si+0x1234]
    add     bx,word [bp+di+0x1234]
    add     bx,word [si+0x1234]
    add     bx,word [di+0x1234]
    add     bx,word [bp+0x1234]
    add     bx,word [bx+0x1234]

;--------------dword
    add     ebx,dword [ebx+esi] ; 00 m/r/m mod=0
    add     ebx,dword [ebx+edi] ; 00 m/r/m mod=0
    add     ebx,dword [ebp+esi] ; 00 m/r/m mod=0
    add     ebx,dword [ebp+edi] ; 00 m/r/m mod=0
    add     ebx,dword [esi]    ; 00 m/r/m mod=0
    add     ebx,dword [edi]    ; 00 m/r/m mod=0
    a32 add ebx,dword [0x12345678]; 00 m/r/m mod=0
    add     ebx,dword [ebx]    ; 00 m/r/m mod=0

    add     ebx,dword [ebx+esi+0x5F] ; 40 m/r/m mod=1
    add     ebx,dword [ebx+esi-0x5F]
    add     ebx,dword [ebx+edi+0x5F] ; 40 m/r/m mod=1
    add     ebx,dword [ebx+edi-0x5F]
    add     ebx,dword [ebp+esi+0x5F] ; 40 m/r/m mod=1
    add     ebx,dword [ebp+esi-0x5F]
    add     ebx,dword [ebp+edi+0x5F] ; 40 m/r/m mod=1
    add     ebx,dword [ebp+edi-0x5F]
    add     ebx,dword [esi+0x5F] ; 40 m/r/m mod=1
    add     ebx,dword [esi-0x5F]
    add     ebx,dword [edi+0x5F] ; 40 m/r/m mod=1
    add     ebx,dword [edi-0x5F]
    add     ebx,dword [ebp+0x5F] ; 40 m/r/m mod=1
    add     ebx,dword [ebp-0x5F]
    add     ebx,dword [ebx+0x5F] ; 40 m/r/m mod=1
    add     ebx,dword [ebx-0x5F]

    add     ebx,dword [ebx+esi+0x1234]
    add     ebx,dword [ebx+edi+0x1234]
    add     ebx,dword [ebp+esi+0x1234]
    add     ebx,dword [ebp+edi+0x1234]
    add     ebx,dword [esi+0x1234]
    add     ebx,dword [edi+0x1234]
    add     ebx,dword [ebp+0x1234]
    add     ebx,dword [ebx+0x1234]

;---------------add imm
    add     al,0x12
    add     al,0xEF
    add     ax,0x1234
    add     ax,0xFEDC
    add     eax,0x12345678
    add     eax,0xFEDCBA98

;--------------byte
    or      byte [bx+si],bl ; 00 m/r/m mod=0
    or      byte [bx+di],bl ; 00 m/r/m mod=0
    or      byte [bp+si],bl ; 00 m/r/m mod=0
    or      byte [bp+di],bl ; 00 m/r/m mod=0
    or      byte [si],bl    ; 00 m/r/m mod=0
    or      byte [di],bl    ; 00 m/r/m mod=0
    or      byte [0x1234],bl; 00 m/r/m mod=0
    or      byte [bx],bl    ; 00 m/r/m mod=0

    or      byte [bx+si+0x5F],bl ; 40 m/r/m mod=1
    or      byte [bx+si-0x5F],bl
    or      byte [bx+di+0x5F],bl ; 40 m/r/m mod=1
    or      byte [bx+di-0x5F],bl
    or      byte [bp+si+0x5F],bl ; 40 m/r/m mod=1
    or      byte [bp+si-0x5F],bl
    or      byte [bp+di+0x5F],bl ; 40 m/r/m mod=1
    or      byte [bp+di-0x5F],bl
    or      byte [si+0x5F],bl ; 40 m/r/m mod=1
    or      byte [si-0x5F],bl
    or      byte [di+0x5F],bl ; 40 m/r/m mod=1
    or      byte [di-0x5F],bl
    or      byte [bp+0x5F],bl ; 40 m/r/m mod=1
    or      byte [bp-0x5F],bl
    or      byte [bx+0x5F],bl ; 40 m/r/m mod=1
    or      byte [bx-0x5F],bl

    or      byte [bx+si+0x1234],bl
    or      byte [bx+di+0x1234],bl
    or      byte [bp+si+0x1234],bl
    or      byte [bp+di+0x1234],bl
    or      byte [si+0x1234],bl
    or      byte [di+0x1234],bl
    or      byte [bp+0x1234],bl
    or      byte [bx+0x1234],bl
    or      bl,al           ; 00 m/r/m mod=3
    or      bl,cl           ; 00 m/r/m mod=3
    or      bl,dl           ; 00 m/r/m mod=3
    or      bl,bl           ; 00 m/r/m mod=3
    or      bl,ah
    or      bl,ch
    or      bl,dh
    or      bl,bh

;--------------word
    or      word [bx+si],bx ; 00 m/r/m mod=0
    or      word [bx+di],bx ; 00 m/r/m mod=0
    or      word [bp+si],bx ; 00 m/r/m mod=0
    or      word [bp+di],bx ; 00 m/r/m mod=0
    or      word [si],bx    ; 00 m/r/m mod=0
    or      word [di],bx    ; 00 m/r/m mod=0
    or      word [0x1234],bx; 00 m/r/m mod=0
    or      word [bx],bx    ; 00 m/r/m mod=0

    or      word [bx+si+0x5F],bx ; 40 m/r/m mod=1
    or      word [bx+si-0x5F],bx
    or      word [bx+di+0x5F],bx ; 40 m/r/m mod=1
    or      word [bx+di-0x5F],bx
    or      word [bp+si+0x5F],bx ; 40 m/r/m mod=1
    or      word [bp+si-0x5F],bx
    or      word [bp+di+0x5F],bx ; 40 m/r/m mod=1
    or      word [bp+di-0x5F],bx
    or      word [si+0x5F],bx ; 40 m/r/m mod=1
    or      word [si-0x5F],bx
    or      word [di+0x5F],bx ; 40 m/r/m mod=1
    or      word [di-0x5F],bx
    or      word [bp+0x5F],bx ; 40 m/r/m mod=1
    or      word [bp-0x5F],bx
    or      word [bx+0x5F],bx ; 40 m/r/m mod=1
    or      word [bx-0x5F],bx

    or      word [bx+si+0x1234],bx
    or      word [bx+di+0x1234],bx
    or      word [bp+si+0x1234],bx
    or      word [bp+di+0x1234],bx
    or      word [si+0x1234],bx
    or      word [di+0x1234],bx
    or      word [bp+0x1234],bx
    or      word [bx+0x1234],bx
    or      bx,ax           ; 00 m/r/m mod=3
    or      bx,cx           ; 00 m/r/m mod=3
    or      bx,dx           ; 00 m/r/m mod=3
    or      bx,bx           ; 00 m/r/m mod=3
    or      bx,si
    or      bx,di
    or      bx,bp
    or      bx,sp

;--------------dword
    or      dword [ebx+esi],ebx ; 00 m/r/m mod=0
    or      dword [ebx+edi],ebx ; 00 m/r/m mod=0
    or      dword [ebp+esi],ebx ; 00 m/r/m mod=0
    or      dword [ebp+edi],ebx ; 00 m/r/m mod=0
    or      dword [esi],ebx    ; 00 m/r/m mod=0
    or      dword [edi],ebx    ; 00 m/r/m mod=0
    a32 or  dword [0x12345678],ebx; 00 m/r/m mod=0
    or      dword [ebx],ebx    ; 00 m/r/m mod=0

    or      dword [ebx+esi+0x5F],ebx ; 40 m/r/m mod=1
    or      dword [ebx+esi-0x5F],ebx
    or      dword [ebx+edi+0x5F],ebx ; 40 m/r/m mod=1
    or      dword [ebx+edi-0x5F],ebx
    or      dword [ebp+esi+0x5F],ebx ; 40 m/r/m mod=1
    or      dword [ebp+esi-0x5F],ebx
    or      dword [ebp+edi+0x5F],ebx ; 40 m/r/m mod=1
    or      dword [ebp+edi-0x5F],ebx
    or      dword [esi+0x5F],ebx ; 40 m/r/m mod=1
    or      dword [esi-0x5F],ebx
    or      dword [edi+0x5F],ebx ; 40 m/r/m mod=1
    or      dword [edi-0x5F],ebx
    or      dword [ebp+0x5F],ebx ; 40 m/r/m mod=1
    or      dword [ebp-0x5F],ebx
    or      dword [ebx+0x5F],ebx ; 40 m/r/m mod=1
    or      dword [ebx-0x5F],ebx

    or      dword [ebx+esi+0x1234],ebx
    or      dword [ebx+edi+0x1234],ebx
    or      dword [ebp+esi+0x1234],ebx
    or      dword [ebp+edi+0x1234],ebx
    or      dword [esi+0x1234],ebx
    or      dword [edi+0x1234],ebx
    or      dword [ebp+0x1234],ebx
    or      dword [ebx+0x1234],ebx
    or      ebx,eax           ; 00 m/r/m mod=3
    or      ebx,ecx           ; 00 m/r/m mod=3
    or      ebx,edx           ; 00 m/r/m mod=3
    or      ebx,ebx           ; 00 m/r/m mod=3
    or      ebx,esi
    or      ebx,edi
    or      ebx,ebp
    or      ebx,esp

;--------------byte
    or      bl,byte [bx+si] ; 00 m/r/m mod=0
    or      bl,byte [bx+di] ; 00 m/r/m mod=0
    or      bl,byte [bp+si] ; 00 m/r/m mod=0
    or      bl,byte [bp+di] ; 00 m/r/m mod=0
    or      bl,byte [si]    ; 00 m/r/m mod=0
    or      bl,byte [di]    ; 00 m/r/m mod=0
    or      bl,byte [0x1234]; 00 m/r/m mod=0
    or      bl,byte [bx]    ; 00 m/r/m mod=0

    or      bl,byte [bx+si+0x5F] ; 40 m/r/m mod=1
    or      bl,byte [bx+si-0x5F]
    or      bl,byte [bx+di+0x5F] ; 40 m/r/m mod=1
    or      bl,byte [bx+di-0x5F]
    or      bl,byte [bp+si+0x5F] ; 40 m/r/m mod=1
    or      bl,byte [bp+si-0x5F]
    or      bl,byte [bp+di+0x5F] ; 40 m/r/m mod=1
    or      bl,byte [bp+di-0x5F]
    or      bl,byte [si+0x5F] ; 40 m/r/m mod=1
    or      bl,byte [si-0x5F]
    or      bl,byte [di+0x5F] ; 40 m/r/m mod=1
    or      bl,byte [di-0x5F]
    or      bl,byte [bp+0x5F] ; 40 m/r/m mod=1
    or      bl,byte [bp-0x5F]
    or      bl,byte [bx+0x5F] ; 40 m/r/m mod=1
    or      bl,byte [bx-0x5F]

    or      bl,byte [bx+si+0x1234]
    or      bl,byte [bx+di+0x1234]
    or      bl,byte [bp+si+0x1234]
    or      bl,byte [bp+di+0x1234]
    or      bl,byte [si+0x1234]
    or      bl,byte [di+0x1234]
    or      bl,byte [bp+0x1234]
    or      bl,byte [bx+0x1234]

;--------------word
    or      bx,word [bx+si] ; 00 m/r/m mod=0
    or      bx,word [bx+di] ; 00 m/r/m mod=0
    or      bx,word [bp+si] ; 00 m/r/m mod=0
    or      bx,word [bp+di] ; 00 m/r/m mod=0
    or      bx,word [si]    ; 00 m/r/m mod=0
    or      bx,word [di]    ; 00 m/r/m mod=0
    or      bx,word [0x1234]; 00 m/r/m mod=0
    or      bx,word [bx]    ; 00 m/r/m mod=0

    or      bx,word [bx+si+0x5F] ; 40 m/r/m mod=1
    or      bx,word [bx+si-0x5F]
    or      bx,word [bx+di+0x5F] ; 40 m/r/m mod=1
    or      bx,word [bx+di-0x5F]
    or      bx,word [bp+si+0x5F] ; 40 m/r/m mod=1
    or      bx,word [bp+si-0x5F]
    or      bx,word [bp+di+0x5F] ; 40 m/r/m mod=1
    or      bx,word [bp+di-0x5F]
    or      bx,word [si+0x5F] ; 40 m/r/m mod=1
    or      bx,word [si-0x5F]
    or      bx,word [di+0x5F] ; 40 m/r/m mod=1
    or      bx,word [di-0x5F]
    or      bx,word [bp+0x5F] ; 40 m/r/m mod=1
    or      bx,word [bp-0x5F]
    or      bx,word [bx+0x5F] ; 40 m/r/m mod=1
    or      bx,word [bx-0x5F]

    or      bx,word [bx+si+0x1234]
    or      bx,word [bx+di+0x1234]
    or      bx,word [bp+si+0x1234]
    or      bx,word [bp+di+0x1234]
    or      bx,word [si+0x1234]
    or      bx,word [di+0x1234]
    or      bx,word [bp+0x1234]
    or      bx,word [bx+0x1234]

;--------------dword
    or      ebx,dword [ebx+esi] ; 00 m/r/m mod=0
    or      ebx,dword [ebx+edi] ; 00 m/r/m mod=0
    or      ebx,dword [ebp+esi] ; 00 m/r/m mod=0
    or      ebx,dword [ebp+edi] ; 00 m/r/m mod=0
    or      ebx,dword [esi]    ; 00 m/r/m mod=0
    or      ebx,dword [edi]    ; 00 m/r/m mod=0
    a32 or  ebx,dword [0x12345678]; 00 m/r/m mod=0
    or      ebx,dword [ebx]    ; 00 m/r/m mod=0

    or      ebx,dword [ebx+esi+0x5F] ; 40 m/r/m mod=1
    or      ebx,dword [ebx+esi-0x5F]
    or      ebx,dword [ebx+edi+0x5F] ; 40 m/r/m mod=1
    or      ebx,dword [ebx+edi-0x5F]
    or      ebx,dword [ebp+esi+0x5F] ; 40 m/r/m mod=1
    or      ebx,dword [ebp+esi-0x5F]
    or      ebx,dword [ebp+edi+0x5F] ; 40 m/r/m mod=1
    or      ebx,dword [ebp+edi-0x5F]
    or      ebx,dword [esi+0x5F] ; 40 m/r/m mod=1
    or      ebx,dword [esi-0x5F]
    or      ebx,dword [edi+0x5F] ; 40 m/r/m mod=1
    or      ebx,dword [edi-0x5F]
    or      ebx,dword [ebp+0x5F] ; 40 m/r/m mod=1
    or      ebx,dword [ebp-0x5F]
    or      ebx,dword [ebx+0x5F] ; 40 m/r/m mod=1
    or      ebx,dword [ebx-0x5F]

    or      ebx,dword [ebx+esi+0x1234]
    or      ebx,dword [ebx+edi+0x1234]
    or      ebx,dword [ebp+esi+0x1234]
    or      ebx,dword [ebp+edi+0x1234]
    or      ebx,dword [esi+0x1234]
    or      ebx,dword [edi+0x1234]
    or      ebx,dword [ebp+0x1234]
    or      ebx,dword [ebx+0x1234]

;---------------or  imm
    or      al,0x12
    or      al,0xEF
    or      ax,0x1234
    or      ax,0xFEDC
    or      eax,0x12345678
    or      eax,0xFEDCBA98

;--------------byte
    adc     byte [bx+si],bl ; 00 m/r/m mod=0
    adc     byte [bx+di],bl ; 00 m/r/m mod=0
    adc     byte [bp+si],bl ; 00 m/r/m mod=0
    adc     byte [bp+di],bl ; 00 m/r/m mod=0
    adc     byte [si],bl    ; 00 m/r/m mod=0
    adc     byte [di],bl    ; 00 m/r/m mod=0
    adc     byte [0x1234],bl; 00 m/r/m mod=0
    adc     byte [bx],bl    ; 00 m/r/m mod=0

    adc     byte [bx+si+0x5F],bl ; 40 m/r/m mod=1
    adc     byte [bx+si-0x5F],bl
    adc     byte [bx+di+0x5F],bl ; 40 m/r/m mod=1
    adc     byte [bx+di-0x5F],bl
    adc     byte [bp+si+0x5F],bl ; 40 m/r/m mod=1
    adc     byte [bp+si-0x5F],bl
    adc     byte [bp+di+0x5F],bl ; 40 m/r/m mod=1
    adc     byte [bp+di-0x5F],bl
    adc     byte [si+0x5F],bl ; 40 m/r/m mod=1
    adc     byte [si-0x5F],bl
    adc     byte [di+0x5F],bl ; 40 m/r/m mod=1
    adc     byte [di-0x5F],bl
    adc     byte [bp+0x5F],bl ; 40 m/r/m mod=1
    adc     byte [bp-0x5F],bl
    adc     byte [bx+0x5F],bl ; 40 m/r/m mod=1
    adc     byte [bx-0x5F],bl

    adc     byte [bx+si+0x1234],bl
    adc     byte [bx+di+0x1234],bl
    adc     byte [bp+si+0x1234],bl
    adc     byte [bp+di+0x1234],bl
    adc     byte [si+0x1234],bl
    adc     byte [di+0x1234],bl
    adc     byte [bp+0x1234],bl
    adc     byte [bx+0x1234],bl
    adc     bl,al           ; 00 m/r/m mod=3
    adc     bl,cl           ; 00 m/r/m mod=3
    adc     bl,dl           ; 00 m/r/m mod=3
    adc     bl,bl           ; 00 m/r/m mod=3
    adc     bl,ah
    adc     bl,ch
    adc     bl,dh
    adc     bl,bh

;--------------word
    adc     word [bx+si],bx ; 00 m/r/m mod=0
    adc     word [bx+di],bx ; 00 m/r/m mod=0
    adc     word [bp+si],bx ; 00 m/r/m mod=0
    adc     word [bp+di],bx ; 00 m/r/m mod=0
    adc     word [si],bx    ; 00 m/r/m mod=0
    adc     word [di],bx    ; 00 m/r/m mod=0
    adc     word [0x1234],bx; 00 m/r/m mod=0
    adc     word [bx],bx    ; 00 m/r/m mod=0

    adc     word [bx+si+0x5F],bx ; 40 m/r/m mod=1
    adc     word [bx+si-0x5F],bx
    adc     word [bx+di+0x5F],bx ; 40 m/r/m mod=1
    adc     word [bx+di-0x5F],bx
    adc     word [bp+si+0x5F],bx ; 40 m/r/m mod=1
    adc     word [bp+si-0x5F],bx
    adc     word [bp+di+0x5F],bx ; 40 m/r/m mod=1
    adc     word [bp+di-0x5F],bx
    adc     word [si+0x5F],bx ; 40 m/r/m mod=1
    adc     word [si-0x5F],bx
    adc     word [di+0x5F],bx ; 40 m/r/m mod=1
    adc     word [di-0x5F],bx
    adc     word [bp+0x5F],bx ; 40 m/r/m mod=1
    adc     word [bp-0x5F],bx
    adc     word [bx+0x5F],bx ; 40 m/r/m mod=1
    adc     word [bx-0x5F],bx

    adc     word [bx+si+0x1234],bx
    adc     word [bx+di+0x1234],bx
    adc     word [bp+si+0x1234],bx
    adc     word [bp+di+0x1234],bx
    adc     word [si+0x1234],bx
    adc     word [di+0x1234],bx
    adc     word [bp+0x1234],bx
    adc     word [bx+0x1234],bx
    adc     bx,ax           ; 00 m/r/m mod=3
    adc     bx,cx           ; 00 m/r/m mod=3
    adc     bx,dx           ; 00 m/r/m mod=3
    adc     bx,bx           ; 00 m/r/m mod=3
    adc     bx,si
    adc     bx,di
    adc     bx,bp
    adc     bx,sp

;--------------dword
    adc     dword [ebx+esi],ebx ; 00 m/r/m mod=0
    adc     dword [ebx+edi],ebx ; 00 m/r/m mod=0
    adc     dword [ebp+esi],ebx ; 00 m/r/m mod=0
    adc     dword [ebp+edi],ebx ; 00 m/r/m mod=0
    adc     dword [esi],ebx    ; 00 m/r/m mod=0
    adc     dword [edi],ebx    ; 00 m/r/m mod=0
    a32 adc dword [0x12345678],ebx; 00 m/r/m mod=0
    adc     dword [ebx],ebx    ; 00 m/r/m mod=0

    adc     dword [ebx+esi+0x5F],ebx ; 40 m/r/m mod=1
    adc     dword [ebx+esi-0x5F],ebx
    adc     dword [ebx+edi+0x5F],ebx ; 40 m/r/m mod=1
    adc     dword [ebx+edi-0x5F],ebx
    adc     dword [ebp+esi+0x5F],ebx ; 40 m/r/m mod=1
    adc     dword [ebp+esi-0x5F],ebx
    adc     dword [ebp+edi+0x5F],ebx ; 40 m/r/m mod=1
    adc     dword [ebp+edi-0x5F],ebx
    adc     dword [esi+0x5F],ebx ; 40 m/r/m mod=1
    adc     dword [esi-0x5F],ebx
    adc     dword [edi+0x5F],ebx ; 40 m/r/m mod=1
    adc     dword [edi-0x5F],ebx
    adc     dword [ebp+0x5F],ebx ; 40 m/r/m mod=1
    adc     dword [ebp-0x5F],ebx
    adc     dword [ebx+0x5F],ebx ; 40 m/r/m mod=1
    adc     dword [ebx-0x5F],ebx

    adc     dword [ebx+esi+0x1234],ebx
    adc     dword [ebx+edi+0x1234],ebx
    adc     dword [ebp+esi+0x1234],ebx
    adc     dword [ebp+edi+0x1234],ebx
    adc     dword [esi+0x1234],ebx
    adc     dword [edi+0x1234],ebx
    adc     dword [ebp+0x1234],ebx
    adc     dword [ebx+0x1234],ebx
    adc     ebx,eax           ; 00 m/r/m mod=3
    adc     ebx,ecx           ; 00 m/r/m mod=3
    adc     ebx,edx           ; 00 m/r/m mod=3
    adc     ebx,ebx           ; 00 m/r/m mod=3
    adc     ebx,esi
    adc     ebx,edi
    adc     ebx,ebp
    adc     ebx,esp

;--------------byte
    adc     bl,byte [bx+si] ; 00 m/r/m mod=0
    adc     bl,byte [bx+di] ; 00 m/r/m mod=0
    adc     bl,byte [bp+si] ; 00 m/r/m mod=0
    adc     bl,byte [bp+di] ; 00 m/r/m mod=0
    adc     bl,byte [si]    ; 00 m/r/m mod=0
    adc     bl,byte [di]    ; 00 m/r/m mod=0
    adc     bl,byte [0x1234]; 00 m/r/m mod=0
    adc     bl,byte [bx]    ; 00 m/r/m mod=0

    adc     bl,byte [bx+si+0x5F] ; 40 m/r/m mod=1
    adc     bl,byte [bx+si-0x5F]
    adc     bl,byte [bx+di+0x5F] ; 40 m/r/m mod=1
    adc     bl,byte [bx+di-0x5F]
    adc     bl,byte [bp+si+0x5F] ; 40 m/r/m mod=1
    adc     bl,byte [bp+si-0x5F]
    adc     bl,byte [bp+di+0x5F] ; 40 m/r/m mod=1
    adc     bl,byte [bp+di-0x5F]
    adc     bl,byte [si+0x5F] ; 40 m/r/m mod=1
    adc     bl,byte [si-0x5F]
    adc     bl,byte [di+0x5F] ; 40 m/r/m mod=1
    adc     bl,byte [di-0x5F]
    adc     bl,byte [bp+0x5F] ; 40 m/r/m mod=1
    adc     bl,byte [bp-0x5F]
    adc     bl,byte [bx+0x5F] ; 40 m/r/m mod=1
    adc     bl,byte [bx-0x5F]

    adc     bl,byte [bx+si+0x1234]
    adc     bl,byte [bx+di+0x1234]
    adc     bl,byte [bp+si+0x1234]
    adc     bl,byte [bp+di+0x1234]
    adc     bl,byte [si+0x1234]
    adc     bl,byte [di+0x1234]
    adc     bl,byte [bp+0x1234]
    adc     bl,byte [bx+0x1234]

;--------------word
    adc     bx,word [bx+si] ; 00 m/r/m mod=0
    adc     bx,word [bx+di] ; 00 m/r/m mod=0
    adc     bx,word [bp+si] ; 00 m/r/m mod=0
    adc     bx,word [bp+di] ; 00 m/r/m mod=0
    adc     bx,word [si]    ; 00 m/r/m mod=0
    adc     bx,word [di]    ; 00 m/r/m mod=0
    adc     bx,word [0x1234]; 00 m/r/m mod=0
    adc     bx,word [bx]    ; 00 m/r/m mod=0

    adc     bx,word [bx+si+0x5F] ; 40 m/r/m mod=1
    adc     bx,word [bx+si-0x5F]
    adc     bx,word [bx+di+0x5F] ; 40 m/r/m mod=1
    adc     bx,word [bx+di-0x5F]
    adc     bx,word [bp+si+0x5F] ; 40 m/r/m mod=1
    adc     bx,word [bp+si-0x5F]
    adc     bx,word [bp+di+0x5F] ; 40 m/r/m mod=1
    adc     bx,word [bp+di-0x5F]
    adc     bx,word [si+0x5F] ; 40 m/r/m mod=1
    adc     bx,word [si-0x5F]
    adc     bx,word [di+0x5F] ; 40 m/r/m mod=1
    adc     bx,word [di-0x5F]
    adc     bx,word [bp+0x5F] ; 40 m/r/m mod=1
    adc     bx,word [bp-0x5F]
    adc     bx,word [bx+0x5F] ; 40 m/r/m mod=1
    adc     bx,word [bx-0x5F]

    adc     bx,word [bx+si+0x1234]
    adc     bx,word [bx+di+0x1234]
    adc     bx,word [bp+si+0x1234]
    adc     bx,word [bp+di+0x1234]
    adc     bx,word [si+0x1234]
    adc     bx,word [di+0x1234]
    adc     bx,word [bp+0x1234]
    adc     bx,word [bx+0x1234]

;--------------dword
    adc     ebx,dword [ebx+esi] ; 00 m/r/m mod=0
    adc     ebx,dword [ebx+edi] ; 00 m/r/m mod=0
    adc     ebx,dword [ebp+esi] ; 00 m/r/m mod=0
    adc     ebx,dword [ebp+edi] ; 00 m/r/m mod=0
    adc     ebx,dword [esi]    ; 00 m/r/m mod=0
    adc     ebx,dword [edi]    ; 00 m/r/m mod=0
    a32 adc ebx,dword [0x12345678]; 00 m/r/m mod=0
    adc     ebx,dword [ebx]    ; 00 m/r/m mod=0

    adc     ebx,dword [ebx+esi+0x5F] ; 40 m/r/m mod=1
    adc     ebx,dword [ebx+esi-0x5F]
    adc     ebx,dword [ebx+edi+0x5F] ; 40 m/r/m mod=1
    adc     ebx,dword [ebx+edi-0x5F]
    adc     ebx,dword [ebp+esi+0x5F] ; 40 m/r/m mod=1
    adc     ebx,dword [ebp+esi-0x5F]
    adc     ebx,dword [ebp+edi+0x5F] ; 40 m/r/m mod=1
    adc     ebx,dword [ebp+edi-0x5F]
    adc     ebx,dword [esi+0x5F] ; 40 m/r/m mod=1
    adc     ebx,dword [esi-0x5F]
    adc     ebx,dword [edi+0x5F] ; 40 m/r/m mod=1
    adc     ebx,dword [edi-0x5F]
    adc     ebx,dword [ebp+0x5F] ; 40 m/r/m mod=1
    adc     ebx,dword [ebp-0x5F]
    adc     ebx,dword [ebx+0x5F] ; 40 m/r/m mod=1
    adc     ebx,dword [ebx-0x5F]

    adc     ebx,dword [ebx+esi+0x1234]
    adc     ebx,dword [ebx+edi+0x1234]
    adc     ebx,dword [ebp+esi+0x1234]
    adc     ebx,dword [ebp+edi+0x1234]
    adc     ebx,dword [esi+0x1234]
    adc     ebx,dword [edi+0x1234]
    adc     ebx,dword [ebp+0x1234]
    adc     ebx,dword [ebx+0x1234]

;---------------adc imm
    adc     al,0x12
    adc     al,0xEF
    adc     ax,0x1234
    adc     ax,0xFEDC
    adc     eax,0x12345678
    adc     eax,0xFEDCBA98

;--------------byte
    sbb     byte [bx+si],bl ; 00 m/r/m mod=0
    sbb     byte [bx+di],bl ; 00 m/r/m mod=0
    sbb     byte [bp+si],bl ; 00 m/r/m mod=0
    sbb     byte [bp+di],bl ; 00 m/r/m mod=0
    sbb     byte [si],bl    ; 00 m/r/m mod=0
    sbb     byte [di],bl    ; 00 m/r/m mod=0
    sbb     byte [0x1234],bl; 00 m/r/m mod=0
    sbb     byte [bx],bl    ; 00 m/r/m mod=0

    sbb     byte [bx+si+0x5F],bl ; 40 m/r/m mod=1
    sbb     byte [bx+si-0x5F],bl
    sbb     byte [bx+di+0x5F],bl ; 40 m/r/m mod=1
    sbb     byte [bx+di-0x5F],bl
    sbb     byte [bp+si+0x5F],bl ; 40 m/r/m mod=1
    sbb     byte [bp+si-0x5F],bl
    sbb     byte [bp+di+0x5F],bl ; 40 m/r/m mod=1
    sbb     byte [bp+di-0x5F],bl
    sbb     byte [si+0x5F],bl ; 40 m/r/m mod=1
    sbb     byte [si-0x5F],bl
    sbb     byte [di+0x5F],bl ; 40 m/r/m mod=1
    sbb     byte [di-0x5F],bl
    sbb     byte [bp+0x5F],bl ; 40 m/r/m mod=1
    sbb     byte [bp-0x5F],bl
    sbb     byte [bx+0x5F],bl ; 40 m/r/m mod=1
    sbb     byte [bx-0x5F],bl

    sbb     byte [bx+si+0x1234],bl
    sbb     byte [bx+di+0x1234],bl
    sbb     byte [bp+si+0x1234],bl
    sbb     byte [bp+di+0x1234],bl
    sbb     byte [si+0x1234],bl
    sbb     byte [di+0x1234],bl
    sbb     byte [bp+0x1234],bl
    sbb     byte [bx+0x1234],bl
    sbb     bl,al           ; 00 m/r/m mod=3
    sbb     bl,cl           ; 00 m/r/m mod=3
    sbb     bl,dl           ; 00 m/r/m mod=3
    sbb     bl,bl           ; 00 m/r/m mod=3
    sbb     bl,ah
    sbb     bl,ch
    sbb     bl,dh
    sbb     bl,bh

;--------------word
    sbb     word [bx+si],bx ; 00 m/r/m mod=0
    sbb     word [bx+di],bx ; 00 m/r/m mod=0
    sbb     word [bp+si],bx ; 00 m/r/m mod=0
    sbb     word [bp+di],bx ; 00 m/r/m mod=0
    sbb     word [si],bx    ; 00 m/r/m mod=0
    sbb     word [di],bx    ; 00 m/r/m mod=0
    sbb     word [0x1234],bx; 00 m/r/m mod=0
    sbb     word [bx],bx    ; 00 m/r/m mod=0

    sbb     word [bx+si+0x5F],bx ; 40 m/r/m mod=1
    sbb     word [bx+si-0x5F],bx
    sbb     word [bx+di+0x5F],bx ; 40 m/r/m mod=1
    sbb     word [bx+di-0x5F],bx
    sbb     word [bp+si+0x5F],bx ; 40 m/r/m mod=1
    sbb     word [bp+si-0x5F],bx
    sbb     word [bp+di+0x5F],bx ; 40 m/r/m mod=1
    sbb     word [bp+di-0x5F],bx
    sbb     word [si+0x5F],bx ; 40 m/r/m mod=1
    sbb     word [si-0x5F],bx
    sbb     word [di+0x5F],bx ; 40 m/r/m mod=1
    sbb     word [di-0x5F],bx
    sbb     word [bp+0x5F],bx ; 40 m/r/m mod=1
    sbb     word [bp-0x5F],bx
    sbb     word [bx+0x5F],bx ; 40 m/r/m mod=1
    sbb     word [bx-0x5F],bx

    sbb     word [bx+si+0x1234],bx
    sbb     word [bx+di+0x1234],bx
    sbb     word [bp+si+0x1234],bx
    sbb     word [bp+di+0x1234],bx
    sbb     word [si+0x1234],bx
    sbb     word [di+0x1234],bx
    sbb     word [bp+0x1234],bx
    sbb     word [bx+0x1234],bx
    sbb     bx,ax           ; 00 m/r/m mod=3
    sbb     bx,cx           ; 00 m/r/m mod=3
    sbb     bx,dx           ; 00 m/r/m mod=3
    sbb     bx,bx           ; 00 m/r/m mod=3
    sbb     bx,si
    sbb     bx,di
    sbb     bx,bp
    sbb     bx,sp

;--------------dword
    sbb     dword [ebx+esi],ebx ; 00 m/r/m mod=0
    sbb     dword [ebx+edi],ebx ; 00 m/r/m mod=0
    sbb     dword [ebp+esi],ebx ; 00 m/r/m mod=0
    sbb     dword [ebp+edi],ebx ; 00 m/r/m mod=0
    sbb     dword [esi],ebx    ; 00 m/r/m mod=0
    sbb     dword [edi],ebx    ; 00 m/r/m mod=0
    a32 sbb dword [0x12345678],ebx; 00 m/r/m mod=0
    sbb     dword [ebx],ebx    ; 00 m/r/m mod=0

    sbb     dword [ebx+esi+0x5F],ebx ; 40 m/r/m mod=1
    sbb     dword [ebx+esi-0x5F],ebx
    sbb     dword [ebx+edi+0x5F],ebx ; 40 m/r/m mod=1
    sbb     dword [ebx+edi-0x5F],ebx
    sbb     dword [ebp+esi+0x5F],ebx ; 40 m/r/m mod=1
    sbb     dword [ebp+esi-0x5F],ebx
    sbb     dword [ebp+edi+0x5F],ebx ; 40 m/r/m mod=1
    sbb     dword [ebp+edi-0x5F],ebx
    sbb     dword [esi+0x5F],ebx ; 40 m/r/m mod=1
    sbb     dword [esi-0x5F],ebx
    sbb     dword [edi+0x5F],ebx ; 40 m/r/m mod=1
    sbb     dword [edi-0x5F],ebx
    sbb     dword [ebp+0x5F],ebx ; 40 m/r/m mod=1
    sbb     dword [ebp-0x5F],ebx
    sbb     dword [ebx+0x5F],ebx ; 40 m/r/m mod=1
    sbb     dword [ebx-0x5F],ebx

    sbb     dword [ebx+esi+0x1234],ebx
    sbb     dword [ebx+edi+0x1234],ebx
    sbb     dword [ebp+esi+0x1234],ebx
    sbb     dword [ebp+edi+0x1234],ebx
    sbb     dword [esi+0x1234],ebx
    sbb     dword [edi+0x1234],ebx
    sbb     dword [ebp+0x1234],ebx
    sbb     dword [ebx+0x1234],ebx
    sbb     ebx,eax           ; 00 m/r/m mod=3
    sbb     ebx,ecx           ; 00 m/r/m mod=3
    sbb     ebx,edx           ; 00 m/r/m mod=3
    sbb     ebx,ebx           ; 00 m/r/m mod=3
    sbb     ebx,esi
    sbb     ebx,edi
    sbb     ebx,ebp
    sbb     ebx,esp

;--------------byte
    sbb     bl,byte [bx+si] ; 00 m/r/m mod=0
    sbb     bl,byte [bx+di] ; 00 m/r/m mod=0
    sbb     bl,byte [bp+si] ; 00 m/r/m mod=0
    sbb     bl,byte [bp+di] ; 00 m/r/m mod=0
    sbb     bl,byte [si]    ; 00 m/r/m mod=0
    sbb     bl,byte [di]    ; 00 m/r/m mod=0
    sbb     bl,byte [0x1234]; 00 m/r/m mod=0
    sbb     bl,byte [bx]    ; 00 m/r/m mod=0

    sbb     bl,byte [bx+si+0x5F] ; 40 m/r/m mod=1
    sbb     bl,byte [bx+si-0x5F]
    sbb     bl,byte [bx+di+0x5F] ; 40 m/r/m mod=1
    sbb     bl,byte [bx+di-0x5F]
    sbb     bl,byte [bp+si+0x5F] ; 40 m/r/m mod=1
    sbb     bl,byte [bp+si-0x5F]
    sbb     bl,byte [bp+di+0x5F] ; 40 m/r/m mod=1
    sbb     bl,byte [bp+di-0x5F]
    sbb     bl,byte [si+0x5F] ; 40 m/r/m mod=1
    sbb     bl,byte [si-0x5F]
    sbb     bl,byte [di+0x5F] ; 40 m/r/m mod=1
    sbb     bl,byte [di-0x5F]
    sbb     bl,byte [bp+0x5F] ; 40 m/r/m mod=1
    sbb     bl,byte [bp-0x5F]
    sbb     bl,byte [bx+0x5F] ; 40 m/r/m mod=1
    sbb     bl,byte [bx-0x5F]

    sbb     bl,byte [bx+si+0x1234]
    sbb     bl,byte [bx+di+0x1234]
    sbb     bl,byte [bp+si+0x1234]
    sbb     bl,byte [bp+di+0x1234]
    sbb     bl,byte [si+0x1234]
    sbb     bl,byte [di+0x1234]
    sbb     bl,byte [bp+0x1234]
    sbb     bl,byte [bx+0x1234]

;--------------word
    sbb     bx,word [bx+si] ; 00 m/r/m mod=0
    sbb     bx,word [bx+di] ; 00 m/r/m mod=0
    sbb     bx,word [bp+si] ; 00 m/r/m mod=0
    sbb     bx,word [bp+di] ; 00 m/r/m mod=0
    sbb     bx,word [si]    ; 00 m/r/m mod=0
    sbb     bx,word [di]    ; 00 m/r/m mod=0
    sbb     bx,word [0x1234]; 00 m/r/m mod=0
    sbb     bx,word [bx]    ; 00 m/r/m mod=0

    sbb     bx,word [bx+si+0x5F] ; 40 m/r/m mod=1
    sbb     bx,word [bx+si-0x5F]
    sbb     bx,word [bx+di+0x5F] ; 40 m/r/m mod=1
    sbb     bx,word [bx+di-0x5F]
    sbb     bx,word [bp+si+0x5F] ; 40 m/r/m mod=1
    sbb     bx,word [bp+si-0x5F]
    sbb     bx,word [bp+di+0x5F] ; 40 m/r/m mod=1
    sbb     bx,word [bp+di-0x5F]
    sbb     bx,word [si+0x5F] ; 40 m/r/m mod=1
    sbb     bx,word [si-0x5F]
    sbb     bx,word [di+0x5F] ; 40 m/r/m mod=1
    sbb     bx,word [di-0x5F]
    sbb     bx,word [bp+0x5F] ; 40 m/r/m mod=1
    sbb     bx,word [bp-0x5F]
    sbb     bx,word [bx+0x5F] ; 40 m/r/m mod=1
    sbb     bx,word [bx-0x5F]

    sbb     bx,word [bx+si+0x1234]
    sbb     bx,word [bx+di+0x1234]
    sbb     bx,word [bp+si+0x1234]
    sbb     bx,word [bp+di+0x1234]
    sbb     bx,word [si+0x1234]
    sbb     bx,word [di+0x1234]
    sbb     bx,word [bp+0x1234]
    sbb     bx,word [bx+0x1234]

;--------------dword
    sbb     ebx,dword [ebx+esi] ; 00 m/r/m mod=0
    sbb     ebx,dword [ebx+edi] ; 00 m/r/m mod=0
    sbb     ebx,dword [ebp+esi] ; 00 m/r/m mod=0
    sbb     ebx,dword [ebp+edi] ; 00 m/r/m mod=0
    sbb     ebx,dword [esi]    ; 00 m/r/m mod=0
    sbb     ebx,dword [edi]    ; 00 m/r/m mod=0
    a32 sbb ebx,dword [0x12345678]; 00 m/r/m mod=0
    sbb     ebx,dword [ebx]    ; 00 m/r/m mod=0

    sbb     ebx,dword [ebx+esi+0x5F] ; 40 m/r/m mod=1
    sbb     ebx,dword [ebx+esi-0x5F]
    sbb     ebx,dword [ebx+edi+0x5F] ; 40 m/r/m mod=1
    sbb     ebx,dword [ebx+edi-0x5F]
    sbb     ebx,dword [ebp+esi+0x5F] ; 40 m/r/m mod=1
    sbb     ebx,dword [ebp+esi-0x5F]
    sbb     ebx,dword [ebp+edi+0x5F] ; 40 m/r/m mod=1
    sbb     ebx,dword [ebp+edi-0x5F]
    sbb     ebx,dword [esi+0x5F] ; 40 m/r/m mod=1
    sbb     ebx,dword [esi-0x5F]
    sbb     ebx,dword [edi+0x5F] ; 40 m/r/m mod=1
    sbb     ebx,dword [edi-0x5F]
    sbb     ebx,dword [ebp+0x5F] ; 40 m/r/m mod=1
    sbb     ebx,dword [ebp-0x5F]
    sbb     ebx,dword [ebx+0x5F] ; 40 m/r/m mod=1
    sbb     ebx,dword [ebx-0x5F]

    sbb     ebx,dword [ebx+esi+0x1234]
    sbb     ebx,dword [ebx+edi+0x1234]
    sbb     ebx,dword [ebp+esi+0x1234]
    sbb     ebx,dword [ebp+edi+0x1234]
    sbb     ebx,dword [esi+0x1234]
    sbb     ebx,dword [edi+0x1234]
    sbb     ebx,dword [ebp+0x1234]
    sbb     ebx,dword [ebx+0x1234]

;---------------sbb imm
    sbb     al,0x12
    sbb     al,0xEF
    sbb     ax,0x1234
    sbb     ax,0xFEDC
    sbb     eax,0x12345678
    sbb     eax,0xFEDCBA98

;--------------byte
    and     byte [bx+si],bl ; 00 m/r/m mod=0
    and     byte [bx+di],bl ; 00 m/r/m mod=0
    and     byte [bp+si],bl ; 00 m/r/m mod=0
    and     byte [bp+di],bl ; 00 m/r/m mod=0
    and     byte [si],bl    ; 00 m/r/m mod=0
    and     byte [di],bl    ; 00 m/r/m mod=0
    and     byte [0x1234],bl; 00 m/r/m mod=0
    and     byte [bx],bl    ; 00 m/r/m mod=0

    and     byte [bx+si+0x5F],bl ; 40 m/r/m mod=1
    and     byte [bx+si-0x5F],bl
    and     byte [bx+di+0x5F],bl ; 40 m/r/m mod=1
    and     byte [bx+di-0x5F],bl
    and     byte [bp+si+0x5F],bl ; 40 m/r/m mod=1
    and     byte [bp+si-0x5F],bl
    and     byte [bp+di+0x5F],bl ; 40 m/r/m mod=1
    and     byte [bp+di-0x5F],bl
    and     byte [si+0x5F],bl ; 40 m/r/m mod=1
    and     byte [si-0x5F],bl
    and     byte [di+0x5F],bl ; 40 m/r/m mod=1
    and     byte [di-0x5F],bl
    and     byte [bp+0x5F],bl ; 40 m/r/m mod=1
    and     byte [bp-0x5F],bl
    and     byte [bx+0x5F],bl ; 40 m/r/m mod=1
    and     byte [bx-0x5F],bl

    and     byte [bx+si+0x1234],bl
    and     byte [bx+di+0x1234],bl
    and     byte [bp+si+0x1234],bl
    and     byte [bp+di+0x1234],bl
    and     byte [si+0x1234],bl
    and     byte [di+0x1234],bl
    and     byte [bp+0x1234],bl
    and     byte [bx+0x1234],bl
    and     bl,al           ; 00 m/r/m mod=3
    and     bl,cl           ; 00 m/r/m mod=3
    and     bl,dl           ; 00 m/r/m mod=3
    and     bl,bl           ; 00 m/r/m mod=3
    and     bl,ah
    and     bl,ch
    and     bl,dh
    and     bl,bh

;--------------word
    and     word [bx+si],bx ; 00 m/r/m mod=0
    and     word [bx+di],bx ; 00 m/r/m mod=0
    and     word [bp+si],bx ; 00 m/r/m mod=0
    and     word [bp+di],bx ; 00 m/r/m mod=0
    and     word [si],bx    ; 00 m/r/m mod=0
    and     word [di],bx    ; 00 m/r/m mod=0
    and     word [0x1234],bx; 00 m/r/m mod=0
    and     word [bx],bx    ; 00 m/r/m mod=0

    and     word [bx+si+0x5F],bx ; 40 m/r/m mod=1
    and     word [bx+si-0x5F],bx
    and     word [bx+di+0x5F],bx ; 40 m/r/m mod=1
    and     word [bx+di-0x5F],bx
    and     word [bp+si+0x5F],bx ; 40 m/r/m mod=1
    and     word [bp+si-0x5F],bx
    and     word [bp+di+0x5F],bx ; 40 m/r/m mod=1
    and     word [bp+di-0x5F],bx
    and     word [si+0x5F],bx ; 40 m/r/m mod=1
    and     word [si-0x5F],bx
    and     word [di+0x5F],bx ; 40 m/r/m mod=1
    and     word [di-0x5F],bx
    and     word [bp+0x5F],bx ; 40 m/r/m mod=1
    and     word [bp-0x5F],bx
    and     word [bx+0x5F],bx ; 40 m/r/m mod=1
    and     word [bx-0x5F],bx

    and     word [bx+si+0x1234],bx
    and     word [bx+di+0x1234],bx
    and     word [bp+si+0x1234],bx
    and     word [bp+di+0x1234],bx
    and     word [si+0x1234],bx
    and     word [di+0x1234],bx
    and     word [bp+0x1234],bx
    and     word [bx+0x1234],bx
    and     bx,ax           ; 00 m/r/m mod=3
    and     bx,cx           ; 00 m/r/m mod=3
    and     bx,dx           ; 00 m/r/m mod=3
    and     bx,bx           ; 00 m/r/m mod=3
    and     bx,si
    and     bx,di
    and     bx,bp
    and     bx,sp

;--------------dword
    and     dword [ebx+esi],ebx ; 00 m/r/m mod=0
    and     dword [ebx+edi],ebx ; 00 m/r/m mod=0
    and     dword [ebp+esi],ebx ; 00 m/r/m mod=0
    and     dword [ebp+edi],ebx ; 00 m/r/m mod=0
    and     dword [esi],ebx    ; 00 m/r/m mod=0
    and     dword [edi],ebx    ; 00 m/r/m mod=0
    a32 and dword [0x12345678],ebx; 00 m/r/m mod=0
    and     dword [ebx],ebx    ; 00 m/r/m mod=0

    and     dword [ebx+esi+0x5F],ebx ; 40 m/r/m mod=1
    and     dword [ebx+esi-0x5F],ebx
    and     dword [ebx+edi+0x5F],ebx ; 40 m/r/m mod=1
    and     dword [ebx+edi-0x5F],ebx
    and     dword [ebp+esi+0x5F],ebx ; 40 m/r/m mod=1
    and     dword [ebp+esi-0x5F],ebx
    and     dword [ebp+edi+0x5F],ebx ; 40 m/r/m mod=1
    and     dword [ebp+edi-0x5F],ebx
    and     dword [esi+0x5F],ebx ; 40 m/r/m mod=1
    and     dword [esi-0x5F],ebx
    and     dword [edi+0x5F],ebx ; 40 m/r/m mod=1
    and     dword [edi-0x5F],ebx
    and     dword [ebp+0x5F],ebx ; 40 m/r/m mod=1
    and     dword [ebp-0x5F],ebx
    and     dword [ebx+0x5F],ebx ; 40 m/r/m mod=1
    and     dword [ebx-0x5F],ebx

    and     dword [ebx+esi+0x1234],ebx
    and     dword [ebx+edi+0x1234],ebx
    and     dword [ebp+esi+0x1234],ebx
    and     dword [ebp+edi+0x1234],ebx
    and     dword [esi+0x1234],ebx
    and     dword [edi+0x1234],ebx
    and     dword [ebp+0x1234],ebx
    and     dword [ebx+0x1234],ebx
    and     ebx,eax           ; 00 m/r/m mod=3
    and     ebx,ecx           ; 00 m/r/m mod=3
    and     ebx,edx           ; 00 m/r/m mod=3
    and     ebx,ebx           ; 00 m/r/m mod=3
    and     ebx,esi
    and     ebx,edi
    and     ebx,ebp
    and     ebx,esp

;--------------byte
    and     bl,byte [bx+si] ; 00 m/r/m mod=0
    and     bl,byte [bx+di] ; 00 m/r/m mod=0
    and     bl,byte [bp+si] ; 00 m/r/m mod=0
    and     bl,byte [bp+di] ; 00 m/r/m mod=0
    and     bl,byte [si]    ; 00 m/r/m mod=0
    and     bl,byte [di]    ; 00 m/r/m mod=0
    and     bl,byte [0x1234]; 00 m/r/m mod=0
    and     bl,byte [bx]    ; 00 m/r/m mod=0

    and     bl,byte [bx+si+0x5F] ; 40 m/r/m mod=1
    and     bl,byte [bx+si-0x5F]
    and     bl,byte [bx+di+0x5F] ; 40 m/r/m mod=1
    and     bl,byte [bx+di-0x5F]
    and     bl,byte [bp+si+0x5F] ; 40 m/r/m mod=1
    and     bl,byte [bp+si-0x5F]
    and     bl,byte [bp+di+0x5F] ; 40 m/r/m mod=1
    and     bl,byte [bp+di-0x5F]
    and     bl,byte [si+0x5F] ; 40 m/r/m mod=1
    and     bl,byte [si-0x5F]
    and     bl,byte [di+0x5F] ; 40 m/r/m mod=1
    and     bl,byte [di-0x5F]
    and     bl,byte [bp+0x5F] ; 40 m/r/m mod=1
    and     bl,byte [bp-0x5F]
    and     bl,byte [bx+0x5F] ; 40 m/r/m mod=1
    and     bl,byte [bx-0x5F]

    and     bl,byte [bx+si+0x1234]
    and     bl,byte [bx+di+0x1234]
    and     bl,byte [bp+si+0x1234]
    and     bl,byte [bp+di+0x1234]
    and     bl,byte [si+0x1234]
    and     bl,byte [di+0x1234]
    and     bl,byte [bp+0x1234]
    and     bl,byte [bx+0x1234]

;--------------word
    and     bx,word [bx+si] ; 00 m/r/m mod=0
    and     bx,word [bx+di] ; 00 m/r/m mod=0
    and     bx,word [bp+si] ; 00 m/r/m mod=0
    and     bx,word [bp+di] ; 00 m/r/m mod=0
    and     bx,word [si]    ; 00 m/r/m mod=0
    and     bx,word [di]    ; 00 m/r/m mod=0
    and     bx,word [0x1234]; 00 m/r/m mod=0
    and     bx,word [bx]    ; 00 m/r/m mod=0

    and     bx,word [bx+si+0x5F] ; 40 m/r/m mod=1
    and     bx,word [bx+si-0x5F]
    and     bx,word [bx+di+0x5F] ; 40 m/r/m mod=1
    and     bx,word [bx+di-0x5F]
    and     bx,word [bp+si+0x5F] ; 40 m/r/m mod=1
    and     bx,word [bp+si-0x5F]
    and     bx,word [bp+di+0x5F] ; 40 m/r/m mod=1
    and     bx,word [bp+di-0x5F]
    and     bx,word [si+0x5F] ; 40 m/r/m mod=1
    and     bx,word [si-0x5F]
    and     bx,word [di+0x5F] ; 40 m/r/m mod=1
    and     bx,word [di-0x5F]
    and     bx,word [bp+0x5F] ; 40 m/r/m mod=1
    and     bx,word [bp-0x5F]
    and     bx,word [bx+0x5F] ; 40 m/r/m mod=1
    and     bx,word [bx-0x5F]

    and     bx,word [bx+si+0x1234]
    and     bx,word [bx+di+0x1234]
    and     bx,word [bp+si+0x1234]
    and     bx,word [bp+di+0x1234]
    and     bx,word [si+0x1234]
    and     bx,word [di+0x1234]
    and     bx,word [bp+0x1234]
    and     bx,word [bx+0x1234]

;--------------dword
    and     ebx,dword [ebx+esi] ; 00 m/r/m mod=0
    and     ebx,dword [ebx+edi] ; 00 m/r/m mod=0
    and     ebx,dword [ebp+esi] ; 00 m/r/m mod=0
    and     ebx,dword [ebp+edi] ; 00 m/r/m mod=0
    and     ebx,dword [esi]    ; 00 m/r/m mod=0
    and     ebx,dword [edi]    ; 00 m/r/m mod=0
    a32 and ebx,dword [0x12345678]; 00 m/r/m mod=0
    and     ebx,dword [ebx]    ; 00 m/r/m mod=0

    and     ebx,dword [ebx+esi+0x5F] ; 40 m/r/m mod=1
    and     ebx,dword [ebx+esi-0x5F]
    and     ebx,dword [ebx+edi+0x5F] ; 40 m/r/m mod=1
    and     ebx,dword [ebx+edi-0x5F]
    and     ebx,dword [ebp+esi+0x5F] ; 40 m/r/m mod=1
    and     ebx,dword [ebp+esi-0x5F]
    and     ebx,dword [ebp+edi+0x5F] ; 40 m/r/m mod=1
    and     ebx,dword [ebp+edi-0x5F]
    and     ebx,dword [esi+0x5F] ; 40 m/r/m mod=1
    and     ebx,dword [esi-0x5F]
    and     ebx,dword [edi+0x5F] ; 40 m/r/m mod=1
    and     ebx,dword [edi-0x5F]
    and     ebx,dword [ebp+0x5F] ; 40 m/r/m mod=1
    and     ebx,dword [ebp-0x5F]
    and     ebx,dword [ebx+0x5F] ; 40 m/r/m mod=1
    and     ebx,dword [ebx-0x5F]

    and     ebx,dword [ebx+esi+0x1234]
    and     ebx,dword [ebx+edi+0x1234]
    and     ebx,dword [ebp+esi+0x1234]
    and     ebx,dword [ebp+edi+0x1234]
    and     ebx,dword [esi+0x1234]
    and     ebx,dword [edi+0x1234]
    and     ebx,dword [ebp+0x1234]
    and     ebx,dword [ebx+0x1234]

;---------------and imm
    and     al,0x12
    and     al,0xEF
    and     ax,0x1234
    and     ax,0xFEDC
    and     eax,0x12345678
    and     eax,0xFEDCBA98

;--------------byte
    sub     byte [bx+si],bl ; 00 m/r/m mod=0
    sub     byte [bx+di],bl ; 00 m/r/m mod=0
    sub     byte [bp+si],bl ; 00 m/r/m mod=0
    sub     byte [bp+di],bl ; 00 m/r/m mod=0
    sub     byte [si],bl    ; 00 m/r/m mod=0
    sub     byte [di],bl    ; 00 m/r/m mod=0
    sub     byte [0x1234],bl; 00 m/r/m mod=0
    sub     byte [bx],bl    ; 00 m/r/m mod=0

    sub     byte [bx+si+0x5F],bl ; 40 m/r/m mod=1
    sub     byte [bx+si-0x5F],bl
    sub     byte [bx+di+0x5F],bl ; 40 m/r/m mod=1
    sub     byte [bx+di-0x5F],bl
    sub     byte [bp+si+0x5F],bl ; 40 m/r/m mod=1
    sub     byte [bp+si-0x5F],bl
    sub     byte [bp+di+0x5F],bl ; 40 m/r/m mod=1
    sub     byte [bp+di-0x5F],bl
    sub     byte [si+0x5F],bl ; 40 m/r/m mod=1
    sub     byte [si-0x5F],bl
    sub     byte [di+0x5F],bl ; 40 m/r/m mod=1
    sub     byte [di-0x5F],bl
    sub     byte [bp+0x5F],bl ; 40 m/r/m mod=1
    sub     byte [bp-0x5F],bl
    sub     byte [bx+0x5F],bl ; 40 m/r/m mod=1
    sub     byte [bx-0x5F],bl

    sub     byte [bx+si+0x1234],bl
    sub     byte [bx+di+0x1234],bl
    sub     byte [bp+si+0x1234],bl
    sub     byte [bp+di+0x1234],bl
    sub     byte [si+0x1234],bl
    sub     byte [di+0x1234],bl
    sub     byte [bp+0x1234],bl
    sub     byte [bx+0x1234],bl
    sub     bl,al           ; 00 m/r/m mod=3
    sub     bl,cl           ; 00 m/r/m mod=3
    sub     bl,dl           ; 00 m/r/m mod=3
    sub     bl,bl           ; 00 m/r/m mod=3
    sub     bl,ah
    sub     bl,ch
    sub     bl,dh
    sub     bl,bh

;--------------word
    sub     word [bx+si],bx ; 00 m/r/m mod=0
    sub     word [bx+di],bx ; 00 m/r/m mod=0
    sub     word [bp+si],bx ; 00 m/r/m mod=0
    sub     word [bp+di],bx ; 00 m/r/m mod=0
    sub     word [si],bx    ; 00 m/r/m mod=0
    sub     word [di],bx    ; 00 m/r/m mod=0
    sub     word [0x1234],bx; 00 m/r/m mod=0
    sub     word [bx],bx    ; 00 m/r/m mod=0

    sub     word [bx+si+0x5F],bx ; 40 m/r/m mod=1
    sub     word [bx+si-0x5F],bx
    sub     word [bx+di+0x5F],bx ; 40 m/r/m mod=1
    sub     word [bx+di-0x5F],bx
    sub     word [bp+si+0x5F],bx ; 40 m/r/m mod=1
    sub     word [bp+si-0x5F],bx
    sub     word [bp+di+0x5F],bx ; 40 m/r/m mod=1
    sub     word [bp+di-0x5F],bx
    sub     word [si+0x5F],bx ; 40 m/r/m mod=1
    sub     word [si-0x5F],bx
    sub     word [di+0x5F],bx ; 40 m/r/m mod=1
    sub     word [di-0x5F],bx
    sub     word [bp+0x5F],bx ; 40 m/r/m mod=1
    sub     word [bp-0x5F],bx
    sub     word [bx+0x5F],bx ; 40 m/r/m mod=1
    sub     word [bx-0x5F],bx

    sub     word [bx+si+0x1234],bx
    sub     word [bx+di+0x1234],bx
    sub     word [bp+si+0x1234],bx
    sub     word [bp+di+0x1234],bx
    sub     word [si+0x1234],bx
    sub     word [di+0x1234],bx
    sub     word [bp+0x1234],bx
    sub     word [bx+0x1234],bx
    sub     bx,ax           ; 00 m/r/m mod=3
    sub     bx,cx           ; 00 m/r/m mod=3
    sub     bx,dx           ; 00 m/r/m mod=3
    sub     bx,bx           ; 00 m/r/m mod=3
    sub     bx,si
    sub     bx,di
    sub     bx,bp
    sub     bx,sp

;--------------dword
    sub     dword [ebx+esi],ebx ; 00 m/r/m mod=0
    sub     dword [ebx+edi],ebx ; 00 m/r/m mod=0
    sub     dword [ebp+esi],ebx ; 00 m/r/m mod=0
    sub     dword [ebp+edi],ebx ; 00 m/r/m mod=0
    sub     dword [esi],ebx    ; 00 m/r/m mod=0
    sub     dword [edi],ebx    ; 00 m/r/m mod=0
    a32 sub dword [0x12345678],ebx; 00 m/r/m mod=0
    sub     dword [ebx],ebx    ; 00 m/r/m mod=0

    sub     dword [ebx+esi+0x5F],ebx ; 40 m/r/m mod=1
    sub     dword [ebx+esi-0x5F],ebx
    sub     dword [ebx+edi+0x5F],ebx ; 40 m/r/m mod=1
    sub     dword [ebx+edi-0x5F],ebx
    sub     dword [ebp+esi+0x5F],ebx ; 40 m/r/m mod=1
    sub     dword [ebp+esi-0x5F],ebx
    sub     dword [ebp+edi+0x5F],ebx ; 40 m/r/m mod=1
    sub     dword [ebp+edi-0x5F],ebx
    sub     dword [esi+0x5F],ebx ; 40 m/r/m mod=1
    sub     dword [esi-0x5F],ebx
    sub     dword [edi+0x5F],ebx ; 40 m/r/m mod=1
    sub     dword [edi-0x5F],ebx
    sub     dword [ebp+0x5F],ebx ; 40 m/r/m mod=1
    sub     dword [ebp-0x5F],ebx
    sub     dword [ebx+0x5F],ebx ; 40 m/r/m mod=1
    sub     dword [ebx-0x5F],ebx

    sub     dword [ebx+esi+0x1234],ebx
    sub     dword [ebx+edi+0x1234],ebx
    sub     dword [ebp+esi+0x1234],ebx
    sub     dword [ebp+edi+0x1234],ebx
    sub     dword [esi+0x1234],ebx
    sub     dword [edi+0x1234],ebx
    sub     dword [ebp+0x1234],ebx
    sub     dword [ebx+0x1234],ebx
    sub     ebx,eax           ; 00 m/r/m mod=3
    sub     ebx,ecx           ; 00 m/r/m mod=3
    sub     ebx,edx           ; 00 m/r/m mod=3
    sub     ebx,ebx           ; 00 m/r/m mod=3
    sub     ebx,esi
    sub     ebx,edi
    sub     ebx,ebp
    sub     ebx,esp

;--------------byte
    sub     bl,byte [bx+si] ; 00 m/r/m mod=0
    sub     bl,byte [bx+di] ; 00 m/r/m mod=0
    sub     bl,byte [bp+si] ; 00 m/r/m mod=0
    sub     bl,byte [bp+di] ; 00 m/r/m mod=0
    sub     bl,byte [si]    ; 00 m/r/m mod=0
    sub     bl,byte [di]    ; 00 m/r/m mod=0
    sub     bl,byte [0x1234]; 00 m/r/m mod=0
    sub     bl,byte [bx]    ; 00 m/r/m mod=0

    sub     bl,byte [bx+si+0x5F] ; 40 m/r/m mod=1
    sub     bl,byte [bx+si-0x5F]
    sub     bl,byte [bx+di+0x5F] ; 40 m/r/m mod=1
    sub     bl,byte [bx+di-0x5F]
    sub     bl,byte [bp+si+0x5F] ; 40 m/r/m mod=1
    sub     bl,byte [bp+si-0x5F]
    sub     bl,byte [bp+di+0x5F] ; 40 m/r/m mod=1
    sub     bl,byte [bp+di-0x5F]
    sub     bl,byte [si+0x5F] ; 40 m/r/m mod=1
    sub     bl,byte [si-0x5F]
    sub     bl,byte [di+0x5F] ; 40 m/r/m mod=1
    sub     bl,byte [di-0x5F]
    sub     bl,byte [bp+0x5F] ; 40 m/r/m mod=1
    sub     bl,byte [bp-0x5F]
    sub     bl,byte [bx+0x5F] ; 40 m/r/m mod=1
    sub     bl,byte [bx-0x5F]

    sub     bl,byte [bx+si+0x1234]
    sub     bl,byte [bx+di+0x1234]
    sub     bl,byte [bp+si+0x1234]
    sub     bl,byte [bp+di+0x1234]
    sub     bl,byte [si+0x1234]
    sub     bl,byte [di+0x1234]
    sub     bl,byte [bp+0x1234]
    sub     bl,byte [bx+0x1234]

;--------------word
    sub     bx,word [bx+si] ; 00 m/r/m mod=0
    sub     bx,word [bx+di] ; 00 m/r/m mod=0
    sub     bx,word [bp+si] ; 00 m/r/m mod=0
    sub     bx,word [bp+di] ; 00 m/r/m mod=0
    sub     bx,word [si]    ; 00 m/r/m mod=0
    sub     bx,word [di]    ; 00 m/r/m mod=0
    sub     bx,word [0x1234]; 00 m/r/m mod=0
    sub     bx,word [bx]    ; 00 m/r/m mod=0

    sub     bx,word [bx+si+0x5F] ; 40 m/r/m mod=1
    sub     bx,word [bx+si-0x5F]
    sub     bx,word [bx+di+0x5F] ; 40 m/r/m mod=1
    sub     bx,word [bx+di-0x5F]
    sub     bx,word [bp+si+0x5F] ; 40 m/r/m mod=1
    sub     bx,word [bp+si-0x5F]
    sub     bx,word [bp+di+0x5F] ; 40 m/r/m mod=1
    sub     bx,word [bp+di-0x5F]
    sub     bx,word [si+0x5F] ; 40 m/r/m mod=1
    sub     bx,word [si-0x5F]
    sub     bx,word [di+0x5F] ; 40 m/r/m mod=1
    sub     bx,word [di-0x5F]
    sub     bx,word [bp+0x5F] ; 40 m/r/m mod=1
    sub     bx,word [bp-0x5F]
    sub     bx,word [bx+0x5F] ; 40 m/r/m mod=1
    sub     bx,word [bx-0x5F]

    sub     bx,word [bx+si+0x1234]
    sub     bx,word [bx+di+0x1234]
    sub     bx,word [bp+si+0x1234]
    sub     bx,word [bp+di+0x1234]
    sub     bx,word [si+0x1234]
    sub     bx,word [di+0x1234]
    sub     bx,word [bp+0x1234]
    sub     bx,word [bx+0x1234]

;---------------sub imm
    sub     al,0x12
    sub     al,0xEF
    sub     ax,0x1234
    sub     ax,0xFEDC
    sub     eax,0x12345678
    sub     eax,0xFEDCBA98

;--------------byte
    xor     byte [bx+si],bl ; 00 m/r/m mod=0
    xor     byte [bx+di],bl ; 00 m/r/m mod=0
    xor     byte [bp+si],bl ; 00 m/r/m mod=0
    xor     byte [bp+di],bl ; 00 m/r/m mod=0
    xor     byte [si],bl    ; 00 m/r/m mod=0
    xor     byte [di],bl    ; 00 m/r/m mod=0
    xor     byte [0x1234],bl; 00 m/r/m mod=0
    xor     byte [bx],bl    ; 00 m/r/m mod=0

    xor     byte [bx+si+0x5F],bl ; 40 m/r/m mod=1
    xor     byte [bx+si-0x5F],bl
    xor     byte [bx+di+0x5F],bl ; 40 m/r/m mod=1
    xor     byte [bx+di-0x5F],bl
    xor     byte [bp+si+0x5F],bl ; 40 m/r/m mod=1
    xor     byte [bp+si-0x5F],bl
    xor     byte [bp+di+0x5F],bl ; 40 m/r/m mod=1
    xor     byte [bp+di-0x5F],bl
    xor     byte [si+0x5F],bl ; 40 m/r/m mod=1
    xor     byte [si-0x5F],bl
    xor     byte [di+0x5F],bl ; 40 m/r/m mod=1
    xor     byte [di-0x5F],bl
    xor     byte [bp+0x5F],bl ; 40 m/r/m mod=1
    xor     byte [bp-0x5F],bl
    xor     byte [bx+0x5F],bl ; 40 m/r/m mod=1
    xor     byte [bx-0x5F],bl

    xor     byte [bx+si+0x1234],bl
    xor     byte [bx+di+0x1234],bl
    xor     byte [bp+si+0x1234],bl
    xor     byte [bp+di+0x1234],bl
    xor     byte [si+0x1234],bl
    xor     byte [di+0x1234],bl
    xor     byte [bp+0x1234],bl
    xor     byte [bx+0x1234],bl
    xor     bl,al           ; 00 m/r/m mod=3
    xor     bl,cl           ; 00 m/r/m mod=3
    xor     bl,dl           ; 00 m/r/m mod=3
    xor     bl,bl           ; 00 m/r/m mod=3
    xor     bl,ah
    xor     bl,ch
    xor     bl,dh
    xor     bl,bh

;--------------word
    xor     word [bx+si],bx ; 00 m/r/m mod=0
    xor     word [bx+di],bx ; 00 m/r/m mod=0
    xor     word [bp+si],bx ; 00 m/r/m mod=0
    xor     word [bp+di],bx ; 00 m/r/m mod=0
    xor     word [si],bx    ; 00 m/r/m mod=0
    xor     word [di],bx    ; 00 m/r/m mod=0
    xor     word [0x1234],bx; 00 m/r/m mod=0
    xor     word [bx],bx    ; 00 m/r/m mod=0

    xor     word [bx+si+0x5F],bx ; 40 m/r/m mod=1
    xor     word [bx+si-0x5F],bx
    xor     word [bx+di+0x5F],bx ; 40 m/r/m mod=1
    xor     word [bx+di-0x5F],bx
    xor     word [bp+si+0x5F],bx ; 40 m/r/m mod=1
    xor     word [bp+si-0x5F],bx
    xor     word [bp+di+0x5F],bx ; 40 m/r/m mod=1
    xor     word [bp+di-0x5F],bx
    xor     word [si+0x5F],bx ; 40 m/r/m mod=1
    xor     word [si-0x5F],bx
    xor     word [di+0x5F],bx ; 40 m/r/m mod=1
    xor     word [di-0x5F],bx
    xor     word [bp+0x5F],bx ; 40 m/r/m mod=1
    xor     word [bp-0x5F],bx
    xor     word [bx+0x5F],bx ; 40 m/r/m mod=1
    xor     word [bx-0x5F],bx

    xor     word [bx+si+0x1234],bx
    xor     word [bx+di+0x1234],bx
    xor     word [bp+si+0x1234],bx
    xor     word [bp+di+0x1234],bx
    xor     word [si+0x1234],bx
    xor     word [di+0x1234],bx
    xor     word [bp+0x1234],bx
    xor     word [bx+0x1234],bx
    xor     bx,ax           ; 00 m/r/m mod=3
    xor     bx,cx           ; 00 m/r/m mod=3
    xor     bx,dx           ; 00 m/r/m mod=3
    xor     bx,bx           ; 00 m/r/m mod=3
    xor     bx,si
    xor     bx,di
    xor     bx,bp
    xor     bx,sp

;--------------byte
    xor     bl,byte [bx+si] ; 00 m/r/m mod=0
    xor     bl,byte [bx+di] ; 00 m/r/m mod=0
    xor     bl,byte [bp+si] ; 00 m/r/m mod=0
    xor     bl,byte [bp+di] ; 00 m/r/m mod=0
    xor     bl,byte [si]    ; 00 m/r/m mod=0
    xor     bl,byte [di]    ; 00 m/r/m mod=0
    xor     bl,byte [0x1234]; 00 m/r/m mod=0
    xor     bl,byte [bx]    ; 00 m/r/m mod=0

    xor     bl,byte [bx+si+0x5F] ; 40 m/r/m mod=1
    xor     bl,byte [bx+si-0x5F]
    xor     bl,byte [bx+di+0x5F] ; 40 m/r/m mod=1
    xor     bl,byte [bx+di-0x5F]
    xor     bl,byte [bp+si+0x5F] ; 40 m/r/m mod=1
    xor     bl,byte [bp+si-0x5F]
    xor     bl,byte [bp+di+0x5F] ; 40 m/r/m mod=1
    xor     bl,byte [bp+di-0x5F]
    xor     bl,byte [si+0x5F] ; 40 m/r/m mod=1
    xor     bl,byte [si-0x5F]
    xor     bl,byte [di+0x5F] ; 40 m/r/m mod=1
    xor     bl,byte [di-0x5F]
    xor     bl,byte [bp+0x5F] ; 40 m/r/m mod=1
    xor     bl,byte [bp-0x5F]
    xor     bl,byte [bx+0x5F] ; 40 m/r/m mod=1
    xor     bl,byte [bx-0x5F]

    xor     bl,byte [bx+si+0x1234]
    xor     bl,byte [bx+di+0x1234]
    xor     bl,byte [bp+si+0x1234]
    xor     bl,byte [bp+di+0x1234]
    xor     bl,byte [si+0x1234]
    xor     bl,byte [di+0x1234]
    xor     bl,byte [bp+0x1234]
    xor     bl,byte [bx+0x1234]

;--------------word
    xor     bx,word [bx+si] ; 00 m/r/m mod=0
    xor     bx,word [bx+di] ; 00 m/r/m mod=0
    xor     bx,word [bp+si] ; 00 m/r/m mod=0
    xor     bx,word [bp+di] ; 00 m/r/m mod=0
    xor     bx,word [si]    ; 00 m/r/m mod=0
    xor     bx,word [di]    ; 00 m/r/m mod=0
    xor     bx,word [0x1234]; 00 m/r/m mod=0
    xor     bx,word [bx]    ; 00 m/r/m mod=0

    xor     bx,word [bx+si+0x5F] ; 40 m/r/m mod=1
    xor     bx,word [bx+si-0x5F]
    xor     bx,word [bx+di+0x5F] ; 40 m/r/m mod=1
    xor     bx,word [bx+di-0x5F]
    xor     bx,word [bp+si+0x5F] ; 40 m/r/m mod=1
    xor     bx,word [bp+si-0x5F]
    xor     bx,word [bp+di+0x5F] ; 40 m/r/m mod=1
    xor     bx,word [bp+di-0x5F]
    xor     bx,word [si+0x5F] ; 40 m/r/m mod=1
    xor     bx,word [si-0x5F]
    xor     bx,word [di+0x5F] ; 40 m/r/m mod=1
    xor     bx,word [di-0x5F]
    xor     bx,word [bp+0x5F] ; 40 m/r/m mod=1
    xor     bx,word [bp-0x5F]
    xor     bx,word [bx+0x5F] ; 40 m/r/m mod=1
    xor     bx,word [bx-0x5F]

    xor     bx,word [bx+si+0x1234]
    xor     bx,word [bx+di+0x1234]
    xor     bx,word [bp+si+0x1234]
    xor     bx,word [bp+di+0x1234]
    xor     bx,word [si+0x1234]
    xor     bx,word [di+0x1234]
    xor     bx,word [bp+0x1234]
    xor     bx,word [bx+0x1234]

;--------------dword
    xor     ebx,dword [ebx+esi] ; 00 m/r/m mod=0
    xor     ebx,dword [ebx+edi] ; 00 m/r/m mod=0
    xor     ebx,dword [ebp+esi] ; 00 m/r/m mod=0
    xor     ebx,dword [ebp+edi] ; 00 m/r/m mod=0
    xor     ebx,dword [esi]    ; 00 m/r/m mod=0
    xor     ebx,dword [edi]    ; 00 m/r/m mod=0
    a32 xor ebx,dword [0x12345678]; 00 m/r/m mod=0
    xor     ebx,dword [ebx]    ; 00 m/r/m mod=0

;---------------xor imm
    xor     al,0x12
    xor     al,0xEF
    xor     ax,0x1234
    xor     ax,0xFEDC
    xor     eax,0x12345678
    xor     eax,0xFEDCBA98

;--------------byte
    cmp     byte [bx+si],bl ; 00 m/r/m mod=0
    cmp     byte [bx+di],bl ; 00 m/r/m mod=0
    cmp     byte [bp+si],bl ; 00 m/r/m mod=0
    cmp     byte [bp+di],bl ; 00 m/r/m mod=0
    cmp     byte [si],bl    ; 00 m/r/m mod=0
    cmp     byte [di],bl    ; 00 m/r/m mod=0
    cmp     byte [0x1234],bl; 00 m/r/m mod=0
    cmp     byte [bx],bl    ; 00 m/r/m mod=0

    cmp     byte [bx+si+0x5F],bl ; 40 m/r/m mod=1
    cmp     byte [bx+si-0x5F],bl
    cmp     byte [bx+di+0x5F],bl ; 40 m/r/m mod=1
    cmp     byte [bx+di-0x5F],bl
    cmp     byte [bp+si+0x5F],bl ; 40 m/r/m mod=1
    cmp     byte [bp+si-0x5F],bl
    cmp     byte [bp+di+0x5F],bl ; 40 m/r/m mod=1
    cmp     byte [bp+di-0x5F],bl
    cmp     byte [si+0x5F],bl ; 40 m/r/m mod=1
    cmp     byte [si-0x5F],bl
    cmp     byte [di+0x5F],bl ; 40 m/r/m mod=1
    cmp     byte [di-0x5F],bl
    cmp     byte [bp+0x5F],bl ; 40 m/r/m mod=1
    cmp     byte [bp-0x5F],bl
    cmp     byte [bx+0x5F],bl ; 40 m/r/m mod=1
    cmp     byte [bx-0x5F],bl

    cmp     byte [bx+si+0x1234],bl
    cmp     byte [bx+di+0x1234],bl
    cmp     byte [bp+si+0x1234],bl
    cmp     byte [bp+di+0x1234],bl
    cmp     byte [si+0x1234],bl
    cmp     byte [di+0x1234],bl
    cmp     byte [bp+0x1234],bl
    cmp     byte [bx+0x1234],bl
    cmp     bl,al           ; 00 m/r/m mod=3
    cmp     bl,cl           ; 00 m/r/m mod=3
    cmp     bl,dl           ; 00 m/r/m mod=3
    cmp     bl,bl           ; 00 m/r/m mod=3
    cmp     bl,ah
    cmp     bl,ch
    cmp     bl,dh
    cmp     bl,bh

;--------------word
    cmp     word [bx+si],bx ; 00 m/r/m mod=0
    cmp     word [bx+di],bx ; 00 m/r/m mod=0
    cmp     word [bp+si],bx ; 00 m/r/m mod=0
    cmp     word [bp+di],bx ; 00 m/r/m mod=0
    cmp     word [si],bx    ; 00 m/r/m mod=0
    cmp     word [di],bx    ; 00 m/r/m mod=0
    cmp     word [0x1234],bx; 00 m/r/m mod=0
    cmp     word [bx],bx    ; 00 m/r/m mod=0

    cmp     word [bx+si+0x5F],bx ; 40 m/r/m mod=1
    cmp     word [bx+si-0x5F],bx
    cmp     word [bx+di+0x5F],bx ; 40 m/r/m mod=1
    cmp     word [bx+di-0x5F],bx
    cmp     word [bp+si+0x5F],bx ; 40 m/r/m mod=1
    cmp     word [bp+si-0x5F],bx
    cmp     word [bp+di+0x5F],bx ; 40 m/r/m mod=1
    cmp     word [bp+di-0x5F],bx
    cmp     word [si+0x5F],bx ; 40 m/r/m mod=1
    cmp     word [si-0x5F],bx
    cmp     word [di+0x5F],bx ; 40 m/r/m mod=1
    cmp     word [di-0x5F],bx
    cmp     word [bp+0x5F],bx ; 40 m/r/m mod=1
    cmp     word [bp-0x5F],bx
    cmp     word [bx+0x5F],bx ; 40 m/r/m mod=1
    cmp     word [bx-0x5F],bx

    cmp     word [bx+si+0x1234],bx
    cmp     word [bx+di+0x1234],bx
    cmp     word [bp+si+0x1234],bx
    cmp     word [bp+di+0x1234],bx
    cmp     word [si+0x1234],bx
    cmp     word [di+0x1234],bx
    cmp     word [bp+0x1234],bx
    cmp     word [bx+0x1234],bx
    cmp     bx,ax           ; 00 m/r/m mod=3
    cmp     bx,cx           ; 00 m/r/m mod=3
    cmp     bx,dx           ; 00 m/r/m mod=3
    cmp     bx,bx           ; 00 m/r/m mod=3
    cmp     bx,si
    cmp     bx,di
    cmp     bx,bp
    cmp     bx,sp

;--------------dword
    cmp     dword [ebx+esi],ebx ; 00 m/r/m mod=0
    cmp     dword [ebx+edi],ebx ; 00 m/r/m mod=0
    cmp     dword [ebp+esi],ebx ; 00 m/r/m mod=0
    cmp     dword [ebp+edi],ebx ; 00 m/r/m mod=0
    cmp     dword [esi],ebx    ; 00 m/r/m mod=0
    cmp     dword [edi],ebx    ; 00 m/r/m mod=0
    a32 cmp dword [0x12345678],ebx; 00 m/r/m mod=0
    cmp     dword [ebx],ebx    ; 00 m/r/m mod=0

;--------------byte
    cmp     bl,byte [bx+si] ; 00 m/r/m mod=0
    cmp     bl,byte [bx+di] ; 00 m/r/m mod=0
    cmp     bl,byte [bp+si] ; 00 m/r/m mod=0
    cmp     bl,byte [bp+di] ; 00 m/r/m mod=0
    cmp     bl,byte [si]    ; 00 m/r/m mod=0
    cmp     bl,byte [di]    ; 00 m/r/m mod=0
    cmp     bl,byte [0x1234]; 00 m/r/m mod=0
    cmp     bl,byte [bx]    ; 00 m/r/m mod=0

    cmp     bl,byte [bx+si+0x5F] ; 40 m/r/m mod=1
    cmp     bl,byte [bx+si-0x5F]
    cmp     bl,byte [bx+di+0x5F] ; 40 m/r/m mod=1
    cmp     bl,byte [bx+di-0x5F]
    cmp     bl,byte [bp+si+0x5F] ; 40 m/r/m mod=1
    cmp     bl,byte [bp+si-0x5F]
    cmp     bl,byte [bp+di+0x5F] ; 40 m/r/m mod=1
    cmp     bl,byte [bp+di-0x5F]
    cmp     bl,byte [si+0x5F] ; 40 m/r/m mod=1
    cmp     bl,byte [si-0x5F]
    cmp     bl,byte [di+0x5F] ; 40 m/r/m mod=1
    cmp     bl,byte [di-0x5F]
    cmp     bl,byte [bp+0x5F] ; 40 m/r/m mod=1
    cmp     bl,byte [bp-0x5F]
    cmp     bl,byte [bx+0x5F] ; 40 m/r/m mod=1
    cmp     bl,byte [bx-0x5F]

    cmp     bl,byte [bx+si+0x1234]
    cmp     bl,byte [bx+di+0x1234]
    cmp     bl,byte [bp+si+0x1234]
    cmp     bl,byte [bp+di+0x1234]
    cmp     bl,byte [si+0x1234]
    cmp     bl,byte [di+0x1234]
    cmp     bl,byte [bp+0x1234]
    cmp     bl,byte [bx+0x1234]

;--------------word
    cmp     bx,word [bx+si] ; 00 m/r/m mod=0
    cmp     bx,word [bx+di] ; 00 m/r/m mod=0
    cmp     bx,word [bp+si] ; 00 m/r/m mod=0
    cmp     bx,word [bp+di] ; 00 m/r/m mod=0
    cmp     bx,word [si]    ; 00 m/r/m mod=0
    cmp     bx,word [di]    ; 00 m/r/m mod=0
    cmp     bx,word [0x1234]; 00 m/r/m mod=0
    cmp     bx,word [bx]    ; 00 m/r/m mod=0

    cmp     bx,word [bx+si+0x5F] ; 40 m/r/m mod=1
    cmp     bx,word [bx+si-0x5F]
    cmp     bx,word [bx+di+0x5F] ; 40 m/r/m mod=1
    cmp     bx,word [bx+di-0x5F]
    cmp     bx,word [bp+si+0x5F] ; 40 m/r/m mod=1
    cmp     bx,word [bp+si-0x5F]
    cmp     bx,word [bp+di+0x5F] ; 40 m/r/m mod=1
    cmp     bx,word [bp+di-0x5F]
    cmp     bx,word [si+0x5F] ; 40 m/r/m mod=1
    cmp     bx,word [si-0x5F]
    cmp     bx,word [di+0x5F] ; 40 m/r/m mod=1
    cmp     bx,word [di-0x5F]
    cmp     bx,word [bp+0x5F] ; 40 m/r/m mod=1
    cmp     bx,word [bp-0x5F]
    cmp     bx,word [bx+0x5F] ; 40 m/r/m mod=1
    cmp     bx,word [bx-0x5F]

    cmp     bx,word [bx+si+0x1234]
    cmp     bx,word [bx+di+0x1234]
    cmp     bx,word [bp+si+0x1234]
    cmp     bx,word [bp+di+0x1234]
    cmp     bx,word [si+0x1234]
    cmp     bx,word [di+0x1234]
    cmp     bx,word [bp+0x1234]
    cmp     bx,word [bx+0x1234]

;---------------cmp imm
    cmp     al,0x12
    cmp     al,0xEF
    cmp     ax,0x1234
    cmp     ax,0xFEDC
    cmp     eax,0x12345678
    cmp     eax,0xFEDCBA98

; GRP1 80h
    add     byte [bx+si],0x12
    add     byte [bx+di],0x12
    add     byte [bp+si],0x12
    add     byte [bp+di],0x12
    add     byte [si],0x12
    add     byte [di],0x12
    add     byte [0x1234],0x12
    add     byte [bx],0x12
    add     cl,0x12
    add     dl,0x12

    or      byte [bx+si],0x12
    or      byte [bx+di],0x12
    or      byte [bp+si],0x12
    or      byte [bp+di],0x12
    or      byte [si],0x12
    or      byte [di],0x12
    or      byte [0x1234],0x12
    or      byte [bx],0x12
    or      cl,0x12
    or      dl,0x12

    adc     byte [bx+si],0x12
    adc     byte [bx+di],0x12
    adc     byte [bp+si],0x12
    adc     byte [bp+di],0x12
    adc     byte [si],0x12
    adc     byte [di],0x12
    adc     byte [0x1234],0x12
    adc     byte [bx],0x12
    adc     cl,0x12
    adc     dl,0x12

    sbb     byte [bx+si],0x12
    sbb     byte [bx+di],0x12
    sbb     byte [bp+si],0x12
    sbb     byte [bp+di],0x12
    sbb     byte [si],0x12
    sbb     byte [di],0x12
    sbb     byte [0x1234],0x12
    sbb     byte [bx],0x12
    sbb     cl,0x12
    sbb     dl,0x12

    and     byte [bx+si],0x12
    and     byte [bx+di],0x12
    and     byte [bp+si],0x12
    and     byte [bp+di],0x12
    and     byte [si],0x12
    and     byte [di],0x12
    and     byte [0x1234],0x12
    and     byte [bx],0x12
    and     cl,0x12
    and     dl,0x12

    sub     byte [bx+si],0x12
    sub     byte [bx+di],0x12
    sub     byte [bp+si],0x12
    sub     byte [bp+di],0x12
    sub     byte [si],0x12
    sub     byte [di],0x12
    sub     byte [0x1234],0x12
    sub     byte [bx],0x12
    sub     cl,0x12
    sub     dl,0x12

    xor     byte [bx+si],0x12
    xor     byte [bx+di],0x12
    xor     byte [bp+si],0x12
    xor     byte [bp+di],0x12
    xor     byte [si],0x12
    xor     byte [di],0x12
    xor     byte [0x1234],0x12
    xor     byte [bx],0x12
    xor     cl,0x12
    xor     dl,0x12

    cmp     byte [bx+si],0x12
    cmp     byte [bx+di],0x12
    cmp     byte [bp+si],0x12
    cmp     byte [bp+di],0x12
    cmp     byte [si],0x12
    cmp     byte [di],0x12
    cmp     byte [0x1234],0x12
    cmp     byte [bx],0x12
    cmp     cl,0x12
    cmp     dl,0x12

; GRP1 81h
    add     word [bx+si],0x1234
    add     word [bx+di],0x1234
    add     word [bp+si],0x1234
    add     word [bp+di],0x1234
    add     word [si],0x1234
    add     word [di],0x1234
    add     word [0x1234],0x1234
    add     word [bx],0x1234
    add     cx,0x1234
    add     dx,0x1234

    or      word [bx+si],0x1234
    or      word [bx+di],0x1234
    or      word [bp+si],0x1234
    or      word [bp+di],0x1234
    or      word [si],0x1234
    or      word [di],0x1234
    or      word [0x1234],0x1234
    or      word [bx],0x1234
    or      cx,0x1234
    or      dx,0x1234

    adc     word [bx+si],0x1234
    adc     word [bx+di],0x1234
    adc     word [bp+si],0x1234
    adc     word [bp+di],0x1234
    adc     word [si],0x1234
    adc     word [di],0x1234
    adc     word [0x1234],0x1234
    adc     word [bx],0x1234
    adc     cx,0x1234
    adc     dx,0x1234

    sbb     word [bx+si],0x1234
    sbb     word [bx+di],0x1234
    sbb     word [bp+si],0x1234
    sbb     word [bp+di],0x1234
    sbb     word [si],0x1234
    sbb     word [di],0x1234
    sbb     word [0x1234],0x1234
    sbb     word [bx],0x1234
    sbb     cx,0x1234
    sbb     dx,0x1234

    and     word [bx+si],0x1234
    and     word [bx+di],0x1234
    and     word [bp+si],0x1234
    and     word [bp+di],0x1234
    and     word [si],0x1234
    and     word [di],0x1234
    and     word [0x1234],0x1234
    and     word [bx],0x1234
    and     cx,0x1234
    and     dx,0x1234

    sub     word [bx+si],0x1234
    sub     word [bx+di],0x1234
    sub     word [bp+si],0x1234
    sub     word [bp+di],0x1234
    sub     word [si],0x1234
    sub     word [di],0x1234
    sub     word [0x1234],0x1234
    sub     word [bx],0x1234
    sub     cx,0x1234
    sub     dx,0x1234

    xor     word [bx+si],0x1234
    xor     word [bx+di],0x1234
    xor     word [bp+si],0x1234
    xor     word [bp+di],0x1234
    xor     word [si],0x1234
    xor     word [di],0x1234
    xor     word [0x1234],0x1234
    xor     word [bx],0x1234
    xor     cx,0x1234
    xor     dx,0x1234

    cmp     word [bx+si],0x1234
    cmp     word [bx+di],0x1234
    cmp     word [bp+si],0x1234
    cmp     word [bp+di],0x1234
    cmp     word [si],0x1234
    cmp     word [di],0x1234
    cmp     word [0x1234],0x1234
    cmp     word [bx],0x1234
    cmp     cx,0x1234
    cmp     dx,0x1234

; GRP1 82h alias
    db      0x82,0x00,0x12

; GRP1 83h
    add     word [bx+si],0x5E
    add     word [bx+di],0x5E
    add     word [bp+si],0x5E
    add     word [bp+di],0x5E
    add     word [si],0x5E
    add     word [di],0x5E
    add     word [0x1234],0x5E
    add     word [bx],0x5E
    add     cx,0x5E
    add     dx,0x5E

    or      word [bx+si],0x5E
    or      word [bx+di],0x5E
    or      word [bp+si],0x5E
    or      word [bp+di],0x5E
    or      word [si],0x5E
    or      word [di],0x5E
    or      word [0x1234],0x5E
    or      word [bx],0x5E
    or      cx,0x5E
    or      dx,0x5E

    adc     word [bx+si],0x5E
    adc     word [bx+di],0x5E
    adc     word [bp+si],0x5E
    adc     word [bp+di],0x5E
    adc     word [si],0x5E
    adc     word [di],0x5E
    adc     word [0x1234],0x5E
    adc     word [bx],0x5E
    adc     cx,0x5E
    adc     dx,0x5E

    sbb     word [bx+si],0x5E
    sbb     word [bx+di],0x5E
    sbb     word [bp+si],0x5E
    sbb     word [bp+di],0x5E
    sbb     word [si],0x5E
    sbb     word [di],0x5E
    sbb     word [0x1234],0x5E
    sbb     word [bx],0x5E
    sbb     cx,0x5E
    sbb     dx,0x5E

    and     word [bx+si],0x5E
    and     word [bx+di],0x5E
    and     word [bp+si],0x5E
    and     word [bp+di],0x5E
    and     word [si],0x5E
    and     word [di],0x5E
    and     word [0x1234],0x5E
    and     word [bx],0x5E
    and     cx,0x5E
    and     dx,0x5E

    sub     word [bx+si],0x5E
    sub     word [bx+di],0x5E
    sub     word [bp+si],0x5E
    sub     word [bp+di],0x5E
    sub     word [si],0x5E
    sub     word [di],0x5E
    sub     word [0x1234],0x5E
    sub     word [bx],0x5E
    sub     cx,0x5E
    sub     dx,0x5E

    xor     word [bx+si],0x5E
    xor     word [bx+di],0x5E
    xor     word [bp+si],0x5E
    xor     word [bp+di],0x5E
    xor     word [si],0x5E
    xor     word [di],0x5E
    xor     word [0x1234],0x5E
    xor     word [bx],0x5E
    xor     cx,0x5E
    xor     dx,0x5E

    cmp     word [bx+si],0x5E
    cmp     word [bx+di],0x5E
    cmp     word [bp+si],0x5E
    cmp     word [bp+di],0x5E
    cmp     word [si],0x5E
    cmp     word [di],0x5E
    cmp     word [0x1234],0x5E
    cmp     word [bx],0x5E
    cmp     cx,0x5E
    cmp     dx,0x5E

; GRP1 83h
    add     word [bx+si],-0x56
    add     word [bx+di],-0x56
    add     word [bp+si],-0x56
    add     word [bp+di],-0x56
    add     word [si],-0x56
    add     word [di],-0x56
    add     word [0x1234],-0x56
    add     word [bx],-0x56
    add     cx,-0x56
    add     dx,-0x56

    or      word [bx+si],-0x56
    or      word [bx+di],-0x56
    or      word [bp+si],-0x56
    or      word [bp+di],-0x56
    or      word [si],-0x56
    or      word [di],-0x56
    or      word [0x1234],-0x56
    or      word [bx],-0x56
    or      cx,-0x56
    or      dx,-0x56

    adc     word [bx+si],-0x56
    adc     word [bx+di],-0x56
    adc     word [bp+si],-0x56
    adc     word [bp+di],-0x56
    adc     word [si],-0x56
    adc     word [di],-0x56
    adc     word [0x1234],-0x56
    adc     word [bx],-0x56
    adc     cx,-0x56
    adc     dx,-0x56

    sbb     word [bx+si],-0x56
    sbb     word [bx+di],-0x56
    sbb     word [bp+si],-0x56
    sbb     word [bp+di],-0x56
    sbb     word [si],-0x56
    sbb     word [di],-0x56
    sbb     word [0x1234],-0x56
    sbb     word [bx],-0x56
    sbb     cx,-0x56
    sbb     dx,-0x56

    and     word [bx+si],-0x56
    and     word [bx+di],-0x56
    and     word [bp+si],-0x56
    and     word [bp+di],-0x56
    and     word [si],-0x56
    and     word [di],-0x56
    and     word [0x1234],-0x56
    and     word [bx],-0x56
    and     cx,-0x56
    and     dx,-0x56

    sub     word [bx+si],-0x56
    sub     word [bx+di],-0x56
    sub     word [bp+si],-0x56
    sub     word [bp+di],-0x56
    sub     word [si],-0x56
    sub     word [di],-0x56
    sub     word [0x1234],-0x56
    sub     word [bx],-0x56
    sub     cx,-0x56
    sub     dx,-0x56

    xor     word [bx+si],-0x56
    xor     word [bx+di],-0x56
    xor     word [bp+si],-0x56
    xor     word [bp+di],-0x56
    xor     word [si],-0x56
    xor     word [di],-0x56
    xor     word [0x1234],-0x56
    xor     word [bx],-0x56
    xor     cx,-0x56
    xor     dx,-0x56

    cmp     word [bx+si],-0x56
    cmp     word [bx+di],-0x56
    cmp     word [bp+si],-0x56
    cmp     word [bp+di],-0x56
    cmp     word [si],-0x56
    cmp     word [di],-0x56
    cmp     word [0x1234],-0x56
    cmp     word [bx],-0x56
    cmp     cx,-0x56
    cmp     dx,-0x56

; GRP1 81h 32-bit
    add     dword [ebx+esi],0x12345678
    add     dword [ebx+edi],0x12345678
    add     dword [ebp+esi],0x12345678
    add     dword [ebp+edi],0x12345678
    add     dword [esi],0x12345678
    add     dword [edi],0x12345678
    a32 add dword [0x12345678],0x12345678
    add     dword [ebx],0x12345678
    add     ecx,0x12345678
    add     edx,0x12345678

    or      dword [ebx+esi],0x12345678
    or      dword [ebx+edi],0x12345678
    or      dword [ebp+esi],0x12345678
    or      dword [ebp+edi],0x12345678
    or      dword [esi],0x12345678
    or      dword [edi],0x12345678
    a32 or  dword [0x12345678],0x12345678
    or      dword [ebx],0x12345678
    or      ecx,0x12345678
    or      edx,0x12345678

    adc     dword [ebx+esi],0x12345678
    adc     dword [ebx+edi],0x12345678
    adc     dword [ebp+esi],0x12345678
    adc     dword [ebp+edi],0x12345678
    adc     dword [esi],0x12345678
    adc     dword [edi],0x12345678
    a32 adc dword [0x12345678],0x12345678
    adc     dword [ebx],0x12345678
    adc     ecx,0x12345678
    adc     edx,0x12345678

    sbb     dword [ebx+esi],0x12345678
    sbb     dword [ebx+edi],0x12345678
    sbb     dword [ebp+esi],0x12345678
    sbb     dword [ebp+edi],0x12345678
    sbb     dword [esi],0x12345678
    sbb     dword [edi],0x12345678
    a32 sbb dword [0x12345678],0x12345678
    sbb     dword [ebx],0x12345678
    sbb     ecx,0x12345678
    sbb     edx,0x12345678

    and     dword [ebx+esi],0x12345678
    and     dword [ebx+edi],0x12345678
    and     dword [ebp+esi],0x12345678
    and     dword [ebp+edi],0x12345678
    and     dword [esi],0x12345678
    and     dword [edi],0x12345678
    a32 and dword [0x12345678],0x12345678
    and     dword [ebx],0x12345678
    and     ecx,0x12345678
    and     edx,0x12345678

    sub     dword [ebx+esi],0x12345678
    sub     dword [ebx+edi],0x12345678
    sub     dword [ebp+esi],0x12345678
    sub     dword [ebp+edi],0x12345678
    sub     dword [esi],0x12345678
    sub     dword [edi],0x12345678
    a32 sub dword [0x12345678],0x12345678
    sub     dword [ebx],0x12345678
    sub     ecx,0x12345678
    sub     edx,0x12345678

    xor     dword [ebx+esi],0x12345678
    xor     dword [ebx+edi],0x12345678
    xor     dword [ebp+esi],0x12345678
    xor     dword [ebp+edi],0x12345678
    xor     dword [esi],0x12345678
    xor     dword [edi],0x12345678
    a32 xor dword [0x12345678],0x12345678
    xor     dword [ebx],0x12345678
    xor     ecx,0x12345678
    xor     edx,0x12345678

    cmp     dword [ebx+esi],0x12345678
    cmp     dword [ebx+edi],0x12345678
    cmp     dword [ebp+esi],0x12345678
    cmp     dword [ebp+edi],0x12345678
    cmp     dword [esi],0x12345678
    cmp     dword [edi],0x12345678
    a32 cmp dword [0x12345678],0x12345678
    cmp     dword [ebx],0x12345678
    cmp     ecx,0x12345678
    cmp     edx,0x12345678

; GRP1 83h 32-bit
    add     dword [ebx+esi],-0x56
    add     dword [ebx+edi],-0x56
    add     dword [ebp+esi],-0x56
    add     dword [ebp+edi],-0x56
    add     dword [esi],-0x56
    add     dword [edi],-0x56
    a32 add dword [0x12345678],-0x56
    add     dword [ebx],-0x56
    add     ecx,-0x56
    add     edx,-0x56

    or      dword [ebx+esi],-0x56
    or      dword [ebx+edi],-0x56
    or      dword [ebp+esi],-0x56
    or      dword [ebp+edi],-0x56
    or      dword [esi],-0x56
    or      dword [edi],-0x56
    a32 or  dword [0x12345678],-0x56
    or      dword [ebx],-0x56
    or      ecx,-0x56
    or      edx,-0x56

    adc     dword [ebx+esi],-0x56
    adc     dword [ebx+edi],-0x56
    adc     dword [ebp+esi],-0x56
    adc     dword [ebp+edi],-0x56
    adc     dword [esi],-0x56
    adc     dword [edi],-0x56
    a32 adc dword [0x12345678],-0x56
    adc     dword [ebx],-0x56
    adc     ecx,-0x56
    adc     edx,-0x56

    sbb     dword [ebx+esi],-0x56
    sbb     dword [ebx+edi],-0x56
    sbb     dword [ebp+esi],-0x56
    sbb     dword [ebp+edi],-0x56
    sbb     dword [esi],-0x56
    sbb     dword [edi],-0x56
    a32 sbb dword [0x12345678],-0x56
    sbb     dword [ebx],-0x56
    sbb     ecx,-0x56
    sbb     edx,-0x56

    and     dword [ebx+esi],-0x56
    and     dword [ebx+edi],-0x56
    and     dword [ebp+esi],-0x56
    and     dword [ebp+edi],-0x56
    and     dword [esi],-0x56
    and     dword [edi],-0x56
    a32 and dword [0x12345678],-0x56
    and     dword [ebx],-0x56
    and     ecx,-0x56
    and     edx,-0x56

    sub     dword [ebx+esi],-0x56
    sub     dword [ebx+edi],-0x56
    sub     dword [ebp+esi],-0x56
    sub     dword [ebp+edi],-0x56
    sub     dword [esi],-0x56
    sub     dword [edi],-0x56
    a32 sub dword [0x12345678],-0x56
    sub     dword [ebx],-0x56
    sub     ecx,-0x56
    sub     edx,-0x56

    xor     dword [ebx+esi],-0x56
    xor     dword [ebx+edi],-0x56
    xor     dword [ebp+esi],-0x56
    xor     dword [ebp+edi],-0x56
    xor     dword [esi],-0x56
    xor     dword [edi],-0x56
    a32 xor dword [0x12345678],-0x56
    xor     dword [ebx],-0x56
    xor     ecx,-0x56
    xor     edx,-0x56

    cmp     dword [ebx+esi],-0x56
    cmp     dword [ebx+edi],-0x56
    cmp     dword [ebp+esi],-0x56
    cmp     dword [ebp+edi],-0x56
    cmp     dword [esi],-0x56
    cmp     dword [edi],-0x56
    a32 cmp dword [0x12345678],-0x56
    cmp     dword [ebx],-0x56
    cmp     ecx,-0x56
    cmp     edx,-0x56

; TEST 84h-85h
    test    bl,cl           ; 84h
    test    bl,dl           ; 84h
    test    cl,dl
    test    dl,al

    test    bx,cx
    test    bx,dx
    test    cx,dx
    test    dx,ax

    test    ebx,ecx
    test    ebx,edx
    test    ecx,edx
    test    edx,eax

    test    byte [bx+si],cl
    test    byte [bx+di],dl
    test    byte [0x1234],dl
    test    byte [bx],dl
    test    byte [bx+si+0x1234],cl
    test    byte [bx+di+0x1234],dl
    test    byte [bp+0x1234],dl
    test    byte [bx+0x1234],dl

    test    word [bx+si],cx
    test    word [bx+di],dx
    test    word [0x1234],dx
    test    word [bx],dx
    test    word [bx+si+0x1234],cx
    test    word [bx+di+0x1234],dx
    test    word [bp+0x1234],dx
    test    word [bx+0x1234],dx

    test    dword [bx+si],ecx
    test    dword [bx+di],edx
    test    dword [0x1234],edx
    test    dword [bx],edx
    test    dword [bx+si+0x1234],ecx
    test    dword [bx+di+0x1234],edx
    test    dword [bp+0x1234],edx
    test    dword [bx+0x1234],edx

    test    dword [ebx+esi],ecx
    test    dword [ebx+edi],edx
    a32 test dword [0x12345678],edx
    test    dword [ebx],edx
    test    dword [ebx+esi+0x1234],ecx
    test    dword [ebx+edi+0x1234],edx
    test    dword [ebp+0x1234],edx
    test    dword [ebx+0x1234],edx

; XOR 86h-87h
    xchg    bl,cl           ; 86h
    xchg    bl,dl           ; 86h
    xchg    cl,dl
    xchg    dl,al

    xchg    bx,cx
    xchg    bx,dx
    xchg    cx,dx
    xchg    dx,ax

    xchg    ebx,ecx
    xchg    ebx,edx
    xchg    ecx,edx
    xchg    edx,eax

    xchg    byte [bx+si],cl
    xchg    byte [bx+di],dl
    xchg    byte [0x1234],dl
    xchg    byte [bx],dl
    xchg    byte [bx+si+0x1234],cl
    xchg    byte [bx+di+0x1234],dl
    xchg    byte [bp+0x1234],dl
    xchg    byte [bx+0x1234],dl

    xchg    word [bx+si],cx
    xchg    word [bx+di],dx
    xchg    word [0x1234],dx
    xchg    word [bx],dx
    xchg    word [bx+si+0x1234],cx
    xchg    word [bx+di+0x1234],dx
    xchg    word [bp+0x1234],dx
    xchg    word [bx+0x1234],dx

    xchg    dword [bx+si],ecx
    xchg    dword [bx+di],edx
    xchg    dword [0x1234],edx
    xchg    dword [bx],edx
    xchg    dword [bx+si+0x1234],ecx
    xchg    dword [bx+di+0x1234],edx
    xchg    dword [bp+0x1234],edx
    xchg    dword [bx+0x1234],edx

    xchg    dword [ebx+esi],ecx
    xchg    dword [ebx+edi],edx
    a32 xchg dword [0x12345678],edx
    xchg    dword [ebx],edx
    xchg    dword [ebx+esi+0x1234],ecx
    xchg    dword [ebx+edi+0x1234],edx
    xchg    dword [ebp+0x1234],edx
    xchg    dword [ebx+0x1234],edx

; MOV 88-8B
    mov     bl,cl
    mov     bl,dl
    mov     cl,dl
    mov     dl,al

    mov     bx,cx
    mov     bx,dx
    mov     cx,dx
    mov     dx,ax

    mov     ebx,ecx
    mov     ebx,edx
    mov     ecx,edx
    mov     edx,eax

    mov     byte [bx+si],cl
    mov     byte [bx+di],dl
    mov     byte [0x1234],dl
    mov     byte [bx],dl
    mov     byte [bx+si+0x1234],cl
    mov     byte [bx+di+0x1234],dl
    mov     byte [bp+0x1234],dl
    mov     byte [bx+0x1234],dl

    mov     word [bx+si],cx
    mov     word [bx+di],dx
    mov     word [0x1234],dx
    mov     word [bx],dx
    mov     word [bx+si+0x1234],cx
    mov     word [bx+di+0x1234],dx
    mov     word [bp+0x1234],dx
    mov     word [bx+0x1234],dx

    mov     dword [bx+si],ecx
    mov     dword [bx+di],edx
    mov     dword [0x1234],edx
    mov     dword [bx],edx
    mov     dword [bx+si+0x1234],ecx
    mov     dword [bx+di+0x1234],edx
    mov     dword [bp+0x1234],edx
    mov     dword [bx+0x1234],edx

    mov     dword [ebx+esi],ecx
    mov     dword [ebx+edi],edx
    a32 mov dword [0x12345678],edx
    mov     dword [ebx],edx
    mov     dword [ebx+esi+0x1234],ecx
    mov     dword [ebx+edi+0x1234],edx
    mov     dword [ebp+0x1234],edx
    mov     dword [ebx+0x1234],edx

    mov     cl,byte [bx+si]
    mov     dl,byte [bx+di]
    mov     dl,byte [0x1234]
    mov     dl,byte [bx]
    mov     cl,byte [bx+si+0x1234]
    mov     dl,byte [bx+di+0x1234]
    mov     dl,byte [bp+0x1234]
    mov     dl,byte [bx+0x1234]

    mov     cx,word [bx+si]
    mov     dx,word [bx+di]
    mov     dx,word [0x1234]
    mov     dx,word [bx]
    mov     cx,word [bx+si+0x1234]
    mov     dx,word [bx+di+0x1234]
    mov     dx,word [bp+0x1234]
    mov     dx,word [bx+0x1234]

    mov     ecx,dword [bx+si]
    mov     edx,dword [bx+di]
    mov     edx,dword [0x1234]
    mov     edx,dword [bx]
    mov     ecx,dword [bx+si+0x1234]
    mov     edx,dword [bx+di+0x1234]
    mov     edx,dword [bp+0x1234]
    mov     edx,dword [bx+0x1234]

    mov     ecx,dword [ebx+esi]
    mov     edx,dword [ebx+edi]
    a32 mov edx,dword [0x12345678]
    mov     edx,dword [ebx]
    mov     ecx,dword [ebx+esi+0x1234]
    mov     edx,dword [ebx+edi+0x1234]
    mov     edx,dword [ebp+0x1234]
    mov     edx,dword [ebx+0x1234]

; mov r/m,sreg
    mov     ax,cs
    mov     cs,ax
    mov     ax,ds
    mov     ds,ax
    mov     ax,es
    mov     es,ax
    mov     ax,ss
    mov     ss,ax

    mov     eax,cs
    o32 mov cs,eax
    mov     eax,ds
    o32 mov ds,eax
    mov     eax,es
    o32 mov es,eax
    mov     eax,ss
    o32 mov ss,eax

    mov     word [bx+si],cs
    mov     word [bx+di],cs
    mov     word [0x1234],cs
    mov     cs,word [bx+si]
    mov     cs,word [bx+di]
    mov     cs,word [0x1234]

    mov     word [bx+si],ds
    mov     word [bx+di],ds
    mov     word [0x1234],ds
    mov     ds,word [bx+si]
    mov     ds,word [bx+di]
    mov     ds,word [0x1234]

; lea
    lea     ax,[si]
    lea     ax,[bx+si]
    lea     ax,[0x1234]
    lea     ax,[bx+si+0x1234]
    lea     bx,[bx+di-0x44]
    db      0x8D,0xC0       ; LEA AX,AX (illegal, YASM won't encode it)
    db      0x8D,0xC1       ; LEA AX,CX
    db      0x8D,0xC8       ; LEA CX,AX

; FIXME: What does the 80386 do with LEA if opcodesize == 32 and addrsize == 16?
    lea     eax,[si]
    lea     eax,[bx+si]
    lea     eax,[0x1234]
    lea     eax,[bx+si+0x1234]
    lea     ebx,[bx+di-0x44]

    lea     eax,[esi]
    lea     eax,[ebx+esi]
    a32 lea eax,[0x12345678]
    lea     eax,[ebx+esi+0x12345678]
    lea     ebx,[ebx+edi-0x44]

; pop 8Fh
    pop     word [si]
    pop     word [di]
    pop     word [bx+si]
    pop     word [bx+di]
    pop     word [0x1234]
    db      0x8F,0xC0       ; POP AX
    db      0x8F,0xC1       ; POP CX

    pop     dword [esi]
    pop     dword [edi]
    pop     dword [ebx+esi]
    pop     dword [ebx+edi]
    a32 pop dword [0x12345678]
    db      0x66,0x8F,0xC0       ; POP EAX
    db      0x66,0x8F,0xC1       ; POP ECX

; call Ap (0x9A)
    call    0x1234:0x5678
    call    0xABCD:0x1234
    o32 call 0x1234:0x56789ABC
    o32 call 0xABCD:0x12345678

; MOV 0xA0-0xA3
    mov     al,[0x1234]
    mov     ax,[0x1234]
    mov     eax,[0x1234]
    mov     [0x1234],al
    mov     [0x1234],ax
    mov     [0x1234],eax

    a32 mov al,[0x12345678]
    a32 mov ax,[0x12345678]
    a32 mov eax,[0x12345678]
    a32 mov [0x12345678],al
    a32 mov [0x12345678],ax
    a32 mov [0x12345678],eax

; TEST A8-A9
    test    al,0x12
    test    ax,0x1234
    test    eax,0x12345678

; MOV B0-B7
    mov     al,0x12
    mov     cl,0x12
    mov     dl,0x12
    mov     bl,0x12
    mov     ah,0x12
    mov     ch,0x12
    mov     dh,0x12
    mov     bh,0x12

; MOV B8-BF
    mov     ax,0x1234
    mov     cx,0x1234
    mov     dx,0x1234
    mov     bx,0x1234
    mov     sp,0x1234
    mov     bp,0x1234
    mov     si,0x1234
    mov     di,0x1234

; MOV B8-BF
    mov     eax,0x12345678
    mov     ecx,0x12345678
    mov     edx,0x12345678
    mov     ebx,0x12345678
    mov     esp,0x12345678
    mov     ebp,0x12345678
    mov     esi,0x12345678
    mov     edi,0x12345678

; lgs (386 or later)
    lgs     ax,[si]
    lgs     ax,[bx+si]
    lgs     ax,[0x1234]
    lgs     ax,[bx+si+0x1234]
    lgs     bx,[bx+di-0x44]
    lgs     eax,[esi]
    lgs     eax,[ebx+esi]
    a32 lgs eax,[0x12345678]
    lgs     eax,[ebx+esi+0x1234]
    lgs     ebx,[ebx+edi-0x44]

; lfs (386 or later)
    lfs     ax,[si]
    lfs     ax,[bx+si]
    lfs     ax,[0x1234]
    lfs     ax,[bx+si+0x1234]
    lfs     bx,[bx+di-0x44]
    lfs     eax,[esi]
    lfs     eax,[ebx+esi]
    a32 lfs eax,[0x12345678]
    lfs     eax,[ebx+esi+0x1234]
    lfs     ebx,[ebx+edi-0x44]

; lss (386 or later)
    lss     ax,[si]
    lss     ax,[bx+si]
    lss     ax,[0x1234]
    lss     ax,[bx+si+0x1234]
    lss     bx,[bx+di-0x44]
    lss     eax,[esi]
    lss     eax,[ebx+esi]
    a32 lss eax,[0x12345678]
    lss     eax,[ebx+esi+0x1234]
    lss     ebx,[ebx+edi-0x44]

; les
    les     ax,[si]
    les     ax,[bx+si]
    les     ax,[0x1234]
    les     ax,[bx+si+0x1234]
    les     bx,[bx+di-0x44]
    les     eax,[esi]
    les     eax,[ebx+esi]
    a32 les eax,[0x12345678]
    les     eax,[ebx+esi+0x1234]
    les     ebx,[ebx+edi-0x44]

; lds
    lds     ax,[si]
    lds     ax,[bx+si]
    lds     ax,[0x1234]
    lds     ax,[bx+si+0x1234]
    lds     bx,[bx+di-0x44]
    lds     eax,[esi]
    lds     eax,[ebx+esi]
    a32 lds eax,[0x12345678]
    lds     eax,[ebx+esi+0x1234]
    lds     ebx,[ebx+edi-0x44]

; MOV C6-C7
    mov     byte [bx+si],0x12
    mov     byte [bx+si+0x1234],0x12
    mov     byte [0x1234],0x12
    mov     byte [bx],0x12
    mov     word [bx+si],0x1234
    mov     word [bx+si+0x1234],0x1234
    mov     word [0x1234],0x1234
    mov     word [bx],0x1234
    db      0xC6,0xC0,0x12      ; MOV AL,0x12
    db      0xC6,0xC1,0x12      ; MOV AH,0x12
    db      0xC7,0xC0,0x34,0x12 ; MOV AX,0x1234
    db      0xC7,0xC1,0x34,0x12 ; MOV CX,0x1234
    db      0x66,0xC7,0xC0,0x78,0x56,0x34,0x12 ; MOV EAX,0x12345678
    db      0x66,0xC7,0xC1,0x78,0x56,0x34,0x12 ; MOV ECX,0x12345678
    mov     dword [ebx+esi],0x12345678
    mov     dword [ebx+esi+0x1234],0x12345678
    a32 mov dword [0x12345678],0x12345678
    mov     dword [ebx],0x12345678

; into
    into

; loop/jcxz
looptest1:
    loopnz  looptest1
    o32 loopnz looptest1
    loopz   looptest1
    o32 loopz looptest1
    loop    looptest1
    o32 loop looptest1
    jcxz    looptest1
    o32 jcxz looptest1

; CALL/JMP E8-EB
calltest1:
    call    near calltest1      ; E8
    o32 call near calltest1
    jmp     near calltest1      ; E9
    o32 jmp near calltest1
    jmp     0x1234:0x5678       ; EA
    o32 jmp 0x1234:0x56789ABC
    jmp     short calltest1     ; EB
    o32 jmp short calltest1
    call    0x1234:0x5678
    o32 call 0x1234:0x56789ABC

; GRP2 D0
    rol     al,1
    rol     bl,1
    rol     byte [bx+si],1
    rol     byte [bx+di],1
    rol     byte [0x1234],1

    ror     al,1
    ror     bl,1
    ror     byte [bx+si],1
    ror     byte [bx+di],1
    ror     byte [0x1234],1

    rcl     al,1
    rcl     bl,1
    rcl     byte [bx+si],1
    rcl     byte [bx+di],1
    rcl     byte [0x1234],1

    rcr     al,1
    rcr     bl,1
    rcr     byte [bx+si],1
    rcr     byte [bx+di],1
    rcr     byte [0x1234],1

    shl     al,1
    shl     bl,1
    shl     byte [bx+si],1
    shl     byte [bx+di],1
    shl     byte [0x1234],1

    shr     al,1
    shr     bl,1
    shr     byte [bx+si],1
    shr     byte [bx+di],1
    shr     byte [0x1234],1

; nasm/yasm won't encode SAL as SAL but as an alias of SHL
    db      0xD0,0xF0           ; sal     al,1
    db      0xD0,0xF3           ; sal     bl,1
    db      0xD0,0x30           ; sal     byte [bx+si],1
    db      0xD0,0x31           ; sal     byte [bx+di],1
    db      0xD0,0x36,0x34,0x12 ; sal     byte [0x1234],1

    sar     al,1
    sar     bl,1
    sar     byte [bx+si],1
    sar     byte [bx+di],1
    sar     byte [0x1234],1

; GRP2 D1
    rol     ax,1
    rol     bx,1
    rol     word [bx+si],1
    rol     word [bx+di],1
    rol     word [0x1234],1

    ror     ax,1
    ror     bx,1
    ror     word [bx+si],1
    ror     word [bx+di],1
    ror     word [0x1234],1

    rcl     ax,1
    rcl     bx,1
    rcl     word [bx+si],1
    rcl     word [bx+di],1
    rcl     word [0x1234],1

    rcr     ax,1
    rcr     bx,1
    rcr     word [bx+si],1
    rcr     word [bx+di],1
    rcr     word [0x1234],1

    shl     ax,1
    shl     bx,1
    shl     word [bx+si],1
    shl     word [bx+di],1
    shl     word [0x1234],1

    shr     ax,1
    shr     bx,1
    shr     word [bx+si],1
    shr     word [bx+di],1
    shr     word [0x1234],1

; nasm/yasm won't encode SAL as SAL but as an alias of SHL
    db      0xD1,0xF0           ; sal     ax,1
    db      0xD1,0xF3           ; sal     bx,1
    db      0xD1,0x30           ; sal     word [bx+si],1
    db      0xD1,0x31           ; sal     word [bx+di],1
    db      0xD1,0x36,0x34,0x12 ; sal     word [0x1234],1

    sar     ax,1
    sar     bx,1
    sar     word [bx+si],1
    sar     word [bx+di],1
    sar     word [0x1234],1

; GRP2 D1 32-bit
    rol     eax,1
    rol     ebx,1
    rol     dword [ebx+esi],1
    rol     dword [ebx+edi],1
    a32 rol dword [0x12345678],1

    ror     eax,1
    ror     ebx,1
    ror     dword [ebx+esi],1
    ror     dword [ebx+edi],1
    a32 ror dword [0x12345678],1

    rcl     eax,1
    rcl     ebx,1
    rcl     dword [ebx+esi],1
    rcl     dword [ebx+edi],1
    a32 rcl dword [0x12345678],1

    rcr     eax,1
    rcr     ebx,1
    rcr     dword [ebx+esi],1
    rcr     dword [ebx+edi],1
    a32 rcr dword [0x12345678],1

    shl     eax,1
    shl     ebx,1
    shl     dword [ebx+esi],1
    shl     dword [ebx+edi],1
    a32 shl dword [0x12345678],1

    shr     eax,1
    shr     ebx,1
    shr     dword [ebx+esi],1
    shr     dword [ebx+edi],1
    a32 shr dword [0x12345678],1

; nasm/yasm won't encode SAL as SAL but as an alias of SHL
    db      0x66,0xD1,0xF0                  ; sal     eax,1
    db      0x66,0xD1,0xF3                  ; sal     ebx,1
    db      0x66,0x67,0xD1,0x34,0x33        ; sal     dword [ebx+esi],1
    db      0x66,0x67,0xD1,0x34,0x3B        ; sal     dword [ebx+edi],1
    db      0x66,0x67,0xD1,0x35,0x78,0x56,0x34,0x12 ; a32 sal dword [0x12345678],1

    sar     eax,1
    sar     ebx,1
    sar     dword [ebx+esi],1
    sar     dword [ebx+edi],1
    a32 sar dword [0x12345678],1

; GRP2 D2
    rol     al,cl
    rol     bl,cl
    rol     byte [bx+si],cl
    rol     byte [bx+di],cl
    rol     byte [0x1234],cl

    ror     al,cl
    ror     bl,cl
    ror     byte [bx+si],cl
    ror     byte [bx+di],cl
    ror     byte [0x1234],cl

    rcl     al,cl
    rcl     bl,cl
    rcl     byte [bx+si],cl
    rcl     byte [bx+di],cl
    rcl     byte [0x1234],cl

    rcr     al,cl
    rcr     bl,cl
    rcr     byte [bx+si],cl
    rcr     byte [bx+di],cl
    rcr     byte [0x1234],cl

    shl     al,cl
    shl     bl,cl
    shl     byte [bx+si],cl
    shl     byte [bx+di],cl
    shl     byte [0x1234],cl

    shr     al,cl
    shr     bl,cl
    shr     byte [bx+si],cl
    shr     byte [bx+di],cl
    shr     byte [0x1234],cl

; nasm/yasm won't encode SAL as SAL but as an alias of SHL
    db      0xD2,0xF0           ; sal     al,cl
    db      0xD2,0xF3           ; sal     bl,cl
    db      0xD2,0x30           ; sal     byte [bx+si],cl
    db      0xD2,0x31           ; sal     byte [bx+di],cl
    db      0xD2,0x36,0x34,0x12 ; sal     byte [0x1234],cl

    sar     al,cl
    sar     bl,cl
    sar     byte [bx+si],cl
    sar     byte [bx+di],cl
    sar     byte [0x1234],cl

; GRP2 D3
    rol     ax,cl
    rol     bx,cl
    rol     word [bx+si],cl
    rol     word [bx+di],cl
    rol     word [0x1234],cl

    ror     ax,cl
    ror     bx,cl
    ror     word [bx+si],cl
    ror     word [bx+di],cl
    ror     word [0x1234],cl

    rcl     ax,cl
    rcl     bx,cl
    rcl     word [bx+si],cl
    rcl     word [bx+di],cl
    rcl     word [0x1234],cl

    rcr     ax,cl
    rcr     bx,cl
    rcr     word [bx+si],cl
    rcr     word [bx+di],cl
    rcr     word [0x1234],cl

    shl     ax,cl
    shl     bx,cl
    shl     word [bx+si],cl
    shl     word [bx+di],cl
    shl     word [0x1234],cl

    shr     ax,cl
    shr     bx,cl
    shr     word [bx+si],cl
    shr     word [bx+di],cl
    shr     word [0x1234],cl

; nasm/yasm won't encode SAL as SAL but as an alias of SHL
    db      0xD3,0xF0           ; sal     ax,cl
    db      0xD3,0xF3           ; sal     bx,cl
    db      0xD3,0x30           ; sal     word [bx+si],cl
    db      0xD3,0x31           ; sal     word [bx+di],cl
    db      0xD3,0x36,0x34,0x12 ; sal     word [0x1234],cl

    sar     ax,cl
    sar     bx,cl
    sar     word [bx+si],cl
    sar     word [bx+di],cl
    sar     word [0x1234],cl

; GRP2 D3 32-bit
    rol     eax,cl
    rol     ebx,cl
    rol     dword [ebx+esi],cl
    rol     dword [ebx+edi],cl
    a32 rol dword [0x12345678],cl

    ror     eax,cl
    ror     ebx,cl
    ror     dword [ebx+esi],cl
    ror     dword [ebx+edi],cl
    a32 ror dword [0x12345678],cl

    rcl     eax,cl
    rcl     ebx,cl
    rcl     dword [ebx+esi],cl
    rcl     dword [ebx+edi],cl
    a32 rcl dword [0x12345678],cl

    rcr     eax,cl
    rcr     ebx,cl
    rcr     dword [ebx+esi],cl
    rcr     dword [ebx+edi],cl
    a32 rcr dword [0x12345678],cl

    shl     eax,cl
    shl     ebx,cl
    shl     dword [ebx+esi],cl
    shl     dword [ebx+edi],cl
    a32 shl dword [0x12345678],cl

    shr     eax,cl
    shr     ebx,cl
    shr     word [ebx+esi],cl
    shr     word [ebx+edi],cl
    a32 shr word [0x12345678],cl

; nasm/yasm won't encode SAL as SAL but as an alias of SHL
    db      0x66,0xD3,0xF0                  ; sal     eax,cl
    db      0x66,0xD3,0xF3                  ; sal     ebx,cl
    db      0x66,0x67,0xD3,0x34,0x33        ; sal     dword [ebx+esi],cl
    db      0x66,0x67,0xD3,0x34,0x3B        ; sal     dword [ebx+edi],cl
    db      0x66,0x67,0xD3,0x35,0x78,0x56,0x34,0x12 ; sal     dword [0x12345678],cl

    sar     eax,cl
    sar     ebx,cl
    sar     dword [ebx+esi],cl
    sar     dword [ebx+edi],cl
    a32 sar dword [0x12345678],cl

; GRP3 F6-F7
    test    byte [bx+si],0x12
    test    byte [bx+di],0x12
    test    byte [0x1234],0x12
    db      0xF6,0x08,0x12      ; TEST BYTE [BX+SI],0x12   reg == 1 undocumented alias
    not     al
    not     ah
    not     bl
    not     bh
    neg     al
    neg     bl
    mul     al
    mul     bl
    imul    al
    imul    bl
    div     al
    div     bl
    idiv    al
    idiv    bl
    not     byte [bx+si]
    neg     byte [bx+si]
    mul     byte [bx+si]
    imul    byte [bx+si]
    div     byte [bx+si]
    idiv    byte [bx+si]

    test    word [bx+si],0x1234
    test    word [bx+di],0x1234
    test    word [0x1234],0x1234
    db      0xF7,0x08,0x32,0x12      ; TEST WORD [BX+SI],0x1234   reg == 1 undocumented alias
    not     ax
    not     bx
    not     cx
    not     dx
    neg     ax
    neg     bx
    mul     ax
    mul     bx
    imul    ax
    imul    bx
    div     ax
    div     bx
    idiv    ax
    idiv    bx
    not     word [bx+si]
    neg     word [bx+si]
    mul     word [bx+si]
    imul    word [bx+si]
    div     word [bx+si]
    idiv    word [bx+si]

    test    dword [ebx+esi],0x12345678
    test    dword [ebx+edi],0x12345678
    a32 test dword [0x12345678],0x12345678
    db      0x66,0x67,0xF7,0x0C,0x33,0x78,0x56,0x32,0x12      ; TEST WORD [EBX+ESI],0x12345678   reg == 1 undocumented alias
    not     eax
    not     ebx
    not     ecx
    not     edx
    neg     eax
    neg     ebx
    mul     eax
    mul     ebx
    imul    eax
    imul    ebx
    div     eax
    div     ebx
    idiv    eax
    idiv    ebx
    not     dword [ebx+esi]
    neg     dword [ebx+esi]
    mul     dword [ebx+esi]
    imul    dword [ebx+esi]
    div     dword [ebx+esi]
    idiv    dword [ebx+esi]

; GRP4 FE-FF
    inc     al
    inc     ah
    inc     bl
    inc     bh
    inc     byte [bx+si]
    inc     byte [bx+di]
    inc     byte [0x1234]
    inc     byte [bx]

    dec     al
    dec     ah
    dec     bl
    dec     bh
    dec     byte [bx+si]
    dec     byte [bx+di]
    dec     byte [0x1234]
    dec     byte [bx]

    db      0xFF,0xC0       ; inc     ax
    db      0xFF,0xC3       ; inc     bx
    db      0xFF,0xC1       ; inc     cx
    db      0xFF,0xC2       ; inc     dx
    inc     word [bx+si]
    inc     word [bx+di]
    inc     word [0x1234]
    inc     word [bx]
    inc     dword [ebx+esi]
    inc     dword [ebx+edi]
    a32 inc dword [0x12345678]
    inc     dword [ebx]

    db      0xFF,0xC8       ; dec     ax
    db      0xFF,0xCB       ; dec     bx
    db      0xFF,0xC9       ; dec     cx
    db      0xFF,0xCA       ; dec     dx
    dec     word [bx+si]
    dec     word [bx+di]
    dec     word [0x1234]
    dec     word [bx]
    dec     dword [ebx+esi]
    dec     dword [ebx+edi]
    a32 dec dword [0x12345678]
    dec     dword [ebx]

    call    ax
    call    word [bx]
    call    word [bx+si]
    call    word [bx+di]
    call    word [0x1234]
    call    bx

    call    eax
    call    dword [ebx]
    call    dword [ebx+esi]
    call    dword [ebx+edi]
    a32 call dword [0x12345678]
    call    ebx

    jmp     ax
    jmp     word [bx]
    jmp     word [bx+si]
    jmp     word [bx+di]
    jmp     word [0x1234]
    jmp     bx

    jmp     eax
    jmp     dword [ebx]
    jmp     dword [ebx+esi]
    jmp     dword [ebx+edi]
    a32 jmp dword [0x12345678]
    jmp     ebx

    call far word [bx]
    call far word [bx+si]
    call far word [bx+di]
    call far word [0x1234]

    call far dword [ebx]
    call far dword [ebx+esi]
    call far dword [ebx+edi]
    a32 call far dword [0x12345678]

    jmp far word [bx]
    jmp far word [bx+si]
    jmp far word [bx+di]
    jmp far word [0x1234]

    jmp far word [ebx]
    jmp far word [ebx+esi]
    jmp far word [ebx+edi]
    a32 jmp far word [0x12345678]

    push    word [bx]
    push    word [bx+si]
    push    word [bx+di]
    push    word [0x1234]
    db      0xFF,0xF0       ; PUSH AX
    db      0xFF,0xF1       ; PUSH CX
    db      0xFF,0xF2       ; PUSH DX
    db      0xFF,0xF3       ; PUSH BX

    push    dword [ebx]
    push    dword [ebx+esi]
    push    dword [ebx+edi]
    a32 push dword [0x12345678]

    db      0xFE,0x10       ; illegal FE (byte CALL!)
    db      0xFE,0x18       ; illegal FE (byte CALL FAR)

    db      0xFF,0xFF       ; illegal FF

; undocumented 8086
    salc
    db      0xF1            ; alias of LOCK prefix (http://www.os2museum.com/wp/undocumented-8086-opcodes/)
    db      0x90,0x90       ; nop

; FPU
    fnop                    ; 0xD9(ESC + 0x1) 0xD0
    fchs                    ; 0xD9(ESC + 0x1) 0xE0
    fabs                    ; 0xD9(ESC + 0x1) 0xE1
    ftst                    ; 0xD9(ESC + 0x1) 0xE4
    fxam                    ; 0xD9(ESC + 0x1) 0xE5
    fld1                    ; 0xD9(ESC + 0x1) 0xE8
    fldl2t                  ; 0xD9(ESC + 0x1) 0xE9
    fldl2e                  ; 0xD9(ESC + 0x1) 0xEA
    fldpi                   ; 0xD9(ESC + 0x1) 0xEB
    fldlg2                  ; 0xD9(ESC + 0x1) 0xEC
    fldln2                  ; 0xD9(ESC + 0x1) 0xED
    fldz                    ; 0xD9(ESC + 0x1) 0xEE
    fyl2x                   ; 0xD9(ESC + 0x1) 0xF1
    fptan                   ; 0xD9(ESC + 0x1) 0xF2
    fpatan                  ; 0xD9(ESC + 0x1) 0xF3
    fxtract                 ; 0xD9(ESC + 0x1) 0xF4
    fprem1                  ; 0xD9(ESC + 0x1) 0xF5
    fdecstp                 ; 0xD9(ESC + 0x1) 0xF6
    fincstp                 ; 0xD9(ESC + 0x1) 0xF7
    fprem                   ; 0xD9(ESC + 0x1) 0xF8
    fyl2xp1                 ; 0xD9(ESC + 0x1) 0xF9
    fsqrt                   ; 0xD9(ESC + 0x1) 0xFA
    frndint                 ; 0xD9(ESC + 0x1) 0xFC
    fscale                  ; 0xD9(ESC + 0x1) 0xFD

; FPU, going down the instruction list in Intel's 8087 datasheet
; Ref:
;   word integer: 16-bit two's complement
;   short integer: 32-bit two's complement
;   long integer: 64-bit two's complement
;   packed BCD: 10-byte packed BCD
;   short real: 32-bit float
;   long real: 64-bit float
;   temporary real: 80-bit float
;
; Memory Format (MF) bits
;   00b = 32-bit real
;   01b = 32-bit integer
;   10b = 64-bit real
;   11b = 16-bit integer

; FLD     | ESCAPE M F 1 | MOD 0 0 0 R/M |     integer/real memory to ST(0)
; assemblers differentiate by using "FILD" for integer load
    fld     dword [bx]                      ; MF=0 op 0xD9
    fld     dword [bx+si]
    fld     dword [bx+di+0x1234]
    fld     dword [si-6]
    fild    dword [bx]                      ; MF=1 op 0xDB
    fild    dword [bx+si]
    fild    dword [bx+di+0x1234]
    fild    dword [si-6]
    fld     qword [bx]                      ; MF=2 op 0xDD
    fld     qword [bx+si]
    fld     qword [bx+di+0x1234]
    fld     qword [si-6]
    fild    word [bx]                       ; MF=3 op 0xDF
    fild    word [bx+si]
    fild    word [bx+di+0x1234]
    fild    word [si-6]

    fld     dword [ebx]                      ; MF=0 op 0xD9
    fld     dword [ebx+esi]
    fld     dword [ebx+edi+0x1234]
    fld     dword [esi-6]
    fild    dword [ebx]                      ; MF=1 op 0xDB
    fild    dword [ebx+esi]
    fild    dword [ebx+edi+0x1234]
    fild    dword [esi-6]
    fld     qword [ebx]                      ; MF=2 op 0xDD
    fld     qword [ebx+esi]
    fld     qword [ebx+edi+0x1234]
    fld     qword [esi-6]
    fild    word [ebx]                       ; MF=3 op 0xDF
    fild    word [ebx+esi]
    fild    word [ebx+edi+0x1234]
    fild    word [esi-6]

; FLD     | ESCAPE 1 1 1 | MOD 1 0 1 R/M |     long integer memory to ST(0)
; assemblers refer to this by "FILD" and a 64-bit datatype
    fild    qword [bx]
    fild    qword [bx+si]
    fild    qword [bx+di+0x1234]
    fild    qword [si-6]

    fild    qword [ebx]
    fild    qword [ebx+esi]
    fild    qword [ebx+edi+0x1234]
    fild    qword [esi-6]

; FLD     | ESCAPE 0 1 1 | MOD 1 0 1 R/M |     temporary real memory to ST(0)
    fld     tword [bx]
    fld     tword [bx+si]
    fld     tword [bx+di+0x1234]
    fld     tword [si-6]

    fld     tword [ebx]
    fld     tword [ebx+esi]
    fld     tword [ebx+edi+0x1234]
    fld     tword [esi-6]

; FLD     | ESCAPE 1 1 1 | MOD 1 0 0 R/M |     packed BCD memory to ST(0)
; assemblers refer to this by "FBLD" and a 80-bit datatype
    fbld    tword [bx]
    fbld    tword [bx+si]
    fbld    tword [bx+di+0x1234]
    fbld    tword [si-6]

    fbld    tword [ebx]
    fbld    tword [ebx+esi]
    fbld    tword [ebx+edi+0x1234]
    fbld    tword [esi-6]

; FLD     | ESCAPE 0 0 1 | 1 1 0 0 0 R/M |     ST(i) to ST(0) where in R/M, MOD == 3, REG == 0, RM == FPU register index
    fld     st0
    fld     st1
    fld     st2
    fld     st3
    fld     st4
    fld     st5
    fld     st6
    fld     st7

; FST     | ESCAPE M F 1 | MOD 0 1 0 R/M |     ST(0) to integer/real memory
; assemblers differentiate by using "FIST" for integer load
    fst     dword [bx]                      ; MF=0 op 0xD9
    fst     dword [bx+si]
    fst     dword [bx+di+0x1234]
    fst     dword [si-6]
    fist    dword [bx]                      ; MF=1 op 0xDB
    fist    dword [bx+si]
    fist    dword [bx+di+0x1234]
    fist    dword [si-6]
    fst     qword [bx]                      ; MF=2 op 0xDD
    fst     qword [bx+si]
    fst     qword [bx+di+0x1234]
    fst     qword [si-6]
    fist    word [bx]                       ; MF=3 op 0xDF
    fist    word [bx+si]
    fist    word [bx+di+0x1234]
    fist    word [si-6]

    fst     dword [ebx]                      ; MF=0 op 0xD9
    fst     dword [ebx+esi]
    fst     dword [ebx+edi+0x1234]
    fst     dword [esi-6]
    fist    dword [ebx]                      ; MF=1 op 0xDB
    fist    dword [ebx+esi]
    fist    dword [ebx+edi+0x1234]
    fist    dword [esi-6]
    fst     qword [ebx]                      ; MF=2 op 0xDD
    fst     qword [ebx+esi]
    fst     qword [ebx+edi+0x1234]
    fst     qword [esi-6]
    fist    word [ebx]                       ; MF=3 op 0xDF
    fist    word [ebx+esi]
    fist    word [ebx+edi+0x1234]
    fist    word [esi-6]

; FST     | ESCAPE 1 0 1 | 1 1 0 1 0 R/M |     ST(0) to ST(i) where in R/M, MOD == 3, REG == 2, RM == FPU register index
    fst     st0
    fst     st1
    fst     st2
    fst     st3
    fst     st4
    fst     st5
    fst     st6
    fst     st7

; FSTP    | ESCAPE M F 1 | MOD 0 1 1 R/M |     ST(0) to integer/real memory
; assemblers differentiate by using "FISTP" for integer load
    fstp    dword [bx]                      ; MF=0 op 0xD9
    fstp    dword [bx+si]
    fstp    dword [bx+di+0x1234]
    fstp    dword [si-6]
    fistp   dword [bx]                      ; MF=1 op 0xDB
    fistp   dword [bx+si]
    fistp   dword [bx+di+0x1234]
    fistp   dword [si-6]
    fstp    qword [bx]                      ; MF=2 op 0xDD
    fstp    qword [bx+si]
    fstp    qword [bx+di+0x1234]
    fstp    qword [si-6]
    fistp   word [bx]                       ; MF=3 op 0xDF
    fistp   word [bx+si]
    fistp   word [bx+di+0x1234]
    fistp   word [si-6]

    fstp    dword [ebx]                      ; MF=0 op 0xD9
    fstp    dword [ebx+esi]
    fstp    dword [ebx+edi+0x1234]
    fstp    dword [esi-6]
    fistp   dword [ebx]                      ; MF=1 op 0xDB
    fistp   dword [ebx+esi]
    fistp   dword [ebx+edi+0x1234]
    fistp   dword [esi-6]
    fstp    qword [ebx]                      ; MF=2 op 0xDD
    fstp    qword [ebx+esi]
    fstp    qword [ebx+edi+0x1234]
    fstp    qword [esi-6]
    fistp   word [ebx]                       ; MF=3 op 0xDF
    fistp   word [ebx+esi]
    fistp   word [ebx+edi+0x1234]
    fistp   word [esi-6]

; FSTP    | ESCAPE 1 1 1 | MOD 1 1 1 R/M |     ST(0) to long integer memory
; assemblers refer to this by "FISTP" and a 64-bit datatype
    fistp   qword [bx]
    fistp   qword [bx+si]
    fistp   qword [bx+di+0x1234]
    fistp   qword [si-6]

    fistp   qword [ebx]
    fistp   qword [ebx+esi]
    fistp   qword [ebx+edi+0x1234]
    fistp   qword [esi-6]

; FSTP    | ESCAPE 0 1 1 | MOD 1 1 1 R/M |     ST(0) to temporary real memory
    fstp    tword [bx]
    fstp    tword [bx+si]
    fstp    tword [bx+di+0x1234]
    fstp    tword [si-6]

    fstp    tword [ebx]
    fstp    tword [ebx+esi]
    fstp    tword [ebx+edi+0x1234]
    fstp    tword [esi-6]

; FSTP    | ESCAPE 1 1 1 | MOD 1 1 0 R/M |     ST(0) to packed BCD memory
; assemblers refer to this by "FBSTP" and a 80-bit datatype
    fbstp   tword [bx]
    fbstp   tword [bx+si]
    fbstp   tword [bx+di+0x1234]
    fbstp   tword [si-6]

    fbstp   tword [ebx]
    fbstp   tword [ebx+esi]
    fbstp   tword [ebx+edi+0x1234]
    fbstp   tword [esi-6]

; FSTP    | ESCAPE 1 0 1 | 1 1 0 1 1 R/M |     ST(0) to ST(i) where in R/M, MOD == 3, REG == 3, RM == FPU register index
    fstp    st0
    fstp    st1
    fstp    st2
    fstp    st3
    fstp    st4
    fstp    st5
    fstp    st6
    fstp    st7

; FXCH    | ESCAPE 0 0 1 | 1 1 0 0 1 R/M |     Exchange ST(0) and ST(i)
    fxch    st0
    fxch    st1
    fxch    st2
    fxch    st3
    fxch    st4
    fxch    st5
    fxch    st6
    fxch    st7

; FCOM    | ESCAPE M F 0 | MOD 0 1 0 R/M |     Integer/Real compare to ST(0)
; also known as "FICOM" for integer compares
    fcom    dword [bx]                      ; MF=0 op 0xD8
    fcom    dword [bx+si]
    fcom    dword [bx+di+0x1234]
    fcom    dword [si-6]
    ficom   dword [bx]                      ; MF=1 op 0xDA
    ficom   dword [bx+si]
    ficom   dword [bx+di+0x1234]
    ficom   dword [si-6]
    fcom    qword [bx]                      ; MF=2 op 0xDC
    fcom    qword [bx+si]
    fcom    qword [bx+di+0x1234]
    fcom    qword [si-6]
    ficom   word [bx]                       ; MF=3 op 0xDE
    ficom   word [bx+si]
    ficom   word [bx+di+0x1234]
    ficom   word [si-6]

    fcom    dword [ebx]                      ; MF=0 op 0xD8
    fcom    dword [ebx+esi]
    fcom    dword [ebx+edi+0x1234]
    fcom    dword [esi-6]
    ficom   dword [ebx]                      ; MF=1 op 0xDA
    ficom   dword [ebx+esi]
    ficom   dword [ebx+edi+0x1234]
    ficom   dword [esi-6]
    fcom    qword [ebx]                      ; MF=2 op 0xDC
    fcom    qword [ebx+esi]
    fcom    qword [ebx+edi+0x1234]
    fcom    qword [esi-6]
    ficom   word [ebx]                       ; MF=3 op 0xDE
    ficom   word [ebx+esi]
    ficom   word [ebx+edi+0x1234]
    ficom   word [esi-6]

; FCOM    | ESCAPE 0 0 0 | 1 1 0 1 0 R/M |     Compare ST(i) to ST(0)
    fcom    st0
    fcom    st1
    fcom    st2
    fcom    st3
    fcom    st4
    fcom    st5
    fcom    st6
    fcom    st7

; FCOMP   | ESCAPE M F 0 | MOD 0 1 0 R/M |     Integer/Real compare to ST(0) and pop
; also known as "FICOMP" for integer compares
    fcomp   dword [bx]                      ; MF=0 op 0xD8
    fcomp   dword [bx+si]
    fcomp   dword [bx+di+0x1234]
    fcomp   dword [si-6]
    ficomp  dword [bx]                      ; MF=1 op 0xDA
    ficomp  dword [bx+si]
    ficomp  dword [bx+di+0x1234]
    ficomp  dword [si-6]
    fcomp   qword [bx]                      ; MF=2 op 0xDC
    fcomp   qword [bx+si]
    fcomp   qword [bx+di+0x1234]
    fcomp   qword [si-6]
    ficomp  word [bx]                       ; MF=3 op 0xDE
    ficomp  word [bx+si]
    ficomp  word [bx+di+0x1234]
    ficomp  word [si-6]

    fcomp   dword [ebx]                      ; MF=0 op 0xD8
    fcomp   dword [ebx+esi]
    fcomp   dword [ebx+edi+0x1234]
    fcomp   dword [esi-6]
    ficomp  dword [ebx]                      ; MF=1 op 0xDA
    ficomp  dword [ebx+esi]
    ficomp  dword [ebx+edi+0x1234]
    ficomp  dword [esi-6]
    fcomp   qword [ebx]                      ; MF=2 op 0xDC
    fcomp   qword [ebx+esi]
    fcomp   qword [ebx+edi+0x1234]
    fcomp   qword [esi-6]
    ficomp  word [ebx]                       ; MF=3 op 0xDE
    ficomp  word [ebx+esi]
    ficomp  word [ebx+edi+0x1234]
    ficomp  word [esi-6]

; FCOMP   | ESCAPE 0 0 0 | 1 1 0 1 0 R/M |     Compare ST(i) to ST(0) and pop
    fcomp   st0
    fcomp   st1
    fcomp   st2
    fcomp   st3
    fcomp   st4
    fcomp   st5
    fcomp   st6
    fcomp   st7

; FCOMPP  | ESCAPE 1 1 0 | 1 1 0 1 1 0 0 1 |   Compare ST(1) to ST(0) and pop both
    fcompp

; FADD    | ESCAPE M F 0 | MOD 0 0 0 R/M |     Add integer/real memory to ST(0)
; also known as "FIADD" for integer addition
    fadd    dword [bx]                      ; MF=0 op 0xD8
    fadd    dword [bx+si]
    fadd    dword [bx+di+0x1234]
    fadd    dword [si-6]
    fiadd   dword [bx]                      ; MF=1 op 0xDA
    fiadd   dword [bx+si]
    fiadd   dword [bx+di+0x1234]
    fiadd   dword [si-6]
    fadd    qword [bx]                      ; MF=2 op 0xDC
    fadd    qword [bx+si]
    fadd    qword [bx+di+0x1234]
    fadd    qword [si-6]
    fiadd   word [bx]                       ; MF=3 op 0xDE
    fiadd   word [bx+si]
    fiadd   word [bx+di+0x1234]
    fiadd   word [si-6]

    fadd    dword [ebx]                      ; MF=0 op 0xD8
    fadd    dword [ebx+esi]
    fadd    dword [ebx+edi+0x1234]
    fadd    dword [esi-6]
    fiadd   dword [ebx]                      ; MF=1 op 0xDA
    fiadd   dword [ebx+esi]
    fiadd   dword [ebx+edi+0x1234]
    fiadd   dword [esi-6]
    fadd    qword [ebx]                      ; MF=2 op 0xDC
    fadd    qword [ebx+esi]
    fadd    qword [ebx+edi+0x1234]
    fadd    qword [esi-6]
    fiadd   word [ebx]                       ; MF=3 op 0xDE
    fiadd   word [ebx+esi]
    fiadd   word [ebx+edi+0x1234]
    fiadd   word [esi-6]

; FADD    | ESCAPE d P 0 | 1 1 0 0 0 R/M |     Add ST(i) to ST(0) d=0 (destination is ST(0)) P=0 don't pop after add
    fadd    st0,st0
    fadd    st0,st1
    fadd    st0,st2
    fadd    st0,st3
    fadd    st0,st4
    fadd    st0,st5
    fadd    st0,st6
    fadd    st0,st7

; FADD    | ESCAPE d P 0 | 1 1 0 0 0 R/M |     Add ST(i) to ST(0) d=0 (destination is ST(0)) P=1 pop after add
; Note that YASM won't let me encode these directly. For one, they don't really make sense to use.
; Why add something to ST(0) just to pop it off the stack?
; Second, later i686 processors (Pentium II or higher) re-use these opcodes for the FCMOV instruction (conditional floating point move).
; NASM's disassembler shows the following as FCMOVB instructions.
; However, Intel's 8086 datasheet implies the original 8087 would treat this as a command to FADD and pop, so, that's what we're testing here.
; The datasheet simply documents FADD ST(i),ST(0) as having a D and P (direction and pop) bit in bits 2 and 1 of the first opcode byte.
    db      0xDA,0xC0       ; faddp   st0,st0
    db      0xDA,0xC1       ; faddp   st0,st1   YASM won't let me encode this
    db      0xDA,0xC2       ; faddp   st0,st2
    db      0xDA,0xC3       ; faddp   st0,st3
    db      0xDA,0xC4       ; faddp   st0,st4
    db      0xDA,0xC5       ; faddp   st0,st5
    db      0xDA,0xC6       ; faddp   st0,st6
    db      0xDA,0xC7       ; faddp   st0,st7

; FADD    | ESCAPE d P 0 | 1 1 0 0 0 R/M |     Add ST(i) to ST(0) d=1 (destination is ST(i)) P=0 don't pop after add
    db      0xDC,0xC0       ; fadd    st0,st0
    fadd    st1,st0
    fadd    st2,st0
    fadd    st3,st0
    fadd    st4,st0
    fadd    st5,st0
    fadd    st6,st0
    fadd    st7,st0

; FADD    | ESCAPE d P 0 | 1 1 0 0 0 R/M |     Add ST(i) to ST(0) d=1 (destination is ST(i)) P=1 pop after add
    faddp   st0,st0
    faddp   st1,st0
    faddp   st2,st0
    faddp   st3,st0
    faddp   st4,st0
    faddp   st5,st0
    faddp   st6,st0
    faddp   st7,st0

; FSUB    | ESCAPE M F 0 | MOD 1 0 R R/M |     Subtract integer/real memory from ST(0) R == 0
; also known as "FISUB" for integer subtraction
    fsub    dword [bx]                      ; MF=0 op 0xD8
    fsub    dword [bx+si]
    fsub    dword [bx+di+0x1234]
    fsub    dword [si-6]
    fisub   dword [bx]                      ; MF=1 op 0xDA
    fisub   dword [bx+si]
    fisub   dword [bx+di+0x1234]
    fisub   dword [si-6]
    fsub    qword [bx]                      ; MF=2 op 0xDC
    fsub    qword [bx+si]
    fsub    qword [bx+di+0x1234]
    fsub    qword [si-6]
    fisub   word [bx]                       ; MF=3 op 0xDE
    fisub   word [bx+si]
    fisub   word [bx+di+0x1234]
    fisub   word [si-6]

    fsub    dword [ebx]                      ; MF=0 op 0xD8
    fsub    dword [ebx+esi]
    fsub    dword [ebx+edi+0x1234]
    fsub    dword [esi-6]
    fisub   dword [ebx]                      ; MF=1 op 0xDA
    fisub   dword [ebx+esi]
    fisub   dword [ebx+edi+0x1234]
    fisub   dword [esi-6]
    fsub    qword [ebx]                      ; MF=2 op 0xDC
    fsub    qword [ebx+esi]
    fsub    qword [ebx+edi+0x1234]
    fsub    qword [esi-6]
    fisub   word [ebx]                       ; MF=3 op 0xDE
    fisub   word [ebx+esi]
    fisub   word [ebx+edi+0x1234]
    fisub   word [esi-6]

; FSUB    | ESCAPE d P 0 | 1 1 1 0 R R/M |     Subtract ST(i) from ST(0) d=0 (destination is ST(0)) P=0 don't pop after add R=0 dest OP src
    fsub    st0,st0
    fsub    st0,st1
    fsub    st0,st2
    fsub    st0,st3
    fsub    st0,st4
    fsub    st0,st5
    fsub    st0,st6
    fsub    st0,st7

; FSUB    | ESCAPE d P 0 | 1 1 1 0 R R/M |     Subtract ST(i) to ST(0) d=0 (destination is ST(0)) P=1 pop after add R=0 dest OP src
; Note that YASM won't let me encode these directly. For one, they don't really make sense to use.
; Why add something to ST(0) just to pop it off the stack?
; Second, later i686 processors (Pentium II or higher) re-use these opcodes for the FCMOV instruction (conditional floating point move).
; NASM's disassembler shows the following as ?????? instructions.
; However, Intel's 8086 datasheet implies the original 8087 would treat this as a command to FADD and pop, so, that's what we're testing here.
; The datasheet simply documents FSUB ST(i),ST(0) as having a D and P (direction and pop) bit in bits 2 and 1 of the first opcode byte.
    db      0xDA,0xE0       ; fsubp   st0,st0
    db      0xDA,0xE1       ; fsubp   st0,st1   YASM won't let me encode this
    db      0xDA,0xE2       ; fsubp   st0,st2
    db      0xDA,0xE3       ; fsubp   st0,st3
    db      0xDA,0xE4       ; fsubp   st0,st4
    db      0xDA,0xE5       ; fsubp   st0,st5
    db      0xDA,0xE6       ; fsubp   st0,st6
    db      0xDA,0xE7       ; fsubp   st0,st7

; FSUB    | ESCAPE d P 0 | 1 1 1 0 R R/M |     Sub ST(i) to ST(0) d=1 (destination is ST(i)) P=0 don't pop after add R=0 dest OP src
    db      0xDC,0xE0       ; fsub    st0,st0   YASM prefers encoding fsub ST(i),ST(0) form with R=1, won't encode this form
    db      0xDC,0xE1       ; fsub    st1,st0
    db      0xDC,0xE2       ; fsub    st2,st0
    db      0xDC,0xE3       ; fsub    st3,st0
    db      0xDC,0xE4       ; fsub    st4,st0
    db      0xDC,0xE5       ; fsub    st5,st0
    db      0xDC,0xE6       ; fsub    st6,st0
    db      0xDC,0xE7       ; fsub    st7,st0

; FSUB    | ESCAPE d P 0 | 1 1 1 0 R R/M |     Sub ST(i) to ST(0) d=1 (destination is ST(i)) P=1 pop after add R=0 dest OP src
    db      0xDE,0xE0       ; fsubp   st0,st0   YASM prefers encoding fsubp ST(i),ST(0) form with R=1, won't encode this form
    db      0xDE,0xE1       ; fsubp   st1,st0
    db      0xDE,0xE2       ; fsubp   st2,st0
    db      0xDE,0xE3       ; fsubp   st3,st0
    db      0xDE,0xE4       ; fsubp   st4,st0
    db      0xDE,0xE5       ; fsubp   st5,st0
    db      0xDE,0xE6       ; fsubp   st6,st0
    db      0xDE,0xE7       ; fsubp   st7,st0

; FSUB    | ESCAPE M F 0 | MOD 1 0 R R/M |     Subtract integer/real memory from ST(0) R == 1
; also known as "FISUB" for integer subtraction
    fsubr   dword [bx]                      ; MF=0 op 0xD8
    fsubr   dword [bx+si]
    fsubr   dword [bx+di+0x1234]
    fsubr   dword [si-6]
    fisubr  dword [bx]                      ; MF=1 op 0xDA
    fisubr  dword [bx+si]
    fisubr  dword [bx+di+0x1234]
    fisubr  dword [si-6]
    fsubr   qword [bx]                      ; MF=2 op 0xDC
    fsubr   qword [bx+si]
    fsubr   qword [bx+di+0x1234]
    fsubr   qword [si-6]
    fisubr  word [bx]                       ; MF=3 op 0xDE
    fisubr  word [bx+si]
    fisubr  word [bx+di+0x1234]
    fisubr  word [si-6]

    fsubr   dword [ebx]                      ; MF=0 op 0xD8
    fsubr   dword [ebx+esi]
    fsubr   dword [ebx+edi+0x1234]
    fsubr   dword [esi-6]
    fisubr  dword [ebx]                      ; MF=1 op 0xDA
    fisubr  dword [ebx+esi]
    fisubr  dword [ebx+edi+0x1234]
    fisubr  dword [esi-6]
    fsubr   qword [ebx]                      ; MF=2 op 0xDC
    fsubr   qword [ebx+esi]
    fsubr   qword [ebx+edi+0x1234]
    fsubr   qword [esi-6]
    fisubr  word [ebx]                       ; MF=3 op 0xDE
    fisubr  word [ebx+esi]
    fisubr  word [ebx+edi+0x1234]
    fisubr  word [esi-6]

; FSUB    | ESCAPE d P 0 | 1 1 1 0 R R/M |     Subtract ST(i) from ST(0) d=0 (destination is ST(0)) P=0 don't pop after add R=1 src OP dest
    fsubr   st0,st0
    fsubr   st0,st1
    fsubr   st0,st2
    fsubr   st0,st3
    fsubr   st0,st4
    fsubr   st0,st5
    fsubr   st0,st6
    fsubr   st0,st7

; FSUB    | ESCAPE d P 0 | 1 1 1 0 R R/M |     Subtract ST(i) to ST(0) d=0 (destination is ST(0)) P=1 pop after add R=1 src OP dest
    db      0xDA,0xE8       ; fsubrp   st0,st0
    db      0xDA,0xE9       ; fsubrp   st0,st1   YASM won't let me encode this
    db      0xDA,0xEA       ; fsubrp   st0,st2
    db      0xDA,0xEB       ; fsubrp   st0,st3
    db      0xDA,0xEC       ; fsubrp   st0,st4
    db      0xDA,0xED       ; fsubrp   st0,st5
    db      0xDA,0xEE       ; fsubrp   st0,st6
    db      0xDA,0xEF       ; fsubrp   st0,st7

; FSUB    | ESCAPE d P 0 | 1 1 1 0 R R/M |     Sub ST(i) to ST(0) d=1 (destination is ST(i)) P=0 don't pop after add R=0 dest OP src
    db      0xDC,0xE8       ; fsubr    st0,st0   YASM prefers encoding fsub ST(i),ST(0) form with R=1, won't encode this form
    db      0xDC,0xE9       ; fsubr    st1,st0
    db      0xDC,0xEA       ; fsubr    st2,st0
    db      0xDC,0xEB       ; fsubr    st3,st0
    db      0xDC,0xEC       ; fsubr    st4,st0
    db      0xDC,0xED       ; fsubr    st5,st0
    db      0xDC,0xEE       ; fsubr    st6,st0
    db      0xDC,0xEF       ; fsubr    st7,st0

; FSUB    | ESCAPE d P 0 | 1 1 1 0 R R/M |     Sub ST(i) to ST(0) d=1 (destination is ST(i)) P=1 pop after add R=0 dest OP src
    db      0xDE,0xE8       ; fsubrp   st0,st0   YASM prefers encoding fsubp ST(i),ST(0) form with R=1, won't encode this form
    db      0xDE,0xE9       ; fsubrp   st1,st0
    db      0xDE,0xEA       ; fsubrp   st2,st0
    db      0xDE,0xEB       ; fsubrp   st3,st0
    db      0xDE,0xEC       ; fsubrp   st4,st0
    db      0xDE,0xED       ; fsubrp   st5,st0
    db      0xDE,0xEE       ; fsubrp   st6,st0
    db      0xDE,0xEF       ; fsubrp   st7,st0

; FMUL    | ESCAPE M F 0 | MOD 0 0 1 R/M |     Multiply integer/real memory to ST(0)
; also known as "FIMUL" for integer addition
    fmul    dword [bx]                      ; MF=0 op 0xD8
    fmul    dword [bx+si]
    fmul    dword [bx+di+0x1234]
    fmul    dword [si-6]
    fimul   dword [bx]                      ; MF=1 op 0xDA
    fimul   dword [bx+si]
    fimul   dword [bx+di+0x1234]
    fimul   dword [si-6]
    fmul    qword [bx]                      ; MF=2 op 0xDC
    fmul    qword [bx+si]
    fmul    qword [bx+di+0x1234]
    fmul    qword [si-6]
    fimul   word [bx]                       ; MF=3 op 0xDE
    fimul   word [bx+si]
    fimul   word [bx+di+0x1234]
    fimul   word [si-6]

    fmul    dword [ebx]                      ; MF=0 op 0xD8
    fmul    dword [ebx+esi]
    fmul    dword [ebx+edi+0x1234]
    fmul    dword [esi-6]
    fimul   dword [ebx]                      ; MF=1 op 0xDA
    fimul   dword [ebx+esi]
    fimul   dword [ebx+edi+0x1234]
    fimul   dword [esi-6]
    fmul    qword [ebx]                      ; MF=2 op 0xDC
    fmul    qword [ebx+esi]
    fmul    qword [ebx+edi+0x1234]
    fmul    qword [esi-6]
    fimul   word [ebx]                       ; MF=3 op 0xDE
    fimul   word [ebx+esi]
    fimul   word [ebx+edi+0x1234]
    fimul   word [esi-6]

; FMUL    | ESCAPE d P 0 | 1 1 0 0 1 R/M |     Multiply ST(i) to ST(0) d=0 (destination is ST(0)) P=0 don't pop after add
    fmul    st0,st0
    fmul    st0,st1
    fmul    st0,st2
    fmul    st0,st3
    fmul    st0,st4
    fmul    st0,st5
    fmul    st0,st6
    fmul    st0,st7

; FMUL    | ESCAPE d P 0 | 1 1 0 0 1 R/M |     Multiply ST(i) to ST(0) d=0 (destination is ST(0)) P=1 pop after add
    db      0xDA,0xC8       ; fmulp   st0,st0
    db      0xDA,0xC9       ; fmulp   st0,st1   YASM won't let me encode this
    db      0xDA,0xCA       ; fmulp   st0,st2
    db      0xDA,0xCB       ; fmulp   st0,st3
    db      0xDA,0xCC       ; fmulp   st0,st4
    db      0xDA,0xCD       ; fmulp   st0,st5
    db      0xDA,0xCE       ; fmulp   st0,st6
    db      0xDA,0xCF       ; fmulp   st0,st7

; FMUL    | ESCAPE d P 0 | 1 1 0 0 1 R/M |     Multiply ST(i) to ST(0) d=1 (destination is ST(i)) P=0 don't pop after add
    db      0xDC,0xC8       ; fmul    st0,st0
    fmul    st1,st0
    fmul    st2,st0
    fmul    st3,st0
    fmul    st4,st0
    fmul    st5,st0
    fmul    st6,st0
    fmul    st7,st0

; FMUL    | ESCAPE d P 0 | 1 1 0 0 1 R/M |     Add ST(i) to ST(0) d=1 (destination is ST(i)) P=1 pop after add
    fmulp   st0,st0
    fmulp   st1,st0
    fmulp   st2,st0
    fmulp   st3,st0
    fmulp   st4,st0
    fmulp   st5,st0
    fmulp   st6,st0
    fmulp   st7,st0

; FDIV    | ESCAPE M F 0 | MOD 1 1 R R/M |     Divide integer/real memory from ST(0) R == 0
; also known as "FIDIV" for integer subtraction
    fdiv    dword [bx]                      ; MF=0 op 0xD8
    fdiv    dword [bx+si]
    fdiv    dword [bx+di+0x1234]
    fdiv    dword [si-6]
    fidiv   dword [bx]                      ; MF=1 op 0xDA
    fidiv   dword [bx+si]
    fidiv   dword [bx+di+0x1234]
    fidiv   dword [si-6]
    fdiv    qword [bx]                      ; MF=2 op 0xDC
    fdiv    qword [bx+si]
    fdiv    qword [bx+di+0x1234]
    fdiv    qword [si-6]
    fidiv   word [bx]                       ; MF=3 op 0xDE
    fidiv   word [bx+si]
    fidiv   word [bx+di+0x1234]
    fidiv   word [si-6]

    fdiv    dword [ebx]                      ; MF=0 op 0xD8
    fdiv    dword [ebx+esi]
    fdiv    dword [ebx+edi+0x1234]
    fdiv    dword [esi-6]
    fidiv   dword [ebx]                      ; MF=1 op 0xDA
    fidiv   dword [ebx+esi]
    fidiv   dword [ebx+edi+0x1234]
    fidiv   dword [esi-6]
    fdiv    qword [ebx]                      ; MF=2 op 0xDC
    fdiv    qword [ebx+esi]
    fdiv    qword [ebx+edi+0x1234]
    fdiv    qword [esi-6]
    fidiv   word [ebx]                       ; MF=3 op 0xDE
    fidiv   word [ebx+esi]
    fidiv   word [ebx+edi+0x1234]
    fidiv   word [esi-6]

; FDIV    | ESCAPE d P 0 | 1 1 1 1 R R/M |     Divide ST(i) from ST(0) d=0 (destination is ST(0)) P=0 don't pop after add R=0 dest OP src
    fdiv    st0,st0
    fdiv    st0,st1
    fdiv    st0,st2
    fdiv    st0,st3
    fdiv    st0,st4
    fdiv    st0,st5
    fdiv    st0,st6
    fdiv    st0,st7

; FDIV    | ESCAPE d P 0 | 1 1 1 1 R R/M |     Divide ST(i) to ST(0) d=0 (destination is ST(0)) P=1 pop after add R=0 dest OP src
    db      0xDA,0xF0       ; fdivp   st0,st0
    db      0xDA,0xF1       ; fdivp   st0,st1   YASM won't let me encode this
    db      0xDA,0xF2       ; fdivp   st0,st2
    db      0xDA,0xF3       ; fdivp   st0,st3
    db      0xDA,0xF4       ; fdivp   st0,st4
    db      0xDA,0xF5       ; fdivp   st0,st5
    db      0xDA,0xF6       ; fdivp   st0,st6
    db      0xDA,0xF7       ; fdivp   st0,st7

; FDIV    | ESCAPE d P 0 | 1 1 1 1 R R/M |     Sub ST(i) to ST(0) d=1 (destination is ST(i)) P=0 don't pop after add R=0 dest OP src
    db      0xDC,0xF0       ; fdiv    st0,st0   YASM prefers encoding fdiv ST(i),ST(0) form with R=1, won't encode this form
    db      0xDC,0xF1       ; fdiv    st1,st0
    db      0xDC,0xF2       ; fdiv    st2,st0
    db      0xDC,0xF3       ; fdiv    st3,st0
    db      0xDC,0xF4       ; fdiv    st4,st0
    db      0xDC,0xF5       ; fdiv    st5,st0
    db      0xDC,0xF6       ; fdiv    st6,st0
    db      0xDC,0xF7       ; fdiv    st7,st0

; FDIV    | ESCAPE d P 0 | 1 1 1 1 R R/M |     Sub ST(i) to ST(0) d=1 (destination is ST(i)) P=1 pop after add R=0 dest OP src
    db      0xDE,0xF0       ; fdivp   st0,st0   YASM prefers encoding fdivp ST(i),ST(0) form with R=1, won't encode this form
    db      0xDE,0xF1       ; fdivp   st1,st0
    db      0xDE,0xF2       ; fdivp   st2,st0
    db      0xDE,0xF3       ; fdivp   st3,st0
    db      0xDE,0xF4       ; fdivp   st4,st0
    db      0xDE,0xF5       ; fdivp   st5,st0
    db      0xDE,0xF6       ; fdivp   st6,st0
    db      0xDE,0xF7       ; fdivp   st7,st0

; FDIV    | ESCAPE M F 0 | MOD 1 1 R R/M |     Subtract integer/real memory from ST(0) R == 1
; also known as "FIDIV" for integer subtraction
    fdivr   dword [bx]                      ; MF=0 op 0xD8
    fdivr   dword [bx+si]
    fdivr   dword [bx+di+0x1234]
    fdivr   dword [si-6]
    fidivr  dword [bx]                      ; MF=1 op 0xDA
    fidivr  dword [bx+si]
    fidivr  dword [bx+di+0x1234]
    fidivr  dword [si-6]
    fdivr   qword [bx]                      ; MF=2 op 0xDC
    fdivr   qword [bx+si]
    fdivr   qword [bx+di+0x1234]
    fdivr   qword [si-6]
    fidivr  word [bx]                       ; MF=3 op 0xDE
    fidivr  word [bx+si]
    fidivr  word [bx+di+0x1234]
    fidivr  word [si-6]

    fdivr   dword [ebx]                      ; MF=0 op 0xD8
    fdivr   dword [ebx+esi]
    fdivr   dword [ebx+edi+0x1234]
    fdivr   dword [esi-6]
    fidivr  dword [ebx]                      ; MF=1 op 0xDA
    fidivr  dword [ebx+esi]
    fidivr  dword [ebx+edi+0x1234]
    fidivr  dword [esi-6]
    fdivr   qword [ebx]                      ; MF=2 op 0xDC
    fdivr   qword [ebx+esi]
    fdivr   qword [ebx+edi+0x1234]
    fdivr   qword [esi-6]
    fidivr  word [ebx]                       ; MF=3 op 0xDE
    fidivr  word [ebx+esi]
    fidivr  word [ebx+edi+0x1234]
    fidivr  word [esi-6]

; FDIV    | ESCAPE d P 0 | 1 1 1 1 R R/M |     Subtract ST(i) from ST(0) d=0 (destination is ST(0)) P=0 don't pop after add R=1 src OP dest
    fdivr   st0,st0
    fdivr   st0,st1
    fdivr   st0,st2
    fdivr   st0,st3
    fdivr   st0,st4
    fdivr   st0,st5
    fdivr   st0,st6
    fdivr   st0,st7

; FDIV    | ESCAPE d P 0 | 1 1 1 1 R R/M |     Subtract ST(i) to ST(0) d=0 (destination is ST(0)) P=1 pop after add R=1 src OP dest
    db      0xDA,0xF8       ; fdivrp   st0,st0
    db      0xDA,0xF9       ; fdivrp   st0,st1   YASM won't let me encode this
    db      0xDA,0xFA       ; fdivrp   st0,st2
    db      0xDA,0xFB       ; fdivrp   st0,st3
    db      0xDA,0xFC       ; fdivrp   st0,st4
    db      0xDA,0xFD       ; fdivrp   st0,st5
    db      0xDA,0xFE       ; fdivrp   st0,st6
    db      0xDA,0xFF       ; fdivrp   st0,st7

; FDIV    | ESCAPE d P 0 | 1 1 1 1 R R/M |     Sub ST(i) to ST(0) d=1 (destination is ST(i)) P=0 don't pop after add R=0 dest OP src
    db      0xDC,0xF8       ; fdivr    st0,st0   YASM prefers encoding fdiv ST(i),ST(0) form with R=1, won't encode this form
    db      0xDC,0xF9       ; fdivr    st1,st0
    db      0xDC,0xFA       ; fdivr    st2,st0
    db      0xDC,0xFB       ; fdivr    st3,st0
    db      0xDC,0xFC       ; fdivr    st4,st0
    db      0xDC,0xFD       ; fdivr    st5,st0
    db      0xDC,0xFE       ; fdivr    st6,st0
    db      0xDC,0xFF       ; fdivr    st7,st0

; FDIV    | ESCAPE d P 0 | 1 1 1 1 R R/M |     Sub ST(i) to ST(0) d=1 (destination is ST(i)) P=1 pop after add R=0 dest OP src
    db      0xDE,0xF8       ; fdivrp   st0,st0   YASM prefers encoding fdivp ST(i),ST(0) form with R=1, won't encode this form
    db      0xDE,0xF9       ; fdivrp   st1,st0
    db      0xDE,0xFA       ; fdivrp   st2,st0
    db      0xDE,0xFB       ; fdivrp   st3,st0
    db      0xDE,0xFC       ; fdivrp   st4,st0
    db      0xDE,0xFD       ; fdivrp   st5,st0
    db      0xDE,0xFE       ; fdivrp   st6,st0
    db      0xDE,0xFF       ; fdivrp   st7,st0

; F2XM1   | ESCAPE 0 0 1 | 1 1 1 1 0 0 0 0
    f2xm1

; FINIT   | ESCAPE 0 1 1 | 1 1 1 0 0 0 1 1     WARNING: YASM inserts WAIT before FINIT
    finit

; FENI    | ESCAPE 0 1 1 | 1 1 1 0 0 0 0 0
    db      0xDB,0xE0                       ; FENI (YASM won't encode it?)

; FDISI   | ESCAPE 0 1 1 | 1 1 1 0 0 0 0 1
    db      0xDB,0xE1                       ; FDISI (YASM won't encode it?)

; FLDCW   | ESCAPE 0 0 1 | MOD 1 0 1 R/M
    fldcw   [bx]
    fldcw   [bx+si]
    fldcw   [bx+si+0x1234]
    fldcw   [0x1234]
    fldcw   [si-6]

    fldcw   [ebx]
    fldcw   [ebx+esi]
    fldcw   [ebx+esi+0x1234]
    a32 fldcw [0x12345678]
    fldcw   [esi-6]

; FSTCW   | ESCAPE 0 0 1 | MOD 1 1 1 R/M
    fstcw   [bx]
    fstcw   [bx+si]
    fstcw   [bx+si+0x1234]
    fstcw   [0x1234]
    fstcw   [si-6]

    fstcw   [ebx]
    fstcw   [ebx+esi]
    fstcw   [ebx+esi+0x1234]
    a32 fstcw [0x12345678]
    fstcw   [esi-6]

; FSTSW   | ESCAPE 1 0 1 | MOD 1 1 1 R/M
    fstsw   [bx]
    fstsw   [bx+si]
    fstsw   [bx+si+0x1234]
    fstsw   [0x1234]
    fstsw   [si-6]
; NTS: The "FSTSW AX" instruction does not appear until Intel's 80287 datasheet,
;      therefore I am assuming that the 8087 does not have that instruction.
    fstsw   [ebx]
    fstsw   [ebx+esi]
    fstsw   [ebx+esi+0x1234]
    fstsw   [esi-6]

; FCLEX   | ESCAPE 0 1 1 | 1 1 1 0 0 0 1 0
    fclex

; FSTENV  | ESCAPE 0 0 1 | MOD 1 1 0 R/M
    fstenv  [bx]
    fstenv  [bx+si]
    fstenv  [bx+di+0x1234]
    fstenv  [0x1234]
    fstenv  [si-6]

; FLDENV  | ESCAPE 0 0 1 | MOD 1 0 0 R/M
    fldenv  [bx]
    fldenv  [bx+si]
    fldenv  [bx+di+0x1234]
    fldenv  [0x1234]
    fldenv  [si-6]

; FSAVE   | ESCAPE 1 0 1 | MOD 1 1 0 R/M
    fsave   [bx]
    fsave   [bx+si]
    fsave   [bx+di+0x1234]
    fsave   [0x1234]
    fsave   [si-6]

; FRSTOR  | ESCAPE 1 0 1 | MOD 1 0 0 R/M
    frstor  [bx]
    frstor  [bx+si]
    frstor  [bx+di+0x1234]
    frstor  [0x1234]
    frstor  [si-6]

; FINCSTP | ESCAPE 0 0 1 | 1 1 1 1 0 1 1 1
    fincstp

; FDECSTP | ESCAPE 0 0 1 | 1 1 1 1 0 1 1 0
    fdecstp

; FFREE   | ESCAPE 1 0 1 | 1 1 0 0 0 REG
    ffree   st0
    ffree   st1
    ffree   st2
    ffree   st3
    ffree   st4
    ffree   st5
    ffree   st6
    ffree   st7

; FNOP    | ESCAPE 0 0 1 | 1 1 0 1 0 0 0 0
    fnop

; 286 instructions
    pusha           ; opcode 0x60
    pushad
    popa            ; opcode 0x61
    popad
    push    word 0x1234 ; opcode 0x68
    push    dword 0x12345678
    push    byte 0x12 ; opcode 0x6A
    o32 push byte 0x12
    push    byte -0x44 ; opcode 0x6A
    o32 push byte -0x44
    imul    ax,4
    imul    bx,4
    imul    cx,4
    imul    ax,bx,4
    imul    bx,cx,4
    imul    cx,dx,4
    imul    dx,si,4
    imul    ax,word [si],4
    imul    bx,word [bx+di],4
    imul    cx,word [0x1234],4
    imul    ax,0x1234
    imul    bx,0x1234
    imul    cx,0x1234
    imul    ax,bx,0x1234
    imul    bx,cx,0x1234
    imul    cx,dx,0x1234
    imul    dx,si,0x1234
    imul    ax,word [si],0x1234
    imul    bx,word [bx+di],0x1234
    imul    cx,word [0x1234],0x1234
; 386 imul extension (0x0F 0xAF /r)
    imul    ax,bx
    imul    bx,cx
    imul    cx,dx
    imul    dx,si
    imul    ax,word [si]
    imul    bx,word [bx+di]
    imul    cx,word [0x1234]
    imul    dx,[si-6]

; 32-bit
    imul    eax,4
    imul    ebx,4
    imul    ecx,4
    imul    eax,ebx,4
    imul    ebx,ecx,4
    imul    ecx,edx,4
    imul    edx,esi,4
    imul    eax,dword [esi],4
    imul    ebx,dword [ebx+edi],4
    a32 imul ecx,dword [0x12345678],4
    imul    eax,0x12345678
    imul    ebx,0x12345678
    imul    ecx,0x12345678
    imul    eax,ebx,0x12345678
    imul    ebx,ecx,0x12345678
    imul    ecx,edx,0x12345678
    imul    edx,esi,0x12345678
    imul    eax,dword [esi],0x12345678
    imul    ebx,dword [ebx+edi],0x12345678
    a32 imul ecx,dword [0x12345678],0x12345678
; 386 imul extension (0x0F 0xAF /r)
    imul    eax,ebx
    imul    ebx,ecx
    imul    ecx,edx
    imul    edx,esi
    imul    eax,dword [esi]
    imul    ebx,dword [ebx+edi]
    a32 imul ecx,dword [0x12345678]
    imul    edx,[esi-6]

; GRP2 C0
    rol     al,7
    rol     bl,7
    rol     byte [bx+si],7
    rol     byte [bx+di],7
    rol     byte [0x1234],7

    ror     al,7
    ror     bl,7
    ror     byte [bx+si],7
    ror     byte [bx+di],7
    ror     byte [0x1234],7

    rcl     al,7
    rcl     bl,7
    rcl     byte [bx+si],7
    rcl     byte [bx+di],7
    rcl     byte [0x1234],7

    rcr     al,7
    rcr     bl,7
    rcr     byte [bx+si],7
    rcr     byte [bx+di],7
    rcr     byte [0x1234],7

    shl     al,7
    shl     bl,7
    shl     byte [bx+si],7
    shl     byte [bx+di],7
    shl     byte [0x1234],7

    shr     al,7
    shr     bl,7
    shr     byte [bx+si],7
    shr     byte [bx+di],7
    shr     byte [0x1234],7

; nasm/yasm won't encode SAL as SAL but as an alias of SHL
    db      0xC0,0xF0,0x07      ; sal     al,7
    db      0xC0,0xF0,0x07      ; sal     bl,7
    db      0xC0,0x30,0x07      ; sal     byte [bx+si],7
    db      0xC0,0x31,0x07      ; sal     byte [bx+di],7
    db      0xC0,0x36,0x34,0x12,0x07 ; sal     byte [0x1234],7

    sar     al,7
    sar     bl,7
    sar     byte [bx+si],7
    sar     byte [bx+di],7
    sar     byte [0x1234],7

; GRP2 C1
    rol     ax,7
    rol     bx,7
    rol     word [bx+si],7
    rol     word [bx+di],7
    rol     word [0x1234],7

    ror     ax,7
    ror     bx,7
    ror     word [bx+si],7
    ror     word [bx+di],7
    ror     word [0x1234],7

    rcl     ax,7
    rcl     bx,7
    rcl     word [bx+si],7
    rcl     word [bx+di],7
    rcl     word [0x1234],7

    rcr     ax,7
    rcr     bx,7
    rcr     word [bx+si],7
    rcr     word [bx+di],7
    rcr     word [0x1234],7

    shl     ax,7
    shl     bx,7
    shl     word [bx+si],7
    shl     word [bx+di],7
    shl     word [0x1234],7

    shr     ax,7
    shr     bx,7
    shr     word [bx+si],7
    shr     word [bx+di],7
    shr     word [0x1234],7

; nasm/yasm won't encode SAL as SAL but as an alias of SHL
    db      0xC1,0xF0,0x07      ; sal     ax,7
    db      0xC1,0xF0,0x07      ; sal     bx,7
    db      0xC1,0x30,0x07      ; sal     word [bx+si],7
    db      0xC1,0x31,0x07      ; sal     word [bx+di],7
    db      0xC1,0x36,0x34,0x12,0x07 ; sal     word [0x1234],7

    sar     ax,7
    sar     bx,7
    sar     word [bx+si],7
    sar     word [bx+di],7
    sar     word [0x1234],7

; more
    insb
    insw
    insd
    outsb
    outsw
    outsd
    rep     insb
    rep     insw
    rep     insd
    rep     outsb
    rep     outsw
    rep     outsd
    enter   0,0
    enter   0x1234,1
    enter   0x1234,0x12
    o32 enter   0,0 
    o32 enter   0x1234,1
    o32 enter   0x1234,0x12
    leave
    leave
    leave
    o32 leave
    o32 leave
    o32 leave
    bound   ax,word [si]
    bound   bx,word [bx+di]
    bound   cx,word [bx+di+0x1234]
    bound   dx,word [0x1234]
    bound   eax,dword [esi]
    bound   ebx,dword [ebx+edi]
    bound   ecx,dword [ebx+edi+0x1234]
    a32 bound edx,dword [0x12345678]
    clts
; 0x0F 0x01 group
    sgdt    word [bx]               ; 0x0F 0x01 MRM   MOD != 3  REG == 0  R/M == location
    sgdt    word [bx+di]
    sgdt    word [bx+di+0x1234]
    sgdt    word [0x1234]

    sidt    word [bx]               ; 0x0F 0x01 MRM   MOD != 3  REG == 1  R/M == location
    sidt    word [bx+di]
    sidt    word [bx+di+0x1234]
    sidt    word [0x1234]

    lgdt    word [bx]               ; 0x0F 0x01 MRM   MOD != 3  REG == 2  R/M == location
    lgdt    word [bx+di]
    lgdt    word [bx+di+0x1234]
    lgdt    word [0x1234]

    lidt    word [bx]               ; 0x0F 0x01 MRM   MOD != 3  REG == 3  R/M == location
    lidt    word [bx+di]
    lidt    word [bx+di+0x1234]
    lidt    word [0x1234]

    smsw    word [bx]               ; 0x0F 0x01 MRM   MOD != 3  REG == 4  R/M == location
    smsw    word [bx+di]
    smsw    word [bx+di+0x1234]
    smsw    word [0x1234]

    lmsw    word [bx]               ; 0x0F 0x01 MRM   MOD != 3  REG == 6  R/M == location
    lmsw    word [bx+di]
    lmsw    word [bx+di+0x1234]
    lmsw    word [0x1234]
; 0x0F 0x01 group 32-bit
    sgdt    dword [ebx]               ; 0x0F 0x01 MRM   MOD != 3  REG == 0  R/M == location
    sgdt    dword [ebx+edi]
    sgdt    dword [ebx+edi+0x1234]
    a32 sgdt dword [0x12345678]

    sidt    dword [ebx]               ; 0x0F 0x01 MRM   MOD != 3  REG == 1  R/M == location
    sidt    dword [ebx+edi]
    sidt    dword [ebx+edi+0x1234]
    a32 sidt dword [0x12345678]

    lgdt    dword [ebx]               ; 0x0F 0x01 MRM   MOD != 3  REG == 2  R/M == location
    lgdt    dword [ebx+edi]
    lgdt    dword [ebx+edi+0x1234]
    a32 lgdt dword [0x12345678]

    lidt    dword [ebx]               ; 0x0F 0x01 MRM   MOD != 3  REG == 3  R/M == location
    lidt    dword [ebx+edi]
    lidt    dword [ebx+edi+0x1234]
    a32 lidt dword [0x12345678]

    smsw    dword [ebx]               ; 0x0F 0x01 MRM   MOD != 3  REG == 4  R/M == location
    smsw    dword [ebx+edi]
    smsw    dword [ebx+edi+0x1234]
    a32 smsw dword [0x12345678]
; 0x0F 0x00 group
    sldt    word [bx]               ; 0x0F 0x00 MRM   MOD != 3  REG == 0  R/M == location
    sldt    word [bx+di]
    sldt    word [bx+di+0x1234]
    sldt    word [0x1234]

    str     word [bx]               ; 0x0F 0x00 MRM   MOD != 3  REG == 1  R/M == location
    str     word [bx+di]
    str     word [bx+di+0x1234]
    str     word [0x1234]

    lldt    word [bx]               ; 0x0F 0x00 MRM   MOD != 3  REG == 2  R/M == location
    lldt    word [bx+di]
    lldt    word [bx+di+0x1234]
    lldt    word [0x1234]

    ltr     word [bx]               ; 0x0F 0x00 MRM   MOD != 3  REG == 3  R/M == location
    ltr     word [bx+di]
    ltr     word [bx+di+0x1234]
    ltr     word [0x1234]

    verr    word [bx]               ; 0x0F 0x00 MRM   MOD != 3  REG == 4  R/M == location
    verr    word [bx+di]
    verr    word [bx+di+0x1234]
    verr    word [0x1234]

    verw    word [bx]               ; 0x0F 0x00 MRM   MOD != 3  REG == 5  R/M == location
    verw    word [bx+di]
    verw    word [bx+di+0x1234]
    verw    word [0x1234]
; 0x0F 0x00 group 32-bit
    sldt    dword [ebx]               ; 0x0F 0x00 MRM   MOD != 3  REG == 0  R/M == location
    sldt    dword [ebx+edi]
    sldt    dword [ebx+edi+0x1234]
    a32 sldt dword [0x12345678]
; LAR/LSL
    lar     ax,bx
    lar     bx,cx
    lar     cx,dx
    lar     ax,[si]
    lar     bx,[bx]
    lar     cx,[bx+di+0x1234]
    lar     dx,[0x1234]
    lar     si,[si-6]

    lsl     ax,bx
    lsl     bx,cx
    lsl     cx,dx
    lsl     ax,[si]
    lsl     bx,[bx]
    lsl     cx,[bx+di+0x1234]
    lsl     dx,[0x1234]
    lsl     si,[si-6]
; LAR/LSL 32-bit
    lar     eax,ebx
    lar     ebx,ecx
    lar     ecx,edx
    lar     eax,[esi]
    lar     ebx,[ebx]
    lar     ecx,[ebx+edi+0x1234]
    a32 lar edx,[0x12345678]
    lar     esi,[esi-6]

    lsl     eax,ebx
    lsl     ebx,ecx
    lsl     ecx,edx
    lsl     eax,[esi]
    lsl     ebx,[ebx]
    lsl     ecx,[ebx+edi+0x1234]
    a32 lsl edx,[0x12345678]
    lsl     esi,[esi-6]
; ARPL
    arpl    ax,bx
    arpl    bx,cx
    arpl    cx,dx
    arpl    [si],ax
    arpl    [bx],bx
    arpl    [bx+di+0x1234],cx
    arpl    [0x1234],dx
    arpl    [si-6],cx
; ARPL 32-bit
    arpl    [esi],ax
    arpl    [ebx],bx
    arpl    [ebx+edi+0x1234],cx
    a32 arpl [0x12345678],dx
    arpl    [esi-6],cx

; FSETPM  | ESCAPE 0 1 1 | 1 1 1 0 0 1 0 0
    fsetpm

; FSTSW AX| ESCAPE 1 1 1 | 1 1 1 0 0 0 0 0
    fstsw   ax

; LOADALL 286
    db      0x0F,0x05       ; LOADALL 286 version. 386 should not recognize it
; LOADALL 386
    db      0x0F,0x07       ; LOADALL 386

; BSF
    bsf     ax,bx
    bsf     bx,cx
    bsf     cx,dx
    bsf     ax,[si]
    bsf     bx,[bx+di]
    bsf     cx,[bx+si+0x1234]
    bsf     dx,[0x1234]
    bsf     si,[si-6]
; BSF
    bsf     eax,ebx
    bsf     ebx,ecx
    bsf     ecx,edx
    bsf     eax,[si]
    bsf     ebx,[bx+di]
    bsf     ecx,[bx+si+0x1234]
    bsf     edx,[0x1234]
    bsf     esi,[si-6]
; BSF
    bsf     eax,[esi]
    bsf     ebx,[ebx+edi]
    bsf     ecx,[ebx+esi+0x12345678]
    a32 bsf edx,[0x12345678]
    bsf     esi,[esi-6]
; BSR
    bsr     ax,bx
    bsr     bx,cx
    bsr     cx,dx
    bsr     ax,[si]
    bsr     bx,[bx+di]
    bsr     cx,[bx+si+0x1234]
    bsr     dx,[0x1234]
    bsr     si,[si-6]
; BSR 32-bit
    bsr     eax,ebx
    bsr     ebx,ecx
    bsr     ecx,edx
    bsr     eax,[si]
    bsr     ebx,[bx+di]
    bsr     ecx,[bx+si+0x1234]
    bsr     edx,[0x1234]
    bsr     esi,[si-6]
; BSR 32-bit
    bsr     eax,[esi]
    bsr     ebx,[ebx+edi]
    bsr     ecx,[ebx+esi+0x12345678]
    a32 bsr edx,[0x12345678]
    bsr     esi,[esi-6]
; BT
    bt      ax,bx
    bt      bx,cx
    bt      cx,dx
    bt      [si],ax
    bt      [bx+di],bx
    bt      [bx+si+0x1234],cx
    bt      [0x1234],dx
    bt      [si-6],si
    bt      ax,1
    bt      bx,2
    bt      cx,3
    bt      dx,4
    bt      word [si],1
    bt      word [bx+di],2
    bt      word [bx+si+0x1234],3
    bt      word [0x1234],4
    bt      word [si-6],5
; BT 32-bit
    bt      eax,ebx
    bt      ebx,ecx
    bt      ecx,edx
    bt      [esi],eax
    bt      [ebx+edi],ebx
    bt      [ebx+esi+0x1234],ecx
    a32 bt  [0x12345678],edx
    bt      [esi-6],esi
    bt      eax,1
    bt      ebx,2
    bt      ecx,3
    bt      edx,4
    bt      dword [esi],1
    bt      dword [ebx+edi],2
    bt      dword [ebx+esi+0x1234],3
    a32 bt  dword [0x12345678],4
    bt      dword [esi-6],5
; BTC
    btc     ax,bx
    btc     bx,cx
    btc     cx,dx
    btc     [si],ax
    btc     [bx+di],bx
    btc     [bx+si+0x1234],cx
    btc     [0x1234],dx
    btc     [si-6],si
    btc     ax,1
    btc     bx,2
    btc     cx,3
    btc     dx,4
    btc     word [si],1
    btc     word [bx+di],2
    btc     word [bx+si+0x1234],3
    btc     word [0x1234],4
    btc     word [si-6],5
; BTC 32-bit
    btc     eax,ebx
    btc     ebx,ecx
    btc     ecx,edx
    btc     [esi],eax
    btc     [ebx+edi],ebx
    btc     [ebx+esi+0x1234],ecx
    a32 btc [0x12345678],dx
    btc     [esi-6],esi
    btc     eax,1
    btc     ebx,2
    btc     ecx,3
    btc     edx,4
    btc     dword [esi],1
    btc     dword [ebx+edi],2
    btc     dword [ebx+esi+0x1234],3
    a32 btc dword [0x12345678],4
    btc     dword [esi-6],5
; BTR
    btr     ax,bx
    btr     bx,cx
    btr     cx,dx
    btr     [si],ax
    btr     [bx+di],bx
    btr     [bx+si+0x1234],cx
    btr     [0x1234],dx
    btr     [si-6],si
    btr     ax,1
    btr     bx,2
    btr     cx,3
    btr     dx,4
    btr     word [si],1
    btr     word [bx+di],2
    btr     word [bx+si+0x1234],3
    btr     word [0x1234],4
    btr     word [si-6],5
; BTS
    bts     ax,bx
    bts     bx,cx
    bts     cx,dx
    bts     [si],ax
    bts     [bx+di],bx
    bts     [bx+si+0x1234],cx
    bts     [0x1234],dx
    bts     [si-6],si
    bts     ax,1
    bts     bx,2
    bts     cx,3
    bts     dx,4
    bts     word [si],1
    bts     word [bx+di],2
    bts     word [bx+si+0x1234],3
    bts     word [0x1234],4
    bts     word [si-6],5

; 386 near Jcc
jmp2:
    jo      near jmp2
    jno     near jmp2
    jb      near jmp2
    jnb     near jmp2
    jz      near jmp2
    jnz     near jmp2
    jbe     near jmp2
    ja      near jmp2
    js      near jmp2
    jns     near jmp2
    jpe     near jmp2
    jpo     near jmp2
    jl      near jmp2
    jge     near jmp2
    jle     near jmp2
    jg      near jmp2

    o32 jo      near jmp2
    o32 jno     near jmp2
    o32 jb      near jmp2
    o32 jnb     near jmp2
    o32 jz      near jmp2
    o32 jnz     near jmp2
    o32 jbe     near jmp2
    o32 ja      near jmp2
    o32 js      near jmp2
    o32 jns     near jmp2
    o32 jpe     near jmp2
    o32 jpo     near jmp2
    o32 jl      near jmp2
    o32 jge     near jmp2
    o32 jle     near jmp2
    o32 jg      near jmp2

; MOV CRx...
    mov     eax,cr0
    mov     cr0,eax
    mov     eax,cr2     ; NTS: YASM won't let me encode "mov eax,cr1"
    mov     cr2,eax
    mov     eax,cr4
    mov     cr4,eax
    mov     ebx,cr0
    mov     cr0,ebx
    mov     ecx,cr2
    mov     cr2,ecx
; MOV DRx...
    mov     eax,dr0
    mov     dr0,eax
    mov     eax,dr2
    mov     dr2,eax
    mov     eax,dr4
    mov     dr4,eax
    mov     ebx,dr0
    mov     dr0,ebx
    mov     ebx,dr0
    mov     dr0,ebx
    mov     ecx,dr0
    mov     dr0,ecx
; MOVSX
    movsx   ax,al
    movsx   ax,bl
    movsx   bx,al
    movsx   ax,byte [si]
    movsx   ax,byte [bx+di]
    movsx   bx,byte [0x1234]
    movsx   cx,byte [si-6]
; deliberately useless MOVSX [word],[word]. YASM won't let me encode this. NASM disassembler refuses to decompile these. It's a good bet the 80386 allowed this though.
    db      0x0F,0xBF,0xC0      ; MOVSX AX,AX
    db      0x0F,0xBF,0xC3      ; MOVSX AX,BX
    db      0x0F,0xBF,0xD8      ; MOVSX BX,AX
    db      0x0F,0xBF,0x04      ; MOVSX AX,[SI]
    db      0x0F,0xBF,0x01      ; MOVSX AX,[BX+DI]
    db      0x0F,0xBF,0x1E,0x34,0x12; MOVSX BX,[0x1234]
    db      0x0F,0xBF,0x4C,0xFA ; MOVSX CX,[SI-6]
; 32-bit versions
    movsx   eax,ax
    movsx   eax,bx
    movsx   ebx,ax
    movsx   eax,word [si]
    movsx   eax,word [bx+di]
    movsx   ebx,word [0x1234]
    movsx   ecx,word [si-6]
; 16-bit with 32-bit addressing
    movsx   ax,byte [eax]
    movsx   ax,byte [ebx]
    movsx   ax,byte [ecx]
    movsx   ax,byte [edx]
    movsx   bx,byte [esi]
    movsx   cx,byte [edi]
    a32 movsx dx,byte [0x12345678]
    movsx   ax,byte [ebx+4]
    movsx   bx,byte [ecx-8]
    movsx   cx,byte [edx+0x12345678]
    movsx   dx,byte [eax+ebx]
    movsx   si,byte [ebx+ecx]
    movsx   di,byte [eax*2+ebx]
    movsx   ax,byte [ecx*4+ecx]
    movsx   bx,byte [edx*8+edx+0x12]
    movsx   cx,byte [esi*4+edi+0x12345678]
; 32-bit with 32-bit addressing
    movsx   eax,byte [eax]
    movsx   eax,byte [ebx]
    movsx   eax,byte [ecx]
    movsx   eax,byte [edx]
    movsx   ebx,byte [esi]
    movsx   ecx,byte [edi]
    a32 movsx   edx,byte [0x12345678]
    movsx   eax,byte [ebx+4]
    movsx   ebx,byte [ecx-8]
    movsx   ecx,byte [edx+0x12345678]
    movsx   edx,byte [eax+ebx]
    movsx   esi,byte [ebx+ecx]
    movsx   edi,byte [eax*2+ebx]
    movsx   eax,byte [ecx*4+ecx]
    movsx   ebx,byte [edx*8+edx+0x12]
    movsx   ecx,byte [esi*4+edi+0x12345678]
; 32-bit with 32-bit addressing
    movsx   eax,word [eax]
    movsx   eax,word [ebx]
    movsx   eax,word [ecx]
    movsx   eax,word [edx]
    movsx   ebx,word [esi]
    movsx   ecx,word [edi]
    a32 movsx   edx,word [0x12345678]
    movsx   eax,word [ebx+4]
    movsx   ebx,word [ecx-8]
    movsx   ecx,word [edx+0x12345678]
    movsx   edx,word [eax+ebx]
    movsx   esi,word [ebx+ecx]
    movsx   edi,word [eax*2+ebx]
    movsx   eax,word [ecx*4+ecx]
    movsx   ebx,word [edx*8+edx+0x12]
    movsx   ecx,word [esi*4+edi+0x12345678]
    movsx   eax,al
    movsx   eax,bl
    movsx   ebx,al
; MOVZX
    movzx   ax,al
    movzx   ax,bl
    movzx   bx,al
    movzx   ax,byte [si]
    movzx   ax,byte [bx+di]
    movzx   bx,byte [0x1234]
    movzx   cx,byte [si-6]
; deliberately useless MOVZX [word],[word]. YASM won't let me encode this. NASM disassembler refuses to decompile these. It's a good bet the 80386 allowed this though.
    db      0x0F,0xB7,0xC0      ; MOVZX AX,AX
    db      0x0F,0xB7,0xC3      ; MOVZX AX,BX
    db      0x0F,0xB7,0xD8      ; MOVZX BX,AX
    db      0x0F,0xB7,0x04      ; MOVZX AX,[SI]
    db      0x0F,0xB7,0x01      ; MOVZX AX,[BX+DI]
    db      0x0F,0xB7,0x1E,0x34,0x12; MOVZX BX,[0x1234]
    db      0x0F,0xB7,0x4C,0xFA ; MOVZX CX,[SI-6]
; MOVZX
    movzx   eax,al
    movzx   eax,bl
    movzx   ebx,al
    movzx   eax,byte [si]
    movzx   eax,byte [bx+di]
    movzx   ebx,byte [0x1234]
    movzx   ecx,byte [si-6]
    movzx   eax,ax
    movzx   eax,bx
    movzx   ebx,ax
    movzx   eax,word [si]
    movzx   eax,word [bx+di]
    movzx   ebx,word [0x1234]
    movzx   ecx,word [si-6]

    seto    bl
    setno   bl
    setb    bl
    setnb   bl
    setz    bl
    setnz   bl
    setbe   bl
    seta    bl
    sets    bl
    setns   bl
    setpe   bl
    setpo   bl
    setl    bl
    setge   bl
    setle   bl
    setg    bl

    seto    byte [si]
    setno   byte [si]
    setb    byte [si]
    setnb   byte [si]
    setz    byte [si]
    setnz   byte [si]
    setbe   byte [si]
    seta    byte [si]
    sets    byte [si]
    setns   byte [si]
    setpe   byte [si]
    setpo   byte [si]
    setl    byte [si]
    setge   byte [si]
    setle   byte [si]
    setg    byte [si]

    seto    byte [bx+si]
    setno   byte [bx+si]
    setb    byte [bx+si]
    setnb   byte [bx+si]
    setz    byte [bx+si]
    setnz   byte [bx+si]
    setbe   byte [bx+si]
    seta    byte [bx+si]
    sets    byte [bx+si]
    setns   byte [bx+si]
    setpe   byte [bx+si]
    setpo   byte [bx+si]
    setl    byte [bx+si]
    setge   byte [bx+si]
    setle   byte [bx+si]
    setg    byte [bx+si]

    seto    byte [ebx+esi]
    setno   byte [ebx+esi]
    setb    byte [ebx+esi]
    setnb   byte [ebx+esi]
    setz    byte [ebx+esi]
    setnz   byte [ebx+esi]
    setbe   byte [ebx+esi]
    seta    byte [ebx+esi]
    sets    byte [ebx+esi]
    setns   byte [ebx+esi]
    setpe   byte [ebx+esi]
    setpo   byte [ebx+esi]
    setl    byte [ebx+esi]
    setge   byte [ebx+esi]
    setle   byte [ebx+esi]
    setg    byte [ebx+esi]

; SHLD
    shld    ax,bx,1
    shld    bx,cx,6
    shld    [si],ax,8
    shld    [bx+di],bx,4
    shld    [0x1234],cx,4
    shld    [si-6],dx,4
; SHLD
    shld    eax,ebx,1
    shld    ebx,ecx,6
    shld    [esi],eax,8
    shld    [ebx+edi],ebx,4
    a32 shld [0x12345678],ecx,4
    shld    [esi-6],edx,4
; SHLD
    shld    ax,bx,cl
    shld    bx,cx,cl
    shld    [si],ax,cl
    shld    [bx+di],bx,cl
    shld    [0x1234],cx,cl
    shld    [si-6],dx,cl
; SHLD
    shld    eax,ebx,cl
    shld    ebx,ecx,cl
    shld    [esi],eax,cl
    shld    [ebx+edi],ebx,cl
    a32 shld [0x12345678],ecx,cl
    shld    [esi-6],edx,cl
; SHRD
    shrd    ax,bx,1
    shrd    bx,cx,6
    shrd    [si],ax,8
    shrd    [bx+di],bx,4
    shrd    [0x1234],cx,4
    shrd    [si-6],dx,4
; SHRD
    shrd    eax,ebx,1
    shrd    ebx,ecx,6
    shrd    [esi],eax,8
    shrd    [ebx+edi],ebx,4
    a32 shrd [0x12345678],ecx,4
    shrd    [esi-6],edx,4
; SHRD
    shrd    ax,bx,cl
    shrd    bx,cx,cl
    shrd    [si],ax,cl
    shrd    [bx+di],bx,cl
    a32 shrd [0x12345678],cx,cl
    shrd    [si-6],dx,cl
; SHRD
    shrd    eax,ebx,cl
    shrd    ebx,ecx,cl
    shrd    [esi],eax,cl
    shrd    [ebx+edi],ebx,cl
    a32 shrd [0x12345678],ecx,cl
    shrd    [esi-6],edx,cl

; FUCOM   | ESCAPE 1 0 1 | 1 1 1 0 0 R/M
    fucom   st0
    fucom   st1
    fucom   st2
    fucom   st3
    fucom   st4
    fucom   st5
    fucom   st6
    fucom   st7

; FUCOMP  | ESCAPE 1 0 1 | 1 1 1 0 1 R/M
    fucomp  st0
    fucomp  st1
    fucomp  st2
    fucomp  st3
    fucomp  st4
    fucomp  st5
    fucomp  st6
    fucomp  st7

; FUCOMPP | ESCAPE 0 1 0 | 1 1 1 0 1 0 0 1
    fucompp

; FCOS    | ESCAPE 0 0 1 | 1 1 1 1 1 1 1 1
    fcos

; FSIN    | ESCAPE 0 0 1 | 1 1 1 1 1 1 1 0
    fsin

; FSINCOS | ESCAPE 0 0 1 | 1 1 1 1 1 0 1 1
    fsincos

; BSWAP
    bswap   eax
    bswap   ebx
    bswap   ecx
    bswap   edx
    bswap   esi
    bswap   edi
    o16 bswap eax
    o16 bswap ebx
    o16 bswap ecx
    o16 bswap edx
    o16 bswap esi
    o16 bswap edi

; XADD
    xadd    al,bl
    xadd    bl,cl
    xadd    cl,dl
    xadd    [si],dl
    xadd    [bx+si],dl
    xadd    [ebx+esi],dl
    xadd    ax,bx
    xadd    bx,cx
    xadd    cx,dx
    xadd    [si],dx
    xadd    [bx+si],dx
    xadd    [ebx+esi],dx
    xadd    eax,ebx
    xadd    ebx,ecx
    xadd    ecx,edx
    xadd    [si],edx
    xadd    [bx+si],edx
    xadd    [ebx+esi],edx

; CMPXCHG
    cmpxchg al,bl
    cmpxchg bl,cl
    cmpxchg cl,dl
    cmpxchg [si],dl
    cmpxchg [bx+si],dl
    cmpxchg [ebx+esi],dl
    cmpxchg ax,bx
    cmpxchg bx,cx
    cmpxchg cx,dx
    cmpxchg [si],dx
    cmpxchg [bx+si],dx
    cmpxchg [ebx+esi],dx
    cmpxchg eax,ebx
    cmpxchg ebx,ecx
    cmpxchg ecx,edx
    cmpxchg [si],edx
    cmpxchg [bx+si],edx
    cmpxchg [ebx+esi],edx

; INVLPG
    invlpg  [si]
    invlpg  [bx+si]
    invlpg  [bx+di+0x12]
    invlpg  [bx+di+0x1234]
    invlpg  [eax]
    invlpg  [ebx+eax]
    invlpg  [ecx*4+eax+0x12345678]

; other
    invd
    wbinvd
    cpuid

; Pentium
    wrmsr
    o32 wrmsr
    a32 wrmsr
    rdtsc
    o32 rdtsc
    a32 rdtsc
    rdmsr
    o32 rdmsr
    a32 rdmsr
    cmpxchg8b   [si]
    cmpxchg8b   [bx+si]
    cmpxchg8b   [bx+si+0x12]
    cmpxchg8b   [bx+si+0x1234]
    cmpxchg8b   [esi]
    cmpxchg8b   [ebx+edx+0x12345678]

; Pentium Pro
    db          0x0F,0x0D,0x1F              ; 0x0F 0x0D NOP
    db          0x0F,0x1F,0xC0              ; ax
    db          0x66,0x0F,0x1F,0xC0         ; eax
    db          0x0F,0x1F,0x00              ; [bx+si]

    cmovo       ax,bx
    cmovo       eax,ebx
    cmovo       ax,[bx+si]
    cmovno      ax,bx
    cmovb       ax,bx
    cmovnb      ax,bx
    cmovz       ax,bx
    cmovnz      ax,bx
    cmovbe      ax,bx
    cmovnbe     ax,bx
    cmovs       ax,bx
    cmovns      ax,bx
    cmovp       ax,bx
    cmovnp      ax,bx
    cmovl       ax,bx
    cmovnl      ax,bx
    cmovle      ax,bx
    cmovnle     ax,bx

    fcmovb      st0,st1
    fcmovb      st0,st3
    fcmovnb     st0,st1
    fcmove      st0,st1
    fcmovne     st0,st1
    fcmovbe     st0,st1
    fcmovnbe    st0,st1
    fcmovu      st0,st1
    fcmovnu     st0,st1
    fucomi      st0,st1
    fcomi       st0,st1
    fucomip     st0,st1
    fcomip      st0,st1

; Pentium II
    sysenter
    sysexit
    fxsave      [si]
    fxrstor     [si]

; Pentium III
    movups      xmm0,xmm1
    movups      xmm0,[si]
    movups      xmm0,[bx+si]
    movups      xmm0,[ebx*4+edi+0x12345678]
    movups      [si],xmm0
    movups      [bx+si],xmm0
    movups      [ebx*4+edi+0x12345678],xmm0
    movss       xmm0,xmm1
    movss       xmm0,[si]
    movss       xmm0,[bx+si]
    movss       xmm0,[ebx*4+edi+0x12345678]
    movss       [si],xmm0
    movss       [bx+si],xmm0
    movss       [ebx*4+edi+0x12345678],xmm0
    movhlps     xmm0,xmm1
    movhlps     xmm1,xmm2
    movlps      xmm0,[si]
    movlps      xmm0,[bx+si]
    movlps      xmm0,[edx*4+eax+0x12345678]
    movlps      [si],xmm0
    movlps      [bx+si],xmm0
    movlps      [edx*4+eax+0x12345678],xmm0
    unpcklps    xmm0,xmm1
    unpcklps    xmm0,xmm2
    unpcklps    xmm0,[si]
    unpcklps    xmm0,[bx+si]
    unpcklps    xmm0,[edx*4+eax+0x12345678]
    unpckhps    xmm0,xmm1
    unpckhps    xmm0,xmm2
    unpckhps    xmm0,[si]
    unpckhps    xmm0,[bx+si]
    unpckhps    xmm0,[edx*4+eax+0x12345678]
    movlhps     xmm0,xmm1
    movlhps     xmm1,xmm2
    movlhps     xmm2,xmm3
    movhps      xmm0,[si]
    movhps      xmm0,[bx+si]
    movhps      xmm0,[edx*4+eax+0x12345678]
    movhps      [si],xmm0
    movhps      [bx+si],xmm0
    movhps      [edx*4+eax+0x12345678],xmm0

    prefetchnta [si]
    prefetchnta [bx+si]
    prefetchnta [edx*4+eax+0x12345678]

    prefetcht0  [si]
    prefetcht0  [bx+si]
    prefetcht0  [edx*4+eax+0x12345678]

    prefetcht1  [si]
    prefetcht1  [bx+si]
    prefetcht1  [edx*4+eax+0x12345678]

    prefetcht2  [si]
    prefetcht2  [bx+si]
    prefetcht2  [edx*4+eax+0x12345678]

    movaps      xmm0,xmm1
    movaps      xmm1,xmm2
    movaps      xmm0,[si]
    movaps      [si],xmm0

    cvtpi2ps    xmm0,mm1
    cvtpi2ps    xmm0,[si]

    cvtsi2ss    xmm0,eax
    cvtsi2ss    xmm0,[si]

    movntps     [si],xmm0

    cvttps2pi   mm0,xmm1
    cvttps2pi   mm0,[si]

    cvttss2si   eax,xmm1
    cvttss2si   eax,[si]

    cvtps2pi    mm0,xmm1
    cvtps2pi    mm0,[si]

    cvtss2si    eax,xmm1
    cvtss2si    eax,[si]

    ucomiss     xmm0,xmm1
    ucomiss     xmm0,[si]

    comiss      xmm0,xmm1
    comiss      xmm0,[si]

    movmskps    eax,xmm0

    sqrtps      xmm0,xmm1
    sqrtps      xmm0,[si]

    sqrtss      xmm0,xmm1
    sqrtss      xmm0,[si]

    rsqrtps     xmm0,xmm1
    rsqrtps     xmm0,[si]

    rsqrtss     xmm0,xmm1
    rsqrtss     xmm0,[si]

    rcpps       xmm0,xmm1
    rcpps       xmm0,[si]

    rcpss       xmm0,xmm1
    rcpss       xmm0,[si]

    andps       xmm0,xmm1
    andps       xmm0,[si]

    andnps      xmm0,xmm1
    andnps      xmm0,[si]

    orps        xmm0,xmm1
    orps        xmm0,[si]

    xorps       xmm0,xmm1
    xorps       xmm0,[si]

    addps       xmm0,xmm1
    addps       xmm0,[si]

    addss       xmm0,xmm1
    addss       xmm0,[si]

    mulps       xmm0,xmm1
    mulps       xmm0,[si]

    mulss       xmm0,xmm1
    mulss       xmm0,[si]

    subps       xmm0,xmm1
    subps       xmm0,[si]

    subss       xmm0,xmm1
    subss       xmm0,[si]

    minps       xmm0,xmm1
    minps       xmm0,[si]

    minss       xmm0,xmm1
    minss       xmm0,[si]

    divps       xmm0,xmm1
    divps       xmm0,[si]

    divss       xmm0,xmm1
    divss       xmm0,[si]

    maxps       xmm0,xmm1
    maxps       xmm0,[si]

    maxss       xmm0,xmm1
    maxss       xmm0,[si]

    ldmxcsr     [si]
    stmxcsr     [si]

    sfence

    cmpps       xmm0,xmm1,1
    cmpps       xmm0,[si],1
    cmpss       xmm0,xmm1,1
    cmpss       xmm0,[si],1

    pinsrw      mm0,eax,1
    pinsrw      mm0,[si],1
    pextrw      eax,mm0,1

    shufps      xmm0,xmm1,1
    shufps      xmm0,[si],1

    pmovmskb    eax,mm0

    pminub      mm0,mm1
    pminub      mm0,[si]

    pmaxub      mm0,mm1
    pmaxub      mm0,[si]

    pavgb       mm0,mm1
    pavgb       mm0,[si]

    pmulhuw     mm0,mm1
    pmulhuw     mm0,[si]

    movntq      [si],mm0
    movntq      [ebx+edi],mm0

    pminsw      mm0,mm1
    pminsw      mm0,[si]

    pmaxsw      mm0,mm1
    pmaxsw      mm0,[si]

    psadbw      mm0,mm1
    psadbw      mm0,[si]

    maskmovq    mm0,mm1

; Pentium 4
    pause

    movupd      xmm0,xmm1
    movupd      xmm0,[si]
    movupd      [si],xmm0
    movsd       xmm0,xmm1
    movsd       xmm0,[si]
    movsd       [si],xmm0

    movlpd      xmm0,[si]
    movlpd      [si],xmm0
    movlps      xmm0,[si]
    movlps      [si],xmm0

    unpcklpd    xmm0,xmm1
    unpcklpd    xmm0,[si]
    unpckhpd    xmm0,xmm1
    unpckhpd    xmm0,[si]

    movhpd      xmm0,[si]
    movhpd      [si],xmm0

    movapd      xmm0,xmm1
    movapd      xmm0,[si]
    movapd      [si],xmm0

    cvtpi2pd    xmm0,mm1
    cvtpi2pd    xmm0,[si]

    cvtsi2sd    xmm0,eax
    cvtsi2sd    xmm0,[si]

    movntpd     [si],xmm0

    cvttpd2pi   mm0,xmm1
    cvttpd2pi   mm0,[si]

    cvttsd2si   eax,xmm1
    cvttsd2si   eax,[si]

    cvtpd2pi    mm0,xmm1
    cvtpd2pi    mm0,[si]

    cvtsd2si    eax,xmm1
    cvtsd2si    eax,[si]

    ucomisd     xmm0,xmm1
    ucomisd     xmm0,[si]

    comisd      xmm0,xmm1
    comisd      xmm0,[si]

    movmskpd    eax,xmm1

    sqrtpd      xmm0,xmm1
    sqrtpd      xmm0,[si]
    sqrtsd      xmm0,xmm1
    sqrtsd      xmm0,[si]

    andpd       xmm0,xmm1
    andpd       xmm0,[si]

    andnpd      xmm0,xmm1
    andnpd      xmm0,[si]

    orpd        xmm0,xmm1
    orpd        xmm0,[si]

    xorpd       xmm0,xmm1
    xorpd       xmm0,[si]

    addpd       xmm0,xmm1
    addpd       xmm0,[si]

    addsd       xmm0,xmm1
    addsd       xmm0,[si]

    mulpd       xmm0,xmm1
    mulpd       xmm0,[si]
    mulsd       xmm0,xmm1
    mulsd       xmm0,[si]

    cvtps2pd    xmm0,xmm1
    cvtps2pd    xmm0,[si]
    cvtpd2ps    xmm0,xmm1
    cvtpd2ps    xmm0,[si]
    cvtss2sd    xmm0,xmm1
    cvtss2sd    xmm0,[si]
    cvtsd2ss    xmm0,xmm1
    cvtsd2ss    xmm0,[si]
    cvtdq2ps    xmm0,xmm1
    cvtdq2ps    xmm0,[si]
    cvtps2dq    xmm0,xmm1
    cvtps2dq    xmm0,[si]
    cvttps2dq   xmm0,xmm1
    cvttps2dq   xmm0,[si]

    subpd       xmm0,xmm1
    subpd       xmm0,[si]
    subsd       xmm0,xmm1
    subsd       xmm0,[si]
    minpd       xmm0,xmm1
    minpd       xmm0,[si]
    minsd       xmm0,xmm1
    minsd       xmm0,[si]
    divpd       xmm0,xmm1
    divpd       xmm0,[si]
    divps       xmm0,xmm1
    divps       xmm0,[si]
    maxpd       xmm0,xmm1
    maxpd       xmm0,[si]
    maxsd       xmm0,xmm1
    maxsd       xmm0,[si]

    punpcklbw   xmm0,xmm1
    punpcklbw   xmm0,[si]
    punpcklwd   xmm0,xmm1
    punpcklwd   xmm0,[si]
    punpckldq   xmm0,xmm1
    punpckldq   xmm0,[si]
    packsswb    xmm0,xmm1
    packsswb    xmm0,[si]
    pcmpgtb     xmm0,xmm1
    pcmpgtb     xmm0,[si]
    pcmpgtw     xmm0,xmm1
    pcmpgtw     xmm0,[si]
    pcmpgtd     xmm0,xmm1
    pcmpgtd     xmm0,[si]
    packuswb    xmm0,xmm1
    packuswb    xmm0,[si]
    punpckhbw   xmm0,xmm1
    punpckhbw   xmm0,[si]
    punpckhwd   xmm0,xmm1
    punpckhwd   xmm0,[si]
    punpckhdq   xmm0,xmm1
    punpckhdq   xmm0,[si]
    packssdw    xmm0,xmm1
    packssdw    xmm0,[si]
    punpcklqdq  xmm0,xmm1
    punpcklqdq  xmm0,[si]
    punpckhqdq  xmm0,xmm1
    punpckhqdq  xmm0,[si]
    movd        xmm0,eax
    movd        xmm0,[si]
    movdqa      xmm0,xmm1
    movdqa      xmm0,[si]
    movdqu      xmm0,xmm1
    movdqu      xmm0,[si]

    pshuflw     xmm0,xmm1,1
    pshuflw     xmm0,[si],1
    pshufhw     xmm0,xmm1,1
    pshufhw     xmm0,[si],1
    pshufd      xmm0,xmm1,1
    pshufd      xmm0,[si],1
    psrlw       xmm0,1
    psraw       xmm0,1
    psllw       xmm0,1
    psrld       xmm0,1
    psrad       xmm0,1
    pslld       xmm0,1
    psrlq       xmm0,1
    psrldq      xmm0,1
    psllq       xmm0,1
    pslldq      xmm0,1
    pcmpeqb     xmm0,xmm1
    pcmpeqb     xmm0,[si]
    pcmpeqw     xmm0,xmm1
    pcmpeqw     xmm0,[si]
    pcmpeqd     xmm0,xmm1
    pcmpeqd     xmm0,[si]
    movd        eax,xmm0
    movd        [si],xmm0
    movdqa      xmm0,xmm1
    movdqa      [si],xmm0
    movdqu      xmm0,xmm1
    movdqu      [si],xmm0

    lfence
    mfence
    clflush     [si]

    cmppd       xmm0,xmm1,1
    cmppd       xmm0,[si],1
    cmpsd       xmm0,xmm1,1
    cmpsd       xmm0,[si],1
    movnti      [si],eax
    shufpd      xmm0,xmm1,1
    shufpd      xmm0,[si],1
    psrlw       xmm0,xmm1
    psrlw       xmm0,[si]
    psrld       xmm0,xmm1
    psrld       xmm0,[si]
    psrlq       xmm0,xmm1
    psrlq       xmm0,[si]
    paddq       xmm0,xmm1
    paddq       xmm0,[si]
    pmullw      xmm0,xmm1
    pmullw      xmm0,[si]
    movq        xmm0,xmm1
    movq        [si],xmm0
    movq2dq     xmm0,mm1
    movdq2q     mm0,xmm1
    psubusb     xmm0,xmm1
    psubusb     xmm0,[si]
    pand        xmm0,xmm1
    pand        xmm0,[si]
    paddusb     xmm0,xmm1
    paddusb     xmm0,[si]
    paddusw     xmm0,xmm1
    paddusw     xmm0,[si]
    pandn       xmm0,xmm1
    pandn       xmm0,[si]
    psraw       xmm0,xmm1
    psraw       xmm0,[si]
    psrad       xmm0,xmm1
    psrad       xmm0,[si]
    pmulhw      xmm0,xmm1
    pmulhw      xmm0,[si]
    cvtpd2dq    xmm0,xmm1
    cvtpd2dq    xmm0,[si]
    cvttpd2dq   xmm0,xmm1
    cvttpd2dq   xmm0,[si]
    cvtdq2pd    xmm0,xmm1
    cvtdq2pd    xmm0,[si]
    movntdq     [si],xmm0
    psubsb      xmm0,xmm1
    psubsb      xmm0,[si]
    psubsw      xmm0,xmm1
    psubsw      xmm0,[si]
    por         xmm0,xmm1
    por         xmm0,[si]
    paddsb      xmm0,xmm1
    paddsb      xmm0,[si]
    paddsw      xmm0,xmm1
    paddsw      xmm0,[si]
    pxor        xmm0,xmm1
    pxor        xmm0,[si]
    psllw       xmm0,xmm1
    psllw       xmm0,[si]
    pslld       xmm0,xmm1
    pslld       xmm0,[si]
    psllq       xmm0,xmm1
    psllq       xmm0,[si]
    pmuludq     mm0,mm1
    pmuludq     mm0,[si]
    pmuludq     xmm0,xmm1
    pmuludq     xmm0,[si]
    pmaddwd     xmm0,xmm1
    pmaddwd     xmm0,[si]
    maskmovdqu  xmm0,xmm1
    psubb       xmm0,xmm1
    psubb       xmm0,[si]
    psubw       xmm0,xmm1
    psubw       xmm0,[si]
    psubd       mm0,mm1
    psubd       mm0,[si]
    psubd       xmm0,xmm1
    psubd       xmm0,[si]
    paddb       xmm0,xmm1
    paddb       xmm0,[si]
    paddw       xmm0,xmm1
    paddw       xmm0,[si]
    paddd       xmm0,xmm1
    paddd       xmm0,[si]

    fisttp      dword [si]
    fisttp      qword [si]
    fisttp      word [si]
    monitor
    mwait
    movddup     xmm0,xmm1
    movddup     xmm0,[si]
    movsldup    xmm0,xmm1
    movsldup    xmm0,[si]
    movshdup    xmm0,xmm1
    movshdup    xmm0,[si]
    haddpd      xmm0,xmm1
    haddpd      xmm0,[si]
    haddps      xmm0,xmm1
    haddps      xmm0,[si]
    hsubpd      xmm0,xmm1
    hsubpd      xmm0,[si]
    hsubps      xmm0,xmm1
    hsubps      xmm0,[si]
    addsubpd    xmm0,xmm1
    addsubpd    xmm0,[si]
    addsubps    xmm0,xmm1
    addsubps    xmm0,[si]
    lddqu       xmm0,[si]

    pshufb      mm0,mm1
    pshufb      mm0,[si]
    pshufb      xmm0,xmm1
    pshufb      xmm0,[si]

    ; test our decoder's mandatory prefix cross-over code (pshufb 0xF2 prefix not exist)
    repnz
    pshufb      mm0,mm1
    repnz
    pshufb      mm0,[si]
    repnz
    pshufb      xmm0,xmm1
    repnz
    pshufb      xmm0,[si]

    phaddw      mm0,mm1
    phaddw      mm0,[si]
    phaddw      xmm0,xmm1
    phaddw      xmm0,[si]

    phaddd      mm0,mm1
    phaddd      mm0,[si]
    phaddd      xmm0,xmm1
    phaddd      xmm0,[si]

    phaddsw     mm0,mm1
    phaddsw     mm0,[si]
    phaddsw     xmm0,xmm1
    phaddsw     xmm0,[si]

    pmaddubsw   mm0,mm1
    pmaddubsw   mm0,[si]
    pmaddubsw   xmm0,xmm1
    pmaddubsw   xmm0,[si]

    phsubw      mm0,mm1
    phsubw      mm0,[si]
    phsubw      xmm0,xmm1
    phsubw      xmm0,[si]

    phsubd      mm0,mm1
    phsubd      mm0,[si]
    phsubd      xmm0,xmm1
    phsubd      xmm0,[si]

    phsubsw     mm0,mm1
    phsubsw     mm0,[si]
    phsubsw     xmm0,xmm1
    phsubsw     xmm0,[si]

    psignb      mm0,mm1
    psignb      mm0,[si]
    psignb      xmm0,xmm1
    psignb      xmm0,[si]

    psignw      mm0,mm1
    psignw      mm0,[si]
    psignw      xmm0,xmm1
    psignw      xmm0,[si]

    psignd      mm0,mm1
    psignd      mm0,[si]
    psignd      xmm0,xmm1
    psignd      xmm0,[si]

    pmulhrsw    mm0,mm1
    pmulhrsw    mm0,[si]
    pmulhrsw    xmm0,xmm1
    pmulhrsw    xmm0,[si]

    pabsb       mm0,mm1
    pabsb       mm0,[si]
    pabsb       xmm0,xmm1
    pabsb       xmm0,[si]

    pabsw       mm0,mm1
    pabsw       mm0,[si]
    pabsw       xmm0,xmm1
    pabsw       xmm0,[si]

    pabsd       mm0,mm1
    pabsd       mm0,[si]
    pabsd       xmm0,xmm1
    pabsd       xmm0,[si]

    palignr     mm0,mm1,1
    palignr     mm0,[si],1
    palignr     xmm0,xmm1,1
    palignr     xmm0,[si],1

    cpu         Sandybridge     ; shut up and encode the following instructions
    cpu         EM64T
    cpu         SVM
    cpu         SMX
    cpu         lzcnt
    cpu         movbe
    cpu         eptvpid

    xgetbv
    xsetbv
    getsec

    pblendvb    xmm0,xmm1
    pblendvb    xmm0,[si]
    pblendvb    xmm0,[bx+si]

    blendvps    xmm0,xmm1
    blendvps    xmm0,[si]
    blendvps    xmm0,[bx+si]

    blendvpd    xmm0,xmm1
    blendvpd    xmm0,[si]
    blendvpd    xmm0,[bx+si]

    ptest       xmm0,xmm1
    ptest       xmm0,[si]
    ptest       xmm0,[bx+si]

    pmovsxbw    xmm0,xmm1
    pmovsxbw    xmm0,[si]
    pmovsxbw    xmm0,[bx+si]

    pmovsxbd    xmm0,xmm1
    pmovsxbd    xmm0,[si]
    pmovsxbd    xmm0,[bx+si]

    pmovsxbq    xmm0,xmm1
    pmovsxbq    xmm0,[si]
    pmovsxbq    xmm0,[bx+si]

    pmovsxwd    xmm0,xmm1
    pmovsxwd    xmm0,[si]
    pmovsxwd    xmm0,[bx+si]

    pmovsxwq    xmm0,xmm1
    pmovsxwq    xmm0,[si]
    pmovsxwq    xmm0,[bx+si]

    pmovsxdq    xmm0,xmm1
    pmovsxdq    xmm0,[si]
    pmovsxdq    xmm0,[bx+si]

    pmuldq      xmm0,xmm1
    pmuldq      xmm0,[si]
    pmuldq      xmm0,[bx+si]

    pcmpeqq     xmm0,xmm1
    pcmpeqq     xmm0,[si]
    pcmpeqq     xmm0,[bx+si]

    ; no register form, mod != 3
    movntdqa    xmm0,[si]
    movntdqa    xmm0,[bx+si]

    packusdw    xmm0,xmm1
    packusdw    xmm0,[si]
    packusdw    xmm0,[bx+si]

    pmovzxbw    xmm0,xmm1
    pmovzxbw    xmm0,[si]
    pmovzxbw    xmm0,[bx+si]

    pmovzxbd    xmm0,xmm1
    pmovzxbd    xmm0,[si]
    pmovzxbd    xmm0,[bx+si]

    pmovzxbq    xmm0,xmm1
    pmovzxbq    xmm0,[si]
    pmovzxbq    xmm0,[bx+si]

    pmovzxwd    xmm0,xmm1
    pmovzxwd    xmm0,[si]
    pmovzxwd    xmm0,[bx+si]

    pmovzxwq    xmm0,xmm1
    pmovzxwq    xmm0,[si]
    pmovzxwq    xmm0,[bx+si]

    pmovzxdq    xmm0,xmm1
    pmovzxdq    xmm0,[si]
    pmovzxdq    xmm0,[bx+si]

    pcmpgtq     xmm0,xmm1
    pcmpgtq     xmm0,[si]

    pminsb      xmm0,xmm1
    pminsb      xmm0,[si]

    pminsd      xmm0,xmm1
    pminsd      xmm0,[si]

    pminuw      xmm0,xmm1
    pminuw      xmm0,[si]

    pminud      xmm0,xmm1
    pminud      xmm0,[si]

    pmaxsb      xmm0,xmm1
    pmaxsb      xmm0,[si]

    pmaxsd      xmm0,xmm1
    pmaxsd      xmm0,[si]

    pmaxuw      xmm0,xmm1
    pmaxuw      xmm0,[si]

    pmaxud      xmm0,xmm1
    pmaxud      xmm0,[si]

    pmulld      xmm0,xmm1
    pmulld      xmm0,[si]

    phminposuw  xmm0,xmm1
    phminposuw  xmm0,[si]

    invept      eax,[si]        ; second must be memory
    invvpid     eax,[si]        ; second must be memory

    movbe       ax,[si]
    movbe       [si],ax
    movbe       eax,[si]
    movbe       [si],eax
    crc32       eax,byte [si]
    crc32       eax,word [si]

    roundps     xmm0,xmm1,1
    roundps     xmm0,[si],1

    roundpd     xmm0,xmm1,1
    roundpd     xmm0,[si],1

    roundss     xmm0,xmm1,1
    roundss     xmm0,[si],1

    roundsd     xmm0,xmm1,1
    roundsd     xmm0,[si],1

    blendps     xmm0,xmm1,1
    blendps     xmm0,[si],1

    blendpd     xmm0,xmm1,1
    blendpd     xmm0,[si],1

    pblendw     xmm0,xmm1,1 
    pblendw     xmm0,[si],1

    pextrb      eax,xmm1,1
    pextrb      [si],xmm1,1

    pextrw      eax,xmm1,1
    pextrw      [si],xmm1,1

    pextrd      eax,xmm1,1
    pextrd      [si],xmm1,1

    extractps   eax,xmm1,1
    extractps   [si],xmm1,1

    pinsrb      xmm0,eax,1
    pinsrb      xmm0,[si],1

    pinsrw      xmm0,eax,1
    pinsrw      xmm0,[si],1

    pinsrd      xmm0,eax,1
    pinsrd      xmm0,[si],1

    dpps        xmm0,xmm1,1
    dpps        xmm0,[si],1

    dppd        xmm0,xmm1,1
    dppd        xmm0,[si],1

    mpsadbw     xmm0,xmm1,1
    mpsadbw     xmm0,[si],1

    pcmpestrm   xmm0,xmm1,1
    pcmpestrm   xmm0,[si],1

    pcmpestri   xmm0,xmm1,1
    pcmpestri   xmm0,[si],1

    pcmpistrm   xmm0,xmm1,1
    pcmpistrm   xmm0,[si],1

    pcmpistri   xmm0,xmm1,1
    pcmpistri   xmm0,[si],1

    xsave       [si]
    xrstor      [si]

    popcnt      ax,bx
    popcnt      ax,[si]
    popcnt      eax,ebx
    popcnt      eax,[si]

    lzcnt       ax,cx
    lzcnt       ax,[si]
    lzcnt       eax,ecx
    lzcnt       eax,[si]

    nop
    nop
    db          0x0F,0xB9       ; UD
    nop
    nop

    nop
    nop
    ud2
    nop
    nop

    vmcall
    vmlaunch
    vmresume
    vmxoff
    vmread      eax,ebx
    vmread      dword [si],ebx
    vmwrite     eax,ebx
    vmwrite     eax,dword [si]
    vmptrld     [si]
    vmclear     [si]
    vmxon       [si]
    vmptrst     [si]

    nop
    nop

; VEX encoding

    vzeroall
    vzeroupper

; more complex encodings

    vperm2f128  ymm1,ymm2,ymm3,4
    vperm2f128  ymm1,ymm2,[si],4
    vperm2f128  ymm1,ymm2,[eax*8+ecx+0x12345678],4

    vpermilpd   xmm1,xmm2,xmm3
    vpermilpd   xmm1,xmm2,[si]
    vpermilpd   xmm1,xmm2,[ebx*4+esi+0x12345678]

    vpermilpd   ymm1,ymm2,ymm3
    vpermilpd   ymm1,ymm2,[si]
    vpermilpd   ymm1,ymm2,[ebx*4+esi+0x12345678]

    vpermilpd   xmm1,xmm2,4
    vpermilpd   xmm1,[si],4
    vpermilpd   xmm1,[ebx*4+esi+0x12345678],4

    vpermilpd   ymm1,ymm2,4
    vpermilpd   ymm1,[si],4
    vpermilpd   ymm1,[ebx*4+esi+0x12345678],4

    vpermilps   xmm1,xmm2,xmm3
    vpermilps   xmm1,xmm2,[si]
    vpermilps   xmm1,xmm2,[ebx*4+esi+0x12345678]

    vpermilps   ymm1,ymm2,ymm3
    vpermilps   ymm1,ymm2,[si]
    vpermilps   ymm1,ymm2,[ebx*4+esi+0x12345678]

    vpermilps   xmm1,xmm2,4
    vpermilps   xmm1,[si],4
    vpermilps   xmm1,[ebx*4+esi+0x12345678],4

    vpermilps   ymm1,ymm2,4
    vpermilps   ymm1,[si],4
    vpermilps   ymm1,[ebx*4+esi+0x12345678],4

    ; this instruction does not have mod == 3 form
    vmaskmovps  xmm1,xmm2,[si]
    vmaskmovps  ymm1,ymm2,[si]
    vmaskmovpd  xmm1,xmm2,[si]
    vmaskmovpd  ymm1,ymm2,[si]

    vmaskmovps  [si],xmm1,xmm2
    vmaskmovps  [si],ymm1,ymm2
    vmaskmovpd  [si],xmm1,xmm2
    vmaskmovpd  [si],ymm1,ymm2

    vextractf128 xmm1,ymm2,3
    vextractf128 [si],ymm2,3

    vinsertf128 ymm1,ymm2,xmm3,4
    vinsertf128 ymm1,ymm2,[si],4

    ; shut up and encode these instructions!
    cpu AVX, CLMUL, AES, XSAVE, SSE4.2, SSE4.1, SSSE3, SSE3, SSE2, SSE, MMX, FPU, SMM, Prot, Priv, FMA, AVX2, FMA4, SSE4
    cpu Sandybridge
    cpu AVX2

    vbroadcastss xmm1,[si]
    vbroadcastss ymm1,[si]
    vbroadcastsd ymm1,[si]
    vbroadcastf128 ymm1,[si]
    vbroadcastss xmm1,xmm2
    vbroadcastss ymm1,xmm2
    vbroadcastsd ymm1,xmm2

    ; VEX new version of SSE instructions
    vaddps      xmm0,xmm1
    vaddps      xmm0,xmm1,xmm2
    vaddps      ymm0,ymm1
    vaddps      ymm0,ymm1,ymm2
    vaddpd      xmm0,xmm1
    vaddpd      xmm0,xmm1,xmm2
    vaddpd      ymm0,ymm1
    vaddpd      ymm0,ymm1,ymm2

    vaddss      xmm0,xmm1               ; no ymm0,ymm1 form
    vaddss      xmm0,xmm1,xmm2          ; no ymm0,ymm1 form
    vaddsd      xmm0,xmm1               ; no ymm0,ymm1 form
    vaddsd      xmm0,xmm1,xmm2          ; no ymm0,ymm1 form

    vmovups     xmm0,xmm1
    vmovups     xmm0,[si]
    vmovups     [si],xmm1
    vmovss      xmm0,xmm1
    vmovss      xmm0,[si]
    vmovss      [si],xmm1
    vmovss      xmm0,xmm1,xmm2
    vmovupd     xmm0,xmm1
    vmovupd     xmm0,[si]
    vmovupd     [si],xmm1
    vmovsd      xmm0,xmm1
    vmovsd      xmm0,[si]
    vmovsd      [si],xmm1
    vmovsd      xmm0,xmm1,xmm2
    vmovhlps    xmm0,xmm1
    vmovhlps    xmm0,xmm1,xmm2
    vunpcklps   xmm0,xmm1
    vunpcklps   xmm0,xmm1,xmm2
    vunpcklps   xmm0,[si]
    vunpcklps   xmm0,xmm1,[si]
    vunpcklps   ymm0,ymm1
    vunpcklps   ymm0,ymm1,ymm2
    vunpcklps   ymm0,[si]
    vunpcklps   ymm0,ymm1,[si]
    vunpcklpd   xmm0,xmm1
    vunpcklpd   xmm0,xmm1,xmm2
    vunpcklpd   xmm0,[si]
    vunpcklpd   xmm0,xmm1,[si]
    vunpcklpd   ymm0,ymm1
    vunpcklpd   ymm0,ymm1,ymm2
    vunpcklpd   ymm0,[si]
    vunpcklpd   ymm0,ymm1,[si]
    vunpckhps   xmm0,xmm1
    vunpckhps   xmm0,xmm1,xmm2
    vunpckhps   xmm0,[si]
    vunpckhps   xmm0,xmm1,[si]
    vunpckhps   ymm0,ymm1
    vunpckhps   ymm0,ymm1,ymm2
    vunpckhps   ymm0,[si]
    vunpckhps   ymm0,ymm1,[si]
    vunpckhpd   xmm0,xmm1
    vunpckhpd   xmm0,xmm1,xmm2
    vunpckhpd   xmm0,[si]
    vunpckhpd   xmm0,xmm1,[si]
    vunpckhpd   ymm0,ymm1
    vunpckhpd   ymm0,ymm1,ymm2
    vunpckhpd   ymm0,[si]
    vunpckhpd   ymm0,ymm1,[si]
    vmovlhps    xmm0,xmm1
    vmovlhps    xmm0,xmm1,xmm2
    vmovhps     xmm0,xmm1,[si]
    vmovhps     [si],xmm1
    vmovhpd     xmm0,xmm1,[si]
    vmovhpd     [si],xmm1
    vmovshdup   xmm0,xmm1
    vmovshdup   xmm0,[si]
    vmovshdup   ymm0,ymm1
    vmovshdup   ymm0,[si]
    vmovaps     xmm0,xmm1
    vmovaps     xmm0,[si]
    vmovaps     ymm0,ymm1
    vmovaps     ymm0,[si]
    vmovaps     [si],xmm0
    vmovaps     [si],ymm0
    vmovapd     xmm0,xmm1
    vmovapd     xmm0,[si]
    vmovapd     ymm0,ymm1
    vmovapd     ymm0,[si]
    vmovapd     [si],xmm0
    vmovapd     [si],ymm0
    vmovntps    [si],xmm0
    vmovntps    [si],ymm0
    vmovntpd    [si],xmm0
    vmovntpd    [si],ymm0
    vucomiss    xmm0,xmm1
    vucomiss    xmm0,[si]
    vucomisd    xmm0,xmm1
    vucomisd    xmm0,[si]
    vcomiss     xmm0,xmm1
    vcomiss     xmm0,[si]
    vcomisd     xmm0,xmm1
    vcomisd     xmm0,[si]
    vpshufb     xmm0,xmm1,xmm2
    vpshufb     xmm0,xmm1,[si]
    vpshufb     ymm0,ymm1,ymm2
    vpshufb     ymm0,ymm1,[si]
    vphaddw     xmm0,xmm1,xmm2
    vphaddw     xmm0,xmm1,[si]
    vphaddw     ymm0,ymm1,ymm2
    vphaddw     ymm0,ymm1,[si]
    vphaddd     xmm0,xmm1,xmm2
    vphaddd     xmm0,xmm1,[si]
    vphaddd     ymm0,ymm1,ymm2
    vphaddd     ymm0,ymm1,[si]
    vphaddsw    xmm0,xmm1,xmm2
    vphaddsw    xmm0,xmm1,[si]
    vphaddsw    ymm0,ymm1,ymm2
    vphaddsw    ymm0,ymm1,[si]
    vpmaddubsw  xmm0,xmm1,xmm2
    vpmaddubsw  xmm0,xmm1,[si]
    vpmaddubsw  ymm0,ymm1,ymm2
    vpmaddubsw  ymm0,ymm1,[si]
    vphsubw     xmm0,xmm1,xmm2
    vphsubw     xmm0,xmm1,[si]
    vphsubw     ymm0,ymm1,ymm2
    vphsubw     ymm0,ymm1,[si]
    vphsubd     xmm0,xmm1,xmm2
    vphsubd     xmm0,xmm1,[si]
    vphsubd     ymm0,ymm1,ymm2
    vphsubd     ymm0,ymm1,[si]
    vphsubsw    xmm0,xmm1,xmm2
    vphsubsw    xmm0,xmm1,[si]
    vphsubsw    ymm0,ymm1,ymm2
    vphsubsw    ymm0,ymm1,[si]
    vpsignb     xmm0,xmm1,xmm2
    vpsignb     xmm0,xmm1,[si]
    vpsignb     ymm0,ymm1,ymm2
    vpsignb     ymm0,ymm1,[si]
    vpsignw     xmm0,xmm1,xmm2
    vpsignw     xmm0,xmm1,[si]
    vpsignw     ymm0,ymm1,ymm2
    vpsignw     ymm0,ymm1,[si]
    vpsignd     xmm0,xmm1,xmm2
    vpsignd     xmm0,xmm1,[si]
    vpsignd     ymm0,ymm1,ymm2
    vpsignd     ymm0,ymm1,[si]
    vpmulhrsw   xmm1,xmm2,xmm3
    vpmulhrsw   xmm1,xmm2,[si]
    vpmulhrsw   ymm1,ymm2,ymm3
    vpmulhrsw   ymm1,ymm2,[si]
    vpblendvb   xmm1,xmm2,xmm3,xmm4
    vpblendvb   xmm1,xmm2,[si],xmm4
    vpblendvb   ymm1,ymm2,ymm3,ymm4
    vpblendvb   ymm1,ymm2,[si],ymm4
    vblendvps   xmm1,xmm2,xmm3,xmm4
    vblendvps   xmm1,xmm2,[si],xmm4
    vblendvps   ymm1,ymm2,ymm3,ymm4
    vblendvps   ymm1,ymm2,[si],ymm4
    vblendvpd   xmm1,xmm2,xmm3,xmm4
    vblendvpd   xmm1,xmm2,[si],xmm4
    vblendvpd   ymm1,ymm2,ymm3,ymm4
    vblendvpd   ymm1,ymm2,[si],ymm4
    vptest      xmm1,xmm2
    vptest      xmm1,[si]
    vptest      ymm1,ymm2
    vptest      ymm1,[si]
    vpabsb      xmm1,xmm2
    vpabsb      xmm1,[si]
    vpabsb      ymm1,ymm2
    vpabsb      ymm1,[si]
    vpabsw      xmm1,xmm2
    vpabsw      xmm1,[si]
    vpabsw      ymm1,ymm2
    vpabsw      ymm1,[si]
    vpabsd      xmm1,xmm2
    vpabsd      xmm1,[si]
    vpabsd      ymm1,ymm2
    vpabsd      ymm1,[si]
    vpmovsxbw   xmm1,xmm2
    vpmovsxbw   xmm1,[si]
    vpmovsxbw   ymm1,xmm2
    vpmovsxbw   ymm1,[si]
    vpmovsxbd   xmm1,xmm2
    vpmovsxbd   xmm1,[si]
    vpmovsxbd   ymm1,xmm2
    vpmovsxbd   ymm1,[si]
    vpmovsxbq   xmm1,xmm2
    vpmovsxbq   xmm1,[si]
    vpmovsxbq   ymm1,xmm2
    vpmovsxbq   ymm1,[si]
    vpmovsxwd   xmm1,xmm2
    vpmovsxwd   xmm1,[si]
    vpmovsxwd   ymm1,xmm2
    vpmovsxwd   ymm1,[si]
    vpmovsxwq   xmm1,xmm2
    vpmovsxwq   xmm1,[si]
    vpmovsxwq   ymm1,xmm2
    vpmovsxwq   ymm1,[si]
    vpmovsxdq   xmm1,xmm2
    vpmovsxdq   xmm1,[si]
    vpmovsxdq   ymm1,xmm2
    vpmovsxdq   ymm1,[si]
    vpmuldq     xmm1,xmm2,xmm3
    vpmuldq     xmm1,xmm2,[si]
    vpmuldq     ymm1,ymm2,ymm3
    vpmuldq     ymm1,ymm2,[si]
    vpcmpeqq    xmm1,xmm2,xmm3
    vpcmpeqq    xmm1,xmm2,[si]
    vpcmpeqq    ymm1,ymm2,ymm3
    vpcmpeqq    ymm1,ymm2,[si]
    vmovntdqa   xmm1,[si]
    vmovntdqa   ymm1,[si]
    vpackusdw   xmm1,xmm2,xmm3
    vpackusdw   xmm1,xmm2,[si]
    vpackusdw   ymm1,ymm2,ymm3
    vpackusdw   ymm1,ymm2,[si]
    vpmovzxbw   xmm1,xmm2
    vpmovzxbw   xmm1,[si]
    vpmovzxbw   ymm1,xmm2
    vpmovzxbw   ymm1,[si]
    vpmovzxbd   xmm1,xmm2
    vpmovzxbd   xmm1,[si]
    vpmovzxbd   ymm1,xmm2
    vpmovzxbd   ymm1,[si]
    vpmovzxbq   ymm1,xmm2
    vpmovzxbq   ymm1,[si]
    vpmovzxbq   xmm1,xmm2
    vpmovzxbq   xmm1,[si]
    vpmovzxwd   xmm1,xmm2
    vpmovzxwd   xmm1,[si]
    vpmovzxwd   ymm1,xmm2
    vpmovzxwd   ymm1,[si]
    vpmovzxwq   xmm1,xmm2
    vpmovzxwq   xmm1,[si]
    vpmovzxwq   ymm1,xmm2
    vpmovzxwq   ymm1,[si]
    vpmovzxdq   xmm1,xmm2
    vpmovzxdq   xmm1,[si]
    vpmovzxdq   ymm1,xmm2
    vpmovzxdq   ymm1,[si]
    vpcmpgtq    xmm1,xmm2,xmm3
    vpcmpgtq    xmm1,xmm2,[si]
    vpcmpgtq    ymm1,ymm2,ymm3
    vpcmpgtq    ymm1,ymm2,[si]
    vpminsb     xmm1,xmm2,xmm3
    vpminsb     xmm1,xmm2,[si]
    vpminsb     ymm1,ymm2,ymm3
    vpminsb     ymm1,ymm2,[si]
    vpminsd     xmm1,xmm2,xmm3
    vpminsd     xmm1,xmm2,[si]
    vpminsd     ymm1,ymm2,ymm3
    vpminsd     ymm1,ymm2,[si]
    vpminuw     xmm1,xmm2,xmm3
    vpminuw     xmm1,xmm2,[si]
    vpminuw     ymm1,ymm2,ymm3
    vpminuw     ymm1,ymm2,[si]
    vpminud     xmm1,xmm2,xmm3
    vpminud     xmm1,xmm2,[si]
    vpminud     ymm1,ymm2,ymm3
    vpminud     ymm1,ymm2,[si]
    vpmaxsb     xmm1,xmm2,xmm3
    vpmaxsb     xmm1,xmm2,[si]
    vpmaxsb     ymm1,ymm2,ymm3
    vpmaxsb     ymm1,ymm2,[si]
    vpmaxsd     xmm1,xmm2,xmm3
    vpmaxsd     xmm1,xmm2,[si]
    vpmaxsd     ymm1,ymm2,ymm3
    vpmaxsd     ymm1,ymm2,[si]
    vpmaxuw     xmm1,xmm2,xmm3
    vpmaxuw     xmm1,xmm2,[si]
    vpmaxuw     ymm1,ymm2,ymm3
    vpmaxuw     ymm1,ymm2,[si]
    vpmaxud     xmm1,xmm2,xmm3
    vpmaxud     xmm1,xmm2,[si]
    vpmaxud     ymm1,ymm2,ymm3
    vpmaxud     ymm1,ymm2,[si]
    vpmulld     xmm1,xmm2,xmm3
    vpmulld     xmm1,xmm2,[si]
    vpmulld     ymm1,ymm2,ymm3
    vpmulld     ymm1,ymm2,[si]
    vphminposuw xmm1,xmm2
    vphminposuw xmm1,[si]
    vroundps    xmm1,xmm2,3
    vroundps    xmm1,[si],3
    vroundps    ymm1,ymm2,3
    vroundps    ymm1,[si],3
    vroundpd    xmm1,xmm2,3
    vroundpd    xmm1,[si],3
    vroundpd    ymm1,ymm2,3
    vroundpd    ymm1,[si],3
    vroundss    xmm1,xmm2,xmm3,4
    vroundss    xmm1,xmm2,[si],4
    vroundsd    xmm1,xmm2,xmm3,4
    vroundsd    xmm1,xmm2,[si],4
    vblendps    xmm1,xmm2,xmm3,4
    vblendps    xmm1,xmm2,[si],4
    vblendps    ymm1,ymm2,ymm3,4
    vblendps    ymm1,ymm2,[si],4
    vblendpd    xmm1,xmm2,xmm3,4
    vblendpd    xmm1,xmm2,[si],4
    vblendpd    ymm1,ymm2,ymm3,4
    vblendpd    ymm1,ymm2,[si],4
    vpblendw    xmm1,xmm2,xmm3,4
    vpblendw    xmm1,xmm2,[si],4
    vpblendw    ymm1,ymm2,ymm3,4
    vpblendw    ymm1,ymm2,[si],4
    vpalignr    xmm1,xmm2,xmm3,4
    vpalignr    xmm1,xmm2,[si],4
    vpalignr    ymm1,ymm2,ymm3,4
    vpalignr    ymm1,ymm2,[si],4
    vpextrb     eax,xmm2,3
    vpextrb     [si],xmm2,3
    vpextrw     eax,xmm1,2
    vpextrw     eax,xmm2,3
    vpextrw     [si],xmm2,3
    vpextrd     eax,xmm2,3
    vpextrd     [si],xmm2,3
    vextractps  eax,xmm1,2
    vextractps  [si],xmm1,2
    vpinsrb     xmm1,xmm2,ecx,4
    vpinsrb     xmm1,xmm2,[si],4
    vpinsrw     xmm1,xmm2,ecx,4
    vpinsrw     xmm1,xmm2,[si],4
    vpinsrd     xmm1,xmm2,ecx,4
    vpinsrd     xmm1,xmm2,[si],4
    vinsertps   xmm1,xmm2,xmm3,4
    vinsertps   xmm1,xmm2,[si],4
    vdpps       xmm1,xmm2,xmm3,4
    vdpps       xmm1,xmm2,[si],4
    vdpps       ymm1,ymm2,ymm3,4
    vdpps       ymm1,ymm2,[si],4
    vdppd       xmm1,xmm2,xmm3,4
    vdppd       xmm1,xmm2,[si],4
    vmpsadbw    xmm1,xmm2,xmm3,4
    vmpsadbw    xmm1,xmm2,[si],4
    vmpsadbw    ymm1,ymm2,ymm3,4
    vmpsadbw    ymm1,ymm2,[si],4
    vpcmpestrm  xmm1,xmm2,3
    vpcmpestrm  xmm1,[si],3
    vpcmpestri  xmm1,xmm2,3
    vpcmpestri  xmm1,[si],3
    vpcmpistrm  xmm1,xmm2,3
    vpcmpistrm  xmm1,[si],3
    vpcmpistri  xmm1,xmm2,3
    vpcmpistri  xmm1,[si],3
    vmovmskps   eax,xmm2
    vmovmskps   eax,ymm2
    vmovmskpd   eax,xmm2
    vmovmskpd   eax,ymm2
    vsqrtps     xmm1,xmm2
    vsqrtps     xmm1,[si]
    vsqrtps     ymm1,ymm2
    vsqrtps     ymm1,[si]
    vsqrtss     xmm1,xmm2,xmm3
    vsqrtss     xmm1,xmm2,[si]
    vsqrtpd     xmm1,xmm2
    vsqrtpd     xmm1,[si]
    vsqrtpd     ymm1,ymm2
    vsqrtpd     ymm1,[si]
    vsqrtsd     xmm1,xmm2,xmm3
    vsqrtsd     xmm1,xmm2,[si]
    vrsqrtps    xmm1,xmm2
    vrsqrtps    xmm1,[si]
    vrsqrtps    ymm1,ymm2
    vrsqrtps    ymm1,[si]
    vrsqrtss    xmm1,xmm2,xmm3
    vrsqrtss    xmm1,xmm2,[si]
    vrcpps      xmm1,xmm2
    vrcpps      xmm1,[si]
    vrcpps      ymm1,ymm2
    vrcpps      ymm1,[si]
    vrcpss      xmm1,xmm2,xmm3
    vrcpss      xmm1,xmm2,[si]
    vandps      xmm1,xmm2,xmm3
    vandps      xmm1,xmm2,[si]
    vandps      ymm1,ymm2,ymm3
    vandps      ymm1,ymm2,[si]
    vandpd      xmm1,xmm2,xmm3
    vandpd      xmm1,xmm2,[si]
    vandpd      ymm1,ymm2,ymm3
    vandpd      ymm1,ymm2,[si]
    vandnps     xmm1,xmm2,xmm3
    vandnps     xmm1,xmm2,[si]
    vandnps     ymm1,ymm2,ymm3
    vandnps     ymm1,ymm2,[si]
    vandnpd     xmm1,xmm2,xmm3
    vandnpd     xmm1,xmm2,[si]
    vandnpd     ymm1,ymm2,ymm3
    vandnpd     ymm1,ymm2,[si]
    vorps       xmm1,xmm2,xmm3
    vorps       xmm1,xmm2,[si]
    vorps       ymm1,ymm2,ymm3
    vorps       ymm1,ymm2,[si]
    vorpd       xmm1,xmm2,xmm3
    vorpd       xmm1,xmm2,[si]
    vorpd       ymm1,ymm2,ymm3
    vorpd       ymm1,ymm2,[si]
    vxorps      xmm1,xmm2,xmm3
    vxorps      xmm1,xmm2,[si]
    vxorps      ymm1,ymm2,ymm3
    vxorps      ymm1,ymm2,[si]
    vxorpd      xmm1,xmm2,xmm3
    vxorpd      xmm1,xmm2,[si]
    vxorpd      ymm1,ymm2,ymm3
    vxorpd      ymm1,ymm2,[si]
    vaddps      xmm1,xmm2,xmm3
    vaddps      xmm1,xmm2,[si]
    vaddps      ymm1,ymm2,ymm3
    vaddps      ymm1,ymm2,[si]
    vaddpd      xmm1,xmm2,xmm3
    vaddpd      xmm1,xmm2,[si]
    vaddpd      ymm1,ymm2,ymm3
    vaddpd      ymm1,ymm2,[si]
    vaddss      xmm1,xmm2,xmm3
    vaddss      xmm1,xmm2,[si]
    vaddsd      xmm1,xmm2,xmm3
    vaddsd      xmm1,xmm2,[si]
    vmulps      xmm1,xmm2,xmm3
    vmulps      xmm1,xmm2,[si]
    vmulps      ymm1,ymm2,ymm3
    vmulps      ymm1,ymm2,[si]
    vmulpd      xmm1,xmm2,xmm3
    vmulpd      xmm1,xmm2,[si]
    vmulpd      ymm1,ymm2,ymm3
    vmulpd      ymm1,ymm2,[si]
    vmulss      xmm1,xmm2,xmm3
    vmulss      xmm1,xmm2,[si]
    vmulsd      xmm1,xmm2,xmm3
    vmulsd      xmm1,xmm2,[si]
    vcvtps2pd   xmm1,xmm2
    vcvtps2pd   xmm1,[si]
    vcvtps2pd   ymm1,xmm2
    vcvtps2pd   ymm1,[si]
    vcvtsi2ss   xmm1,xmm2,eax
    vcvtsi2ss   xmm1,xmm2,[si]
    vmovntps    [si],xmm1
    vmovntps    [si],ymm1
    vmovntpd    [si],xmm1
    vmovntpd    [si],ymm1
    vucomiss    xmm1,xmm2
    vucomiss    xmm1,[si]
    vucomisd    xmm1,xmm2
    vucomisd    xmm1,[si]
    vcomiss     xmm1,xmm2
    vcomiss     xmm1,[si]
    vcomisd     xmm1,xmm2
    vcomisd     xmm1,[si]
    vsubps      xmm1,xmm2,xmm3
    vsubps      xmm1,xmm2,[si]
    vsubps      ymm1,ymm2,ymm3
    vsubps      ymm1,ymm2,[si]
    vsubpd      xmm1,xmm2,xmm3
    vsubpd      xmm1,xmm2,[si]
    vsubpd      ymm1,ymm2,ymm3
    vsubpd      ymm1,ymm2,[si]
    vsubss      xmm1,xmm2,xmm3
    vsubss      xmm1,xmm2,[si]
    vsubsd      xmm1,xmm2,xmm3
    vsubsd      xmm1,xmm2,[si]
    vminps      xmm1,xmm2,xmm3
    vminps      xmm1,xmm2,[si]
    vminps      ymm1,ymm2,ymm3
    vminps      ymm1,ymm2,[si]
    vminpd      xmm1,xmm2,xmm3
    vminpd      xmm1,xmm2,[si]
    vminpd      ymm1,ymm2,ymm3
    vminpd      ymm1,ymm2,[si]
    vminss      xmm1,xmm2,xmm3
    vminss      xmm1,xmm2,[si]
    vminsd      xmm1,xmm2,xmm3
    vminsd      xmm1,xmm2,[si]
    vdivps      xmm1,xmm2,xmm3
    vdivps      xmm1,xmm2,[si]
    vdivps      ymm1,ymm2,ymm3
    vdivps      ymm1,ymm2,[si]
    vdivpd      xmm1,xmm2,xmm3
    vdivpd      xmm1,xmm2,[si]
    vdivpd      ymm1,ymm2,ymm3
    vdivpd      ymm1,ymm2,[si]
    vdivss      xmm1,xmm2,xmm3
    vdivss      xmm1,xmm2,[si]
    vdivsd      xmm1,xmm2,xmm3
    vdivsd      xmm1,xmm2,[si]
    vmaxps      xmm1,xmm2,xmm3
    vmaxps      xmm1,xmm2,[si]
    vmaxps      ymm1,ymm2,ymm3
    vmaxps      ymm1,ymm2,[si]
    vmaxpd      xmm1,xmm2,xmm3
    vmaxpd      xmm1,xmm2,[si]
    vmaxpd      ymm1,ymm2,ymm3
    vmaxpd      ymm1,ymm2,[si]
    vmaxss      xmm1,xmm2,xmm3
    vmaxss      xmm1,xmm2,[si]
    vmaxsd      xmm1,xmm2,xmm3
    vmaxsd      xmm1,xmm2,[si]
    vpunpcklbw  xmm1,xmm2,xmm3
    vpunpcklbw  xmm1,xmm2,[si]
    vpunpcklbw  ymm1,ymm2,ymm3
    vpunpcklbw  ymm1,ymm2,[si]
    vpunpcklwd  xmm1,xmm2,xmm3
    vpunpcklwd  xmm1,xmm2,[si]
    vpunpcklwd  ymm1,ymm2,ymm3
    vpunpcklwd  ymm1,ymm2,[si]
    vpunpckldq  xmm1,xmm2,xmm3
    vpunpckldq  xmm1,xmm2,[si]
    vpunpckldq  ymm1,ymm2,ymm3
    vpunpckldq  ymm1,ymm2,[si]
    vpunpcklqdq xmm1,xmm2,xmm3
    vpunpcklqdq xmm1,xmm2,[si]
    vpunpcklqdq ymm1,ymm2,ymm3
    vpunpcklqdq ymm1,ymm2,[si]
    vpacksswb   xmm1,xmm2,xmm3
    vpacksswb   xmm1,xmm2,[si]
    vpacksswb   ymm1,ymm2,ymm3
    vpacksswb   ymm1,ymm2,[si]
    vpackssdw   xmm1,xmm2,xmm3
    vpackssdw   xmm1,xmm2,[si]
    vpackssdw   ymm1,ymm2,ymm3
    vpackssdw   ymm1,ymm2,[si]
    vpcmpgtb    xmm1,xmm2,xmm3
    vpcmpgtb    xmm1,xmm2,[si]
    vpcmpgtb    ymm1,ymm2,ymm3
    vpcmpgtb    ymm1,ymm2,[si]
    vpcmpgtw    xmm1,xmm2,xmm3
    vpcmpgtw    xmm1,xmm2,[si]
    vpcmpgtw    ymm1,ymm2,ymm3
    vpcmpgtw    ymm1,ymm2,[si]
    vpcmpgtd    xmm1,xmm2,xmm3
    vpcmpgtd    xmm1,xmm2,[si]
    vpcmpgtd    ymm1,ymm2,ymm3
    vpcmpgtd    ymm1,ymm2,[si]
    vpackuswb   xmm1,xmm2,xmm3
    vpackuswb   xmm1,xmm2,[si]
    vpackuswb   ymm1,ymm2,ymm3
    vpackuswb   ymm1,ymm2,[si]
    vpunpckhbw  xmm1,xmm2,xmm3
    vpunpckhbw  xmm1,xmm2,[si]
    vpunpckhbw  ymm1,ymm2,ymm3
    vpunpckhbw  ymm1,ymm2,[si]
    vpunpckhwd  xmm1,xmm2,xmm3
    vpunpckhwd  xmm1,xmm2,[si]
    vpunpckhwd  ymm1,ymm2,ymm3
    vpunpckhwd  ymm1,ymm2,[si]
    vpunpckhdq  xmm1,xmm2,xmm3
    vpunpckhdq  xmm1,xmm2,[si]
    vpunpckhdq  ymm1,ymm2,ymm3
    vpunpckhdq  ymm1,ymm2,[si]
    vpunpckhqdq xmm1,xmm2,xmm3
    vpunpckhqdq xmm1,xmm2,[si]
    vpunpckhqdq ymm1,ymm2,ymm3
    vpunpckhqdq ymm1,ymm2,[si]
    vmovd       xmm1,eax
    vmovd       xmm1,[si]
    vmovd       eax,xmm1
    vmovd       [si],xmm1
    vmovdqa     xmm1,xmm2
    vmovdqa     xmm1,[si]
    vmovdqa     [si],xmm2
    vmovdqa     ymm1,ymm2
    vmovdqa     ymm1,[si]
    vmovdqa     [si],ymm2
    vpshuflw    xmm1,xmm2,3
    vpshuflw    xmm1,[si],3
    vpshuflw    ymm1,ymm2,3
    vpshuflw    ymm1,[si],3
    vpshufhw    xmm1,xmm2,3
    vpshufhw    xmm1,[si],3
    vpshufhw    ymm1,ymm2,3
    vpshufhw    ymm1,[si],3
    vpshufd     xmm1,xmm2,3
    vpshufd     xmm1,[si],3
    vpshufd     ymm1,ymm2,3
    vpshufd     ymm1,[si],3
    vpsrlw      xmm1,xmm2,xmm3
    vpsrlw      xmm1,xmm2,[si]
    vpsrlw      xmm1,xmm2,3
    vpsrld      xmm1,xmm2,xmm3
    vpsrld      xmm1,xmm2,[si]
    vpsrld      xmm1,xmm2,3
    vpsrlq      xmm1,xmm2,xmm3
    vpsrlq      xmm1,xmm2,[si]
    vpsrlq      xmm1,xmm2,3
    vpsrldq     xmm1,xmm2,3
    vpsrlw      ymm1,ymm2,xmm3
    vpsrlw      ymm1,ymm2,[si]
    vpsrlw      ymm1,ymm2,3
    vpsrld      ymm1,ymm2,xmm3
    vpsrld      ymm1,ymm2,[si]
    vpsrld      ymm1,ymm2,3
    vpsrlq      ymm1,ymm2,xmm3
    vpsrlq      ymm1,ymm2,[si]
    vpsrlq      ymm1,ymm2,3
    vpsrldq     ymm1,ymm2,3
    vpsraw      xmm1,xmm2,xmm3
    vpsraw      xmm1,xmm2,[si]
    vpsraw      xmm1,xmm2,3
    vpsrad      xmm1,xmm2,xmm3
    vpsrad      xmm1,xmm2,[si]
    vpsrad      xmm1,xmm2,3
    vpsraw      ymm1,ymm2,xmm3
    vpsraw      ymm1,ymm2,[si]
    vpsraw      ymm1,ymm2,3
    vpsrad      ymm1,ymm2,xmm3
    vpsrad      ymm1,ymm2,[si]
    vpsrad      ymm1,ymm2,3
    vpsllw      xmm1,xmm2,xmm3
    vpsllw      xmm1,xmm2,[si]
    vpsllw      xmm1,xmm2,3
    vpslld      xmm1,xmm2,xmm3
    vpslld      xmm1,xmm2,[si]
    vpslld      xmm1,xmm2,3
    vpsllq      xmm1,xmm2,xmm3
    vpsllq      xmm1,xmm2,[si]
    vpsllq      xmm1,xmm2,3
    vpslldq     xmm1,xmm2,3
    vpsllw      ymm1,ymm2,xmm3
    vpsllw      ymm1,ymm2,[si]
    vpsllw      ymm1,ymm2,3
    vpslld      ymm1,ymm2,xmm3
    vpslld      ymm1,ymm2,[si]
    vpslld      ymm1,ymm2,3
    vpsllq      ymm1,ymm2,xmm3
    vpsllq      ymm1,ymm2,[si]
    vpsllq      ymm1,ymm2,3
    vpslldq     ymm1,ymm2,3
    vpcmpeqb    xmm1,xmm2,xmm3
    vpcmpeqb    xmm1,xmm2,[si]
    vpcmpeqb    ymm1,ymm2,ymm3
    vpcmpeqb    ymm1,ymm2,[si]
    vpcmpeqw    xmm1,xmm2,xmm3
    vpcmpeqw    xmm1,xmm2,[si]
    vpcmpeqw    ymm1,ymm2,ymm3
    vpcmpeqw    ymm1,ymm2,[si]
    vpcmpeqd    xmm1,xmm2,xmm3
    vpcmpeqd    xmm1,xmm2,[si]
    vpcmpeqd    ymm1,ymm2,ymm3
    vpcmpeqd    ymm1,ymm2,[si]
    vhaddpd     xmm1,xmm2,xmm3
    vhaddpd     xmm1,xmm2,[si]
    vhaddpd     ymm1,ymm2,ymm3
    vhaddpd     ymm1,ymm2,[si]
    vhaddps     xmm1,xmm2,xmm3
    vhaddps     xmm1,xmm2,[si]
    vhaddps     ymm1,ymm2,ymm3
    vhaddps     ymm1,ymm2,[si]
    vhsubpd     xmm1,xmm2,xmm3
    vhsubpd     xmm1,xmm2,[si]
    vhsubpd     ymm1,ymm2,ymm3
    vhsubpd     ymm1,ymm2,[si]
    vhsubps     xmm1,xmm2,xmm3
    vhsubps     xmm1,xmm2,[si]
    vhsubps     ymm1,ymm2,ymm3
    vhsubps     ymm1,ymm2,[si]
    vldmxcsr    [si]
    vstmxcsr    [si]
    vcmpps      xmm1,xmm2,xmm3,4
    vcmpps      xmm1,xmm2,[si],4
    vcmpps      ymm1,ymm2,ymm3,4
    vcmpps      ymm1,ymm2,[si],4
    vcmppd      xmm1,xmm2,xmm3,4
    vcmppd      xmm1,xmm2,[si],4
    vcmppd      ymm1,ymm2,ymm3,4
    vcmppd      ymm1,ymm2,[si],4
    vcmpss      xmm1,xmm2,xmm3,4
    vcmpss      xmm1,xmm2,[si],4
    vcmpsd      xmm1,xmm2,xmm3,4
    vcmpsd      xmm1,xmm2,[si],4
    vshufps     xmm1,xmm2,xmm3,4
    vshufps     xmm1,xmm2,[si],4
    vshufps     ymm1,ymm2,ymm3,4
    vshufps     ymm1,ymm2,[si],4
    vshufpd     xmm1,xmm2,xmm3,4
    vshufpd     xmm1,xmm2,[si],4
    vshufpd     ymm1,ymm2,ymm3,4
    vshufpd     ymm1,ymm2,[si],4
    vaddsubpd   xmm1,xmm2,xmm3
    vaddsubpd   xmm1,xmm2,[si]
    vaddsubpd   ymm1,ymm2,ymm3
    vaddsubpd   ymm1,ymm2,[si]
    vaddsubps   xmm1,xmm2,xmm3
    vaddsubps   xmm1,xmm2,[si]
    vaddsubps   ymm1,ymm2,ymm3
    vaddsubps   ymm1,ymm2,[si]
    vpaddq      xmm1,xmm2,xmm3
    vpaddq      xmm1,xmm2,[si]
    vpaddq      ymm1,ymm2,ymm3
    vpaddq      ymm1,ymm2,[si]
    vpmullw     xmm1,xmm2,xmm3
    vpmullw     xmm1,xmm2,[si]
    vpmullw     ymm1,ymm2,ymm3
    vpmullw     ymm1,ymm2,[si]
    vmovq       xmm1,xmm2
    vmovq       xmm1,[si]
    vmovq       [si],xmm2
    vpsubusb    xmm1,xmm2,xmm3
    vpsubusb    xmm1,xmm2,[si]
    vpsubusb    ymm1,ymm2,ymm3
    vpsubusb    ymm1,ymm2,[si]
    vpsubusw    xmm1,xmm2,xmm3
    vpsubusw    xmm1,xmm2,[si]
    vpsubusw    ymm1,ymm2,ymm3
    vpsubusw    ymm1,ymm2,[si]
    vpminub     xmm1,xmm2,xmm3
    vpminub     xmm1,xmm2,[si]
    vpminub     ymm1,ymm2,ymm3
    vpminub     ymm1,ymm2,[si]
    vpand       xmm1,xmm2,xmm3
    vpand       xmm1,xmm2,[si]
    vpand       ymm1,ymm2,ymm3
    vpand       ymm1,ymm2,[si]
    vpaddusb    xmm1,xmm2,xmm3
    vpaddusb    xmm1,xmm2,[si]
    vpaddusb    ymm1,ymm2,ymm3
    vpaddusb    ymm1,ymm2,[si]
    vpaddusw    xmm1,xmm2,xmm3
    vpaddusw    xmm1,xmm2,[si]
    vpaddusw    ymm1,ymm2,ymm3
    vpaddusw    ymm1,ymm2,[si]
    vpmaxub     xmm1,xmm2,xmm3
    vpmaxub     xmm1,xmm2,[si]
    vpmaxub     ymm1,ymm2,ymm3
    vpmaxub     ymm1,ymm2,[si]
    vpandn      xmm1,xmm2,xmm3
    vpandn      xmm1,xmm2,[si]
    vpandn      ymm1,ymm2,ymm3
    vpandn      ymm1,ymm2,[si]
    vpavgb      xmm1,xmm2,xmm3
    vpavgb      xmm1,xmm2,[si]
    vpavgb      ymm1,ymm2,ymm3
    vpavgb      ymm1,ymm2,[si]
    vpavgw      xmm1,xmm2,xmm3
    vpavgw      xmm1,xmm2,[si]
    vpavgw      ymm1,ymm2,ymm3
    vpavgw      ymm1,ymm2,[si]
    vpmulhuw    xmm1,xmm2,xmm3
    vpmulhuw    xmm1,xmm2,[si]
    vpmulhuw    ymm1,ymm2,ymm3
    vpmulhuw    ymm1,ymm2,[si]
    vpmulhw     xmm1,xmm2,xmm3
    vpmulhw     xmm1,xmm2,[si]
    vpmulhw     ymm1,ymm2,ymm3
    vpmulhw     ymm1,ymm2,[si]
    vcvtpd2dq   xmm1,xmm2
    vcvtpd2dq   xmm1,dqword [si]
    vcvtpd2dq   xmm1,ymm2
    vcvtpd2dq   xmm1,yword [si]
    vcvttpd2dq  xmm1,xmm2
    vcvttpd2dq  xmm1,dqword [si]
    vcvttpd2dq  xmm1,ymm2
    vcvttpd2dq  xmm1,yword [si]
    vcvtdq2pd   xmm1,xmm2
    vcvtdq2pd   xmm1,[si]
    vcvtdq2pd   ymm1,xmm2
    vcvtdq2pd   ymm1,dqword [si]
    vmovntdq    [si],xmm1
    vmovntdq    [si],ymm1
    vpsubsb     xmm1,xmm2,xmm3
    vpsubsb     xmm1,xmm2,[si]
    vpsubsb     ymm1,ymm2,ymm3
    vpsubsb     ymm1,ymm2,[si]
    vpsubsw     xmm1,xmm2,xmm3
    vpsubsw     xmm1,xmm2,[si]
    vpsubsw     ymm1,ymm2,ymm3
    vpsubsw     ymm1,ymm2,[si]
    vpminsw     xmm1,xmm2,xmm3
    vpminsw     xmm1,xmm2,[si]
    vpminsw     ymm1,ymm2,ymm3
    vpminsw     ymm1,ymm2,[si]
    vpor        xmm1,xmm2,xmm3
    vpor        xmm1,xmm2,[si]
    vpor        ymm1,ymm2,ymm3
    vpor        ymm1,ymm2,[si]
    vpaddsb     xmm1,xmm2,xmm3
    vpaddsb     xmm1,xmm2,[si]
    vpaddsb     ymm1,ymm2,ymm3
    vpaddsb     ymm1,ymm2,[si]
    vpaddsw     xmm1,xmm2,xmm3
    vpaddsw     xmm1,xmm2,[si]
    vpaddsw     ymm1,ymm2,ymm3
    vpaddsw     ymm1,ymm2,[si]
    vpmaxsw     xmm1,xmm2,xmm3
    vpmaxsw     xmm1,xmm2,[si]
    vpmaxsw     ymm1,ymm2,ymm3
    vpmaxsw     ymm1,ymm2,[si]
    vpxor       xmm1,xmm2,xmm3
    vpxor       xmm1,xmm2,[si]
    vpxor       ymm1,ymm2,ymm3
    vpxor       ymm1,ymm2,[si]
    vlddqu      xmm1,[si]
    vlddqu      ymm1,[si]
    vpmuludq    xmm1,xmm2,xmm3
    vpmuludq    xmm1,xmm2,[si]
    vpmuludq    ymm1,ymm2,ymm3
    vpmuludq    ymm1,ymm2,[si]
    vpmaddwd    xmm1,xmm2,xmm3
    vpmaddwd    xmm1,xmm2,[si]
    vpmaddwd    ymm1,ymm2,ymm3
    vpmaddwd    ymm1,ymm2,[si]
    vpsadbw     xmm1,xmm2,xmm3
    vpsadbw     xmm1,xmm2,[si]
    vpsadbw     ymm1,ymm2,ymm3
    vpsadbw     ymm1,ymm2,[si]
    vmaskmovdqu xmm1,xmm2

