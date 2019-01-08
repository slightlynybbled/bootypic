/// Device-specific implementation details
#include "xc.h"
#include "boot_user.h"

void initOsc(void){
    CLKDIV = 0;
    return;
}

#ifdef BOOT_PORT_NONE
bool SHOULD_SKIP_BOOTLOADER __attribute__((persistent, address(0x400)));
#endif

void onStartup(void){
	#if defined(BOOT_PORT_NONE)
	if (RCONbits.POR // reset due to power outage
		|| RCONbits.IOPUWR // reset due to bad opcode
		){
		// If we didn't reset cleanly, this persistent variable will contain garbage data
		// or the firmware may be corrupt. Should run the bootloader.
		SHOULD_SKIP_BOOTLOADER = false;	
	}
	#endif
	RCON = 0;
}	

void initPins(void){
    /* no analog, all digital */
    AD1PCFGL = 0xffff;
    AD1PCFGH = 0x3;

#if defined(BOOT_PORT_NONE)
#elif defined(BOOT_PORT_B)
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

void uart_map_rx(uint16_t rpn) {
	// map that pin to UART RX
	_U1RXR = rpn;
}

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
    uart_map_rx(RX_PIN);
    uart_map_tx(TX_PIN);

	if (UART_BAUD_RATE < FCY/4.0f){
		U1MODEbits.BRGH = 0; // High Baud Rate Select bit = off
		U1BRG = FCY / 16 / UART_BAUD_RATE - 1;
	} 	else {
		U1MODEbits.BRGH = 1;
		U1BRG = FCY / 4 / UART_BAUD_RATE - 1;
	}

	/* note UART module overrides the PORT, LAT, and TRIS bits, so no
     * need to set them */
    
    U1MODEbits.UARTEN = 1;  /* enable UART */
    U1STAbits.UTXEN = 1;    /* transmit enabled */
    
    while(U1STAbits.URXDA) U1RXREG; /* clear anything in the buffer */
}

void initTimers(void){
	T2CON = T3CON = 0;
	TMR2 = TMR3 = 0;
	PR2 = PR3 = 0xffff;
	
	T2CONbits.T32 = 1; // merge timer 2 and timer 3 into a 32 bit timer
	T2CONbits.TCKPS = 0b11; // 256 prescale
	T2CONbits.TON = 1;
}

uint32_t getTimeTicks(){
	uint32_t n_ticks = 0;
	n_ticks |= (uint32_t)TMR2;
	n_ticks |= ((uint32_t)TMR3HLD)<<16;
	return n_ticks;
}

bool should_abort_boot() {
	static const uint32_t BOOTLOADER_TIMEOUT_TICKS = (FCY / 256.0 * BOOT_LOADER_TIME);
	if(getTimeTicks() > BOOTLOADER_TIMEOUT_TICKS){
       return true;
    }

	#if defined(BOOT_PORT_NONE)
		if (SHOULD_SKIP_BOOTLOADER)
			return true;
    #elif defined(BOOT_PORT_A)
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

bool tryRxByte(uint8_t *outbyte){
	if (U1STAbits.URXDA){
		*outbyte = U1RXREG;
		return true;
	}
	else{
		return false;
	}
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