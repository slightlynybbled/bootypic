#ifndef _BOOTLOADER_H
#define _BOOTLOADER_H

#include <stdbool.h>
#include <stdint.h>

// static bool __attribute__ ((address(0x1000))) __attribute__((persistent)) should_run_bootloader;


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
#define BOOT_LOADER_TIME (10.0)

/* _FLASH_PAGE should be the maximum erase page (in instructions) */
#define _FLASH_PAGE   512
// FLASH_ROW = maximum write row (in instructions)
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

// instruction clock frequency
#define FCY 16000000UL

#define UART_BAUD_RATE 57600.0f

/// number of seconds we will wait before giving up on the bootloader
#define INITIAL_IDLE_TIME 30.0f

// idle time between received bytes before we time out
#define RX_IDLE_TIME 0.01f

// remappable pin to UART transmit
// these are specified in the device header as _RP1R, _RP2R, ...
// should agree with TX_RPNUM
#define TX_RPxR _RP7R
#define TX_RPNUM 7
// remappable pin for UART input
#define RX_RPNUM 6

#endif