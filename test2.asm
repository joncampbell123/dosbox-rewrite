; 80286/80287 test instruction set

    cpu     80286 FPU Undoc

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

    db      0x0F    ; pop cs. make sure it DOESN'T decode to POP CS
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
    db      0xD0,0xF0           ; sal     bl,1
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
    db      0xD1,0xF0           ; sal     bx,1
    db      0xD1,0x30           ; sal     word [bx+si],1
    db      0xD1,0x31           ; sal     word [bx+di],1
    db      0xD1,0x36,0x34,0x12 ; sal     word [0x1234],1

    sar     ax,1
    sar     bx,1
    sar     word [bx+si],1
    sar     word [bx+di],1
    sar     word [0x1234],1

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
    db      0xD2,0xF0           ; sal     bl,cl
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
    db      0xD3,0xF0           ; sal     bx,cl
    db      0xD3,0x30           ; sal     word [bx+si],cl
    db      0xD3,0x31           ; sal     word [bx+di],cl
    db      0xD3,0x36,0x34,0x12 ; sal     word [0x1234],cl

    sar     ax,cl
    sar     bx,cl
    sar     word [bx+si],cl
    sar     word [bx+di],cl
    sar     word [0x1234],cl

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

    db      0xFF,0xC8       ; dec     ax
    db      0xFF,0xCB       ; dec     bx
    db      0xFF,0xC9       ; dec     cx
    db      0xFF,0xCA       ; dec     dx
    dec     word [bx+si]
    dec     word [bx+di]
    dec     word [0x1234]
    dec     word [bx]

    call    ax
    call    word [bx]
    call    word [bx+si]
    call    word [bx+di]
    call    word [0x1234]
    call    bx

    jmp     ax
    jmp     word [bx]
    jmp     word [bx+si]
    jmp     word [bx+di]
    jmp     word [0x1234]
    jmp     bx

    call far word [bx]
    call far word [bx+si]
    call far word [bx+di]
    call far word [0x1234]

    jmp far word [bx]
    jmp far word [bx+si]
    jmp far word [bx+di]
    jmp far word [0x1234]

    push    word [bx]
    push    word [bx+si]
    push    word [bx+di]
    push    word [0x1234]
    db      0xFF,0xF0       ; PUSH AX
    db      0xFF,0xF1       ; PUSH CX
    db      0xFF,0xF2       ; PUSH DX
    db      0xFF,0xF3       ; PUSH BX

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

; FLD     | ESCAPE 1 1 1 | MOD 1 0 1 R/M |     long integer memory to ST(0)
; assemblers refer to this by "FILD" and a 64-bit datatype
    fild    qword [bx]
    fild    qword [bx+si]
    fild    qword [bx+di+0x1234]
    fild    qword [si-6]

; FLD     | ESCAPE 0 1 1 | MOD 1 0 1 R/M |     temporary real memory to ST(0)
    fld     tword [bx]
    fld     tword [bx+si]
    fld     tword [bx+di+0x1234]
    fld     tword [si-6]

; FLD     | ESCAPE 1 1 1 | MOD 1 0 0 R/M |     packed BCD memory to ST(0)
; assemblers refer to this by "FBLD" and a 80-bit datatype
    fbld    tword [bx]
    fbld    tword [bx+si]
    fbld    tword [bx+di+0x1234]
    fbld    tword [si-6]

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

; FSTP    | ESCAPE 1 1 1 | MOD 1 1 1 R/M |     ST(0) to long integer memory
; assemblers refer to this by "FISTP" and a 64-bit datatype
    fistp   qword [bx]
    fistp   qword [bx+si]
    fistp   qword [bx+di+0x1234]
    fistp   qword [si-6]

; FSTP    | ESCAPE 0 1 1 | MOD 1 1 1 R/M |     ST(0) to temporary real memory
    fstp    tword [bx]
    fstp    tword [bx+si]
    fstp    tword [bx+di+0x1234]
    fstp    tword [si-6]

; FSTP    | ESCAPE 1 1 1 | MOD 1 1 0 R/M |     ST(0) to packed BCD memory
; assemblers refer to this by "FBSTP" and a 80-bit datatype
    fbstp   tword [bx]
    fbstp   tword [bx+si]
    fbstp   tword [bx+di+0x1234]
    fbstp   tword [si-6]

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

; FSTCW   | ESCAPE 0 0 1 | MOD 1 1 1 R/M
    fstcw   [bx]
    fstcw   [bx+si]
    fstcw   [bx+si+0x1234]
    fstcw   [0x1234]
    fstcw   [si-6]

; FSTSW   | ESCAPE 1 0 1 | MOD 1 1 1 R/M
    fstsw   [bx]
    fstsw   [bx+si]
    fstsw   [bx+si+0x1234]
    fstsw   [0x1234]
    fstsw   [si-6]
; NTS: The "FSTSW AX" instruction does not appear until Intel's 80287 datasheet,
;      therefore I am assuming that the 8087 does not have that instruction.

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
    popa            ; opcode 0x61
    push    word 0x1234 ; opcode 0x68
    push    byte 0x12 ; opcode 0x6A
    push    byte -0x44 ; opcode 0x6A
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
    outsb
    outsw
    rep     insb
    rep     insw
    rep     outsb
    rep     outsw
    enter   0,0
    enter   0x1234,1
    enter   0x1234,0x12
    leave
    leave
    leave

