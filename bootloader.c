#include "xc.h"
#include "config.h" 
#include "bootloader.h"

/* bootloader starting address (cannot write to addresses between
 * BOOTLOADER_START_ADDRESS and APPLICATION_START_ADDRESS) */
#if _FLASH_PAGE == 128
#define BOOTLOADER_START_ADDRESS 0x200
#elif _FLASH_PAGE == 512
#define BOOTLOADER_START_ADDRESS 0x400
#elif _FLASH_PAGE == 1024
#define BOOTLOADER_START_ADDRESS 0x800
#endif

static uint8_t rxBuffer[RX_BUF_LEN];
static uint16_t rxBufferIndex = 0;

static uint8_t f16_sum1 = 0, f16_sum2 = 0;
static uint16_t t2Counter = 0;

int main(void){
    /* initialize the peripherals */
    initOsc();
    initPins();
    initUart();
    initTimers();
    
    /* wait until something is received on the serial port */
    while((t2Counter < NUM_OF_TMR2_OVERFLOWS) || (readBootPin() == false)){
        receiveBytes();
        processReceived();
        
        if(TMR2 > 50000){
            TMR2 = 0;
            t2Counter++;
        }
    }
    
    startApp(APPLICATION_START_ADDRESS);
    
    return 0;
}

void initPins(void){
#if BOOT_PORT == PORT_A
    ANSELA &= ~(1 << BOOT_PIN);
    TRISA |= (1 << BOOT_PIN);
#elif BOOT_PORT == PORT_B
    ANSELB &= ~(1 << BOOT_PIN);
    TRISB |= (1 << BOOT_PIN);
#elif BOOT_PORT == PORT_C
    ANSELC &= ~(1 << BOOT_PIN);
    TRISC |= (1 << BOOT_PIN);
#else
#error "boot port not specified"
#endif
}

void receiveBytes(void){
    while(U1STAbits.URXDA){
        rxBuffer[rxBufferIndex] = U1RXREG;
        rxBufferIndex++;
        
        TMR1 = TMR2 = 0;
        t2Counter = 0;
        T1CONbits.TON = 1;
    }
    
    /* if the time since the last received has expired, then
     * turn off the timer and reset the buffer */
    if(TMR1 > 50000){
        uint16_t i;
        T1CONbits.TON = 0;
        TMR1 = 0;
        
        rxBufferIndex = 0;
        for(i=0; i<RX_BUF_LEN; i++)   rxBuffer[i] = 0;
    }
}

void processReceived(void){
    /* look for start and end bytes in the sequence */
    bool startFound = false, endFound = false;
    uint16_t indexOfStart = 0, indexOfEnd = 0;
    uint16_t index = 0, messageIndex = 0, i;
    uint8_t message[RX_BUF_LEN];
    uint16_t fletcher;
    
    /* find the start of frame */
    while((index < RX_BUF_LEN) && (index < rxBufferIndex)){
        if(rxBuffer[index] == START_OF_FRAME){
            startFound = true;
            indexOfStart = index;
        }
        index++;
        
        if(startFound) break;
    }
    
    /* find the end of frame */
    while((index < RX_BUF_LEN) && (index < rxBufferIndex)){
        if(rxBuffer[index] == END_OF_FRAME){
            endFound = true;
            indexOfEnd = index;
        }
        index++;
        
        if(endFound) break;
    }
    
    if(startFound && endFound){
        bool escapeNext = false;
        index = indexOfStart;
        messageIndex = 0;
        
        /* read the message out, removing escape characters as necessary */
        for(index=indexOfStart+1; index<indexOfEnd; index++){
            if(!escapeNext){
                if(rxBuffer[index] == ESC){
                    escapeNext = true;
                }else{
                    message[messageIndex] = rxBuffer[index];
                    messageIndex++;
                }
            }else{
                message[messageIndex] = rxBuffer[index] ^ ESC_XOR;
                messageIndex++;
                escapeNext = false;
            }
        }
        
        /* read the fletcher number from the message */
        fletcher = (uint16_t)message[messageIndex - 2] 
                + ((uint16_t)message[messageIndex - 1] << 8);
        
        /* if they match, then process the message */
        if(fletcher == fletcher16(message, messageIndex - 2)){
            processCommand(message);
        }
        
        /* stop the timer, clear the buffer */
        T1CONbits.TON = 0;
        TMR1 = 0;
        rxBufferIndex = 0;
        for(i=0; i<RX_BUF_LEN; i++)   rxBuffer[i] = 0;
    }
}

void processCommand(uint8_t* data){
    uint16_t i;
    
    /* length is the length of the data block only, not including the command */
    uint8_t cmd = data[2];
    uint16_t address;
    uint16_t word;
    uint32_t longWord;
    uint32_t progData[MAX_PROG_SIZE + 1];
    
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
                progData[0] = 0x040000 + BOOTLOADER_START_ADDRESS;
                progData[1] = 0x000000;
                
#if defined(__dsPIC33EP32MC204__) | defined(__dsPIC33EP64MC504__)
                doubleWordWrite(address, progData);
#elif defined(__PIC24FV16KM202__)
                for(i=2; i<_FLASH_ROW; i++){
                    progData[i] = readAddress(i << 1);
                }
                
                writeInst32(address, progData);
#endif
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
                progData[i+1] = readAddress((address + (i << 1)));
            }
            
            txArray32bit(cmd, progData, MAX_PROG_SIZE + 1);
            
            break;
            
        case CMD_WRITE_ROW:
            address = (uint32_t)data[3] 
                    + ((uint32_t)data[4] << 8)
                    + ((uint32_t)data[5] << 16)
                    + ((uint32_t)data[6] << 24);
            
            for(i=0; i<_FLASH_ROW; i++){
                progData[i] = (uint32_t)data[i * 4 + 7] 
                    + ((uint32_t)data[i * 4 + 8] << 8)
                    + ((uint32_t)data[i * 4 + 9] << 16)
                    + ((uint32_t)data[i * 4 + 10] << 24);
            }
            
            /* do not allow the bootloader to be overwritten */
            if((word >= BOOTLOADER_START_ADDRESS) && (word < APPLICATION_START_ADDRESS))
                break;
            
            /* do not allow the reset vector to be changed by the application */
            if(word < __IVT_BASE)
                break;
            
#if defined(__dsPIC33EP32MC204__) | defined(__dsPIC33EP64MC504__)
            doubleWordWrite(address, progData);
#elif defined __PIC24FV16KM202__
            writeInst32(address, progData);
#endif
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
            
            /* program as a sequence of double-words */
#if defined(__dsPIC33EP32MC204__) | defined(__dsPIC33EP64MC504__)
            for(i = 0; i < (MAX_PROG_SIZE << 1); i += 4){
                uint32_t addr = address + i;
                
                /* do not allow application to overwrite the reset vector */
                if(addr >= __IVT_BASE)
                    doubleWordWrite(addr, &progData[i >> 1]);
            }
#else 
            word = (uint16_t)(MAX_PROG_SIZE/_FLASH_ROW);
            for(i = 0; i < word; i++){
                writeInst32(address + ((i * _FLASH_ROW) << 1), &progData[i*_FLASH_ROW]);
            }
#endif
            break;
            
        case CMD_START_APP:
            startApp(APPLICATION_START_ADDRESS);
            break;
            
        default:
        {}
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

/* todo: unify the array transmissions */
void txArray8bit(uint8_t cmd, uint8_t* bytes, uint16_t len){
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
    uint16_t i;
    uint16_t length = len << 1;
    
    txStart();
    
    txByte((uint8_t)(length & 0x00ff));
    txByte((uint8_t)((length & 0xff00) >> 8));
    
    txByte(cmd);
    
    for(i=0; i<len; i++){
        txByte((uint8_t)(words[i] & 0x00ff));
        txByte((uint8_t)((words[i] & 0xff00) >> 8));
    }
    
    txEnd();
}

void txArray32bit(uint8_t cmd, uint32_t* words, uint16_t len){
    uint16_t i;
    uint16_t length = len << 2;
    
    txStart();
    
    txByte((uint8_t)(length & 0x00ff));
    txByte((uint8_t)((length & 0xff00) >> 8));
    
    txByte(cmd);
    
    for(i=0; i<len; i++){
        uint32_t word = words[i];
        txByte((uint8_t)(word & 0x000000ff));
        txByte((uint8_t)((word & 0x0000ff00) >> 8));
        txByte((uint8_t)((word & 0x00ff0000) >> 16));
        txByte((uint8_t)((word & 0xff000000) >> 24));
    }
    
    txEnd();
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

void writeInst32(uint32_t address, uint32_t* progDataArray){
    uint16_t offset, i;
    uint16_t tempTblPag = TBLPAG;
    
    /* set up NVMCON for row programming */
    NVMCON = 0x4004; /* Initialize NVMCON to write 1 row */
    
    /* Set up pointer to the first memory location to be written */
    TBLPAG = (uint16_t)((address & 0x00ff0000) >> 16); /* initialize PM Page Boundary */
    offset = (uint16_t)(address & 0x0000ffff);  /* initialize lower word of address */
    
    /* perform TBLWT instructions to write necessary number of latches */
    for(i=0; i < 32; i++){
        __builtin_tblwtl(offset, (uint16_t)(progDataArray[i] & 0x0000ffff));            /* Write to address low word */
        __builtin_tblwth(offset, (uint16_t)((progDataArray[i] & 0xffff0000) >> 16));    /* Write to upper byte */
        offset += 2; /* Increment add */
    }
    
    __builtin_write_NVM();
    
    TBLPAG = tempTblPag;
}
