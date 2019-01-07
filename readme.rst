========================
Purpose
========================

To provide a relatively easy-to-use bootloader that is compatible with most PIC24 and DSPIC33
series processors.  This bootloader does NOT use interrupts, so your default compilation
steps should work with only *minor* changes to the linker script.

The communications protocol is described in the comm-protocol.rst document.

Use the command-line utility `booty <https://github.com/slightlynybbled/booty>`_ to upload
your hex file!

========================
Features
========================

* uses no interrupts - you only need to change a couple of lines in the linker file 
* simple
  - the application is located at the same location in memory across devices 
  - easy to write your own loader
* small - the bootloader is located between 0x400 and 0x1000 on most devices, leaving lots of room for the application above 0x1000
* protects itself - the bootloader will not allow a self-write
* configurable - see boot_config.h file, pull a pin low to keep bootloader activated or simply keep communicating with the board
* linker scripts protect application area - if you make a change to the code which results in a bootloader overrunning its allotted space, then the linker will throw an error

========================
Supported Devices
========================

This list will be updated as I test more devices (which will be slowly).  Each supported device 
will have a linker script for the bootloader (this project) and the application located in the 
``gld`` directory.  You must add the proper linker file to your ``linker files`` in your MPLAB
project in order to get the bootloader to operate correctly.  Modification of the stock linker 
files to be compatible with bootypic is described in the gld directory readme.

- dsPIC33EP32MC204
- dsPIC33EP64MC504
- PIC24FJ256GB106

Contributions in this area are welcome!

This bootloader utilizes 1 UART, 2 timers, and 1 GPIO.  There is no reason that the application
cannot assume control of these peripherals.  No interrupts are utilized so the application has full
control of the device once control is passed to the application.

========================
Environment
========================

The anticipated environment is MPLAB X with an XC16 compiler.  This will probably work in other 
environments, but I am only testing with Microchip-provided tools at this time.  

In order to be most compatible with devices, the bootloader must fit within a small and well-defined
memory footprint.  As a result, optimizations must be turned up to ``-O1`` or the application will have 
to be moved to a higher memory location on some devices with larger flash erase pages.

------------------------
Bootloader
------------------------

Your MPLAB X project for the bootloader should contain:

* devices/<my_device>_boot.gld - bootloader linker script (device specific)
* devices/<my_device>/config.h - configuration defines
* devices/<my_device>/boot_config.h - some handy defines for your device
* devices/<my_device>/bootloaderasm.s - implementations in assembly (device specific)
* bootloader.h - header for bootloader 
* bootloader.c - implementations in C 

The intent is for the ``config.h`` and ``boot_config.h`` should be customized for your device and application
while all else simply works.  I still haven't worked out how I should set up the oscillator on all platforms for 
consistency, but I will get to that.

------------------------
Application
------------------------

Your MPLAB X project for the application should be structured as you please.  The only difference
between the default application and the bootloader is that the linker script must be modified to move 
the application to a higher place in memory.  The <my_device>_app.gld scripts located in this repository
provide a pretty decent place to start.

========================
Compiling/Loading
========================

------------------------
Supported Devices
------------------------

1. Copy ``bootloader.c`` and ``bootloader.h`` into a directory.
2. Create your MPLAB X project, add ``bootloader.h``, ``bootloaderasm.s``, and ``<my device>_boot.gld``.
3. Create an appropriate ``config.h`` file, add to your project (examples provided).
4. Modify the ``boot_config.h`` as required for your application (pin settings, etc) and add to your project.
5. Compile, load using MPLAB ICD3 or similar to get into your device
6. Use `booty <https://github.com/slightlynybbled/booty>`_ to load your application hex file 

------------------------
Unsupported Devices
------------------------

There are many devices that will work well, but may require additional defines to allocate the correct code.  This should be 
a relatively simple task for anyone who has done any PIC24 or dsPIC programming.

------------------------
Troubleshooting
------------------------

You can step through this code just like any other project.  There are a couple of timers, one UART, and a few IO that need 
to be set up.  In addition, different devices support somewhat different interfaces to flash memory.  Most devices will 
support some flavor of what is already here.  If you are having trouble, try to answer these questions:

- is the oscillator correctly set up, including configuration bytes?
- are the rx/tx and boot pins correctly set up?
- are the timers correctly set up on my device?

Please post any issues within the issues, and if you add device support, please do a pull request!

========================
Performance
========================

The current default transmission unit is 128 instructions and may be adjusted in ``bootloader.h``
under the ``MAX_PROG_SIZE`` define.  The 128 value was chosen since it is a value that should 
perform well enough on all platforms.  This value results in a loading time of 17.1s for a 32kB
device at 115200 baud, using `booty <https://github.com/slightlynybbled/booty>`_.  This could
likely be significantly improved if the ``MAX_PROG_SIZE`` were increased.

====================
Linker Scripts
====================

The linker scripts herein are slight modifications of those that can be found as part of the default installation
of MPLAB XC16 compilers.  The program memory has been offset so that it makes room for the booloader at or 
near the beginning of flash memory.  On some devices, the bootloader will reside at 0x400 while on others, it will
reside at 0x800 (depending on page erase size).  On all of these devices, the application should reside at 0x1000.

By locating the application memory further back than the default 0x200, the application will have fewer
instructions in program memory in which to reside.  For instance, a dsPIC33EP32MC204 has 32226 bytes of program memory 
available (10742 instructions).  The application will reside at 0x1000 instead of 0x200, so it will lose access
to 0xe00 addresses (3584 addresses, or 5376 bytes) due to allocated space for the bootloader.

------------------------------
Sizes
------------------------------

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

-------------------------------------------
Creating a New Linker Script (Bootloader)
-------------------------------------------

1. Copy the linker script from the <XC16 installation dir>/support/<device>/gld
2. Rename to <device>_boot.gld (optional)
3. Find the ``MEMORY`` region, modify the ``program (xr)`` line

   a. ``ORIGIN`` should be ``0x400`` or ``0x800`` depending on the page erase memory
   b. ``LENGTH`` should be the current ``LENGTH - 0xe00`` for bootloaders at 0x400 or ``LENGTH - 0xa00`` for bootloaders located at 0x800 (you can do this in the google search engine, simply type ``0x55ec - 0xe00``)
   
4. Scroll down a bit, find ``__CODE_BASE``, make it equal to ``ORIGIN``
5. Find ``__CODE_LENGTH``, make it equal to your computed ``LENGTH``

-------------------------------------------
Creating a New Linker Script (Application)
-------------------------------------------

1. Copy the linker script from the <XC16 installation dir>/support/<device>/gld
2. Rename to <device>_app.gld (optional)
3. Find the ``MEMORY`` region, modify the ``program (xr)`` line

   a. ``ORIGIN`` should be ``0x1000``
   b. ``LENGTH`` should be the current ``LENGTH`` - ``0xe00`` (you can do this in the google search engine, simply type ``0x55ec - 0xe00``)
   
4. Scroll down a bit, find ``__CODE_BASE``, make it equal to ``ORIGIN``
5. Find ``__CODE_LENGTH``, make it equal to your computed ``LENGTH``

