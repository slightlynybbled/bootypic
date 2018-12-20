#include <stdint.h>
#include "xc.h"
void eraseByAddress(uint32_t address){
	uint16_t tempTblPag = TBLPAG;
	TBLPAG = (uint16_t)((address & 0x00ff0000) >> 16); // initialize PM Page Boundary
	
	NVMCONbits.NVMOP = 0b0010; // page erase
	NVMCONbits.ERASE = 1;
	NVMCONbits.WREN=1;

	NVMKEY = 0x55;
	NVMKEY = 0xAA;
	NVMCONbits.WR=1;
	while (NVMCONbits.WR){}
	if (NVMCONbits.WRERR){
		__builtin_nop();
		__builtin_software_breakpoint();
		__builtin_nop();
	}

	__builtin_write_NVM();
	TBLPAG = tempTblPag;
}

uint32_t readAddress(uint32_t address){
    uint16_t offset;
    uint16_t tempTblPag = TBLPAG;
	uint32_t result = 0;
    //Set up pointer to the first memory location to be written
    TBLPAG = (uint16_t)((address & 0x00ff0000) >> 16); // initialize PM Page Boundary
    offset = (uint16_t)(address & 0x0000ffff);  // initialize lower word of address
    
	result |= (uint32_t)__builtin_tblrdh(offset) << 16;// read from address high word
	result |= (uint32_t)__builtin_tblrdl(offset) << 0; // read from address low word

    TBLPAG = tempTblPag;
	return result;
}

void startApp(uint32_t applicationAddress){
	goto *(applicationAddress);
}