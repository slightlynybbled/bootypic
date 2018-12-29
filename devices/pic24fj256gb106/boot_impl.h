/// Device-specific implementation details

#include <stdint.h>
#include "xc.h"

#define PLATFORM_STRING "pic24fj256gb106"

// instruction clock frequency, in Hz.
#define FCY 16000000UL
/* _FLASH_PAGE should be the maximum page (in instructions) */
#define _FLASH_PAGE   512
/* _FLASH_ROW = maximum write row (in instructions) */
#define _FLASH_ROW    64

#define UART_MAP_RX(rpn) uart_map_rx(rpn)
void uart_map_rx(uint16_t rpn) {
	// mark the pin as digital
	if (rpn < 16) {
		AD1PCFGL |= (1<<rpn);
	} else if (rpn < 18) {
		AD1PCFGH |= (1<<rpn);
	}
	// map that pin to UART RX
	_U1RXR = rpn;
}

#define UART_MAP_TX(rpn) uart_map_tx(rpn)
void uart_map_tx(uint16_t rpn) {
	// mark the pin as digital
	if (rpn < 16) {
		AD1PCFGL |= (1<<rpn);
	} else if (rpn < 18) {
		AD1PCFGH |= (1<<rpn);
	}
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

void writeRow(uint32_t address, uint32_t words[_FLASH_ROW]){

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

void startApp(uint16_t applicationAddress){
	asm("goto w0");
}