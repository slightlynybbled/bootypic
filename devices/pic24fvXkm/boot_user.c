#include "xc.h"
#include "boot_user.h"

void initOsc(void){
    CLKDIV = 0;
    
    /* Disable nested interrupts */
    INTCON1bits.NSTDIS = 1;

    return;
}

void initUart(void){
    U1MODE = 0;
    U1STA = 0x2000;

    /* Calculate the baud rate generator contents   */
    /*           instFreq                           */
    /*  BRG = --------------- - 1                   */
    /*        (16 * baudRate)                       */
    U1BRG = 12;     // assumes 12MIPS, 57600baud
    
    /* make the RX pin an input */
    #if defined RX_PORT_A
        TRISA |= (1 << RX_PIN);
        ANSELA &= ~(1 << RX_PIN);
    #elif defined RX_PORT_B
        TRISB |= (1 << RX_PIN);
        ANSELB &= ~(1 << RX_PIN);
    #elif defined RX_PORT_C
        TRISC |= (1 << RX_PIN);
        ANSELC &= ~(1 << RX_PIN);
    #else 
    #error "RX_PORT_X not specified"
    #endif
    
    /* make the TX pin an output */
    #if defined TX_PORT_A
        TRISA &= ~(1 << TX_PIN);
        ANSELA &= ~(1 << TX_PIN);
    #elif defined TX_PORT_B 
        TRISB &= ~(1 << TX_PIN);
        ANSELB &= ~(1 << TX_PIN);
    #elif defined TX_PORT_C 
        TRISC &= ~(1 << TX_PIN);
        ANSELC &= ~(1 << TX_PIN);
    #else 
        #error "TX_PORT_X not specified"
    #endif
    
    U1MODEbits.UARTEN = 1;  /* enable UART */
    U1STAbits.UTXEN = 1;    /* transmit enabled */
    
    while(U1STAbits.URXDA) U1RXREG; /* clear anything in the buffer */
}

void initTimers(void){
    /* initialize timer1 registers - timer 1 is used for determining if the 
     * rx buffer should be flushed b/c of local stale data (mis-transfers, 
     * etc) */
    T1CON = 0x0030; /* prescaler = 256 (4.27us/tick) */
    
    /* initialize timer2 registers - timer 2 is used for determining if the
     * the bootloader has been engaged recently */
    TMR2 = 0;

    /* on some devices, use the CCP1 module as a timer */
    CCP1CON1H = 0x0000;
    CCP1CON1L = 0x00c0; /* prescaler = 64 (4us/tick) */
    CCP1CON1Lbits.CCPON = 1;
}

bool readBootPin(void){
#if defined(BOOT_PORT_A)
    if(PORTA & (1 << BOOT_PIN))
        return true;
    else
        return false;
    
#elif defined(BOOT_PORT_B)
    if(PORTB & (1 << BOOT_PIN))
        return true;
    else
        return false;
    
#elif defined(BOOT_PORT_C)
    if(PORTC & (1 << BOOT_PIN))
        return true;
    else
        return false;
    
#else
#error "boot port not specified"
#endif
}
