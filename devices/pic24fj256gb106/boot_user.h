#ifndef _BOOT_USER_H
#define _BOOT_USER_H

#include <stdbool.h>
#include <stdint.h>

#ifndef __PIC24FJ256GB106__
#error "platform settings do not match header file"
#endif
#define PLATFORM_STRING "pic24fj256gb106"

/**
 * @brief these defines will determine the boot pin to be utilized
 * 
 * When the boot pin is pulled low, then bootloader will start, otherwise
 * the application will start on reset.
 */
#define BOOT_PORT_NONE
#ifdef BOOT_PORT_NONE
extern bool SHOULD_SKIP_BOOTLOADER;
#endif


// UART communication baud rate, in Hz
#define UART_BAUD_RATE 57600

// remappable pin for UART input
#define RX_PIN 6
// remappable pin for UART output
#define TX_PIN 7

/**
 * @brief this is an approximation of the time that the bootloader will remain
 * active at startup before moving on to the application
 */
#define BOOT_LOADER_TIME (10.0f)

/* @brief this is the maximum size that can be programmed into the microcontroller
 * as part of one transaction using the CMD_WRITE_MAX_PROG_SIZE command 
 * 
 * A value of 0x80 should work on all microcontrollers.  Larger values will
 * allow faster programming operations, but will consume more RAM.
 */
#define MAX_PROG_SIZE 0x80
#define APPLICATION_START_ADDRESS 0x2000
#define FCY 16000000UL  /* instruction clock frequency, in Hz */

#define _FLASH_PAGE   512  /* _FLASH_PAGE should be the maximum page (in instructions) */
#define _FLASH_ROW    64  /* _FLASH_ROW = maximum write row (in instructions) */

/**
 * @brief determines if the bootloader should abort
 * @return true if the bootloader should abort, else false
 */
bool should_abort_boot();

/**
 * @brief reads the value at the address
 * @param address
 * @return the value of the address
 */
uint32_t readAddress(uint32_t address);

/**
 * @brief erases the flash page starting at the address
 * @param address
 */
void eraseByAddress(uint32_t address);

/**
 * @brief writes one instruction, at the address
 * @param address the address of the instruction (must be even)
 * @param progDataArray the instruction to write
 * words to be written to flash
 */
void writeInstr(uint32_t address, uint32_t instruction);

/**
 * @brief writes two instructions, starting at the address
 * @param address the starting address (must be even)
 * @param progDataArray a 32-bit, 2-element array containing the instructions
 * words to be written to flash
 */
void doubleWordWrite(uint32_t address, uint32_t* progDataArray);

/**
 * @brief writes an entire row of instructions, starting at the address
 * @param address the starting address (must start a flash row)
 * @param words a buffer containing the _FLASH_ROW instructions to write
 */
void writeRow(uint32_t address, uint32_t* words);

/**
 * @brief writes the maximum number of instructions
 * @param address the starting address
 * @param progData a 32-bit, MAX_PROG_SIZE-element array containing the instruction words
 * to be written to flash
 */
void writeMax(uint32_t address, uint32_t* progData);

#endif