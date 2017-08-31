========================
Purpose
========================

To provide a relatively easy-to-use bootloader that is compatible with most PIC24 and DSPIC33
series processors.  This bootloader does NOT use interrupts, so your default compilation
steps should work with only *minor* changes to the linker script.

========================
Supported Devices
========================

This list will be updated as I test more devices (which will be slowly).  Each supported device 
will have a linker script for the bootloader (this project) and the application located in the 
``gld`` directory.  You must add the proper linker file to your ``linker files`` in your MPLAB
project in order to get the bootloader to operate correctly.

 - dsPIC33EP32MC204

Future Support:

 - dsPIC33EP64MC204

Contributions in this area are welcome!

========================
Environment
========================

The anticipated environment is MPLAB X with an XC16 compiler.  This will probably work in other 
environments, but I am only testing with Microchip-provided tools.  I am compiling with optimizations
set to -01.


