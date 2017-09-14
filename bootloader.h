#ifndef _BOOTLOADER_H
#define _BOOTLOADER_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief these defines will determine the boot pin to be utilized
 * 
 * When the boot pin is pulled low, then bootloader will start, otherwise
 * the application will start on reset.
 */
#define BOOT_PORT_A
#define BOOT_PIN 0

/**
 * @brief choose the RX and TX pins by RP function and port/pin
 * 
 * Each pin should have three defines associated with it:
 *   * port
 *   * pin number
 *   * RP assignment
 * 
 * Assign the port by defining RX_PORT_A, RX_PORT_B, etc.
 * Assign the pin by defining RX_PIN 1, RX_PIN 2, etc.
 * Assign the remappable input by define RX_RPNUM to an RP number
 * 
 * Same pattern for TX pins.
 * 
 * If the pin has not been utilized before, it may be necessary to add this 
 * bit to the source code as well, but in those places, it should be clear
 * what the user should do in the source code to make compatible with their
 * application.
 */
#define RX_PORT_B
#define RX_PIN 2
// #define RX_RPNUM 39  // not necessary on PIC24FV16KM202

#define TX_PORT_B 
#define TX_PIN 7
//#define TX_RPNUM 20

/**
 * @brief this is an approximation of the time that the bootloader will remain
 * active at startup before moving on to the application
 */
#define BOOT_LOADER_TIME (10.0)

/* @brief this is the maximum size that can be programmed into the microcontroller
 * as part of one transaction using the CMD_WRITE_MAX_PROG_SIZE command 
 * 
 * A value of 0x80 should work on all microcontrollers.  Larger values will
 * allow faster programming operations, but will consume more RAM.
 */
#define MAX_PROG_SIZE 0x80

/* @brief this is the starting address of the application - must be 
 * on an even erase page boundary
 */
#define APPLICATION_START_ADDRESS 0x1000

/** @brief the microcontroller platform
 */
#if defined(__dsPIC33EP32MC204__)
#define PLATFORM_STRING "dspic33ep32mc204"
#elif defined(__dsPIC33EP64MC504__)
#define PLATFORM_STRING "dspic33ep64mc504"
#elif defined(__PIC24FV16KM202__)
#define PLATFORM_STRING "pic24fv16km202"
#endif

/** @brief the version of the transmission protocol
 */
#define VERSION_STRING "0.1"

/******************************************************************/
/* end user space - user should not change values below this line**/
/******************************************************************/

#define TX_BUF_LEN  ((MAX_PROG_SIZE * 4) + 0x10)
#define RX_BUF_LEN  ((MAX_PROG_SIZE * 4) + 0x10)

/**
 * @brief the byte that indicates the start of a frame
 */
#define START_OF_FRAME 0xf7

/** 
 * @brief the byte that indicates the end of a frame
 */
#define END_OF_FRAME   0x7f

/** 
 * @brief the escape byte, which indicates that the following byte will be
 * XORed with the ESC_XOR byte before being transmitted
 */
#define ESC         0xf6

/**
 * @brief the value to use to escape characters
 */
#define ESC_XOR     0x20

/** 
 * @brief commands available for the bootloader
 */
typedef enum{
    /* textual commands */
    CMD_READ_PLATFORM       = 0x00,
    CMD_READ_VERSION        = 0x01,
    CMD_READ_ROW_LEN        = 0x02,
    CMD_READ_PAGE_LEN       = 0x03,
    CMD_READ_PROG_LEN       = 0x04,
    CMD_READ_MAX_PROG_SIZE  = 0x05,
    CMD_READ_APP_START_ADDR = 0x06,

    /* erase operations */
    CMD_ERASE_PAGE  = 0x10,
            
    /* flash read memory operations */
    CMD_READ_ADDR   = 0x20,
    CMD_READ_MAX    = 0x21,
    
    /* flash write operations */
    CMD_WRITE_ROW   = 0x30,
    CMD_WRITE_MAX_PROG_SIZE = 0x31,
            
    /* application */
    CMD_START_APP   = 0x40
}CommCommand;

/**
 * @brief initializes the oscillator
 */
void initOsc(void);

/**
 * @brief initializes the pins
 */
void initPins(void);

/**
 * @brief initializes the UART
 */
void initUart(void);

/**
 * @brief initializes TMR1 and TMR2
 */
void initTimers(void);

/**
 * @brief reads the boot pin state
 * @return true if the pin is high, else false
 */
bool readBootPin(void);

/**
 * @brief receives data from the UART
 */
void receiveBytes(void);

/**
 * @brief processes the rx buffer into commands with parameters from packets
 */
void processReceived(void);

/**
 * @brief processes commands
 * @param data byte buffer of data that has been stripped of transmission
 * protocols
 */
void processCommand(uint8_t* data);

/**
 * @brief send the start byte and initialize fletcher checksum accumulator
 */
void txStart(void);

/**
 * @brief transmits a single byte, escaping if necessary, along with 
 * accumulating the fletcher checksum
 * @param byte a byte of data to transmit
 */
void txByte(uint8_t byte);

/**
 * @brief convenience function for transmitting an array of bytes with the
 * associated command
 * @param cmd the type of message
 * @param bytes an array of bytes to transmit
 * @param len the number of bytes to transmit from the array
 */
void txArray8bit(uint8_t cmd, uint8_t* bytes, uint16_t len);

/**
 * @brief convenience function for transmitting an array of 16-bit words
 * with the associated command
 * @param cmd the type of message
 * @param bytes an array of 16-bit words to transmit
 * @param len the number of bytes to transmit from the array
 */
void txArray16bit(uint8_t cmd, uint16_t* words, uint16_t len);

/**
 * @brief convenience function for transmitting an array of 32-bit words
 * with the associated command
 * @param cmd the type of message
 * @param bytes an array of 32-bit words to transmit
 * @param len the number of bytes to transmit from the array
 */
void txArray32bit(uint8_t cmd, uint32_t* words, uint16_t len);

/**
 * @brief convenience function for transmitting a string
 * @param cmd the type of message
 * @param str a pointer to a string buffer to transmit
 */
void txString(uint8_t cmd, char* str);

/**
 * @brief appends the checksum, properly escaping the sequence where necessary,
 * and sends the end byte
 */
void txEnd(void);

/**
 * @brief accumulates a single byte into the running accumulator
 * @param byte the byte to accumulate
 * @return the current fletcher16 value
 */
uint16_t fletcher16Accum(uint8_t byte);

/**
 * @brief calculate the fletcher16 value of an array given the array pointer
 * and length
 * @param data an 8-bit array pointer
 * @param length the length of the array
 * @return the fletcher16 value
 */
uint16_t fletcher16(uint8_t* data, uint16_t length);

/**
 * @brief reads the value at the address
 * @param address
 * @return the value of the address
 */
extern uint32_t readAddress(uint32_t address);

/**
 * @brief erases the flash page starting at the address
 * @param address
 */
extern void eraseByAddress(uint32_t address);

/**
 * @brief writes two instructions, starting at the address
 * @param address the starting address (must be even)
 * @param progDataArray a 32-bit, 2-element array containing the instruction 
 * words to be written to flash
 */
extern void doubleWordWrite(uint32_t address, uint32_t* progDataArray);

void writeInst32(uint32_t address, uint32_t* progDataArray);

/**
 * @brief starts the application
 * @param applicationAddress
 */
extern void startApp(uint32_t applicationAddress);

#endif