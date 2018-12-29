#include "xc.h"
#include "boot_config.h"

// abort boot if boot pin is activated
void pre_bootloader(){
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

bool should_abort_boot(float seconds_since_last_data) {
	ClrWdt();

	if (seconds_since_last_data>BOOT_LOADER_TIME){
		return true	;
	}

    #if defined(BOOT_PORT_A)
        if(PORTA & (1 << BOOT_PIN))
            return true;
    #elif defined(BOOT_PORT_B)
        if(PORTB & (1 << BOOT_PIN))
            return true;
    #elif defined(BOOT_PORT_C)
        if(PORTC & (1 << BOOT_PIN))
            return true;
    #else
    #error "boot port not specified"
    #endif

   return false;
}
