/// Device-specific implementation details
#include "xc.h"
#include "boot_user.h"

bool readBootPin(void);

void initOsc(void){
    CLKDIV = 0;

    return;
}

void initPins(void){
    /* no analog, all digital */
    AD1PCFGL = 0xffff;
    AD1PCFGH = 0x3;
    
#if defined(BOOT_PORT_B)
    TRISB |= (1 << BOOT_PIN);
#elif defined(BOOT_PORT_C)
    TRISC |= (1 << BOOT_PIN);
#elif defined(BOOT_PORT_D)
    TRISD |= (1 << BOOT_PIN);
#elif defined(BOOT_PORT_E)
    TRISE |= (1 << BOOT_PIN);
#elif defined(BOOT_PORT_F)
    TRISF |= (1 << BOOT_PIN);
#elif defined(BOOT_PORT_G)
    TRISG |= (1 << BOOT_PIN);
#else
#error "boot port not specified or invalid"
#endif
}

#define UART_MAP_RX(rpn) uart_map_rx(rpn)
void uart_map_rx(uint16_t rpn) {
	// map that pin to UART RX
	_U1RXR = rpn;
}

#define UART_MAP_TX(rpn) uart_map_tx(rpn)
void uart_map_tx(uint16_t rpn) {
	#define _RPxR(x) _RP ## x ## R
	#define CASE(x) case x: _RPxR(x)=_RPOUT_U1TX; break;
	switch (rpn){
		CASE(0)
		CASE(1)
		CASE(2)
		CASE(3)
		CASE(4)
		// no RP5R on this chip
		CASE(6)
		CASE(7)
		CASE(8)
		CASE(9)
		CASE(10)
		CASE(11)
		CASE(12)
		CASE(13)
		CASE(14)
		// no RP15R on this chip
		CASE(16)
		CASE(17)
		CASE(18)
		CASE(19)
		CASE(20)
		CASE(21)
		CASE(22)
		CASE(23)
		CASE(24)
		CASE(25)
		CASE(26)
		CASE(27)
		CASE(28)
		CASE(29)
		CASE(30)
	}
	#undef CASE
	#undef _RPxR
}

void initUart(void){
    U1MODE = 0;
    U1STA = 0x2000;

	if (UART_BAUD_RATE < FCY/4.0f){
		U1MODEbits.BRGH = 0; // High Baud Rate Select bit = off
		U1BRG = FCY / (16.0f*UART_BAUD_RATE) - 1;
	} 	else {
		U1MODEbits.BRGH = 1;
		U1BRG = FCY / (4.0f*UART_BAUD_RATE) - 1;
	}   
	
/* make the RX pin an input */
#if defined(UART_MAP_RX)
	UART_MAP_RX(RX_PIN);
#elif defined RX_PORT_A
    TRISA |= (1 << RX_PIN);
    ANSELA &= ~(1 << RX_PIN);
#elif defined RX_PORT_B
    TRISB |= (1 << RX_PIN);
    ANSELB &= ~(1 << RX_PIN);
#elif defined RX_PORT_C
    TRISC |= (1 << RX_PIN);
    ANSELC &= ~(1 << RX_PIN);
#endif
   
/* make the TX pin an output */
#if defined UART_MAP_TX
	UART_MAP_TX(TX_PIN);
#elif defined TX_PORT_A
    TRISA &= ~(1 << TX_PIN);
    ANSELA &= ~(1 << TX_PIN);
#elif defined TX_PORT_B 
    TRISB &= ~(1 << TX_PIN);
    ANSELB &= ~(1 << TX_PIN);
#elif defined TX_PORT_C 
    TRISC &= ~(1 << TX_PIN);
    ANSELC &= ~(1 << TX_PIN);
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

bool should_abort_boot(uint16_t counterValue) {
	if(counterValue > NUM_OF_TMR2_OVERFLOWS){
		return true;
	}

    #if defined(BOOT_PORT_A)
        if(PORTA & (1 << BOOT_PIN))
            return true;
    #elif defined(BOOT_PORT_B)
        if(PORTB & (1 << BOOT_PIN))
            return true;
    #elif defined(BOOT_PORT_C)
        if(PORTC & (1 << BOOT_PIN))
            return true;
    #else
    #error "boot port not specified"
    #endif

   return false;
}

/// Device-specific implementations of bootloader operations
void eraseByAddress(uint32_t address){
	uint16_t offset;
	uint16_t tempTblPag = TBLPAG;
	TBLPAG = (uint16_t)((address & 0x00ff0000) >> 16); // initialize PM Page Boundary
	offset = (uint16_t)((address & 0x0000ffff) >> 0);
	NVMCON = 0x4042; // page erase operation
	__builtin_tblwtl(offset, 0);
	__builtin_disi(5);
	__builtin_write_NVM();

	TBLPAG = tempTblPag;
}

uint32_t readAddress(uint32_t address){
    uint16_t offset;
    uint16_t tempTblPag = TBLPAG;
	uint32_t result = 0;
    //Set up pointer to the first memory location to be written
    TBLPAG = (uint16_t)((address & 0x00ff0000) >> 16); // initialize PM Page Boundary
    offset = (uint16_t)((address & 0x0000ffff) >> 0);  // initialize lower word of address
    
	result |= (((uint32_t)__builtin_tblrdh(offset)) << 16);// read from address high word
	result |= (((uint32_t)__builtin_tblrdl(offset)) << 0); // read from address low word

    TBLPAG = tempTblPag;
	return result;
}

void writeInstr(uint32_t address, uint32_t instruction){
	uint16_t tempTblPag = TBLPAG; 

	uint16_t offset = (uint16_t)(address & 0x0000ffff);
	TBLPAG = (uint16_t)((address & 0xffff0000) >> 16); /* initialize PM Page Boundary */

	NVMCON = 0x4003; // Memory word program operation
	
	__builtin_tblwtl(offset, (uint16_t)((instruction & 0x0000ffff) >> 0));
	__builtin_tblwth(offset, (uint16_t)((instruction & 0x00ff0000) >> 16));
	__builtin_disi(5);
	__builtin_write_NVM();

	TBLPAG = tempTblPag;
}

void writeRow(uint32_t address, uint32_t* words){

	// see "Row Programming in C with Built-in Functions (Unmapped Latches)"
	uint16_t i;
	uint16_t tempTblPag = TBLPAG; 
	uint16_t offset = (uint16_t)(address & 0x0000ff80);
	TBLPAG = (uint16_t)((address & 0x00ff0000) >> 16); /* initialize PM Page Boundary */

	NVMCON = 0x4001; // Memory row program operation
	for (i=0; i<_FLASH_ROW; i++){
		__builtin_tblwtl(offset + i*2, (uint16_t)((words[i] & 0x0000ffff) >> 0));
		__builtin_tblwth(offset + i*2, (uint16_t)((words[i] & 0x00ff0000) >> 16));
	}
	__builtin_disi(5);
	__builtin_write_NVM();

	TBLPAG = tempTblPag;
}

void doubleWordWrite(uint32_t address, uint32_t* progDataArray){
	writeInstr(address, progDataArray[0]);
	writeInstr(address+2, progDataArray[1]);
}

void writeMax(uint32_t address, uint32_t* progData){
	uint16_t i;
	uint16_t length = (uint16_t)(MAX_PROG_SIZE/_FLASH_ROW);
	
	for (i=0; i < length; i++){
		writeRow(address + ((i * _FLASH_ROW) << 1), &progData[i*_FLASH_ROW]); 
	}
}

void startApp(uint16_t applicationAddress){
	asm("goto w0");
}