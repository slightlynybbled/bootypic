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
 * small - the bootloader is located between 0x400 and 0x1000 on all devices, leaving lots of room for the application above 0x1000
 * protects itself - the bootloader will not allow a self-write
 * configurable - see header file, pull a pin low to keep bootloader activated or simply keep communicating with the board

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
 - PIC24FV16KM202 (partial support, requires external oscillator at 24MHz)

Contributions in this area are welcome!

========================
Environment
========================

The anticipated environment is MPLAB X with an XC16 compiler.  This will probably work in other 
environments, but I am only testing with Microchip-provided tools.  

In order to be most compatible with devices, the bootloader must fit within a small and well-defined
memory footprint.  As a result, optimizations must be turned up to -O1 or the application will have 
to be moved to a higher memory location on some devices with larger flash erase pages.

Your MPLAB X project should be structured similarly to the below:

    .. image:: https://github.com/slightlynybbled/bootypic/blob/master/docs/img/project-structure-33ep64mc504.png

The intent is for the ``config.h`` and ``bootloader_33epXkm.h`` should be customized for your device and application
while all else simply works.  I still haven't worked out how I should set up the oscillator on all platforms for 
consistency, but I will get to that.

========================
Performance
========================

The current default transmission unit is 128 instructions and may be adjusted in ``bootloader.h``
under the ``MAX_PROG_SIZE`` define.  The 128 value was chosen since it is a value that should 
perform well enough on all platforms.  This value results in a loading time of 17.1s for a 32kB
device at 115200 baud, using `booty <https://github.com/slightlynybbled/booty>`_.  This could
likely be significantly improved if the ``MAX_PROG_SIZE`` were increased.
