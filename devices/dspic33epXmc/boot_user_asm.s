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
    ; on entry, address is contained within [W1:W0]
    push    TBLPAG
    
    clr	    TBLPAG
    mov	    W0, NVMADR
    mov	    W1, NVMADRU
    
    mov	    #0x4003, W0
    mov	    W0, NVMCON
    
    ; Write the KEY Sequence
    mov	    #0x55,W0
    mov	    W0, NVMKEY
    mov	    #0xAA,W0
    mov	    W0, NVMKEY
    
    ; Start the erase operation
    bset    NVMCON, #15
    
    ; Insert two NOPs after the erase cycle (required)
    nop
    nop
    
    pop	    TBLPAG
    
    return
    
load_write_latch_words:
    ; W2 points to the address of the data to write to the latches
    ; Set up a pointer to the first latch location to be written
    mov	    #0xfa, W0
    mov	    W0, TBLPAG
    mov	    #0, W1
    
    ; use TBLWT instructions to write the latches
    tblwtl  [W2++], [W1]
    tblwth  [W2++], [W1++]
    tblwtl  [W2++], [W1]
    tblwth  [W2++], [W1++]
    
    return
    
_doubleWordWrite:
    push    TBLPAG
    
    ; [W1:W0] contain the NVMADRx values, W2 contains the pointer to the data
    mov	    W0, NVMADR
    mov	    W1, NVMADRU
    
    call    load_write_latch_words
    
    mov	    #0x4001, W0
    mov	    W0, NVMCON
    
    ; write key sequence
    disi    #10
    
    mov	    #0x55, W0
    mov	    W0, NVMKEY
    mov	    #0xaa, W0
    mov	    W0, NVMKEY
    
    bset    NVMCON, #15
    nop
    nop
    
    pop	    TBLPAG
    
    return
    
_startApp:
    goto    W0

    .end
    
    