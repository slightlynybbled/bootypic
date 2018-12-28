#include <stdint.h>
#include "boot_config.h"
#include "xc.h"

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

void writeWord(uint32_t address, uint32_t word){
	uint16_t tempTblPag = TBLPAG; 

	uint16_t offset = (uint16_t)(address & 0x0000ffff);
	TBLPAG = (uint16_t)((address & 0xffff0000) >> 16); /* initialize PM Page Boundary */

	NVMCON = 0x4003; // Memory word program operation
	
	__builtin_tblwtl(offset, (uint16_t)((word & 0x0000ffff) >> 0));
	__builtin_tblwth(offset, (uint16_t)((word & 0x00ff0000) >> 16));
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
	asm volatile ("goto w0");
	//goto *(applicationAddress);
}