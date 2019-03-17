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
#define BOOT_LOADER_TIME (10.0f)
#define STALE_MESSAGE_TIME (0.05f)

/* @brief this is the maximum size that can be programmed into the microcontroller
 * as part of one transaction using the CMD_WRITE_MAX_PROG_SIZE command 
 * 
 * A value of 0x80 should work on all microcontrollers.  Larger values will
 * allow faster programming operations, but will consume more RAM.
 */
#define MAX_PROG_SIZE 0x80
#define APPLICATION_START_ADDRESS 0x1400
#define TIME_PER_TMR2_50k 0.213
#define FCY 60000000UL  /* instruction clock frequency, in Hz */

#define NUM_OF_TMR2_OVERFLOWS (uint16_t)((BOOT_LOADER_TIME/TIME_PER_TMR2_50k) + 1.0)

#define PLATFORM_STRING "dspic33ep64mc504"

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
 * @brief determines if the bootloader should abort
 * @return true if the bootloader should abort, else false
 */
bool should_abort_boot(uint16_t counterValue);

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
 * @param progDataArray a 32-bit, 2-element array containing the instruction 
 * words to be written to flash
 */
void doubleWordWrite(uint32_t address, uint32_t* progDataArray);

/**
 * @brief writes an entire row of instructions, starting at the address
 * @param address the starting address (must start a flash row)
 * @param words a buffer containing the instructions to write
 */
void writeRow(uint32_t address, uint32_t words[_FLASH_ROW]);

/**
 * @brief writes the maximum number of instructions
 * @param address the starting address
 * @param progData a 32-bit, 2-element array containing the instruction words
 * to be written to flash
 */
void writeMax(uint32_t address, uint32_t* progData);

#endif