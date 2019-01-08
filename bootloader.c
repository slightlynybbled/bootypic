#include "xc.h"
#include "config.h" 
#include "bootloader.h"


#if MAX_PROG_SIZE % _FLASH_ROW != 0
#error "MAX_PROG_SIZE must be a multiple of _FLASH_ROW"
#endif

/* bootloader starting address (cannot write to addresses between
 * BOOTLOADER_START_ADDRESS and APPLICATION_START_ADDRESS) */
#if _FLASH_PAGE == 128
#define BOOTLOADER_START_ADDRESS 0x200
#elif _FLASH_PAGE == 512
#define BOOTLOADER_START_ADDRESS 0x400
#elif _FLASH_PAGE == 1024
#define BOOTLOADER_START_ADDRESS 0x800
#endif

static uint8_t message[RX_BUF_LEN] = {0};
static uint8_t f16_sum1 = 0, f16_sum2 = 0;

int main(void) {
	/* handle power outage */
	onStartup();
	/* initialize the peripherals from the user-supplied initialization functions */
	initPins();
    initOsc();
    initUart();
    initTimers();
	
    /* wait until something is received on the serial port */
    while(!should_abort_boot()){
		ClrWdt();
        
        receiveBytes();
    }

    startApp(APPLICATION_START_ADDRESS);
    
    return 0;
}

typedef enum ParseState {
	STATE_WAIT_FOR_START, ///< Have not yet received a start byte.
	STATE_READ_ESCAPED,   ///< Have received an escape byte, next data byte must be decoded
	STATE_READ_VERBATIM,  ///< Have not received an escape byte, next data byte should be read as-is
	STATE_END_OF_MESSAGE, 
} ParseState;

void receiveBytes(void){
	static uint16_t messageIndex = 0;
	static ParseState state = STATE_WAIT_FOR_START;
	// keep reading until one of the following:
	// 1. we don't have any data available in the uart buffer to read in which case we return
	//    and wait for this function to be called again
	// 2. we find an END OF MESSAGE, in which case we call processCommand to deal with the data
	// 3. we overflow the receive buffer, in which case we throw out the data
    while(true)
	{
		uint8_t a_byte;

		if (!tryRxByte(&a_byte))
			return;
			
		switch (state){
		case STATE_WAIT_FOR_START:
			if (a_byte == START_OF_FRAME){
				state = STATE_READ_VERBATIM;
			}
			// otherwise, ignore data until we see a start byte
			break;
		case STATE_READ_VERBATIM:
			if (a_byte == ESC){
				state = STATE_READ_ESCAPED;
			} else if (a_byte == END_OF_FRAME) {
				state = STATE_END_OF_MESSAGE;
			} else {
				message[messageIndex++] = a_byte;
			}
			break;
		case STATE_READ_ESCAPED:
			message[messageIndex++] = a_byte ^ ESC_XOR;
			state = STATE_READ_VERBATIM;
			break;
		default:
			break;
		}
		
		if (state == STATE_END_OF_MESSAGE) {
			uint16_t fletcher = (uint16_t)message[messageIndex - 2] 
	                + ((uint16_t)message[messageIndex - 1] << 8);
	        if(fletcher == fletcher16(message, messageIndex - 2)){
	            processCommand(message);
	            // we got a valid message! reset the stall timer
				TMR3HLD = 0;
				TMR2 = 0;
	        }
		}
	    if (messageIndex >= RX_BUF_LEN || state == STATE_END_OF_MESSAGE) {
			uint16_t i;
		    for(i=0; i<RX_BUF_LEN; i++) message[i] = 0;
		    messageIndex = 0;
		    state = STATE_WAIT_FOR_START;
		    return;
		}
	}
}

void processCommand(uint8_t* data){
    uint16_t i;
    
    /* length is the length of the data block only, not including the command */
    uint8_t cmd = data[2];
    uint32_t address;
    uint16_t word;
    uint32_t longWord;
    uint32_t progData[MAX_PROG_SIZE + 1] = {0};
    
    char strVersion[16] = VERSION_STRING;
    char strPlatform[20] = PLATFORM_STRING;

    switch(cmd){
        case CMD_READ_PLATFORM:
            txString(cmd, strPlatform);
            break;
        
        case CMD_READ_VERSION:
            txString(cmd, strVersion);
            break;
            
        case CMD_READ_ROW_LEN:
            word = _FLASH_ROW;
            txArray16bit(cmd, &word, 1);
            break;
            
        case CMD_READ_PAGE_LEN:
            word = _FLASH_PAGE;
            txArray16bit(cmd, &word, 1);
            break;
            
        case CMD_READ_PROG_LEN:
            longWord = __PROGRAM_LENGTH;
            txArray32bit(cmd, &longWord, 1);
            break;
            
        case CMD_READ_MAX_PROG_SIZE:
            word = MAX_PROG_SIZE;
            txArray16bit(cmd, &word, 1);
            break;
            
        case CMD_READ_APP_START_ADDR:
            word = APPLICATION_START_ADDRESS;
            txArray16bit(cmd, &word, 1);
            break;
            
        case CMD_READ_BOOT_START_ADDR:
            word = BOOTLOADER_START_ADDRESS;
            txArray16bit(cmd, &word, 1);
            break;
            
        case CMD_ERASE_PAGE:
            /* should correspond to a border */
            address = (uint32_t)data[3] 
                    + ((uint32_t)data[4] << 8)
                    + ((uint32_t)data[5] << 16)
                    + ((uint32_t)data[6] << 24);
            
            /* do not allow the bootloader to be erased */
            if((address >= BOOTLOADER_START_ADDRESS) && (address < APPLICATION_START_ADDRESS))
                break;
            
			eraseByAddress(address);
            
            /* re-initialize the bootloader start address */
            if(address == 0){
                address = 0x00000000;
                
                /* this is the GOTO BOOTLOADER instruction */
                progData[0] = 0x040000 + BOOTLOADER_START_ADDRESS;
                progData[1] = 0x000000;
                
                /* write the data */
                doubleWordWrite(address, progData);
            }
            
            break;
            
        case CMD_READ_ADDR:
            address = (uint32_t)data[3] 
                    + ((uint32_t)data[4] << 8)
                    + ((uint32_t)data[5] << 16)
                    + ((uint32_t)data[6] << 24);
            progData[0] = address;
            progData[1] = readAddress(address);
            
            txArray32bit(cmd, progData, 2);
            break;
            
        case CMD_READ_MAX:
            address = (uint32_t)data[3] 
                    + ((uint32_t)data[4] << 8)
                    + ((uint32_t)data[5] << 16)
                    + ((uint32_t)data[6] << 24);
            
            progData[0] = address;
            
            for(i=0; i<MAX_PROG_SIZE; i++){
                progData[i+1] = readAddress(address + 2*i);
            }
            
            txArray32bit(cmd, progData, MAX_PROG_SIZE + 1);
            
            break;
            
        case CMD_WRITE_ROW:
            address = ((uint32_t)data[3] << 0) 
                    + ((uint32_t)data[4] << 8)
                    + ((uint32_t)data[5] << 16)
                    + ((uint32_t)data[6] << 24);
            
            for(i=0; i<_FLASH_ROW; i++){
                progData[i] = ((uint32_t)data[i * 4 + 7] << 0) 
                    + ((uint32_t)data[i * 4 + 8] << 8)
                    + ((uint32_t)data[i * 4 + 9] << 16)
                    + ((uint32_t)data[i * 4 + 10] << 24);
            }
            
            /* do not allow the bootloader to be overwritten */
            if((address >= BOOTLOADER_START_ADDRESS) && (address < APPLICATION_START_ADDRESS))
                break;
            
            /* do not allow the reset vector to be changed by the application */
            if(address < __IVT_BASE)
                break;

			writeRow(address, progData);
            break;
            
        case CMD_WRITE_MAX_PROG_SIZE:
            address = (uint32_t)data[3] 
                    + ((uint32_t)data[4] << 8)
                    + ((uint32_t)data[5] << 16)
                    + ((uint32_t)data[6] << 24);
           		 
            /* fill the progData array */
            for(i=0; i<MAX_PROG_SIZE; i++){
            	progData[i] = (uint32_t)data[7 + (i * 4)]
                        + ((uint32_t)data[8 + (i * 4)] << 8)
                        + ((uint32_t)data[9 + (i * 4)] << 16)
                        + ((uint32_t)data[10 + (i * 4)] << 24);
                 /* the zero address should always go to the bootloader */
	             if(address == 0){
	                   if(i == 0){
	                       progData[i] = 0x040000 | BOOTLOADER_START_ADDRESS;
	                   }else if(i == 1){
	                       progData[i] = 0x000000;
	                   }
	             }
			}

			/* write to flash memory, one row at a time */
			for (i=0; i*_FLASH_ROW<MAX_PROG_SIZE; i++){
				 writeRow(address + i * _FLASH_ROW * 2, progData + i*_FLASH_ROW);
			}
            break;
            
        case CMD_START_APP:
            startApp(APPLICATION_START_ADDRESS);
            break;
            
        default:
        	break;
    }
}

void txStart(void){
    f16_sum1 = f16_sum2 = 0;
    
    while(U1STAbits.UTXBF); /* wait for tx buffer to empty */
    U1TXREG = START_OF_FRAME;
}

void txByte(uint8_t byte){
    if((byte == START_OF_FRAME) || (byte == END_OF_FRAME) || (byte == ESC)){
        while(U1STAbits.UTXBF); /* wait for tx buffer to empty */
        U1TXREG = ESC;          /* send escape character */
        
        while(U1STAbits.UTXBF); /* wait */
        U1TXREG = ESC_XOR ^ byte;
    }else{
        while(U1STAbits.UTXBF); /* wait */
        U1TXREG = byte;
    }
    
    fletcher16Accum(byte);
}

void txEnd(void){
    /* append checksum */
    uint8_t sum1 = f16_sum1;
    uint8_t sum2 = f16_sum2;
    
    txByte(sum1);
    txByte(sum2);
    
    while(U1STAbits.UTXBF); /* wait for tx buffer to empty */
    U1TXREG = END_OF_FRAME;
}

void txBytes(uint8_t cmd, uint8_t* bytes, uint16_t len){
    uint16_t i;
    
    txStart();
    txByte((uint8_t)(len & 0x00ff));
    txByte((uint8_t)((len & 0xff00) >> 8));
    txByte(cmd);
    
    for(i=0; i<len; i++){
        txByte(bytes[i]);
    }
    
    txEnd();
}

void txArray16bit(uint8_t cmd, uint16_t* words, uint16_t len){
    uint16_t length = len << 1;
    txBytes(cmd, (uint8_t*) words, length);
}

void txArray32bit(uint8_t cmd, uint32_t* words, uint16_t len){
    uint16_t length = len << 2;
    txBytes(cmd, (uint8_t*) words, length);
}

void txString(uint8_t cmd, char* str){
    uint16_t i, length = 0;
    
    /* find the length of the version string */
    while(str[length] != 0)  length++;
    length++;       /* be sure to get the string terminator */
    
    txStart();

    /* begin transmitting */
    txByte((uint8_t)(length & 0xff));
    txByte((uint8_t)((length & 0xff00) >> 8));
    
    txByte(cmd);

    for(i=0; i<length; i++){
        txByte((uint8_t)str[i]);
    }

    txEnd();
}

uint16_t fletcher16Accum(uint8_t byte){
    f16_sum1 = (f16_sum1 + (uint16_t)byte) & 0xff;
    f16_sum2 = (f16_sum2 + f16_sum1) & 0xff;
    return (f16_sum2 << 8) | f16_sum1;
}

uint16_t fletcher16(uint8_t* data, uint16_t length){
	uint16_t sum1 = 0, sum2 = 0, checksum;
    
    uint16_t i = 0;
    while(i < length){
        sum1 = (sum1 + (uint16_t)data[i]) & 0xff;
        sum2 = (sum2 + sum1) & 0xff;
        i++;
    }
    
    checksum = (sum2 << 8) | sum1;
    
	return checksum;
}
