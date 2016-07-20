
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

;---------------or  imm
    or      al,0x12
    or      al,0xEF
    or      ax,0x1234
    or      ax,0xFEDC

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

;---------------adc imm
    adc     al,0x12
    adc     al,0xEF
    adc     ax,0x1234
    adc     ax,0xFEDC

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

;---------------sbb imm
    sbb     al,0x12
    sbb     al,0xEF
    sbb     ax,0x1234
    sbb     ax,0xFEDC

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

;---------------and imm
    and     al,0x12
    and     al,0xEF
    and     ax,0x1234
    and     ax,0xFEDC

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

;---------------xor imm
    xor     al,0x12
    xor     al,0xEF
    xor     ax,0x1234
    xor     ax,0xFEDC

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

; GRP1 80h
    add     byte [bx+si],0x12
    add     byte [bx+di],0x12
    add     byte [bp+si],0x12
    add     byte [bp+di],0x12
    add     byte [si],0x12
    add     byte [di],0x12
    add     byte [0x1234],0x12
    add     byte [bx],0x12

    or      byte [bx+si],0x12
    or      byte [bx+di],0x12
    or      byte [bp+si],0x12
    or      byte [bp+di],0x12
    or      byte [si],0x12
    or      byte [di],0x12
    or      byte [0x1234],0x12
    or      byte [bx],0x12

    adc     byte [bx+si],0x12
    adc     byte [bx+di],0x12
    adc     byte [bp+si],0x12
    adc     byte [bp+di],0x12
    adc     byte [si],0x12
    adc     byte [di],0x12
    adc     byte [0x1234],0x12
    adc     byte [bx],0x12

    sbb     byte [bx+si],0x12
    sbb     byte [bx+di],0x12
    sbb     byte [bp+si],0x12
    sbb     byte [bp+di],0x12
    sbb     byte [si],0x12
    sbb     byte [di],0x12
    sbb     byte [0x1234],0x12
    sbb     byte [bx],0x12

    and     byte [bx+si],0x12
    and     byte [bx+di],0x12
    and     byte [bp+si],0x12
    and     byte [bp+di],0x12
    and     byte [si],0x12
    and     byte [di],0x12
    and     byte [0x1234],0x12
    and     byte [bx],0x12

    sub     byte [bx+si],0x12
    sub     byte [bx+di],0x12
    sub     byte [bp+si],0x12
    sub     byte [bp+di],0x12
    sub     byte [si],0x12
    sub     byte [di],0x12
    sub     byte [0x1234],0x12
    sub     byte [bx],0x12

    xor     byte [bx+si],0x12
    xor     byte [bx+di],0x12
    xor     byte [bp+si],0x12
    xor     byte [bp+di],0x12
    xor     byte [si],0x12
    xor     byte [di],0x12
    xor     byte [0x1234],0x12
    xor     byte [bx],0x12

    cmp     byte [bx+si],0x12
    cmp     byte [bx+di],0x12
    cmp     byte [bp+si],0x12
    cmp     byte [bp+di],0x12
    cmp     byte [si],0x12
    cmp     byte [di],0x12
    cmp     byte [0x1234],0x12
    cmp     byte [bx],0x12

; GRP1 81h
    add     word [bx+si],0x1234
    add     word [bx+di],0x1234
    add     word [bp+si],0x1234
    add     word [bp+di],0x1234
    add     word [si],0x1234
    add     word [di],0x1234
    add     word [0x1234],0x1234
    add     word [bx],0x1234

    or      word [bx+si],0x1234
    or      word [bx+di],0x1234
    or      word [bp+si],0x1234
    or      word [bp+di],0x1234
    or      word [si],0x1234
    or      word [di],0x1234
    or      word [0x1234],0x1234
    or      word [bx],0x1234

    adc     word [bx+si],0x1234
    adc     word [bx+di],0x1234
    adc     word [bp+si],0x1234
    adc     word [bp+di],0x1234
    adc     word [si],0x1234
    adc     word [di],0x1234
    adc     word [0x1234],0x1234
    adc     word [bx],0x1234

    sbb     word [bx+si],0x1234
    sbb     word [bx+di],0x1234
    sbb     word [bp+si],0x1234
    sbb     word [bp+di],0x1234
    sbb     word [si],0x1234
    sbb     word [di],0x1234
    sbb     word [0x1234],0x1234
    sbb     word [bx],0x1234

    and     word [bx+si],0x1234
    and     word [bx+di],0x1234
    and     word [bp+si],0x1234
    and     word [bp+di],0x1234
    and     word [si],0x1234
    and     word [di],0x1234
    and     word [0x1234],0x1234
    and     word [bx],0x1234

    sub     word [bx+si],0x1234
    sub     word [bx+di],0x1234
    sub     word [bp+si],0x1234
    sub     word [bp+di],0x1234
    sub     word [si],0x1234
    sub     word [di],0x1234
    sub     word [0x1234],0x1234
    sub     word [bx],0x1234

    xor     word [bx+si],0x1234
    xor     word [bx+di],0x1234
    xor     word [bp+si],0x1234
    xor     word [bp+di],0x1234
    xor     word [si],0x1234
    xor     word [di],0x1234
    xor     word [0x1234],0x1234
    xor     word [bx],0x1234

    cmp     word [bx+si],0x1234
    cmp     word [bx+di],0x1234
    cmp     word [bp+si],0x1234
    cmp     word [bp+di],0x1234
    cmp     word [si],0x1234
    cmp     word [di],0x1234
    cmp     word [0x1234],0x1234
    cmp     word [bx],0x1234

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

    or      word [bx+si],0x5E
    or      word [bx+di],0x5E
    or      word [bp+si],0x5E
    or      word [bp+di],0x5E
    or      word [si],0x5E
    or      word [di],0x5E
    or      word [0x1234],0x5E
    or      word [bx],0x5E

    adc     word [bx+si],0x5E
    adc     word [bx+di],0x5E
    adc     word [bp+si],0x5E
    adc     word [bp+di],0x5E
    adc     word [si],0x5E
    adc     word [di],0x5E
    adc     word [0x1234],0x5E
    adc     word [bx],0x5E

    sbb     word [bx+si],0x5E
    sbb     word [bx+di],0x5E
    sbb     word [bp+si],0x5E
    sbb     word [bp+di],0x5E
    sbb     word [si],0x5E
    sbb     word [di],0x5E
    sbb     word [0x1234],0x5E
    sbb     word [bx],0x5E

    and     word [bx+si],0x5E
    and     word [bx+di],0x5E
    and     word [bp+si],0x5E
    and     word [bp+di],0x5E
    and     word [si],0x5E
    and     word [di],0x5E
    and     word [0x1234],0x5E
    and     word [bx],0x5E

    sub     word [bx+si],0x5E
    sub     word [bx+di],0x5E
    sub     word [bp+si],0x5E
    sub     word [bp+di],0x5E
    sub     word [si],0x5E
    sub     word [di],0x5E
    sub     word [0x1234],0x5E
    sub     word [bx],0x5E

    xor     word [bx+si],0x5E
    xor     word [bx+di],0x5E
    xor     word [bp+si],0x5E
    xor     word [bp+di],0x5E
    xor     word [si],0x5E
    xor     word [di],0x5E
    xor     word [0x1234],0x5E
    xor     word [bx],0x5E

    cmp     word [bx+si],0x5E
    cmp     word [bx+di],0x5E
    cmp     word [bp+si],0x5E
    cmp     word [bp+di],0x5E
    cmp     word [si],0x5E
    cmp     word [di],0x5E
    cmp     word [0x1234],0x5E
    cmp     word [bx],0x5E

; GRP1 83h
    add     word [bx+si],-0x56
    add     word [bx+di],-0x56
    add     word [bp+si],-0x56
    add     word [bp+di],-0x56
    add     word [si],-0x56
    add     word [di],-0x56
    add     word [0x1234],-0x56
    add     word [bx],-0x56

    or      word [bx+si],-0x56
    or      word [bx+di],-0x56
    or      word [bp+si],-0x56
    or      word [bp+di],-0x56
    or      word [si],-0x56
    or      word [di],-0x56
    or      word [0x1234],-0x56
    or      word [bx],-0x56

    adc     word [bx+si],-0x56
    adc     word [bx+di],-0x56
    adc     word [bp+si],-0x56
    adc     word [bp+di],-0x56
    adc     word [si],-0x56
    adc     word [di],-0x56
    adc     word [0x1234],-0x56
    adc     word [bx],-0x56

    sbb     word [bx+si],-0x56
    sbb     word [bx+di],-0x56
    sbb     word [bp+si],-0x56
    sbb     word [bp+di],-0x56
    sbb     word [si],-0x56
    sbb     word [di],-0x56
    sbb     word [0x1234],-0x56
    sbb     word [bx],-0x56

    and     word [bx+si],-0x56
    and     word [bx+di],-0x56
    and     word [bp+si],-0x56
    and     word [bp+di],-0x56
    and     word [si],-0x56
    and     word [di],-0x56
    and     word [0x1234],-0x56
    and     word [bx],-0x56

    sub     word [bx+si],-0x56
    sub     word [bx+di],-0x56
    sub     word [bp+si],-0x56
    sub     word [bp+di],-0x56
    sub     word [si],-0x56
    sub     word [di],-0x56
    sub     word [0x1234],-0x56
    sub     word [bx],-0x56

    xor     word [bx+si],-0x56
    xor     word [bx+di],-0x56
    xor     word [bp+si],-0x56
    xor     word [bp+di],-0x56
    xor     word [si],-0x56
    xor     word [di],-0x56
    xor     word [0x1234],-0x56
    xor     word [bx],-0x56

    cmp     word [bx+si],-0x56
    cmp     word [bx+di],-0x56
    cmp     word [bp+si],-0x56
    cmp     word [bp+di],-0x56
    cmp     word [si],-0x56
    cmp     word [di],-0x56
    cmp     word [0x1234],-0x56
    cmp     word [bx],-0x56

; TEST 84h-85h
    test    bl,cl           ; 84h
    test    bl,dl           ; 84h
    test    cl,dl
    test    dl,al

    test    bx,cx
    test    bx,dx
    test    cx,dx
    test    dx,ax

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

; XOR 86h-87h
    xchg    bl,cl           ; 86h
    xchg    bl,dl           ; 86h
    xchg    cl,dl
    xchg    dl,al

    xchg    bx,cx
    xchg    bx,dx
    xchg    cx,dx
    xchg    dx,ax

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

; MOV 88-8B
    mov     bl,cl
    mov     bl,dl
    mov     cl,dl
    mov     dl,al

    mov     bx,cx
    mov     bx,dx
    mov     cx,dx
    mov     dx,ax

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

; mov r/m,sreg
    mov     ax,cs
    mov     cs,ax
    mov     ax,ds
    mov     ds,ax
    mov     ax,es
    mov     es,ax
    mov     ax,ss
    mov     ss,ax

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

; pop 8Fh
    pop     word [si]
    pop     word [di]
    pop     word [bx+si]
    pop     word [bx+di]
    pop     word [0x1234]
    db      0x8F,0xC0       ; POP AX
    db      0x8F,0xC1       ; POP CX

; call Ap (0x9A)
    call    0x1234:0x5678
    call    0xABCD:0x1234

; MOV 0xA0-0xA3
    mov     al,[0x1234]
    mov     ax,[0x1234]
    mov     [0x1234],al
    mov     [0x1234],ax

; TEST A8-A9
    test    al,0x12
    test    ax,0x1234

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

; les
    les     ax,[si]
    les     ax,[bx+si]
    les     ax,[0x1234]
    les     ax,[bx+si+0x1234]
    les     bx,[bx+di-0x44]
    db      0xC4,0xC0       ; LES AX,AX (illegal, YASM won't encode it)
    db      0xC4,0xC1       ; LES AX,CX
    db      0xC4,0xC8       ; LES CX,AX

; lds
    lds     ax,[si]
    lds     ax,[bx+si]
    lds     ax,[0x1234]
    lds     ax,[bx+si+0x1234]
    lds     bx,[bx+di-0x44]
    db      0xC5,0xC0       ; LDS AX,AX (illegal, YASM won't encode it)
    db      0xC5,0xC1       ; LDS AX,CX
    db      0xC5,0xC8       ; LDS CX,AX

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

; into
    into

; loop/jcxz
looptest1:
    loopnz  looptest1
    loopz   looptest1
    loop    looptest1
    jcxz    looptest1

; CALL/JMP E8-EB
calltest1:
    call    near calltest1      ; E8
    jmp     near calltest1      ; E9
    jmp     0x1234:0x5678       ; EA
    jmp     short calltest1     ; EB

