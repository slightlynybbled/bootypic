#ifndef _BOOTLOADER_H
#define _BOOTLOADER_H

#include <stdbool.h>
#include <stdint.h>

#define APPLICATION_START_ADDRESS 0x2000
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

/**
 * @brief this is an approximation of the time that the bootloader will remain
 * active at startup before moving on to the application
 */
#define BOOT_LOADER_TIME (15.0f)
// Number of seconds between bytes before we time out and reset the buffer
#define MESSAGE_TIME (0.2f)

/* _FLASH_PAGE should be the maximum page (in instructions) */
#define _FLASH_PAGE   512
/* _FLASH_ROW = maximum write row (in instructions) */
#define _FLASH_ROW     64


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

// instruction clock frequency, in Hz.
#define FCY 16000000UL

// UART communication baud rate, in Hz
#define UART_BAUD_RATE 57600

// remappable pin for UART input
#define RX_RPNUM 6
// remappable pin for UART output
#define TX_RPNUM 7

extern void writeWord(uint32_t address, uint32_t word);
extern void writeRow(uint32_t address, uint32_t words[_FLASH_ROW]);

#endif