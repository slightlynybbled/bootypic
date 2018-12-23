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
#define BOOT_PIN 9

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
#define RX_PIN 7
#define RX_RPNUM 39

#define TX_PORT_A
#define TX_PIN 4
#define TX_RPNUM 20

/**
 * @brief this is an approximation of the time that the bootloader will remain
 * active at startup before moving on to the application
 */
#define BOOT_LOADER_TIME (0.5)

/* @brief this is the maximum size that can be programmed into the microcontroller
 * as part of one transaction using the CMD_WRITE_MAX_PROG_SIZE command 
 * 
 * A value of 0x80 should work on all microcontrollers.  Larger values will
 * allow faster programming operations, but will consume more RAM.
 */
#define MAX_PROG_SIZE 0x80

#define TIME_PER_TMR2_50k 0.213
#define NUM_OF_TMR2_OVERFLOWS (uint16_t)((BOOT_LOADER_TIME/TIME_PER_TMR2_50k) + 1.0)

#endif