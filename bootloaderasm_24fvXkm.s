; This file includes assembly functions for reading from, 
; erasing, and writing to flash memory.

    .include "xc.inc"

    .text
    .global _readAddress
    .global _eraseByAddress
    .global _doubleWordWrite
    .global _startApp
    
_readAddress:
    ; on entry, address is contained within [W1:W0]
    
    push    TBLPAG
    push    W4
    
    mov	    #0, W4
    mov	    W4, TBLPAG
    
    ; find the offset, store in W2
    mov	    W0, W2
    
    tblrdl  [W2], W0
    tblrdh  [W2], W1    
    
    pop	    W4
    pop	    TBLPAG
    
    return
    
_eraseByAddress:
    
    
    return
    
load_write_latch_words:
    
    
    return
    
_doubleWordWrite:
    
    
    return
    
_startApp:
    call    W0

    return

    .end
    
    