#include "xc.h"
#include "boot_config.h"
#include "boot_hooks.h"

#define CELL_A_MOS_EN(a) _TRISD3 = !(a)
#define CELL_B_MOS_EN(a) _TRISD2 = !(a)

#define Cell_A_MOS _LATD3
#define Cell_B_MOS _LATD2

typedef enum BatteryState {
    CELL_OFF = 0,
    CELL_ON = 1,
} BatteryState;
typedef enum BatteryChannel {
    CELL_A = 0,
    CELL_B = 1,
} BatteryChannel;

static void set_battery_state(BatteryChannel channel, BatteryState state) {
    switch (channel) {
    case CELL_A:
        Cell_A_MOS = state;
        break;
    case CELL_B:
        Cell_B_MOS = state;
        break;
    }
}

extern void __delay32(unsigned long cycles); 
#define __delay_ms(d) { __delay32( (unsigned long) (((unsigned long long) d)*(FCY)/1000ULL)); }

void block_ms(uint16_t ms) {
    const int ms_chunk = 128;
    int i;
    ClrWdt();
    for (i = 0; i < (ms / ms_chunk); i++) {
        __delay_ms(ms_chunk);
        ClrWdt();
    }

    __delay_ms(ms % ms_chunk);
    ClrWdt();
}

static void turn_on_power_bus_hybrid_method(void) {
	CELL_A_MOS_EN(1);
	CELL_B_MOS_EN(1);
    unsigned int i;
    unsigned int j;
    // k=20,000 is about 15ms
    unsigned int k = 2000;
    unsigned int k0 = 2000;

    for (i = 0; i < 200; i++) {
        set_battery_state(CELL_A, CELL_ON);
        set_battery_state(CELL_B, CELL_ON);
        for (j = 0; j < k; j++)
            Nop();
        set_battery_state(CELL_A, CELL_OFF);
        set_battery_state(CELL_B, CELL_OFF);

        block_ms(40);
        k = k0 + i * i / 4;
        if (k > 20000)
            k = 20000;
    }

    set_battery_state(CELL_A, CELL_ON);
    set_battery_state(CELL_B, CELL_ON);
}

void pre_bootloader(){
	turn_on_power_bus_hybrid_method();
}

bool should_abort_boot(float seconds_since_last_data) {
	if (seconds_since_last_data>BOOT_LOADER_TIME){
		return true	;
	}
	return false;
}
