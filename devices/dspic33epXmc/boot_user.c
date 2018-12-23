#include "xc.h"
#include "boot_user.h"

#define U1TX_RPOR_NUM 0b000001

void initOsc(void){
    /*
    Input Frequency: 7.370000e+00
    Output Frequency: 120
    Error in MHz: 3.277778e-02
    N1: 9
    M: 293
    N2: 2
    */
    CLKDIVbits.PLLPRE = 7;
    PLLFBDbits.PLLDIV = 291;
    CLKDIVbits.PLLPOST = 0x0;

    /* Clock switch to incorporate PLL */
    OSCCONbits.NOSC = 1; /* Request new oscillator to be PRI with PLL */
    OSCCONbits.OSWEN = 1; /* Initiate switch */
    while (OSCCONbits.COSC != 1); /* Wait for Clock switch to occur */

    /* Wait for PLL to lock */
    while (OSCCONbits.LOCK != 1);
    
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
    U1BRG = 31;
    RPINR18bits.U1RXR = RX_RPNUM; /* U1RX assigned to RP25 */
    
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
    
    /* assign the UART1TX peripheral to a remappable output */
    #if TX_RPNUM == 20
        RPOR0bits.RP20R = U1TX_RPOR_NUM;
    #elif TX_RPNUM == 41
        RPOR3bits.RP41R = U1TX_RPOR_NUM;
    #elif TX_RPNUM == 54
        RPOR5bits.RP54R = U1TX_RPOR_NUM;
    #elif TX_RPNUM == 55
        RPOR5bits.RP55R = U1TX_RPOR_NUM;
    #else 
    #error "TX_RPNUM not specified"
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

    T2CON = 0x8030; /* prescaler = 256 */
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
