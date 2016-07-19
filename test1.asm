
    bits    16
    org     100h

    nop
    cbw
    cwd
    cli
    sti
    pushf
    popf
    ret
    ret     4
    retf
    retf    4
    iret
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
    out     0x44,al
    out     dx,al
    out     0x44,ax
    out     dx,ax

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

    cs
    nop
    ds
    nop
    es
    nop
    ss
    nop

    xlatb

    movsb
    movsw
    cmpsb
    cmpsw
    stosb
    stosw
    lodsb
    lodsw
    scasb
    scasw

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
    cmpsb
    repz
    cmpsw
    repz
    stosb
    repz
    stosw
    repz
    lodsb
    repz
    lodsw
    repz
    scasb
    repz
    scasw

    repnz
    movsb
    repnz
    movsw
    repnz
    cmpsb
    repnz
    cmpsw
    repnz
    stosb
    repnz
    stosw
    repnz
    lodsb
    repnz
    lodsw
    repnz
    scasb
    repnz
    scasw

    lock    xchg    ax,ax

    xchg    cx,ax
    xchg    dx,ax
    xchg    bx,ax
    xchg    sp,ax
    xchg    bp,ax
    xchg    si,ax
    xchg    di,ax

    wait
    sahf
    lahf

    push    es
    pop     es
    push    cs
    db      0x0F    ; pop cs
    nop
    nop
    push    ss
    pop     ss
    push    ds
    pop     ds

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

;---------------add imm
    add     al,0x12
    add     al,0xEF
    add     ax,0x1234
    add     ax,0xFEDC

