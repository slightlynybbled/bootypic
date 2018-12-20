/// Application-specific implementable hooks that may be needed for 

#ifndef BOOT_HOOKS_H
#define BOOT_HOOKS_H

#include <stdbool.h>


/// code to run before bootloader starts up
extern void pre_bootloader();

/// runs every loop. Should return true if bootloader should proceed
extern bool should_abort_boot(float seconds_since_last_data);

#endif