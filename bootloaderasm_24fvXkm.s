; This file includes assembly functions for reading from, 
; erasing, and writing to flash memory.

    .include "xc.inc"

    .text
    .global _readAddress
    .global _eraseByAddress
    .global _startApp
    
_readAddress:
    ; on entry, address is contained within [W1:W0]
    
    push    TBLPAG
    push    W4
    
    mov	    W1, W4
    mov	    W4, TBLPAG
    
    ; find the offset, store in W2
    mov	    W0, W2
    
    tblrdl  [W2], W0
    tblrdh  [W2], W1    
    
    pop	    W4
    pop	    TBLPAG
    
    return
    
_eraseByAddress:
    push    TBLPAG
    
    mov	    W1, TBLPAG	; PM page boundary
    tblwtl  W0, [W0]
    
    mov	    #0x405a, W0	    ; erase 4 rows programming memory
    mov	    W0, NVMCON
    
    mov	    #0x55, W0
    mov	    W0, NVMKEY
    mov	    #0xAA, W1
    mov	    W1, NVMKEY
    bset    NVMCON, #WR
    
    nop
    nop
    
    pop	    TBLPAG
    
    return
    
_startApp:
    call    W0

    return

    .end
    
    