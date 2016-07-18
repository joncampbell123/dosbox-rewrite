
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

