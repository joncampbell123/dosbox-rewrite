
    bits    16
    org     100h

    nop
    cbw
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

