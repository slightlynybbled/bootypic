====================
Purpose
====================

The linker scripts herein are slight modifications of those that can be found as part of the default installation
of MPLAB XC16 compilers.  The program memory has been offset so that it makes room for the booloader at or 
near the beginning of flash memory.  On some devices, the bootloader will reside at 0x400 while on others, it will
reside at 0x800 (depending on page erase size).  On all of these devices, the application should reside at 0x1000.

By locating the application memory further back than the default 0x200, the application will have fewer
instructions in program memory in which to reside.  For instance, a dsPIC33EP32MC204 has 32226 bytes of program memory 
available (10742 instructions).  The application will reside at 0x1000 instead of 0x200, so it will lose access
to 0xe00 addresses (3584 addresses, or 5376 bytes) due to allocated space for the bootloader.

====================
Sizes
====================

In the first few pages of most device datasheets, there is a table for the family that lists the 'Page Erase Size' in
instructions.  This is the same table in which the peripherals, packages, and memory are listed out.  Use that to determine
where the bootloader and application should be located.

+--------------+--------------+--------------+
| erase page   | bootloader   | application  |
| size         | address      | address      |
+--------------+--------------+--------------+
| 512          | 0x400        | 0x1000       |
+--------------+--------------+--------------+
| 1024         | 0x800        | 0x1000       |
+--------------+--------------+--------------+

====================
Creating a New Linker Script (Bootloader)
====================

1. Copy the linker script from the <XC16 installation dir>/support/<device>/gld
2. Rename to <device>_boot.gld (optional)
3. Find the ``MEMORY`` region, modify the ``program (xr)`` line
  a. ``ORIGIN`` should be ``0x400`` or ``0x800`` depending on the page erase memory
  b. ``LENGTH`` should be the current ``LENGTH - 0xe00`` for bootloaders at 0x400 or ``LENGTH - 0xa00``
  for bootloaders located at 0x800 (you can do this in the google search engine, simply
  type ``0x55ec - 0xe00``)
  c. Scroll down a bit, find ``__CODE_BASE``, make it equal to ``0x1000``
  d. Find ``__CODE_LENGTH``, make it equal to your computed length in part b

====================
Creating a New Linker Script (Application)
====================

1. Copy the linker script from the <XC16 installation dir>/support/<device>/gld
2. Rename to <device>_app.gld (optional)
3. Find the ``MEMORY`` region, modify the ``program (xr)`` line
  a. ``ORIGIN`` should be ``0x1000``
  b. ``LENGTH`` should be the current ``LENGTH`` - ``0xe00`` (you can do this in the google search engine, simply
  type ``0x55ec - 0xe00``)
  c. Scroll down a bit, find ``__CODE_BASE``, make it equal to ``0x1000``
  d. Find ``__CODE_LENGTH``, make it equal to your computed length in part b
