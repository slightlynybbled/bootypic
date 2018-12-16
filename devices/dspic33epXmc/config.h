/* DSPIC33EP32MC204 Configuration Bit Settings */

#include <xc.h>

/* FICD */
#pragma config ICS = PGD2               /* ICD Communication Channel Select bits (Communicate on PGEC2 and PGED2) */
#pragma config JTAGEN = OFF             /* JTAG Enable bit (JTAG is disabled) */

/* FPOR */
#pragma config ALTI2C1 = OFF            /* Alternate I2C1 pins (I2C1 mapped to SDA1/SCL1 pins) */
#pragma config ALTI2C2 = OFF            /* Alternate I2C2 pins (I2C2 mapped to SDA2/SCL2 pins) */
#pragma config WDTWIN = WIN25           /* Watchdog Window Select bits (WDT Window is 25% of WDT period) */

/* FWDT */
#pragma config WDTPOST = PS32768        /* Watchdog Timer Postscaler bits (1:32,768) */
#pragma config WDTPRE = PR128           /* Watchdog Timer Prescaler bit (1:128) */
#pragma config PLLKEN = ON              /* PLL Lock Enable bit (Clock switch to PLL source will wait until the PLL lock signal is valid.) */
#pragma config WINDIS = OFF             /* Watchdog Timer Window Enable bit (Watchdog Timer in Non-Window mode) */
#pragma config FWDTEN = OFF             /* Watchdog Timer Enable bit (Watchdog timer enabled/disabled by user software) */

/* FOSC */
#pragma config POSCMD = NONE            /* Primary Oscillator Mode Select bits (Primary Oscillator disabled) */
#pragma config OSCIOFNC = ON            /* OSC2 Pin Function bit (OSC2 is digital) */
#pragma config IOL1WAY = OFF            /* Peripheral pin select configuration (Allow multiple reconfigurations) */
#pragma config FCKSM = CSECMD           /* Clock Switching Mode bits (Clock switching is enabled,Fail-safe Clock Monitor is disabled) */

/* FOSCSEL */
#pragma config FNOSC = FRCPLL           /* Oscillator Source Selection (Fast RC Oscillator with divide-by-N with PLL module (FRCPLL)) */
#pragma config PWMLOCK = OFF            /* PWM Lock Enable bit (PWM registers may be written without key sequence) */
#pragma config IESO = ON                /* Two-speed Oscillator Start-up Enable bit (Start up device with FRC, then switch to user-selected oscillator source) */

/* FGS */
#pragma config GWRP = OFF               /* General Segment Write-Protect bit (General Segment may be written) */
#pragma config GCP = OFF                /* General Segment Code-Protect bit (General Segment Code protect is Disabled) */
