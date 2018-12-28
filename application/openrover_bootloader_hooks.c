#include "xc.h"
#include "boot_config.h"
#include "boot_hooks.h"

void pre_bootloader(){
}

bool should_abort_boot(float seconds_since_last_data) {
	if (seconds_since_last_data>BOOT_LOADER_TIME){
		return true	;
	}
	return false;
}
