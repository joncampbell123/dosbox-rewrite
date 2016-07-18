
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

